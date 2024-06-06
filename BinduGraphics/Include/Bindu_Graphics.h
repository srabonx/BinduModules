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
		Graphics(HWND* hwnd, DXGI_MODE_DESC backBufferDisplayMode, DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM);
		~Graphics();

		void	InitDirect3D();
		void	FlushCommandQueue();
		void	OnResize(int width, int height);

	private:
		
		HRESULT CheckMSAAQuality();

		void	LogAdapters();

		void	LogAdapterOutputs(IDXGIAdapter* pAdapter);

		void	LogOutputDisplayModes(IDXGIOutput* pOutput, DXGI_FORMAT backBufferFormat);

		void	CreateCommandObjects();

		void	CreateSwapChain();

		void	CreateRtvAndDsvDescriptorHeaps();

		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;
		
	private:

		// Pointer to the Output Window
		HWND* m_hwndOutputWindow{ nullptr };

		// Pipeline objects
		Microsoft::WRL::ComPtr<ID3D12Device>	m_d3dDevice{ nullptr };
		

		Microsoft::WRL::ComPtr<IDXGIFactory>	m_dxgiFactory{ nullptr };

		// Cpu Gpu synchronization objects
		UINT64	m_currentFence{ 0 };
		Microsoft::WRL::ComPtr<ID3D12Fence>	m_fence{ nullptr };


		// Command objects
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>	m_commandQueue{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator>	m_commandAlloc{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12DebugCommandList>	m_debugCommandList{ nullptr };

		// Descriptor Heaps
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_rtvDescriptorHeap{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_dsvDescriptorHeap{ nullptr };


		// Buffer/ SwapChain stuffs
		Microsoft::WRL::ComPtr<IDXGISwapChain>	m_dxgiSwapChain{ nullptr };
		static const int m_swapChainBufferCount{ 2 };
		int	m_currentBackBuffer{ 0 };
		Microsoft::WRL::ComPtr<ID3D12Resource> m_dxgiSwapChainBuffer[m_swapChainBufferCount]{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer{ nullptr };
		DXGI_FORMAT m_backBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
		DXGI_MODE_DESC	m_dxgiModeDesc = { 0 };

		D3D12_VIEWPORT	m_viewport;
		D3D12_RECT		m_scissorRect;
		
		// 4X MSAA stuffs
		bool m_4xMSAAState{ false };
		UINT m_4xMSAAQuality{ 0 };


	};
}