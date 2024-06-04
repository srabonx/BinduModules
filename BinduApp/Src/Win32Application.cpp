#include "../Include/Win32Application.h"

int BINDU::Win32Application::Run(BinduApp* pApp, BINDU::Window* pWindow, int nCmdShow)
{

	pWindow->CreateMainWindow();

	pApp->OnInit();

	MSG msg = { 0 };

	bool done{ false };

	while (!done)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				done = true;

		}
		else
		{
			if (!pApp->isPaused())
			{
				pApp->Run();
			}
			else
				Sleep(100);
		}
	}

	pApp->OnDestroy();

	delete pApp;

	return (int)msg.wParam;
}
