#include "../Include/Bindu_App.h"
#include <cassert>
#include <Windows.h>


BINDU::BinduApp::BinduApp()
{
	
}

BINDU::BinduApp::~BinduApp()
{

}

bool BINDU::BinduApp::OnInit()
{

	return true;
}

void BINDU::BinduApp::Run()
{
	Update();
	Render();
}

bool BINDU::BinduApp::OnDestroy()
{
	return false;
}


