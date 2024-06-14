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
