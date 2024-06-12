#include "../include/Win32Input.h"
#include <windowsx.h>

BINDU::MouseInputMap BINDU::Win32Input::m_mouseInputMap = { 0 };
BINDU::KeyboardInputMap BINDU::Win32Input::m_keyboardInputMap = { 0 };
bool BINDU::Win32Input::m_mouseMoving = { false };


bool BINDU::Win32Input::IsMouseButtonPressed(MouseButton btn)
{
	bool res = m_mouseInputMap.MouseButtons[btn].IsDown;
	m_mouseInputMap.MouseButtons[btn].IsDown = false;

	return !m_mouseInputMap.MouseButtons[btn].WasDown && res;
}

bool BINDU::Win32Input::IsMouseButtonReleased(MouseButton btn)
{
	bool res = m_mouseInputMap.MouseButtons[btn].WasDown;
	m_mouseInputMap.MouseButtons[btn].WasDown = false;

	return !m_mouseInputMap.MouseButtons[btn].IsDown && res;
}

bool BINDU::Win32Input::IsMouseButtonHold(MouseButton btn)
{
	return m_mouseInputMap.MouseButtons[btn].IsDown;
}

bool BINDU::Win32Input::IsMouseMoved(MouseButton btn)
{
	return (m_mouseInputMap.MouseButtons[btn].IsDown && ((m_mouseInputMap.MousePosition.prevX != m_mouseInputMap.MousePosition.currX) &&
		(m_mouseInputMap.MousePosition.prevY != m_mouseInputMap.MousePosition.currY)));
}

bool BINDU::Win32Input::IsKeyPressed(uint8_t key)
{

	bool res = m_keyboardInputMap.KeyboardKeyState[key].IsDown;
	m_keyboardInputMap.KeyboardKeyState[key].IsDown = false;
	m_keyboardInputMap.KeyboardKeyState[key].RepeatCount = 1;

	return !m_keyboardInputMap.KeyboardKeyState[key].WasDown && res;
}

bool BINDU::Win32Input::IsKeyReleased(uint8_t key)
{
	bool res = m_keyboardInputMap.KeyboardKeyState[key].WasDown;
	m_keyboardInputMap.KeyboardKeyState[key].WasDown = false;
	m_keyboardInputMap.KeyboardKeyState[key].RepeatCount = 1;

	return !m_keyboardInputMap.KeyboardKeyState[key].IsDown && res;
}

void BINDU::Win32Input::UpdateMousePos(LPARAM lParam)
{
	m_mouseMoving = true;
	m_mouseInputMap.MousePosition.prevX = m_mouseInputMap.MousePosition.currX;
	m_mouseInputMap.MousePosition.prevY = m_mouseInputMap.MousePosition.currY;

	m_mouseInputMap.MousePosition.currX = GET_X_LPARAM(lParam);
	m_mouseInputMap.MousePosition.currY = GET_Y_LPARAM(lParam);
}

bool BINDU::Win32Input::IsKeyHold(uint8_t key)
{
	return m_keyboardInputMap.KeyboardKeyState[key].IsDown && m_keyboardInputMap.KeyboardKeyState[key].WasDown;
}


void BINDU::Win32Input::ProcessKeyboardInput(WPARAM wParam, LPARAM lParam)
{
	WORD vkCode = LOWORD(wParam);
	WORD keyFlag = HIWORD(lParam);

	bool wasDown = (keyFlag & KF_REPEAT) == KF_REPEAT;
	WORD repeatCount = LOWORD(lParam);
	bool isDown = (keyFlag & KF_UP) != KF_UP;

	m_keyboardInputMap.KeyboardKeyState[vkCode].WasDown = wasDown;
	m_keyboardInputMap.KeyboardKeyState[vkCode].IsDown = isDown;
	m_keyboardInputMap.KeyboardKeyState[vkCode].RepeatCount = repeatCount;
	

}

void BINDU::Win32Input::ProcessMouseInput(WPARAM wParam)
{
	m_mouseInputMap.MouseButtons[MouseButton::LEFT].WasDown = m_mouseInputMap.MouseButtons[MouseButton::LEFT].IsDown;
	m_mouseInputMap.MouseButtons[MouseButton::RIGHT].WasDown = m_mouseInputMap.MouseButtons[MouseButton::RIGHT].IsDown;
	m_mouseInputMap.MouseButtons[MouseButton::MIDDLE].WasDown = m_mouseInputMap.MouseButtons[MouseButton::MIDDLE].IsDown;

	m_mouseInputMap.MouseButtons[MouseButton::LEFT].IsDown = wParam & MK_LBUTTON;
	m_mouseInputMap.MouseButtons[MouseButton::RIGHT].IsDown = wParam & MK_RBUTTON;
	m_mouseInputMap.MouseButtons[MouseButton::MIDDLE].IsDown = wParam & MK_MBUTTON;

}

void BINDU::Win32Input::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		BINDU::Win32Input::ProcessMouseInput(wParam);
		break;
	case WM_MOUSEMOVE:
		BINDU::Win32Input::ProcessMouseInput(wParam);
		BINDU::Win32Input::UpdateMousePos(lParam);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		BINDU::Win32Input::ProcessKeyboardInput(wParam, lParam);
		break;
	}

}
