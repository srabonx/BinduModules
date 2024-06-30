#pragma once
#include <windows.h>
#include <cstdint>



namespace BINDU
{
constexpr uint8_t MAX_MOUSE_BUTTON = 3;					// Currently supports
constexpr uint8_t MAX_KEYBOARD_KEY = 255;				// Currently supports


	enum MouseButton
	{
		LEFT = 0,
		RIGHT,
		MIDDLE
	};

	enum KeyBoardKey
	{
		BND_0 = 48,
		BND_1,
		BND_2,
		BND_3,
		BND_4,
		BND_5,
		BND_6,
		BND_7,
		BND_8,
		BND_9,

		A = 'A',
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z
	};

	struct KeyState
	{
		bool WasDown{ false };
		bool IsDown{ false };
		unsigned short RepeatCount{ 0 };
	};

	struct KeyboardInputMap
	{
		KeyState KeyboardKeyState[MAX_KEYBOARD_KEY] = { 0 };
	};

	struct ButtonState
	{
		bool WasDown{ false };
		bool IsDown{ false };
		bool IsMoving{ false };
	};

	struct MousePos
	{
		int prevX{ 0 };
		int prevY{ 0 };

		int currX{ 0 };
		int currY{ 0 };
	};

	struct MouseInputMap
	{
		ButtonState	MouseButtons[MAX_MOUSE_BUTTON];
		MousePos	MousePosition;
	};

	class Win32Input
	{
	
	public:

		virtual void OnMouseDown(MouseButton btn, int x, int y) = 0;
		virtual void OnMouseMove(MouseButton btn, int x, int y) = 0;
		virtual void OnMouseUp(MouseButton btn, int x, int y) = 0;

		static bool	IsMouseButtonPressed(MouseButton btn);
		static bool IsMouseButtonReleased(MouseButton btn);
		static bool IsMouseButtonHold(MouseButton btn);
		static bool IsMouseMoved(MouseButton btn);

		static bool IsKeyPressed(uint8_t key);
		static bool IsKeyReleased(uint8_t key);
		static bool IsKeyHold(uint8_t key);

		void InputMsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // Needs to be called on the Window message loop


		// Getters 
		inline static MousePos GetMousePosition() { return m_mouseInputMap.MousePosition; }
		inline static int GetCurrMouseX() { return m_mouseInputMap.MousePosition.currX; }
		inline static int GetCurrMouseY() { return m_mouseInputMap.MousePosition.currY; }
		inline static int GetPrevMouseX() { return m_mouseInputMap.MousePosition.prevX; }
		inline static int GetPrevMouseY() { return m_mouseInputMap.MousePosition.prevY; }
		inline static void GetCurrMouseXY(int& x, int& y) { x = m_mouseInputMap.MousePosition.currX; y = m_mouseInputMap.MousePosition.currY; }
		inline static void GetPrevMouseXY(int& x, int& y) { x = m_mouseInputMap.MousePosition.prevX; y = m_mouseInputMap.MousePosition.prevY; }

		inline static KeyState GetKeyState(uint8_t key) { return m_keyboardInputMap.KeyboardKeyState[key]; }

	private:

		static void ProcessKeyboardInput(WPARAM wParam, LPARAM lParam);			// Needs to be called on the Window message loop
		void ProcessMouseDown(WPARAM wParam, LPARAM lParam);			// Needs to be called on the Window message loop
		void ProcessMouseUp(WPARAM wParam, LPARAM lParam);
		void ProcessMouseMove(WPARAM wParam, LPARAM lParam);
		static void ProcessMouseMovement(WPARAM wParam);						// Needs to be called on the Window message loop
		static void UpdateMousePos(LPARAM lParam);								// Needs to be called on the Window message loop



	private:

		static MouseInputMap m_mouseInputMap;
		static bool	m_mouseMoving;
		static KeyboardInputMap m_keyboardInputMap;

	};
}