#include "../Include/Win32Application.h"
#include <Bindu_Timer.h>
#include <thread>
#include <windows.h>

int BINDU::Win32Application::Run(BinduApp* pApp, BINDU::Win32Window* pWindow, int nCmdShow)
{

	pWindow->CreateMainWindow();

	pApp->OnInit();

	MSG msg = { 0 };

	bool done{ false };

	while (!done)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				done = true;

		}
		else
		{

			if (!pApp->isPaused())
				pApp->Run();

		}
		
	}

	pApp->OnDestroy();

	delete pApp;
	
	return static_cast<int>(msg.wParam);
}
