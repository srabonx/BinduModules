#include "../Include/WavesAndValleys.h"

#include <DirectXColors.h>

WavesAndValleys::WavesAndValleys(HINSTANCE hInstance, const BINDU::BINDU_WINDOW_DESC& wndDesc, DXGI_MODE_DESC backBufferDisplayMode ) :
Win32Window(hInstance, wndDesc), DX12Graphics(&this->m_windowHandle, backBufferDisplayMode)
{

}

bool WavesAndValleys::OnInit()
{
	this->InitDirect3D();
	this->OnResize(m_windowWidth, m_windowHeight);


	m_timer.Reset();
	return false;
}

void WavesAndValleys::Run()
{
	m_timer.Tick();

	int fps{ 0 };
	float mspf{0.0f} ;

	if(this->CalculateFrameStats(fps, mspf, m_timer.TotalTime()))
	{
		std::wstring title = StringToWstring(this->m_windowTitle) + L" FPS: " + std::to_wstring(fps) + L"\tMSPF: " + std::to_wstring(mspf) + L"\tTOTAL TIME: " + std::to_wstring(m_timer.TotalTime());

		this->SetWindowTitle(title);
	}

	this->Update();
	this->Render();
}

bool WavesAndValleys::OnDestroy()
{

	return false;
}

void WavesAndValleys::Render()
{
	// Resetting the command allocator every frame to reuse the memory
	DX::ThrowIfFailed(m_commandAlloc->Reset());
	DX::ThrowIfFailed(m_commandList->Reset(m_commandAlloc.Get(), nullptr));

	m_commandList->RSSetViewports(1, this->GetViewPort());
	m_commandList->RSSetScissorRects(1, this->GetScissorRect());

	// Transition and prepare the current back buffer as the render target for this frame
	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_dxgiSwapChainBuffers[m_currentBackBuffer].Get(), D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_commandList->ResourceBarrier(1, &resourceBarrier);

	// set command to clear the Render target view and depth stencil view
	m_commandList->ClearRenderTargetView(GetCurrentBackBufferView(), DirectX::Colors::Red, 0, nullptr);
	m_commandList->ClearDepthStencilView(this->GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0, nullptr);

	// specify the buffer we are going to render to
	m_commandList->OMSetRenderTargets(1, &this->GetCurrentBackBufferView(), true,
		&this->GetDepthStencilView());


	// Transition and prepare the current back buffer as the present buffer for this frame

	resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_dxgiSwapChainBuffers[m_currentBackBuffer].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	m_commandList->ResourceBarrier(1, &resourceBarrier);

	// done recording commands for this frame
	DX::ThrowIfFailed(m_commandList->Close());

	// Execute the commands up to this point
	ID3D12CommandList* commands[] = {m_commandList.Get()};
	m_commandQueue->ExecuteCommandLists(_countof(commands), commands);

	// Present the current back buffer
	DX::ThrowIfFailed(m_dxgiSwapChain->Present(0, 0));

	m_currentBackBuffer = (m_currentBackBuffer + 1) % m_swapChainBufferCount;

	// Flush the command queue ( make the cpu wait until gpu finishes executing commands)
	this->FlushCommandQueue();
}

void WavesAndValleys::Update()
{
}



LRESULT WavesAndValleys::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

		if (m_d3dInitialized)
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

	return Win32Window::MsgProc(hWnd, msg, wParam, lParam);
}

