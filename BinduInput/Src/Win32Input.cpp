#include "../include/Win32Input.h"
#include <windowsx.h>



void BINDU::Win32Input::ProcessMouseUp(WPARAM wParam, LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	if (wParam & MK_LBUTTON)
		OnMouseUp(LEFT, x, y);
	else if (wParam & MK_RBUTTON)
		OnMouseUp(RIGHT, x, y);
	else if (wParam & MK_MBUTTON)
		OnMouseUp(MIDDLE, x, y);
	else 
		OnMouseUp(NONE, x, y);
}

void BINDU::Win32Input::ProcessMouseMove(WPARAM wParam, LPARAM lParam)
{
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	if (wParam & MK_LBUTTON)
		OnMouseMove(LEFT, x, y);
	else if (wParam & MK_RBUTTON)
		OnMouseMove(RIGHT, x, y);
	else if (wParam & MK_MBUTTON)
		OnMouseMove(MIDDLE, x, y);
	else
		OnMouseMove(NONE, x, y);
}

void BINDU::Win32Input::ProcessMouseDown(WPARAM wParam, LPARAM lParam)
{

	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	if (wParam & MK_LBUTTON)
		OnMouseDown(LEFT, x, y);
	else if (wParam & MK_RBUTTON)
		OnMouseDown(RIGHT, x, y);
	else if (wParam & MK_MBUTTON)
		OnMouseDown(MIDDLE, x, y);
	else 
		OnMouseDown(NONE, x, y);
}


void BINDU::Win32Input::ProcessKeyboardDown(WPARAM wParam, LPARAM lParam)
{
	KeyBoardKey key = (KeyBoardKey)LOWORD(wParam);
	WORD keyFlag = HIWORD(lParam);

	bool repeat = (keyFlag & KF_REPEAT) == KF_REPEAT;
	WORD repeatCount = LOWORD(lParam);
	bool isDown = (keyFlag & KF_UP) != KF_UP;

	OnKeyboardDown(key, isDown, repeat);
}

void BINDU::Win32Input::ProcessKeyboardUp(WPARAM wParam, LPARAM lParam)
{
	KeyBoardKey key = (KeyBoardKey)LOWORD(wParam);
	WORD keyFlag = HIWORD(lParam);

	bool repeat = (keyFlag & KF_REPEAT) != KF_REPEAT;
	bool isUp = (keyFlag & KF_UP) == KF_UP;

	OnKeyboardUp(key, isUp, repeat);
}


void BINDU::Win32Input::InputMsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		this->ProcessMouseDown(wParam, lParam);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		this->ProcessMouseUp(wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		
		this->ProcessMouseMove(wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		this->ProcessKeyboardDown(wParam, lParam);
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		this->ProcessKeyboardUp(wParam, lParam);
		break;
	}

}
