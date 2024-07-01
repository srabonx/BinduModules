#pragma once
#include <windows.h>
#include <cstdint>



namespace BINDU
{
constexpr uint8_t MAX_MOUSE_BUTTON = 3;					// Currently supports
constexpr uint8_t MAX_KEYBOARD_KEY = 255;				// Currently supports


	enum MouseButton
	{
		NONE = 0,
		LEFT ,
		RIGHT,
		MIDDLE
	};

	enum KeyBoardKey
	{
		BND_0 = '0',
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

	class Win32Input
	{
	
	public:

		virtual void OnMouseDown(MouseButton btn, int x, int y) = 0;
		virtual void OnMouseMove(MouseButton btn, int x, int y) = 0;
		virtual void OnMouseUp(MouseButton btn, int x, int y) = 0;

		virtual void OnKeyboardDown(KeyBoardKey key, bool isDown, bool repeat) = 0;
		virtual void OnKeyboardUp(KeyBoardKey key, bool isUp, bool repeat) = 0;


		void InputMsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // Needs to be called on the Window message loop


	private:

		void ProcessMouseDown(WPARAM wParam, LPARAM lParam);			// Needs to be called on the Window message loop
		void ProcessMouseUp(WPARAM wParam, LPARAM lParam);
		void ProcessMouseMove(WPARAM wParam, LPARAM lParam);

		void ProcessKeyboardDown(WPARAM wParam, LPARAM lParam);			// Needs to be called on the Window message loop
		void ProcessKeyboardUp(WPARAM wParam, LPARAM lParam);

	};
}