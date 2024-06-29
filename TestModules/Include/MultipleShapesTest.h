#pragma once
#include <Bindu_App.h>
#include <Win32Window.h>
#include <Timer.h>
#include <Bindu_Graphics.h>
#include <MathHelper.h>
#include <wrl.h>
#include <DirectXPackedVector.h>

using namespace Microsoft::WRL;
using namespace DirectX;

constexpr int gNumFrameResources = 3;

struct PerPassConstants
{
	XMFLOAT4X4	ViewMatrix = MathHelper::Identity4X4();
	XMFLOAT4X4	InvViewMatrix = MathHelper::Identity4X4();
	XMFLOAT4X4	ProjMatrix = MathHelper::Identity4X4();
	XMFLOAT4X4	InvProjMatrix = MathHelper::Identity4X4();
	XMFLOAT4X4	ViewProjMatrix = MathHelper::Identity4X4();
	XMFLOAT4X4	InvViewProjMatrix = MathHelper::Identity4X4();
	XMFLOAT3	EyePosW = { 0.0f,0.0f,0.0f };
	float		Pad1 = 0.0f;
	
	XMFLOAT2	RenderTargetSize = { 0.0f,0.0f };
	XMFLOAT2	InvRenderTargetSize = { 0.0f,0.0f };
	float		NearZ = 0.0f;
	float		FarZ = 0.0f;
	float		TotalTime = 0.0f;
	float		DeltaTime = 0.0f;
};

struct PerObjectConstants
{
	XMFLOAT4X4	WorldMatrix = MathHelper::Identity4X4();
};

struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
};

struct FrameResource
{
public:
	FrameResource(ID3D12Device* pD3DDevice, UINT passCount, UINT objectCount)
	{
		pD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(CommandAllocator.ReleaseAndGetAddressOf()));

		PassCB = std::make_unique<UploadBuffer<PerPassConstants>>(pD3DDevice, passCount, true);

		ObjectCB = std::make_unique<UploadBuffer<PerObjectConstants>>(pD3DDevice, objectCount, true);
	}

	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator = (const FrameResource& rhs) = delete;
	~FrameResource() = default;
	
	ComPtr<ID3D12CommandAllocator>	CommandAllocator{ nullptr };
	std::unique_ptr < UploadBuffer<PerPassConstants>>	PassCB{ nullptr };
	std::unique_ptr<UploadBuffer<PerObjectConstants>>	ObjectCB{ nullptr };

	// Fence value to mark command up to this fence point
	UINT64 Fence{ 0 };
};

	// Objects that can be rendered
struct RenderItem
{
public:
	RenderItem() = default;
	~RenderItem() = default;

	// Every render item need a world matrix
	XMFLOAT4X4	World;

	int NumFramesDirty{ gNumFrameResources };

	// Index into the GPU constant buffer for this render item
	UINT Index{ -1 };

	// Geometry associated with this render item
	MeshGeometry* Geometry{ nullptr };

	// Primitive topology
	D3D12_PRIMITIVE_TOPOLOGY	PrimitiveType{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };

	// DrawIndexedInstance parameters
	UINT IndexCount{ 0 };
	UINT StartIndexLocation{ 0 };
	int	 BaseVertexLocation{ 0 };
};

class MultiShape: public BINDU::BinduApp, public BINDU::Win32Window
{
public:
	MultiShape(HINSTANCE hInstance);
	MultiShape(HINSTANCE hInstance, BINDU::BINDU_WINDOW_DESC desc);
	~MultiShape();

	bool	OnInit() override;
	void	Run()	override;
	void	Update() override;
	void	Render() override;

private:

	void	BuildFrameResources();
	void	BuildDescriptorHeaps();
	void	BuildConstantBufferViews();
	void	BuildRootSignature();
	void	BuildShadersAndInputLayout();
	void	BuildShapeGeometry();
	void	BuildRenderItems();

	void	DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritem);

	void	UpdatePerObjectCB();
	void	UpdatePerPassCB();

private:
	GameTimer	m_timer;
	std::unique_ptr<BINDU::Graphics>	m_graphics{ nullptr };

	//static const int	m_numFrameResource{ 3 };
	std::vector<std::unique_ptr<FrameResource>>	m_frameResources;
	FrameResource* m_pCurrFrameResource{ nullptr };
	int	m_currFrameResourceIndex{ 0 };

	// List of all render items
	std::vector<std::unique_ptr<RenderItem>>	m_allRItem;

	// Render items divided by PSO
	std::vector<RenderItem*>	m_opaqueRItem;
	std::vector<RenderItem*>	m_transparentRItem;


	XMFLOAT4X4 m_viewMatrix{ MathHelper::Identity4X4() };
	XMFLOAT4X4 m_projMatrix{ MathHelper::Identity4X4() };
	//XMFLOAT4X4 m_viewProjMatrix{ MathHelper::Identity4X4() };
	XMFLOAT3   m_eyePosW{ 0.0f,0.0f,0.0f };

	ComPtr<ID3D12RootSignature>		m_rootSig{ nullptr };

	std::unordered_map<std::string, ComPtr<ID3DBlob>>	m_shaders;

	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_geometries;

	std::vector<D3D12_INPUT_ELEMENT_DESC>	m_inputLayout;

	ComPtr<ID3D12DescriptorHeap>	m_cbvHeap{ nullptr };
	UINT	m_perPassCBVOffset{ 0 };

	bool m_isWireframe{ false };

};
