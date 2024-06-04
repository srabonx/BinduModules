#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <Bindu_Util.h>

#pragma comment (lib,"d3dcompiler.lib")
#pragma comment (lib,"D3D12.lib")
#pragma comment (lib,"dxgi.lib")

namespace BINDU
{
	class Graphics
	{
	public:
		Graphics(DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM);
		~Graphics();

		void	InitDirect3D();


	private:
		
		HRESULT CheckMSAAQuality();

		void	LogAdapters();

		void	LogAdapterOutputs(IDXGIAdapter* pAdapter);

		void	LogOutputDisplayModes(IDXGIOutput* pOutput, DXGI_FORMAT backBufferFormat);
		
	private:

		// Pipeline objects
		Microsoft::WRL::ComPtr<ID3D12Device10>	m_d3dDevice{ nullptr };
		

		Microsoft::WRL::ComPtr<IDXGIFactory7>	m_dxgiFactory{ nullptr };

		// Cpu Gpu synchronization objects
		Microsoft::WRL::ComPtr<ID3D12Fence1>	m_fence{ nullptr };


		// BackBuffer stuffs
		static const int m_swapChainBufferCount{ 2 };
		DXGI_FORMAT m_backBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
		
		// 4X MSAA stuffs
		UINT m_4xMSAAQuality{ 0 };


	};
}