#pragma once
#include "Bindu_App.h"
#include "Win32Window.h"

namespace BINDU
{
	class Win32Application
	{
	public:
		static int Run(BinduApp* pApp, BINDU::Win32Window* pWindow, int nCmdShow);

	};
}
