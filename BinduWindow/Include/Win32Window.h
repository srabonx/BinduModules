#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WindowsX.h>
#include <string>
#include <Bindu_Util.h>
#include <assert.h>

namespace BINDU
{

	struct BINDU_WINDOW_DESC
	{
		std::string windowTitle{ "Window" };
		int			windowWidth{ 200 };
		int			windowHeight{ 200 };
	};

	class Win32Window
	{
	public:
		Win32Window(HINSTANCE hInstance);
		Win32Window(HINSTANCE hInstance, const BINDU_WINDOW_DESC windowDesc);
		Win32Window(const Win32Window& window) = delete;
		Win32Window& operator= (const Win32Window& window) = delete;
		virtual ~Win32Window();

		bool CreateMainWindow();

		virtual LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


		static Win32Window* GetWindow() { return m_window; }

		inline float   GetAspectRatio() { return static_cast<float>(m_windowWidth) / m_windowHeight; }

		inline void SetWindowTitle(const std::wstring& title) { SetWindowTextW(m_windowHandle, title.c_str()); }

		inline HWND* GetWindowHandle() { return &m_windowHandle; }

		inline int	GetWindowHeight() const { return m_windowHeight; }
		inline int GetWindowWidth() const { return m_windowWidth; }

	private:
		static LRESULT CALLBACK WindowMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:
		static Win32Window* m_window;
		HWND		m_windowHandle{ nullptr };
		HINSTANCE   m_applicationInstanceHandle{ nullptr };

		bool		m_resizing{ false };
		bool		m_minimized{ false };
		bool		m_maximized{ false };

		std::string m_windowTitle{ "Window" };
		int			m_windowWidth{ 200 };
		int			m_windowHeight{ 200 };
	};
}
