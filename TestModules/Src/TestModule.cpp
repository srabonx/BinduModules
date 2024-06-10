#include "../Include/TestModule.h"
#include <Windows.h>
#include <d3dx12.h>
#include <DirectXColors.h>
#include <array>

DemoClass::DemoClass(BINDU::Window* window) : m_window(window)
{
	m_timer.Reset();
}

DemoClass::~DemoClass()
{
	
}

bool DemoClass::OnInit()
{
	DXGI_MODE_DESC desc;
	desc.Height = m_window->GetWindowHeight();
	desc.Width = m_window->GetWindowWidth();
	desc.RefreshRate.Denominator = 1;
	desc.RefreshRate.Numerator = 60;

	m_graphics = std::make_unique<BINDU::Graphics>(m_window->GetWindowHandle(), desc);

	m_graphics->InitDirect3D();

	DX::ThrowIfFailed(m_graphics->GetCommandList()->Reset(m_graphics->GetCommandAllocator(), nullptr));

	this->CreateDescriptorHeaps();
	this->CreateConstantBuffers();
	this->CreateRootSignature();
	this->CreateInputLayoutAndShaders();
	this->CreateBoxGeometry();
	this->CreatePSO();

	// execute initialization command list

	DX::ThrowIfFailed(m_graphics->GetCommandList()->Close());

	ID3D12CommandList* cmdLists[] = { m_graphics->GetCommandList() };

	m_graphics->GetCommandQueue()->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	m_graphics->FlushCommandQueue();

	return false;
}

void DemoClass::Run()
{
	m_timer.Tick();

	int fps{ 0 };
	float mspf{ 0.0f };
	
	double totalTime = m_timer.TotalTime();

	if (this->CalculateFrameStats(fps, mspf, totalTime))
	{
		std::wstring text = L"FPS: " + std::to_wstring(fps) + L" MSPF: " + std::to_wstring(mspf) + L" TOTAL TIME: " + std::to_wstring(totalTime);

		m_window->SetWindowTitle(text);
	}

	this->Update();
	this->Render();

	

}

void DemoClass::Update()
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, this->GetWindow()->GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_proj, P);

	float x = m_radius * sinf(m_phi) * cosf(m_theta);
	float z = m_radius * sinf(m_phi) * sinf(m_theta);
	float y = m_radius * cosf(m_phi);

	// build the view matrix
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, view);

	XMMATRIX world = XMLoadFloat4x4(&m_world);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);
	XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest world view proj matrix

	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldviewMatrix, XMMatrixTranspose(worldViewProj));

	m_objectCB->CopyData(0, objConstants);

	

}

void DemoClass::Render()
{
	DX::ThrowIfFailed(m_graphics->GetCommandAllocator()->Reset());

	m_graphics->GetCommandList()->Reset(m_graphics->GetCommandAllocator(), m_pso.Get());

	m_graphics->GetCommandList()->RSSetViewports(1, m_graphics->GetViewPort());
	m_graphics->GetCommandList()->RSSetScissorRects(1, m_graphics->GetScissorRect());

	m_graphics->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_graphics->GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_graphics->GetCommandList()->ClearRenderTargetView(m_graphics->GetCurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);

	m_graphics->GetCommandList()->ClearDepthStencilView(m_graphics->GetDepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_graphics->GetCommandList()->OMSetRenderTargets(1, &m_graphics->GetCurrentBackBufferView(), true,
		&m_graphics->GetDepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbHeap.Get() };
	m_graphics->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_graphics->GetCommandList()->SetGraphicsRootSignature(m_rootSig.Get());

	m_graphics->GetCommandList()->IASetVertexBuffers(0, 1, &m_boxGeo->GetVertexBufferView());
	m_graphics->GetCommandList()->IASetIndexBuffer(&m_boxGeo->GetIndexBufferView());
	m_graphics->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_graphics->GetCommandList()->SetGraphicsRootDescriptorTable(0, m_cbHeap->GetGPUDescriptorHandleForHeapStart());

	m_graphics->GetCommandList()->DrawIndexedInstanced(m_boxGeo->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);


	m_graphics->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_graphics->GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	DX::ThrowIfFailed(m_graphics->GetCommandList()->Close());

	ID3D12CommandList* cmdLists[] = {m_graphics->GetCommandList()};

	m_graphics->GetCommandQueue()->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	DX::ThrowIfFailed(m_graphics->GetSwapChain()->Present(0, 0));

	int currBBuffer = m_graphics->GetCurrentBackBufferIndex();

	currBBuffer = (currBBuffer + 1) % m_graphics->GetNumberOfSwapChainBuffer();

	m_graphics->SetCurrentBackBufferIndex(currBBuffer);

	m_graphics->FlushCommandQueue();
	
}

void DemoClass::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NumDescriptors = 1;
	heapDesc.NodeMask = 0;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	m_graphics->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbHeap));

}

void DemoClass::CreateConstantBuffers()
{
	m_objectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_graphics->GetDevice(), 1, true);

	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS objCBBaddress = m_objectCB->Resource()->GetGPUVirtualAddress();

	// offset to the ith constant buffer in the buffer
	int boxCBIndex = 0;
	objCBBaddress += boxCBIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = objCBBaddress;
	cbvDesc.SizeInBytes = objCBByteSize;

	m_graphics->GetDevice()->CreateConstantBufferView(&cbvDesc, m_cbHeap->GetCPUDescriptorHandleForHeapStart());

}

