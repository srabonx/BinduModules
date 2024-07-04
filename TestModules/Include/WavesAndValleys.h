#pragma once
#include <Bindu_Timer.h>

#include <Bindu_App.h>

#include <Win32Window.h>

#include <Bindu_Graphics.h>

class WavesAndValleys : public BINDU::BinduApp, public BINDU::Win32Window, public BINDU::DX12Graphics
{
public:
	// Constructor
	WavesAndValleys(HINSTANCE hInstance, const BINDU::BINDU_WINDOW_DESC& wndDesc, DXGI_MODE_DESC backBufferDisplayMode);

	// Destructor
	~WavesAndValleys() = default;

	// Initialization stuff goes in here
	bool OnInit() override;

	// Overriden base class method to calculate the frame stats/ called every frame
	void Run() override;
	
	// Closing stuff goes in here
	bool OnDestroy() override;

	// Window message processing is done here
	LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:

	// Update and Render gets called every frame in order
	void Update() override;
	void Render() override;



private:

	BINDU::Timer m_timer;
	
};
