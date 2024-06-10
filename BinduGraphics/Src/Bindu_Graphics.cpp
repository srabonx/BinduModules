#include "../Include/Bindu_Graphics.h"
#include <vector>
#include "../Include/d3dx12.h"


BINDU::Graphics::Graphics(HWND* hwnd, DXGI_MODE_DESC backBufferDisplayMode, DXGI_FORMAT backBufferFormat) : m_hwndOutputWindow(hwnd), m_backBufferFormat(backBufferFormat)
{
	backBufferDisplayMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	backBufferDisplayMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	backBufferDisplayMode.Format = backBufferFormat;

	m_dxgiModeDesc = backBufferDisplayMode;
}

BINDU::Graphics::~Graphics()
{
}

void BINDU::Graphics::InitDirect3D()
{
	// using this namespace for convenience.
	using namespace Microsoft::WRL;

	UINT dxgiFactoryFlags{ 0 };
	// Enable the Debug layer.
#if defined(DEBUG) || defined(_DEBUG)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		DX::ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();

		// Additional dxgi debug layer.
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	// DXGI factory object
	ComPtr<IDXGIFactory7>	dxgiFactory;

	DX::ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));
	m_dxgiFactory.As(&dxgiFactory);

	// Try to create hardware device
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice));

	// if failed, create warp device
	if (FAILED(hr))
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		DX::ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		DX::ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_d3dDevice.ReleaseAndGetAddressOf())));
	}


	


	// Creating the fence
	DX::ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));


	// Check for 4X MSAA quality support
	// All Direct3D 11 capable cards supports 4X MSAA for all render target, so we just need to check for quality support.
	DX::ThrowIfFailed(this->CheckMSAAQuality());

#if defined(DEBUG) || (_DEBUG)
	// Log the Adapter and Display modes
	this->LogAdapters();
#endif

	// Create Command objects.
	CreateCommandObjects();

	// Create Swap chain.
	CreateSwapChain();

	// Create RTV and DSV descriptor heaps.
	CreateRtvAndDsvDescriptorHeaps();

	// Creating debug command list
	ComPtr<ID3D12DebugCommandList1>	debugCmdList;
	DX::ThrowIfFailed(m_commandList->QueryInterface(IID_PPV_ARGS(&m_debugCommandList)));
	m_debugCommandList.As(&debugCmdList);

	D3D12_DEBUG_COMMAND_LIST_PARAMETER_TYPE parameterType = D3D12_DEBUG_COMMAND_LIST_PARAMETER_TYPE::D3D12_DEBUG_COMMAND_LIST_PARAMETER_GPU_BASED_VALIDATION_SETTINGS;
	DX::ThrowIfFailed(debugCmdList->GetDebugParameter(parameterType, &parameterType, sizeof(parameterType)));


	// Initial resize
	this->OnResize(800, 800);
}

void BINDU::Graphics::FlushCommandQueue()
{
	// Advance the fence value to mark commands upto this fence point
	m_currentFence++;

	// Add instruction to the command queue to set a new fence point.
	// The new fence point won't be set until the gpu processes all the commands prior to this Signal().
	DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));

	if (m_fence->GetCompletedValue() < m_currentFence)
	{
		HANDLE eventHandle = CreateEventExW(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when gpu hits current fence
		DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFence, eventHandle));
		
		// Wait until gpu hit current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

}

