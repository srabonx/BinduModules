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

bool BINDU::BinduApp::CalculateFrameStats(int& fps, float& mspf, float totalTime)
{
	static int frameCount{ 0 };
	static float timeElapsed{ 0.0f };

	frameCount++;

	// Compute average over 1 second period
	if (totalTime - timeElapsed >= 1.0f)
	{
		fps = frameCount;
		mspf = 1000.0f / fps ;

		frameCount = 0;
		timeElapsed += 1.0f;
		
		return true;
	}

	return false;
}


