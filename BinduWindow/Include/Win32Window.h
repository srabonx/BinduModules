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
		Win32Window(HINSTANCE hInstance, const BINDU_WINDOW_DESC& windowDesc);
		Win32Window(const Win32Window& window) = delete;
		Win32Window& operator= (const Win32Window& window) = delete;
		virtual ~Win32Window();

		bool CreateMainWindow();

		Win32Window* GetWindow() { return this; }

		inline float   GetAspectRatio() const { return static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight); }

		inline void SetWindowTitle(const std::wstring& title) { SetWindowTextW(m_windowHandle, title.c_str()); }
		inline std::string GetWindowTitle() const { return m_windowTitle; }

		inline HWND& GetWindowHandle() { return m_windowHandle; }

		inline uint16_t	GetWindowHeight() const { return m_windowHeight; }
		inline uint16_t GetWindowWidth() const { return m_windowWidth; }

		inline void SetWindowWidth(const uint16_t width) { m_windowWidth = width; }
		inline void SetWindowHeight(const uint16_t height) { m_windowHeight = height; }

		inline bool IsResizing() const { return m_resizing; }
		inline void SetResizing(const bool resizing) { m_resizing = resizing; }

		inline bool IsWindowMinimized() const { return m_minimized; }
		inline void SetWindowMinimized(const bool minimized) { m_minimized = minimized; }

		inline bool IsWindowMaximized() const { return m_maximized; }
		inline void SetWindowMaximized(const bool maximized) { m_maximized = maximized; }

	private:
		static LRESULT CALLBACK WindowMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	protected:
		HWND		m_windowHandle{ nullptr };
		HINSTANCE   m_applicationInstanceHandle{ nullptr };

		bool		m_active{false};
		bool		m_resizing{ false };
		bool		m_minimized{ false };
		bool		m_maximized{ false };

		std::string m_windowTitle{ "Window" };
		uint16_t	m_windowWidth{ 200 };
		uint16_t	m_windowHeight{ 200 };

	};
}
