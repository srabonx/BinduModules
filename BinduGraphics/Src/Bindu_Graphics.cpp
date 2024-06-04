#include "../Include/Bindu_Graphics.h"
#include <vector>


BINDU::Graphics::Graphics(DXGI_FORMAT backBufferFormat) : m_backBufferFormat(backBufferFormat)
{

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
	ComPtr<IDXGIFactory>	dxgiFactory;

	DX::ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
	m_dxgiFactory.As(&dxgiFactory);

	// Try to create hardware device
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice));

	// if failed, create warp device
	if (FAILED(hr))
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		DX::ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

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

	// Create Swap chain.

	// Create RTV and DSV descriptor heaps.

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

	while (pAdapter->EnumOutputs(i, &pOutput))
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
