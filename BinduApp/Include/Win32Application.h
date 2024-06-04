#pragma once
#include "Bindu_App.h"
#include "Bindu_Window.h"

namespace BINDU
{
	class Win32Application
	{
	public:
		static int Run(BinduApp* pApp, BINDU::Window* pWindow, int nCmdShow);

	};
}
