#pragma once
#include "Bindu_App.h"

#define WIN32_LEAN_AND_MEAN
#include <string>
#include <Windows.h>

namespace BINDU
{
	struct BINDU_WINDOW_DESC
	{
		std::string windowTitle{ "Window" };
		int			windowWidth{ 200 };
		int			windowHeight{ 200 };
	};

	class Win32Application : public BinduApp
	{
	public:

		Win32Application(HINSTANCE hInstance, const BINDU_WINDOW_DESC& windowDesc);
		virtual ~Win32Application() = default;

		virtual LRESULT WindowMessageProc(UINT& msg, WPARAM& wParam, LPARAM& lParam) = 0;

		int Start();

		bool CreateMainWindow();

		float   GetAspectRatio() const { return static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight); }

		void SetWindowTitle(const std::wstring& title) { SetWindowTextW(m_windowHandle, title.c_str()); }
		std::string GetWindowTitle() const { return m_windowTitle; }

		HWND& GetWindowHandle() { return m_windowHandle; }

		uint16_t	GetWindowHeight() const { return m_windowHeight; }
		uint16_t GetWindowWidth() const { return m_windowWidth; }

		void SetWindowWidth(const uint16_t width) { m_windowWidth = width; }
		void SetWindowHeight(const uint16_t height) { m_windowHeight = height; }

		bool IsResizing() const { return m_resizing; }
		void SetResizing(const bool resizing) { m_resizing = resizing; }

		bool IsWindowMinimized() const { return m_minimized; }
		void SetWindowMinimized(const bool minimized) { m_minimized = minimized; }

		bool IsWindowMaximized() const { return m_maximized; }
		void SetWindowMaximized(const bool maximized) { m_maximized = maximized; }

	private:
		static LRESULT CALLBACK WindowMsgProcCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:

		HWND		m_windowHandle{ nullptr };
		HINSTANCE   m_applicationInstanceHandle{ nullptr };

		bool		m_active{ false };
		bool		m_resizing{ false };
		bool		m_minimized{ false };
		bool		m_maximized{ false };

		std::string m_windowTitle{ "Window" };
		uint16_t	m_windowWidth{ 200 };
		uint16_t	m_windowHeight{ 200 };

		bool m_isPaused{ false };
	};
}
