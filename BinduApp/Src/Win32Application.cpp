#include "../Include/Win32Application.h"
#include <Bindu_Timer.h>
#include <thread>
#include <windows.h>

#include "Bindu_Util.h"

BINDU::Win32Application::Win32Application(HINSTANCE hInstance, const BINDU_WINDOW_DESC& windowDesc)
{

}

int BINDU::Win32Application::Start()
{

	this->CreateMainWindow();
	this->OnInit();

	MSG msg = { 0 };

	bool done{ false };

	while (!done)
	{

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{

			if (msg.message == WM_QUIT)
				done = true;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{

			if (!m_isPaused)
			{
				this->Run();
			}

		}
		
	}

	this->OnDestroy();
	
	return static_cast<int>(msg.wParam);
}

bool BINDU::Win32Application::CreateMainWindow()
{
	WNDCLASSEX wndClass = { 0 };
	//ZeroMemory(&wndClass, sizeof(wndClass));

	wndClass.cbSize = sizeof(wndClass);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WindowMsgProcCallback;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = m_applicationInstanceHandle;
	wndClass.hIcon = LoadIcon(m_applicationInstanceHandle, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(m_applicationInstanceHandle, IDC_ARROW);
	wndClass.hbrBackground = nullptr;
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = L"MAINWINDOW";
	wndClass.hIconSm = nullptr;

	if (!RegisterClassEx(&wndClass))
	{
		MessageBox(0, L"Register window class failed!", 0, 0);
		return false;
	}

	RECT r = { 0,0,m_windowWidth,m_windowHeight };
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);

	int width = r.right - r.left;
	int height = r.bottom - r.top;

	m_windowHandle = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
		L"MAINWINDOW",
		StringToWstring(m_windowTitle).c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		0,
		0,
		m_applicationInstanceHandle,
		0);
	if (!m_windowHandle)
	{
		MessageBox(0, L"Failed to create window!", 0, 0);
		return false;
	}

	ShowWindow(m_windowHandle, SW_SHOW);
	UpdateWindow(m_windowHandle);

	SetWindowLongPtr(m_windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	return true;
}

LRESULT BINDU::Win32Application::WindowMsgProcCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Win32Application* pWin32App = reinterpret_cast<Win32Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if(msg == WM_DESTROY)
	{
		delete pWin32App;
		pWin32App = nullptr;
	}

	if (pWin32App)
	{
		return pWin32App->WindowMessageProc(msg, wParam, lParam);
	}
	else
		return DefWindowProc(hWnd, msg, wParam, lParam);

}
