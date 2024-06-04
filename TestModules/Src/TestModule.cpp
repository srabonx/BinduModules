#include "../Include/TestModule.h"
#include <Windows.h>

DemoClass::DemoClass()
{

}

DemoClass::~DemoClass()
{
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

	BINDU::BinduApp* demoClass = new DemoClass();

	int n = BINDU::Win32Application::Run(demoClass, &window, cmdShow);
	
	_CrtDumpMemoryLeaks();

	return n;
}