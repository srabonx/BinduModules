#pragma once

//#define WIN32_LEAN_AND_MEAN
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

	class Window
	{
	public:
		Window(HINSTANCE hInstance);
		Window(HINSTANCE hInstance, const BINDU_WINDOW_DESC windowDesc);
		Window(const Window& window) = delete;
		Window& operator= (const Window& window) = delete;
		virtual ~Window();

		bool CreateMainWindow();

		virtual LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


		static Window* GetWindow() { return m_window; }

		inline int	GetAspectRatio() { return m_windowWidth / m_windowHeight; }

		inline void SetWindowTitle(const std::wstring& title) { SetWindowTextW(m_windowHandle, title.c_str()); }

		inline HWND* GetWindowHandle() { return &m_windowHandle; }

		inline int	GetWindowHeight() const { return m_windowHeight; }
		inline int GetWindowWidth() const { return m_windowWidth; }

	private:
		static LRESULT CALLBACK WindowMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:
		static Window* m_window;
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
