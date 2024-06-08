#pragma once
#include <Win32Application.h>
#include <Timer.h>
#include <Bindu_Window.h>
#include <Bindu_Graphics.h>

class DemoClass : public BINDU::BinduApp
{
public:
	DemoClass(BINDU::Window* window);
	~DemoClass() override;

	bool OnInit() override;
	void Run() override;
	void Update() override;
	void Render() override;

private:

	void CreateDescriptorHeaps();

	inline BINDU::Window* GetWindow() { return m_window; }

private:
	GameTimer m_timer;
	std::unique_ptr<BINDU::Graphics> m_graphics{ nullptr };
	BINDU::Window* m_window{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_cbHeap{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource>	m_constantBuffer{ nullptr };
	
};