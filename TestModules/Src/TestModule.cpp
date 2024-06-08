#include "../Include/TestModule.h"
#include <Windows.h>
#include <d3dx12.h>
#include <DirectXColors.h>

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

	this->CreateDescriptorHeaps();

	return false;
}

void DemoClass::Run()
{
	m_timer.Tick();

	int fps{ 0 };
	float mspf{ 0.0f };
	
	float totalTime = m_timer.TotalTime();

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

}

void DemoClass::Render()
{
	DX::ThrowIfFailed(m_graphics->GetCommandAllocator()->Reset());

	m_graphics->GetCommandList()->Reset(m_graphics->GetCommandAllocator(), nullptr);

	m_graphics->GetCommandList()->RSSetViewports(1, m_graphics->GetViewPort());
	m_graphics->GetCommandList()->RSSetScissorRects(1, m_graphics->GetScissorRect());

	m_graphics->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_graphics->GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_graphics->GetCommandList()->ClearRenderTargetView(m_graphics->GetCurrentBackBufferView(), DirectX::Colors::Red, 0, nullptr);

	m_graphics->GetCommandList()->ClearDepthStencilView(m_graphics->GetDepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_graphics->GetCommandList()->OMSetRenderTargets(1, &m_graphics->GetCurrentBackBufferView(), true,
		&m_graphics->GetDepthStencilView());

	m_graphics->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_graphics->GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	m_graphics->GetCommandList()->Close();

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

	int elementByteSize{ 256 };
	
	m_graphics->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(elementByteSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_constantBuffer));

		
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = elementByteSize; 

	m_graphics->GetDevice()->CreateConstantBufferView(&cbvDesc, m_cbHeap->GetCPUDescriptorHandleForHeapStart());

}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int cmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	BINDU::BINDU_WINDOW_DESC wndDesc = { "TestWindow",400,400 };
	BINDU::Window window(hInstance, wndDesc);

	BINDU::BinduApp* demoClass = new DemoClass(&window);

	int n = BINDU::Win32Application::Run(demoClass, &window, cmdShow);
	
	_CrtDumpMemoryLeaks();

	return n;
}