void BINDU::Graphics::OnResize(int width, int height)
{
	assert(m_d3dDevice);
	assert(m_dxgiSwapChain);
	assert(m_commandAlloc);

	// Flush before changing any resources
	FlushCommandQueue();

	DX::ThrowIfFailed(m_commandList->Reset(m_commandAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating
	for (int i = 0; i < m_swapChainBufferCount; ++i)
		m_dxgiSwapChainBuffer[i].Reset();
	m_depthStencilBuffer.Reset();

	// Resize the swapChain
	DX::ThrowIfFailed(m_dxgiSwapChain->ResizeBuffers(m_swapChainBufferCount, width, height,
		m_backBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_currentBackBuffer = 0;

	// Create RTV for both buffers

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	UINT rtvHeapHandleSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (UINT i = 0; i < m_swapChainBufferCount; i++)
	{
		DX::ThrowIfFailed(m_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_dxgiSwapChainBuffer[i])));
		m_d3dDevice->CreateRenderTargetView(m_dxgiSwapChainBuffer[i].Get(), nullptr, rtvDescriptorHandle);
		rtvDescriptorHandle.Offset(1, rtvHeapHandleSize);
	}


	// Create the depth stencil buffer and view

	D3D12_RESOURCE_DESC	depthStencilDesc;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.Height = height;
	depthStencilDesc.Width = width;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.SampleDesc.Count = m_4xMSAAState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_4xMSAAState ? (m_4xMSAAQuality - 1) : 0;
	

	D3D12_CLEAR_VALUE	optClear;
	optClear.Format = m_depthStencilFormat;		// Depth stencil format
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	DX::ThrowIfFailed(m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(&m_depthStencilBuffer)));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, this->GetDepthStencilView());

	// Transition the resource from it's initial state to be used as a depth buffer
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands
	DX::ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// Wait until the resize is complete
	this->FlushCommandQueue();

	// Update the viewport and scissor rect

	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Height = static_cast<FLOAT>(height);
	m_viewport.Width = static_cast<FLOAT>(width);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
		
}

HRESULT BINDU::Graphics::CheckMSAAQuality()
{
	HRESULT hr = S_OK;

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS mSQualityLvls;
	mSQualityLvls.Format = m_backBufferFormat;
	mSQualityLvls.NumQualityLevels = 0;
	mSQualityLvls.SampleCount = 4;
	mSQualityLvls.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

	hr = m_d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &mSQualityLvls, sizeof(mSQualityLvls));

	m_4xMSAAQuality = mSQualityLvls.NumQualityLevels;

	assert(m_4xMSAAQuality > 0 && "Unexpected MSAA quality level.");

	return hr;
}

void BINDU::Graphics::LogAdapters()
{
	IDXGIAdapter* pAdapter{ nullptr };
	UINT i{ 0 };
	std::vector<IDXGIAdapter*> pAdapterList;

	while (m_dxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc = { 0 };
		pAdapter->GetDesc(&desc);

		std::wstring adapterName = desc.Description;
		SIZE_T dedicatedSysMem = (desc.DedicatedSystemMemory == 0) ? desc.DedicatedSystemMemory : (desc.DedicatedSystemMemory / 1024) / 1024;
		SIZE_T dedicatedVidMem = (desc.DedicatedVideoMemory / 1024) / 1024;
		SIZE_T sharedSysMem = (desc.SharedSystemMemory / 1024) / 1024;
		UINT   deviceId = desc.DeviceId;
		UINT   vendorId = desc.VendorId;
		UINT   subSysId = desc.SubSysId;
		UINT   revision = desc.Revision;

		std::wstring text = L"*****		ADAPTER	DESCRIPTION		*****\n";
		text += L"ADAPTER INDEX				: " + std::to_wstring(i + 1) + L"\n";
		text += L"NAME						: " + adapterName + L"\n";
		text += L"DEDICATED SYSTEM MEMORY		: " + std::to_wstring(dedicatedSysMem) + L" MB\n";
		text += L"DEDICATED VIDEO MEMORY		: " + std::to_wstring(dedicatedVidMem) + L" MB\n";
		text += L"SHARED SYSTEM MEMORY		: " + std::to_wstring(sharedSysMem) + L" MB\n";
		text += L"DEVICE ID					: " + std::to_wstring(deviceId) + L"\n";
		text += L"VENDOR ID					: " + std::to_wstring(vendorId) + L"\n";
		text += L"SUB-SYSTEM ID				: " + std::to_wstring(subSysId) + L"\n";
		text += L"REVISION					: " + std::to_wstring(revision) + L"\n";
		text += L"	**********************************************	\n";

		OutputDebugString(text.c_str());

		pAdapterList.push_back(pAdapter);
		
		++i;
	}

	for (size_t i = 0; i < pAdapterList.size(); ++i)
	{
		this->LogAdapterOutputs(pAdapterList[i]);
		SAFE_RELEASE(pAdapterList[i]);
	}
}

