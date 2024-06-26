#include "../Include/MultipleShapesTest.h"

MultiShape::MultiShape(HINSTANCE hInstance) : BINDU::Win32Window(hInstance)
{

}

MultiShape::MultiShape(HINSTANCE hInstance, BINDU::BINDU_WINDOW_DESC desc) : BINDU::Win32Window(hInstance, desc)
{

}

MultiShape::~MultiShape()
{
}

bool MultiShape::OnInit()
{
	DXGI_MODE_DESC dxgiModeDesc;
	dxgiModeDesc.Width = m_windowWidth;
	dxgiModeDesc.Height = m_windowHeight;
	dxgiModeDesc.RefreshRate.Numerator = 60;
	dxgiModeDesc.RefreshRate.Denominator = 1;
	dxgiModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	m_graphics = std::make_unique<BINDU::Graphics>(&m_windowHandle, dxgiModeDesc);

	m_graphics->InitDirect3D();
	
	m_timer.Reset();
	return true;
}

void MultiShape::Run()
{
	m_timer.Tick();

	static int fps{ 0 };
	static float mspf{ 0.f };
	
	if (this->CalculateFrameStats(fps, mspf, static_cast<float>(m_timer.TotalTime())))
	{

		std::wstring title = L"FPS: " + std::to_wstring(fps) + L"\tMSPF: " + std::to_wstring(mspf) + L"\tTOTAL TIME: " + std::to_wstring(m_timer.TotalTime());

		this->SetWindowTitle(title);
	}

	this->Update();
	this->Render();
}

void MultiShape::Update()
{
}

void MultiShape::Render()
{
}

void MultiShape::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
		m_frameResources.push_back(std::make_unique<FrameResource>(m_graphics->GetDevice(),
			1, 10));
}

void MultiShape::BuildDescriptorHeaps()
{
	UINT objcount = static_cast<UINT>(m_opaqueRItem.size());

	UINT numOfDescriptor = (objcount + 1) * gNumFrameResources;

	m_perPassCBVOffset = objcount * gNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NumDescriptors = numOfDescriptor;
	heapDesc.NodeMask = 0;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DX::ThrowIfFailed(m_graphics->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void MultiShape::UpdatePerObjectCB()
{
	auto currObjectCB = m_pCurrFrameResource->ObjectCB.get();

	for (auto& e : m_allRItem)
	{
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX worldMatrix = XMLoadFloat4x4(&e->World);

			PerObjectConstants	perObjConstants;
			XMStoreFloat4x4(&perObjConstants.WorldMatrix, XMMatrixTranspose(worldMatrix));

			currObjectCB->CopyData(e->Index, perObjConstants);

			e->NumFramesDirty--;
		}
	}
}

void MultiShape::BuildRootSignature()
{

	ComPtr<ID3DBlob>	serializedRootSig{ nullptr };
	ComPtr<ID3DBlob>	errorBlob{ nullptr };

	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	// Create root CBVs
	CD3DX12_ROOT_PARAMETER rootParameter[2];
	rootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	rootParameter[1].InitAsDescriptorTable(1, &cbvTable1);



	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, rootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	}

	DX::ThrowIfFailed(hr);

	DX::ThrowIfFailed(m_graphics->GetDevice()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(), IID_PPV_ARGS(m_rootSig.ReleaseAndGetAddressOf())));
}

void MultiShape::BuildConstantBufferViews()
{
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PerObjectConstants));

	UINT objCount = static_cast<UINT>(m_opaqueRItem.size());

	// we need a constant buffer view for each object in each frame resource
	for (int frameIndex = 0; frameIndex < gNumFrameResources; frameIndex++)
	{
		auto objectCB = m_frameResources[frameIndex]->ObjectCB->Resource();

		for (UINT i = 0; i < objCount; i++)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

			// offset to the ith object constant buffer in the buffer
			cbAddress += i * objCBByteSize;

			// offset to the object cbv in the descriptor heap
			int heapIndex = frameIndex * objCount + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, m_graphics->GetCbvSrvUavDescriptorIncreamentSize());

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			m_graphics->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	UINT passCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PerPassConstants));

	// last 3 descriptor are the pass cbv for each frame resource

	for (int frameIndex = 0; frameIndex < gNumFrameResources; frameIndex++)
	{
		auto passCB = m_frameResources[frameIndex]->PassCB->Resource();

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// offset to pass cbv in the descriptor heap
		int heapIndex = m_perPassCBVOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_graphics->GetCbvSrvUavDescriptorIncreamentSize());

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		m_graphics->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
	}

}

void MultiShape::UpdatePerPassCB()
{
	auto currPassCB = m_pCurrFrameResource->PassCB.get();

	PerPassConstants	passConstant;

	XMMATRIX viewMat = XMLoadFloat4x4(&m_viewMatrix);
	XMMATRIX projMat = XMLoadFloat4x4(&m_projMatrix);

	XMMATRIX viewProjMat = XMMatrixMultiply(viewMat, projMat);

	XMMATRIX invViewMat = XMMatrixInverse(&XMMatrixDeterminant(viewMat), viewMat);
	XMMATRIX invProjMat = XMMatrixInverse(&XMMatrixDeterminant(projMat), projMat);
	XMMATRIX invViewProjMat = XMMatrixInverse(&XMMatrixDeterminant(viewProjMat), viewProjMat);

	XMStoreFloat4x4(&passConstant.ViewMatrix, XMMatrixTranspose(viewMat));
	XMStoreFloat4x4(&passConstant.ProjMatrix, XMMatrixTranspose(projMat));
	XMStoreFloat4x4(&passConstant.ViewProjMatrix, XMMatrixTranspose(viewProjMat));
	XMStoreFloat4x4(&passConstant.InvViewMatrix, XMMatrixTranspose(invViewMat));
	XMStoreFloat4x4(&passConstant.InvProjMatrix, XMMatrixTranspose(invProjMat));
	XMStoreFloat4x4(&passConstant.InvViewProjMatrix, XMMatrixTranspose(invViewProjMat));

	passConstant.EyePosW = m_eyePosW;
	passConstant.RenderTargetSize = { static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight) };
	passConstant.InvRenderTargetSize = { 1.0f / passConstant.RenderTargetSize.x, 1.0f / passConstant.RenderTargetSize.y };

	passConstant.NearZ = 1.0f;
	passConstant.FarZ = 1000.0f;

	passConstant.TotalTime = m_timer.TotalTime();
	passConstant.DeltaTime = m_timer.DeltaTime();

	currPassCB->CopyData(0, passConstant);
}

void MultiShape::BuildShadersAndInputLayout()
{
	m_shaders["standardVS"] = D3DUtil::CompileShader(RelativeResourcePath("Shaders\\color.hlsl"), 
		nullptr, "VS", "vs_5_1");
	m_shaders["opaquePS"] = D3DUtil::CompileShader(RelativeResourcePath("Shaders\\color.hlsl"),
		nullptr, "PS", "ps_5_1");

	m_inputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
}
