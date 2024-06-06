#pragma once
#include <Win32Application.h>
#include <Timer.h>
#include <Bindu_Window.h>

class DemoClass : public BINDU::BinduApp
{
public:
	DemoClass(BINDU::Window* window);
	~DemoClass() override;

	void Run() override;
	void Update() override;
	void Render() override;

	//inline BINDU::Window* GetWindow() { return &m_window; }

private:
	GameTimer m_timer;
	BINDU::Window* m_window{ nullptr };
	/*int	fps{ 0 };
	float mspf{ 0.0f };*/
};