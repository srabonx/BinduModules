#include "../Include/TestModule.h"
#include <Windows.h>
#include <d3dx12.h>
#include <DirectXColors.h>
#include <array>
#include <Win32Input.h>
#include <MathHelper.h>
#include <fstream>


DemoClass::DemoClass(HINSTANCE hInstance) : BINDU::Win32Window(hInstance)
{
	m_timer.Reset();
}

DemoClass::DemoClass(HINSTANCE hInstance, BINDU::BINDU_WINDOW_DESC windowDesc) : BINDU::Win32Window(hInstance, windowDesc)
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
	m_graphics->OnResize(800, 600);

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

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, this->GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_proj, P);
	m_objectConstants.PulseColor = XMFLOAT4(DirectX::Colors::Red);
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

		this->SetWindowTitle(text);
	}

	this->Update();
	this->Render();

	

}

void DemoClass::Update()
{

	//if (BINDU::Win32Input::IsMouseButtonPressed(BINDU::MouseButton::LEFT))
	//{

	//	m_lastMousePos.x = BINDU::Win32Input::GetMousePosition().currX;
	//	m_lastMousePos.y = BINDU::Win32Input::GetMousePosition().currY;
	//}

	if (BINDU::Win32Input::IsMouseMoved(BINDU::MouseButton::LEFT))
	{

		SetCapture(m_windowHandle);
		float dx = XMConvertToRadians(0.15f * static_cast<float>(BINDU::Win32Input::GetMousePosition().currX - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.15f * static_cast<float>(BINDU::Win32Input::GetMousePosition().currY - m_lastMousePos.y));

		m_theta += dx;
		m_phi += dy;

		m_phi = MathHelper::Clamp(m_phi, 0.1f, 3.1419f - 0.1f);
		m_lastMousePos.x = BINDU::Win32Input::GetMousePosition().currX;
		m_lastMousePos.y = BINDU::Win32Input::GetMousePosition().currY;
		
	}
	else if (BINDU::Win32Input::IsMouseMoved(BINDU::MouseButton::RIGHT))
	{
		SetCapture(m_windowHandle);
		float dx = 0.005f * static_cast<float>(BINDU::Win32Input::GetMousePosition().currX - m_lastMousePos.x);
		float dy = 0.005f * static_cast<float>(BINDU::Win32Input::GetMousePosition().currY - m_lastMousePos.y);

		// Update the camera radius based on input.
		m_radius += dx - dy;

		// Restrict the radius.
		m_radius = MathHelper::Clamp(m_radius, 3.0f, 15.0f);

		m_lastMousePos.x = BINDU::Win32Input::GetMousePosition().currX;
		m_lastMousePos.y = BINDU::Win32Input::GetMousePosition().currY;
	}
	else if (BINDU::Win32Input::IsMouseButtonReleased(BINDU::MouseButton::LEFT))
	{
		ReleaseCapture();
		m_lastMousePos.x = BINDU::Win32Input::GetMousePosition().currX;
		m_lastMousePos.y = BINDU::Win32Input::GetMousePosition().currY;
	}

	static float angle = 0.f;

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
	objConstants.GTime = m_timer.TotalTime();
	objConstants.PulseColor = XMFLOAT4(Colors::Red);

	m_objectCB->CopyData(0, objConstants);

	//world = XMLoadFloat4x4(&m_world) * XMMatrixTranslation(angle += 0.001f, 0, 0);
	//
	//worldViewProj = world * view * proj;
	//XMStoreFloat4x4(&m_objectConstants.WorldviewMatrix, XMMatrixTranspose(worldViewProj));
	//m_objectCB->CopyData(1, m_objectConstants);
	//
	//if (angle > 5)
	//	angle = -5;
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
//	m_graphics->GetCommandList()->IASetVertexBuffers(0, 1, &m_vBufferView[0]);
//	m_graphics->GetCommandList()->IASetVertexBuffers(1, 1, &m_vBufferView[1]);
	m_graphics->GetCommandList()->IASetIndexBuffer(&m_boxGeo->GetIndexBufferView());
	m_graphics->GetCommandList()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	m_graphics->GetCommandList()->SetGraphicsRootDescriptorTable(0, m_cbHeap->GetGPUDescriptorHandleForHeapStart());

	m_graphics->GetCommandList()->DrawIndexedInstanced(m_boxGeo->DrawArgs["box"].IndexCount, 1, 0, 0, 0);
//	m_graphics->GetCommandList()->DrawIndexedInstanced(m_geometry->DrawArgs["box"].IndexCount,
//		1, m_geometry->DrawArgs["box"].StartIndexLocation, m_geometry->DrawArgs["box"].BaseVertexLocation, 0);

//	boxCBIndex = 1;
//	objCBaddress += boxCBIndex * objCBByteSize;

//	cbvHandle.Offset(boxCBIndex, m_graphics->GetCbvSrvUavDescriptorIncreamentSize());

//	m_graphics->GetCommandList()->SetGraphicsRootDescriptorTable(0, cbvHandle);

//	m_graphics->GetCommandList()->DrawIndexedInstanced(m_geometry->DrawArgs["pyramid"].IndexCount,
//		1, m_geometry->DrawArgs["pyramid"].StartIndexLocation, m_geometry->DrawArgs["pyramid"].BaseVertexLocation, 0);

	
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

LRESULT DemoClass::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_ACTIVATE:

		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_appPaused = true;
			m_timer.Stop();
		}
		else
		{
			m_appPaused = false;
			m_timer.Start();
		}

		break;

	case WM_ENTERSIZEMOVE:
		m_appPaused = true;
		m_resizing = true;
		m_timer.Stop();
		break;

	case WM_EXITSIZEMOVE:
		m_appPaused = false;
		m_resizing = false;
		m_timer.Start();
		this->OnResize(m_windowWidth, m_windowHeight);
		break;

	case WM_SIZE:

		m_windowWidth = LOWORD(lParam);
		m_windowHeight = HIWORD(lParam);

		if (m_graphics)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_appPaused = true;
				m_minimized = true;
				m_maximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_appPaused = false;
				m_minimized = false;
				m_maximized = true;
				this->OnResize(m_windowWidth, m_windowHeight);
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (m_minimized)
				{
					m_appPaused = false;
					m_minimized = false;
					this->OnResize(m_windowWidth, m_windowHeight);
				}
				else if (m_maximized)
				{
					m_appPaused = false;
					m_maximized = false;
					this->OnResize(m_windowWidth, m_windowHeight);
				}
				else if (m_resizing)
				{

				}
				else
					this->OnResize(m_windowWidth, m_windowHeight);
			}

		}

		break;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		break;
	
	default:
		break;
	}

	BINDU::Win32Input::MsgProc(hWnd, msg, wParam, lParam);

	return DefWindowProc(hWnd, msg, wParam, lParam);
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


		D3D12_GPU_VIRTUAL_ADDRESS objCBaddress = m_objectCB->Resource()->GetGPUVirtualAddress();


		// offset to the ith constant buffer in the buffer
		int boxCBIndex = 0;
		objCBaddress += boxCBIndex * objCBByteSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = objCBaddress;
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
	

	m_vsByteCode = D3DUtil::CompileShader(RelativeResourcePath("Shaders/color.hlsl"), nullptr, "VS", "vs_5_0");
	m_psByteCode = D3DUtil::CompileShader(RelativeResourcePath("Shaders/color.hlsl"), nullptr, "PS", "ps_5_0");

	
	m_inputLayout =
	{
		{"POSITION",0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},	
		{"COLOR",0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	m_inputLayout2 =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TANJENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"TEX0", 0 , DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEX1", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR", 0 , DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 52, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	m_inputLayout3 =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
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


	std::array<Vertex, 5>	pyraVertices =
	{
		Vertex({XMFLOAT3(-1.0f,-1.0f,-1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(-1.0f,-1.0f,+1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(+1.0f,-1.0f,-1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(+1.0f,-1.0f,+1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(0.0f,+1.0f,0.0f), XMFLOAT4(Colors::Red)}),

	};

	std::array<std::uint16_t, 18> pyraIndices =
	{
		0, 1, 2,
		1, 3, 2,
		1, 4, 0,
		0, 4, 2,
		2, 4, 3,
		3, 4, 1
	};

	UINT pyraVByteSize = static_cast<UINT>(pyraVertices.size() * sizeof(Vertex));
	UINT pyraIByteSize = static_cast<UINT>(pyraIndices.size() * sizeof(uint16_t));

	m_pyramidGeo = std::make_unique<MeshGeometry>();

	m_pyramidGeo->Name = "pyramidGeo";

	DX::ThrowIfFailed(D3DCreateBlob(pyraVByteSize, &m_pyramidGeo->VertexBufferCPU));
	RtlCopyMemory(m_pyramidGeo->VertexBufferCPU->GetBufferPointer(), pyraVertices.data(), pyraVByteSize);

	DX::ThrowIfFailed(D3DCreateBlob(pyraIByteSize, &m_pyramidGeo->IndexBufferCPU));
	RtlCopyMemory(m_pyramidGeo->IndexBufferCPU->GetBufferPointer(), pyraIndices.data(), pyraIByteSize);

	m_pyramidGeo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_graphics->GetDevice(),
		m_graphics->GetCommandList(), pyraVertices.data(), pyraVByteSize, m_pyramidGeo->VertexBufferUploader);

	m_pyramidGeo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_graphics->GetDevice(),
		m_graphics->GetCommandList(), pyraIndices.data(), pyraIByteSize, m_pyramidGeo->IndexBufferUploader);

	m_pyramidGeo->VertexBufferByteSize = pyraVByteSize;
	m_pyramidGeo->IndexBufferByteSize = pyraIByteSize;
	m_pyramidGeo->VertexByteStride = sizeof(Vertex);
	m_pyramidGeo->IndexFormat = DXGI_FORMAT_R16_UINT;

	subMesh.IndexCount = static_cast<UINT>(pyraIndices.size());
	subMesh.BaseVertexLocation = 0;
	subMesh.StartIndexLocation = 0;

	m_pyramidGeo->DrawArgs["pyramid"] = subMesh;




	std::array<Vertex, 13> geometryVertices =
	{
		Vertex({XMFLOAT3(-1.0f,-1.0f,-1.0f), XMFLOAT4(Colors::Red)}),
		Vertex({XMFLOAT3(-1.0f,+1.0f,-1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(+1.0f,+1.0f,-1.0f), XMFLOAT4(Colors::Blue)}),
		Vertex({XMFLOAT3(+1.0f,-1.0f,-1.0f), XMFLOAT4(Colors::Yellow)}),
		Vertex({XMFLOAT3(-1.0f,-1.0f,+1.0f), XMFLOAT4(Colors::Violet)}),
		Vertex({XMFLOAT3(-1.0f,+1.0f,+1.0f), XMFLOAT4(Colors::Indigo)}),
		Vertex({XMFLOAT3(+1.0f,+1.0f,+1.0f), XMFLOAT4(Colors::Orange)}),
		Vertex({XMFLOAT3(+1.0f,-1.0f,+1.0f), XMFLOAT4(Colors::Black)}),

		Vertex({XMFLOAT3(-1.0f,-1.0f,-1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(-1.0f,-1.0f,+1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(+1.0f,-1.0f,-1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(+1.0f,-1.0f,+1.0f), XMFLOAT4(Colors::Green)}),
		Vertex({XMFLOAT3(0.0f,+1.0f,0.0f), XMFLOAT4(Colors::Red)}),

	};

	std::array<uint16_t, 54> geometryIndices =
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
		4, 3, 7,


		// pyramid indices
		0, 1, 2,
		1, 3, 2,
		1, 4, 0,
		0, 4, 2,
		2, 4, 3,
		3, 4, 1
	};

	m_geometry = std::make_unique<MeshGeometry>();

	UINT geometryVBByteSize = static_cast<UINT>(geometryVertices.size() * sizeof(Vertex));
	UINT geometryIBByteSize = static_cast<UINT>(geometryIndices.size() * sizeof(uint16_t));


	DX::ThrowIfFailed(D3DCreateBlob(geometryVBByteSize, &m_geometry->VertexBufferCPU));
	RtlCopyMemory(m_geometry->VertexBufferCPU->GetBufferPointer(), geometryVertices.data(), geometryVBByteSize);

	DX::ThrowIfFailed(D3DCreateBlob(geometryIBByteSize, &m_geometry->IndexBufferCPU));
	RtlCopyMemory(m_geometry->IndexBufferCPU->GetBufferPointer(), geometryIndices.data(), geometryIBByteSize);

	m_geometry->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_graphics->GetDevice(),
		m_graphics->GetCommandList(), geometryVertices.data(), geometryVBByteSize, m_geometry->VertexBufferUploader);


	m_geometry->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_graphics->GetDevice(),
		m_graphics->GetCommandList(), geometryIndices.data(), geometryIBByteSize, m_geometry->IndexBufferUploader);

	m_geometry->VertexBufferByteSize = geometryVBByteSize;
	m_geometry->VertexByteStride = sizeof(Vertex);
	m_geometry->IndexBufferByteSize = geometryIBByteSize;
	m_geometry->IndexFormat = DXGI_FORMAT_R16_UINT;

	SubmeshGeometry boxGeo;
	boxGeo.BaseVertexLocation = 0;
	boxGeo.IndexCount = 36;
	boxGeo.StartIndexLocation = 0;

	m_geometry->DrawArgs["box"] = boxGeo;

	SubmeshGeometry pyramidGeo;
	pyramidGeo.BaseVertexLocation = 8;
	pyramidGeo.IndexCount = 18;
	pyramidGeo.StartIndexLocation = 36;

	m_geometry->DrawArgs["pyramid"] = pyramidGeo;


	std::array<VPosData, 8 > vPos =
	{
		VPosData({XMFLOAT3(-1.0f,-1.0f,-1.0f)}),
		VPosData({XMFLOAT3(-1.0f,+1.0f,-1.0f)}),
		VPosData({XMFLOAT3(+1.0f,+1.0f,-1.0f)}),
		VPosData({XMFLOAT3(+1.0f,-1.0f,-1.0f)}),
		VPosData({XMFLOAT3(-1.0f,-1.0f,+1.0f)}),
		VPosData({XMFLOAT3(-1.0f,+1.0f,+1.0f)}),
		VPosData({XMFLOAT3(+1.0f,+1.0f,+1.0f)}),
		VPosData({XMFLOAT3(+1.0f,-1.0f,+1.0f)})
	};

	std::array<VColorData, 8> vColor =
	{
		VColorData({XMFLOAT4(Colors::Red)}),
		VColorData({XMFLOAT4(Colors::Green)}),
		VColorData({XMFLOAT4(Colors::Blue)}),
		VColorData({XMFLOAT4(Colors::Yellow)}),
		VColorData({XMFLOAT4(Colors::Orange)}),
		VColorData({XMFLOAT4(Colors::White)}),
		VColorData({XMFLOAT4(Colors::Black)}),
		VColorData({XMFLOAT4(Colors::Indigo)})
	};

	UINT vPosByteSize = static_cast<UINT>(vPos.size() * sizeof(VPosData));
	UINT vColorByteSize = static_cast<UINT>(vColor.size() * sizeof(VColorData));

	//ComPtr<ID3DBlob>	m_copyBuffer;
	//ComPtr<ID3DBlob>	m_errorBuffer;

	//DX::ThrowIfFailed(D3DCreateBlob(vPosByteSize, m_copyBuffer.ReleaseAndGetAddressOf()));

	//RtlCopyMemory(m_copyBuffer->GetBufferPointer(), vPos.data(), vPosByteSize);

	m_vPosBuffer = D3DUtil::CreateDefaultBuffer(m_graphics->GetDevice(),
		m_graphics->GetCommandList(), vPos.data(), vPosByteSize, m_vPosBufferUploader);

	m_vColorBuffer = D3DUtil::CreateDefaultBuffer(m_graphics->GetDevice(),
		m_graphics->GetCommandList(), vColor.data(), vColorByteSize, m_vColorBufferUploader);


	m_vBufferView[0].BufferLocation = m_vPosBuffer->GetGPUVirtualAddress();
	m_vBufferView[0].SizeInBytes = vPosByteSize;
	m_vBufferView[0].StrideInBytes = sizeof(VPosData);

	m_vBufferView[1].BufferLocation = m_vColorBuffer->GetGPUVirtualAddress();
	m_vBufferView[1].SizeInBytes = vColorByteSize;
	m_vBufferView[1].StrideInBytes = sizeof(VColorData);

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

void DemoClass::OnResize(int width, int height)
{
	m_graphics->OnResize(width, height);
	
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, this->GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_proj, P);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int cmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	BINDU::BINDU_WINDOW_DESC wndDesc = { "TestWindow",800,600 };
	/*BINDU::Win32Window window(hInstance, wndDesc);*/

	BINDU::BinduApp* demoClass = new DemoClass(hInstance, wndDesc);

	DemoClass* pDemoClass = reinterpret_cast<DemoClass*>(demoClass);

	int n = BINDU::Win32Application::Run(demoClass, pDemoClass->GetWindow(), cmdShow);
	
	_CrtDumpMemoryLeaks();

	return n;
}