void BINDU::Graphics::LogAdapterOutputs(IDXGIAdapter* pAdapter)
{
	
	IDXGIOutput* pOutput{ nullptr };
	UINT i{ 0 };

	while (pAdapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc = { 0 };
		pOutput->GetDesc(&desc);

		std::wstring deviceName = desc.DeviceName;
		RECT desktopCoor = desc.DesktopCoordinates;

		std::wstring outputCoordStr = L"LEFT: " + std::to_wstring(desktopCoor.left) +
			L" RIGHT: " + std::to_wstring(desktopCoor.right) +
			L" TOP: " + std::to_wstring(desktopCoor.top) +
			L" BOTTOM: " + std::to_wstring(desktopCoor.bottom);

		std::wstring text = L"\n*****		OUTPUT	DESCRIPTION		*****\n";
		text += L"OUTPUT INDEX			: " + std::to_wstring(i + 1) + L"\n";
		text += L"OUTPUT NAME				: " + deviceName + L"\n";
		text += L"OUTPUT COORDINATES		: " + outputCoordStr + L"\n";
		text += L"SUPPORTED OUTPUT MODES	:\n";

		OutputDebugString(text.c_str());

		this->LogOutputDisplayModes(pOutput, m_backBufferFormat);

		SAFE_RELEASE(pOutput);

		++i;
	}
}

void BINDU::Graphics::LogOutputDisplayModes(IDXGIOutput* pOutput, DXGI_FORMAT backBufferFormat)
{
	UINT count{ 0 };
	UINT flags{ 0 };

	// Calling with nullptr to get the list count.
	pOutput->GetDisplayModeList(backBufferFormat, flags, &count, nullptr);
	
	std::vector<DXGI_MODE_DESC> modeList(count);

	pOutput->GetDisplayModeList(backBufferFormat, flags, &count, &modeList[0]);

	UINT i{ 1 };
	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		float rt = static_cast<float>(n) / static_cast<float> (d);

		std::wstring text =
			std::to_wstring(i) + L". WIDTH = " + std::to_wstring(x.Width) + L" " +
			L"HEIGHT = " + std::to_wstring(x.Height) + L" " +
			L"REFRESH RATE = " + std::to_wstring(n) + L"/" + std::to_wstring(d) + L" = " + std::to_wstring(rt) + L" HZ"
			L"\n";

		OutputDebugString(text.c_str());
		++i;
	}

}

void BINDU::Graphics::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC cmdQdesc;
	cmdQdesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQdesc.NodeMask = 0;
	cmdQdesc.Priority = 0;
	cmdQdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	DX::ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&cmdQdesc, IID_PPV_ARGS(&m_commandQueue)));

	DX::ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAlloc)));

	DX::ThrowIfFailed(m_d3dDevice->CreateCommandList(0,
													D3D12_COMMAND_LIST_TYPE_DIRECT,
													m_commandAlloc.Get(),			// Associated command allocator
													nullptr,						// Initial pipeline state
													IID_PPV_ARGS(&m_commandList)));
	
	// Command list needs to be closed before reset. We'll reset command list first time we refer to it
	m_commandList->Close();
}

void BINDU::Graphics::CreateSwapChain()
{
	// Release the previous swapchain, if any available.
	m_dxgiSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC desc = { 0 };
	desc.BufferCount = m_swapChainBufferCount;
	desc.BufferDesc = m_dxgiModeDesc;
	desc.Windowed = true;
	desc.SampleDesc.Count = m_4xMSAAState ? 4 : 1;
	desc.SampleDesc.Quality = m_4xMSAAState ? (m_4xMSAAQuality - 1) : 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	desc.OutputWindow = *m_hwndOutputWindow;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Dxgi uses command queue to perform flush
	DX::ThrowIfFailed(m_dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &desc, &m_dxgiSwapChain));
}

void BINDU::Graphics::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = m_swapChainBufferCount;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NodeMask = 0;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvDescriptorHeap)));
}

D3D12_CPU_DESCRIPTOR_HANDLE BINDU::Graphics::GetCurrentBackBufferView() const
{
	UINT incrSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_currentBackBuffer, incrSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE BINDU::Graphics::GetDepthStencilView() const
{
	return m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}
