#include "../Include/MultipleShapesTest.h"
#include "../Include/TestModule.h"
#include <Win32Application.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int cmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	BINDU::BINDU_WINDOW_DESC wndDesc;
	wndDesc.windowHeight = 600;
	wndDesc.windowWidth = 800;
	wndDesc.windowTitle = "MultiShape";

	BINDU::BinduApp* app = new DemoClass(hInstance, wndDesc);

	DemoClass* pApp = reinterpret_cast<DemoClass*>(app);

	int n = BINDU::Win32Application::Run(app, pApp->GetWindow(), cmdShow);

#if defined(DEBUG) || defined(_DEBUG)
	_CrtDumpMemoryLeaks();
#endif
	return n;
}