void DemoClass::CreateRootSignature()
{
	// Create root parameters
	CD3DX12_ROOT_PARAMETER	rootParameterSlot[1];

	// Create descriptor table
	CD3DX12_DESCRIPTOR_RANGE	cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	rootParameterSlot[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, rootParameterSlot, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// Create a root signature with single slot which points to a descriptor table consisting of a single constant buffer
	Microsoft::WRL::ComPtr<ID3DBlob>	serializedRootSig{ nullptr };
	Microsoft::WRL::ComPtr<ID3DBlob>	errorBlob{ nullptr };

	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf());

	if (errorBlob)
	{
		::OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
	}
	DX::ThrowIfFailed(hr);



	DX::ThrowIfFailed(m_graphics->GetDevice()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_rootSig.ReleaseAndGetAddressOf())));
}

void DemoClass::CreateInputLayoutAndShaders()
{
	m_vsByteCode = D3DUtil::CompileShader("C:/Users/letsd/visual studio workspace/BinduModules/TestModules/Shaders/color.hlsl", nullptr, "VS", "vs_5_0");
	m_psByteCode = D3DUtil::CompileShader("C:/Users/letsd/visual studio workspace/BinduModules/TestModules/Shaders/color.hlsl", nullptr, "PS", "ps_5_0");

	m_inputLayout =
	{
		{"POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR",0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};
}

// the box that will be drawn
void DemoClass::CreateBoxGeometry()
{
	std::array<Vertex, 8>	vertices =
	{
		Vertex({XMFLOAT3(-1.0f,-1.0f,-1.0f), XMFLOAT4(Colors::Red)}),
		Vertex({XMFLOAT3(-1.0f,+1.0f,-1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(+1.0f,+1.0f,-1.0f), XMFLOAT4(Colors::Blue)}),
		Vertex({XMFLOAT3(+1.0f,-1.0f,-1.0f), XMFLOAT4(Colors::Yellow)}),
		Vertex({XMFLOAT3(-1.0f,-1.0f,+1.0f), XMFLOAT4(Colors::Violet)}),
		Vertex({XMFLOAT3(-1.0f,+1.0f,+1.0f), XMFLOAT4(Colors::Indigo)}),
		Vertex({XMFLOAT3(+1.0f,+1.0f,+1.0f), XMFLOAT4(Colors::Orange)}),
		Vertex({XMFLOAT3(+1.0f,-1.0f,+1.0f), XMFLOAT4(Colors::Black)}),

	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7

	};

	const UINT vbByteSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
	const UINT ibByteSize = static_cast<UINT>(indices.size() * sizeof(std::uint16_t));

	m_boxGeo = std::make_unique<MeshGeometry>();

	m_boxGeo->Name = "boxGeo";

	DX::ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_boxGeo->VertexBufferCPU));
	CopyMemory(m_boxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	DX::ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_boxGeo->IndexBufferCPU));
	CopyMemory(m_boxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	m_boxGeo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(
		m_graphics->GetDevice(), m_graphics->GetCommandList(),
		vertices.data(), vbByteSize, m_boxGeo->VertexBufferUploader);

	m_boxGeo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(
		m_graphics->GetDevice(), m_graphics->GetCommandList(),
		indices.data(), ibByteSize, m_boxGeo->IndexBufferUploader);

	m_boxGeo->VertexByteStride = sizeof(Vertex);
	m_boxGeo->VertexBufferByteSize = vbByteSize;
	m_boxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	m_boxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry subMesh;
	subMesh.IndexCount = static_cast<UINT>(indices.size());
	subMesh.StartIndexLocation = 0;
	subMesh.BaseVertexLocation = 0;

	m_boxGeo->DrawArgs["box"] = subMesh;

}

void DemoClass::CreatePSO()
{

	D3D12_INPUT_LAYOUT_DESC ilDesc;
	ilDesc.pInputElementDescs = m_inputLayout.data();
	ilDesc.NumElements = static_cast<UINT>(m_inputLayout.size());

	CD3DX12_RASTERIZER_DESC rsDesc(D3D12_DEFAULT);
	rsDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rsDesc.CullMode = D3D12_CULL_MODE_BACK;

	D3D12_SHADER_BYTECODE vsByteCode;
	vsByteCode.pShaderBytecode = reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer());
	vsByteCode.BytecodeLength = m_vsByteCode->GetBufferSize();

	D3D12_SHADER_BYTECODE psByteCode;
	psByteCode.pShaderBytecode = reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer());
	psByteCode.BytecodeLength = m_psByteCode->GetBufferSize();


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psd;
	ZeroMemory(&psd, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psd.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psd.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psd.DSVFormat = m_graphics->GetDepthStencilFormat();
	psd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psd.InputLayout = ilDesc;
	psd.NodeMask = 0;
	psd.NumRenderTargets = 1;
	psd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psd.pRootSignature = m_rootSig.Get();
	psd.RasterizerState = rsDesc;
	psd.RTVFormats[0] = m_graphics->GetBackBufferFormat();
	psd.SampleDesc.Count = m_graphics->Get4XMSAAState() ? 4 : 1;
	psd.SampleDesc.Quality = m_graphics->Get4XMSAAState() ? (m_graphics->Get4XMSAAQuality() - 1) : 0;
	psd.SampleMask = UINT_MAX;
	psd.VS = vsByteCode;
	psd.PS = psByteCode;

	DX::ThrowIfFailed(m_graphics->GetDevice()->CreateGraphicsPipelineState(&psd, IID_PPV_ARGS(m_pso.GetAddressOf())));
}




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int cmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	BINDU::BINDU_WINDOW_DESC wndDesc = { "TestWindow",800,800 };
	BINDU::Window window(hInstance, wndDesc);

	BINDU::BinduApp* demoClass = new DemoClass(&window);

	int n = BINDU::Win32Application::Run(demoClass, &window, cmdShow);
	
	_CrtDumpMemoryLeaks();

	return n;
}