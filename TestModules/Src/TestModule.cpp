#include "../Include/TestModule.h"
#include <Windows.h>

DemoClass::DemoClass(BINDU::Window* window) : m_window(window)
{
	m_timer.Reset();
}

DemoClass::~DemoClass()
{
	
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