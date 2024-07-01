#pragma once
#include <Win32Application.h>
#include <Timer.h>
#include <Win32Window.h>
#include <Bindu_Graphics.h>
#include <DirectXMath.h>
#include <Win32Input.h>

using namespace DirectX;


class DemoClass : public BINDU::BinduApp, public BINDU::Win32Window, public BINDU::Win32Input
{
public:

	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT4 color;
	};

	struct Vertex2
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Tanjent;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex0;
		XMFLOAT3 Tex1;
		XMFLOAT4 Color;
	};

	struct VPosData
	{
		XMFLOAT3 Pos;
	};

	struct VColorData
	{
		XMFLOAT4 Color;
	};


	struct ObjectConstants
	{
		DirectX::XMFLOAT4X4 WorldviewMatrix;
		XMFLOAT4	PulseColor;
		double	GTime;
	};


public:
	DemoClass(HINSTANCE hInstance);
	DemoClass(HINSTANCE hInstance, BINDU::BINDU_WINDOW_DESC windowDesc);
	~DemoClass() override;


	bool OnInit() override;
	void Run() override;
	void Update() override;
	void Render() override;
	

private:

	void	OnMouseDown(BINDU::MouseButton btn, int x, int y) override;
	void	OnMouseUp(BINDU::MouseButton btn, int x, int y) override;
	void	OnMouseMove(BINDU::MouseButton btn, int x, int y) override;

	void	OnKeyboardDown(BINDU::KeyBoardKey key, bool isDown, bool repeat) override;
	void	OnKeyboardUp(BINDU::KeyBoardKey key, bool isUp, bool repeat) override;


	void CreateDescriptorHeaps();

	void CreateConstantBuffers();

	void CreateRootSignature();

	void CreateInputLayoutAndShaders();

	void CreateBoxGeometry();

	void CreatePSO();

	void OnResize(int width, int height);

	LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wPARAM, LPARAM lPARAM) override;


private:
	GameTimer m_timer;
	std::unique_ptr<BINDU::Graphics> m_graphics{ nullptr };
	

	Microsoft::WRL::ComPtr<ID3D12PipelineState>		m_pso{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_cbHeap{ nullptr };
	
	std::unique_ptr<UploadBuffer<ObjectConstants>>  m_objectCB{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12RootSignature>		m_rootSig{ nullptr };

	Microsoft::WRL::ComPtr<ID3DBlob>				m_vsByteCode{ nullptr };
	Microsoft::WRL::ComPtr<ID3DBlob>				m_psByteCode{ nullptr };

	std::vector<D3D12_INPUT_ELEMENT_DESC>			m_inputLayout;

	std::vector<D3D12_INPUT_ELEMENT_DESC>			m_inputLayout2;

	std::vector<D3D12_INPUT_ELEMENT_DESC>			m_inputLayout3;

	Microsoft::WRL::ComPtr<ID3D12Resource>			m_vPosBuffer{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_vColorBuffer{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12Resource>			m_vPosBufferUploader{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_vColorBufferUploader{ nullptr };

	D3D12_VERTEX_BUFFER_VIEW						m_vBufferView[2];

	std::unique_ptr<MeshGeometry>					m_boxGeo{ nullptr };
	std::unique_ptr<MeshGeometry>					m_pyramidGeo{ nullptr };

	std::unique_ptr<MeshGeometry>					m_geometry{ nullptr };

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

	POINT m_lastMousePos;

	ObjectConstants	m_objectConstants;

};