#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <Bindu_Util.h>
#include "D3D12Util.h"
#include "UploadBuffer.h"

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

	
		void	LogAdapters();


		// Getter and Setters

		inline ID3D12Device* GetDevice() const { return m_d3dDevice.Get(); }

		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const;

		inline ID3D12Resource* GetCurrentBackBuffer() const { return m_dxgiSwapChainBuffer[m_currentBackBuffer].Get(); }

		D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

		inline ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }

		inline ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }

		inline ID3D12CommandAllocator* GetCommandAllocator() { return m_commandAlloc.Get(); }

		inline D3D12_VIEWPORT* GetViewPort() { return &m_viewport; }

		inline D3D12_RECT* GetScissorRect() { return &m_scissorRect; }

		inline IDXGISwapChain* GetSwapChain() { return m_dxgiSwapChain.Get(); }

		inline void SetCurrentBackBufferIndex(int bufferIndex) { m_currentBackBuffer = bufferIndex; }

		inline int GetCurrentBackBufferIndex() const { return m_currentBackBuffer; }

		inline int GetNumberOfSwapChainBuffer() const { return m_swapChainBufferCount; }

		inline DXGI_FORMAT GetDepthStencilFormat() const { return m_depthStencilFormat; }

		inline DXGI_FORMAT GetBackBufferFormat() const { return m_backBufferFormat; }

		inline bool Get4XMSAAState() const { return m_4xMSAAState; }

		inline UINT Get4XMSAAQuality() const { return m_4xMSAAQuality; }

	private:

		void	LogAdapterOutputs(IDXGIAdapter* pAdapter);

		void	LogOutputDisplayModes(IDXGIOutput* pOutput, DXGI_FORMAT backBufferFormat);

		HRESULT CheckMSAAQuality();

		void	CreateCommandObjects();

		void	CreateSwapChain();

		void	CreateRtvAndDsvDescriptorHeaps();

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
		DXGI_FORMAT m_depthStencilFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };
		DXGI_MODE_DESC	m_dxgiModeDesc = { 0 };

		D3D12_VIEWPORT	m_viewport;
		D3D12_RECT		m_scissorRect;
		
		// 4X MSAA stuffs
		bool m_4xMSAAState{ false };
		UINT m_4xMSAAQuality{ 0 };


	};
}