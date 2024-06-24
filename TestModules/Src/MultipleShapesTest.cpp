#include "../Include/MultipleShapesTest.h"

MultiShape::MultiShape(HINSTANCE hInstance) : BINDU::Win32Window(hInstance)
{

}

MultiShape::MultiShape(HINSTANCE hInstance, BINDU::BINDU_WINDOW_DESC desc) : BINDU::Win32Window(hInstance, desc)
{

}

MultiShape::~MultiShape()
{
}

bool MultiShape::OnInit()
{
	DXGI_MODE_DESC dxgiModeDesc;
	dxgiModeDesc.Width = m_windowWidth;
	dxgiModeDesc.Height = m_windowHeight;
	dxgiModeDesc.RefreshRate.Numerator = 60;
	dxgiModeDesc.RefreshRate.Denominator = 1;
	dxgiModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	m_graphics = std::make_unique<BINDU::Graphics>(&m_windowHandle, dxgiModeDesc);

	m_graphics->InitDirect3D();
	
	m_timer.Reset();
	return true;
}

void MultiShape::Run()
{
	m_timer.Tick();

	static int fps{ 0 };
	static float mspf{ 0.f };
	
	if (this->CalculateFrameStats(fps, mspf, static_cast<float>(m_timer.TotalTime())))
	{

		std::wstring title = L"FPS: " + std::to_wstring(fps) + L"\tMSPF: " + std::to_wstring(mspf) + L"\tTOTAL TIME: " + std::to_wstring(m_timer.TotalTime());

		this->SetWindowTitle(title);
	}

	this->Update();
	this->Render();
}

void MultiShape::Update()
{
}

void MultiShape::Render()
{
}

void MultiShape::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
		m_frameResources.push_back(std::make_unique<FrameResource>(m_graphics->GetDevice(),
			1, 10));
}

void MultiShape::UpdatePerObjectCB()
{
	auto currObjectCB = m_pCurrFrameResource->ObjectCB.get();

	for (auto& e : m_allRItem)
	{
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX worldMatrix = XMLoadFloat4x4(&e->World);

			PerObjectConstants	perObjConstants;
			XMStoreFloat4x4(&perObjConstants.WorldMatrix, XMMatrixTranspose(worldMatrix));

			currObjectCB->CopyData(e->Index, perObjConstants);

			e->NumFramesDirty--;
		}
	}
}

void MultiShape::UpdatePerPassCB()
{
	auto currPassCB = m_pCurrFrameResource->PassCB.get();

	PerPassConstants	passConstant;

	XMMATRIX viewMat = XMLoadFloat4x4(&m_viewMatrix);
	XMMATRIX projMat = XMLoadFloat4x4(&m_projMatrix);

	XMMATRIX viewProjMat = XMMatrixMultiply(viewMat, projMat);

	XMMATRIX invViewMat = XMMatrixInverse(&XMMatrixDeterminant(viewMat), viewMat);
	XMMATRIX invProjMat = XMMatrixInverse(&XMMatrixDeterminant(projMat), projMat);
	XMMATRIX invViewProjMat = XMMatrixInverse(&XMMatrixDeterminant(viewProjMat), viewProjMat);

	XMStoreFloat4x4(&passConstant.ViewMatrix, XMMatrixTranspose(viewMat));
	XMStoreFloat4x4(&passConstant.ProjMatrix, XMMatrixTranspose(projMat));
	XMStoreFloat4x4(&passConstant.ViewProjMatrix, XMMatrixTranspose(viewProjMat));
	XMStoreFloat4x4(&passConstant.InvViewMatrix, XMMatrixTranspose(invViewMat));
	XMStoreFloat4x4(&passConstant.InvProjMatrix, XMMatrixTranspose(invProjMat));
	XMStoreFloat4x4(&passConstant.InvViewProjMatrix, XMMatrixTranspose(invViewProjMat));

	passConstant.EyePosW = m_eyePosW;
	passConstant.RenderTargetSize = { static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight) };
	passConstant.InvRenderTargetSize = { 1.0f / passConstant.RenderTargetSize.x, 1.0f / passConstant.RenderTargetSize.y };

	passConstant.NearZ = 1.0f;
	passConstant.FarZ = 1000.0f;

	passConstant.TotalTime = m_timer.TotalTime();
	passConstant.DeltaTime = m_timer.DeltaTime();

	currPassCB->CopyData(0, passConstant);
}