#include "../Include/Win32Window.h"

//LRESULT CALLBACK WindowMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
//{
//	return BINDU::Win32Window::GetWindow()->MsgProc(hWnd, msg, wParam, lParam);
//}

BINDU::Win32Window* BINDU::Win32Window::m_window = nullptr;

BINDU::Win32Window::Win32Window(HINSTANCE hInstance) : m_applicationInstanceHandle(hInstance)
{
	assert(m_window == nullptr);
	m_window = this;
}

BINDU::Win32Window::Win32Window(HINSTANCE hInstance, const BINDU_WINDOW_DESC windowDesc) : m_applicationInstanceHandle(hInstance),
															m_windowTitle(windowDesc.windowTitle),
															m_windowWidth(windowDesc.windowWidth),
															m_windowHeight(windowDesc.windowHeight)
{
	assert(m_window == nullptr);
	m_window = this;
}

BINDU::Win32Window::~Win32Window()
{
}

bool BINDU::Win32Window::CreateMainWindow()
{
	
	WNDCLASSEX wndClass = { 0 };
	//ZeroMemory(&wndClass, sizeof(wndClass));

	wndClass.cbSize = sizeof(wndClass);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WindowMsgProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = m_applicationInstanceHandle;
	wndClass.hIcon = LoadIcon(m_applicationInstanceHandle, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(m_applicationInstanceHandle, IDC_ARROW);
	wndClass.hbrBackground = NULL;
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = L"MAINWINDOW";
	wndClass.hIconSm = NULL;

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
									width,height,
									0,
									0,
									m_applicationInstanceHandle,
									0);

	if (!m_windowHandle)
	{
		MessageBox(0, L"Failed to create window!", 0, 0);
		return false;
	}

	ShowWindow(m_windowHandle,SW_SHOW);
	UpdateWindow(m_windowHandle);

	return true;
}



LRESULT BINDU::Win32Window::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT BINDU::Win32Window::WindowMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return BINDU::Win32Window::GetWindow()->MsgProc(hWnd, msg, wParam, lParam);
}

