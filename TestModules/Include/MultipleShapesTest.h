#pragma once
#include <Bindu_App.h>
#include <Win32Window.h>
#include <Timer.h>
#include <Bindu_Graphics.h>
#include <wrl.h>
#include <DirectXPackedVector.h>

using namespace Microsoft::WRL;
using namespace DirectX;

constexpr int gNumFrameResources = 3;

struct PerPassConstants
{

};

struct ObjectConstants
{

};

struct FrameResource
{
public:
	FrameResource(ID3D12Device* pD3DDevice, UINT passCount, UINT objectCount)
	{
		pD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(CommandAllocator.ReleaseAndGetAddressOf()));

		PerPassCB = std::make_unique<UploadBuffer<PerPassConstants>>(pD3DDevice, passCount, true);

		ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pD3DDevice, objectCount, true);
	}

	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator = (const FrameResource& rhs) = delete;
	~FrameResource() = default;
	
	ComPtr<ID3D12CommandAllocator>	CommandAllocator{ nullptr };
	std::unique_ptr < UploadBuffer<PerPassConstants>>	PerPassCB{ nullptr };
	std::unique_ptr<UploadBuffer<ObjectConstants>>	ObjectCB{ nullptr };

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

private:
	GameTimer	m_timer;
	std::unique_ptr<BINDU::Graphics>	m_graphics{ nullptr };

	//static const int	m_numFrameResource{ 3 };
	std::vector<std::unique_ptr<FrameResource>>	m_frameResources;
	FrameResource* m_pCurrFrameResource{ nullptr };
	int	m_currFrameResourceIndex{ 0 };
};
