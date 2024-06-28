#include "../Include/MultipleShapesTest.h"
#include <GeometryGenerator.h>
#include <DirectXColors.h>

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
			1, (UINT)m_allRItem.size()));
}

void MultiShape::BuildDescriptorHeaps()
{
	UINT objcount = static_cast<UINT>(m_opaqueRItem.size());

	UINT numOfDescriptor = (objcount + 1) * gNumFrameResources;

	m_perPassCBVOffset = objcount * gNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.NumDescriptors = numOfDescriptor;
	heapDesc.NodeMask = 0;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DX::ThrowIfFailed(m_graphics->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));
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

void MultiShape::BuildRootSignature()
{

	ComPtr<ID3DBlob>	serializedRootSig{ nullptr };
	ComPtr<ID3DBlob>	errorBlob{ nullptr };

	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	// Create root CBVs
	CD3DX12_ROOT_PARAMETER rootParameter[2];
	rootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	rootParameter[1].InitAsDescriptorTable(1, &cbvTable1);



	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, rootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	}

	DX::ThrowIfFailed(hr);

	DX::ThrowIfFailed(m_graphics->GetDevice()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(), IID_PPV_ARGS(m_rootSig.ReleaseAndGetAddressOf())));
}

void MultiShape::BuildConstantBufferViews()
{
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PerObjectConstants));

	UINT objCount = static_cast<UINT>(m_opaqueRItem.size());

	// we need a constant buffer view for each object in each frame resource
	for (int frameIndex = 0; frameIndex < gNumFrameResources; frameIndex++)
	{
		auto objectCB = m_frameResources[frameIndex]->ObjectCB->Resource();

		for (UINT i = 0; i < objCount; i++)
		{
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

			// offset to the ith object constant buffer in the buffer
			cbAddress += i * objCBByteSize;

			// offset to the object cbv in the descriptor heap
			int heapIndex = frameIndex * objCount + i;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, m_graphics->GetCbvSrvUavDescriptorIncreamentSize());

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			m_graphics->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	UINT passCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PerPassConstants));

	// last 3 descriptor are the pass cbv for each frame resource

	for (int frameIndex = 0; frameIndex < gNumFrameResources; frameIndex++)
	{
		auto passCB = m_frameResources[frameIndex]->PassCB->Resource();

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

		// offset to pass cbv in the descriptor heap
		int heapIndex = m_perPassCBVOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, m_graphics->GetCbvSrvUavDescriptorIncreamentSize());

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		m_graphics->GetDevice()->CreateConstantBufferView(&cbvDesc, handle);
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

void MultiShape::BuildShadersAndInputLayout()
{
	m_shaders["standardVS"] = D3DUtil::CompileShader(RelativeResourcePath("Shaders\\color.hlsl"), 
		nullptr, "VS", "vs_5_1");
	m_shaders["opaquePS"] = D3DUtil::CompileShader(RelativeResourcePath("Shaders\\color.hlsl"),
		nullptr, "PS", "ps_5_1");

	m_inputLayout =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0 , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
}

