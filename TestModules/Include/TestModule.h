#pragma once
#include <Win32Application.h>
#include <Timer.h>
#include <Bindu_Window.h>
#include <Bindu_Graphics.h>
#include <DirectXMath.h>

using namespace DirectX;
struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

struct ObjectConstants
{
	DirectX::XMFLOAT4X4 WorldviewMatrix;
};

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

	void CreateConstantBuffers();

	void CreateRootSignature();

	void CreateInputLayoutAndShaders();

	void CreateBoxGeometry();

	void CreatePSO();

	inline BINDU::Window* GetWindow() { return m_window; }

private:
	GameTimer m_timer;
	std::unique_ptr<BINDU::Graphics> m_graphics{ nullptr };
	BINDU::Window* m_window{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12PipelineState>		m_pso{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_cbHeap{ nullptr };
	
	std::unique_ptr<UploadBuffer<ObjectConstants>>  m_objectCB{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature>		m_rootSig{ nullptr };

	Microsoft::WRL::ComPtr<ID3DBlob>				m_vsByteCode{ nullptr };
	Microsoft::WRL::ComPtr<ID3DBlob>				m_psByteCode{ nullptr };

	std::vector<D3D12_INPUT_ELEMENT_DESC>			m_inputLayout;

	std::unique_ptr<MeshGeometry>					m_boxGeo{ nullptr };

	XMFLOAT4X4 m_world = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	XMFLOAT4X4 m_view = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	XMFLOAT4X4 m_proj = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	float m_theta = 1.5f * XM_PI;
	float m_phi = XM_PIDIV4;
	float m_radius = 5.0f;

};