void MultiShape::BuildShapeGeometry()
{
	GeometryGenerator geoGen;

	GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.f, 30.f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	// we will concat all the vertices and indices in a big buffer. so will define regions in the buffer each submesh 
	// will cover. Cache the vertex offset of each object in the concatinated vertex/index buffer

	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = static_cast<UINT>(box.Vertices.size());
	UINT sphereVertexOffset = static_cast<UINT>(gridVertexOffset + grid.Vertices.size());
	UINT cylinderVertexOffset = static_cast<UINT>(sphereVertexOffset + sphere.Vertices.size());

	// Cache the starting index for each object in the concatinated index buffer

	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	// Define submesh that covers different region of the vertex index buffer
	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	// Pack the vertices of all elements into a big vertex buffer

	auto totalVertexCount = box.Vertices.size() + grid.Vertices.size() + sphere.Vertices.size() + cylinder.Vertices.size();

	std::vector<Vertex>	vertices(totalVertexCount);

	UINT k{ 0 };
	for (size_t i = 0; i < box.Vertices.size(); i++, k++)
	{
		vertices[k].Position = box.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::DarkGreen);
	}

	for (size_t i = 0; i < grid.Vertices.size(); i++, k++)
	{
		vertices[k].Position = grid.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::ForestGreen);
	}

	for (size_t i = 0; i < sphere.Vertices.size(); i++, k++)
	{
		vertices[k].Position = sphere.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::Crimson);
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); i++, k++)
	{
		vertices[k].Position = cylinder.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::SteelBlue);
	}

	// concatinated index buffer
	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));


	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	DX::ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	DX::ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_graphics->GetDevice(),
		m_graphics->GetCommandList(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_graphics->GetDevice(),
		m_graphics->GetCommandList(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;

	m_geometries[geo->Name] = std::move(geo);
}

void MultiShape::BuildRenderItems()
{
	auto boxRItem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRItem->World,
		XMMatrixScaling(2.f, 2.f, 2.f) * XMMatrixTranslation(0.f, 0.5f, 0.f));
	boxRItem->Index = 0;
	boxRItem->Geometry = m_geometries["shapeGeo"].get();
	boxRItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRItem->IndexCount = boxRItem->Geometry->DrawArgs["box"].IndexCount;
	boxRItem->StartIndexLocation = boxRItem->Geometry->DrawArgs["box"].StartIndexLocation;
	boxRItem->BaseVertexLocation = boxRItem->Geometry->DrawArgs["box"].BaseVertexLocation;
	m_allRItem.push_back(std::move(boxRItem));

	auto gridRItem = std::make_unique<RenderItem>();
	gridRItem->World = MathHelper::Identity4X4();
	gridRItem->Index = 1;
	gridRItem->Geometry = m_geometries["shapeGeo"].get();
	gridRItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRItem->IndexCount = gridRItem->Geometry->DrawArgs["grid"].IndexCount;
	gridRItem->StartIndexLocation = gridRItem->Geometry->DrawArgs["grid"].StartIndexLocation;
	gridRItem->BaseVertexLocation = gridRItem->Geometry->DrawArgs["grid"].BaseVertexLocation;
	m_allRItem.push_back(std::move(gridRItem));

	// cylinder and spheres
	UINT objCBindex = 2;
	for (size_t i = 0; i < 5; i++)
	{
		auto leftCylRItem = std::make_unique<RenderItem>();
		auto rightCylRItem = std::make_unique<RenderItem>();
		auto leftSphereRItem = std::make_unique<RenderItem>();
		auto rightSphereRItem = std::make_unique<RenderItem>();

		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.f, +1.5f, -10.f + i * 5.f);
		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.f, +1.5f, -10.f + i * 5.f);

		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.f, +3.5f, -10.f + i * 5.f);
		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.f, +1.5f, -10.f + i * 5.f);

		XMStoreFloat4x4(&leftCylRItem->World, leftCylWorld);
		leftCylRItem->Index = objCBindex++;
		leftCylRItem->Geometry = m_geometries["shapeGeo"].get();
		leftCylRItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRItem->StartIndexLocation = leftCylRItem->Geometry->DrawArgs["cylinder"].StartIndexLocation;
		leftCylRItem->IndexCount = leftCylRItem->Geometry->DrawArgs["cylinder"].IndexCount;
		leftCylRItem->BaseVertexLocation = leftCylRItem->Geometry->DrawArgs["cylinder"].BaseVertexLocation;
		
		XMStoreFloat4x4(&rightCylRItem->World, rightCylWorld);
		rightCylRItem->Index = objCBindex++;
		rightCylRItem->Geometry = m_geometries["shapeGeo"].get();
		rightCylRItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRItem->StartIndexLocation = rightCylRItem->Geometry->DrawArgs["cylinder"].StartIndexLocation;
		rightCylRItem->IndexCount = rightCylRItem->Geometry->DrawArgs["cylinder"].IndexCount;
		rightCylRItem->BaseVertexLocation = rightCylRItem->Geometry->DrawArgs["cylinder"].BaseVertexLocation;

		XMStoreFloat4x4(&leftSphereRItem->World, leftSphereWorld);
		leftSphereRItem->Index = objCBindex++;
		leftSphereRItem->Geometry = m_geometries["shapeGeo"].get();
		leftSphereRItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRItem->StartIndexLocation = leftSphereRItem->Geometry->DrawArgs["sphere"].StartIndexLocation;
		leftSphereRItem->IndexCount = leftSphereRItem->Geometry->DrawArgs["sphere"].IndexCount;
		leftSphereRItem->BaseVertexLocation = leftSphereRItem->Geometry->DrawArgs["sphere"].BaseVertexLocation;

		XMStoreFloat4x4(&rightSphereRItem->World, rightSphereWorld);
		rightSphereRItem->Index = objCBindex++;
		rightSphereRItem->Geometry = m_geometries["shapeGeo"].get();
		rightSphereRItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRItem->StartIndexLocation = rightSphereRItem->Geometry->DrawArgs["sphere"].StartIndexLocation;
		rightSphereRItem->IndexCount = rightSphereRItem->Geometry->DrawArgs["sphere"].IndexCount;
		rightSphereRItem->BaseVertexLocation = rightSphereRItem->Geometry->DrawArgs["sphere"].BaseVertexLocation;


		m_allRItem.push_back(std::move(leftCylRItem));
		m_allRItem.push_back(std::move(rightCylRItem));
		m_allRItem.push_back(std::move(leftSphereRItem));
		m_allRItem.push_back(std::move(rightSphereRItem));
	}

	// All render items are opaque
	for (auto& e : m_allRItem)
		m_opaqueRItem.push_back(e.get());
}

void MultiShape::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritem)
{
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PerObjectConstants));
	auto objectCB = m_pCurrFrameResource->ObjectCB->Resource();

	// For each RenderItem
	for (size_t i = 0; i < ritem.size(); i++)
	{
		auto ri = ritem[i];
		
		cmdList->IASetVertexBuffers(0, 1, &ri->Geometry->GetVertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geometry->GetIndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		// offset the cbv in the descriptor heap for the object
		UINT cbvIndex = m_currFrameResourceIndex * (UINT)m_opaqueRItem.size() + ri->Index;

		auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
		cbvHandle.Offset(cbvIndex, m_graphics->GetCbvSrvUavDescriptorIncreamentSize());

		cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}
