#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include "d3dx12.h"
#include <DirectXCollision.h>
#include <wrl.h>
#include <unordered_map>
#include <fstream>

using namespace Microsoft::WRL;

class D3DUtil
{

public:
	

	static inline UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		// Constant buffer must be a multiple of minimum hardware allocation size (usually 256 bytes)
		return (byteSize + 255) & ~255;
	}

	static inline ComPtr<ID3DBlob> CompileShader(const std::string& filename, const D3D_SHADER_MACRO* pDefines, const std::string& entryPoint, const std::string& target)
	{
		// Use debug flag in debug mode
		UINT compileFlag = 0;
#if defined(_DEBUG) || defined (DEBUG)
		compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		HRESULT hr = S_OK;

		std::wstring shaderFileName = StringToWstring(filename);
		ComPtr<ID3DBlob> byteCode{ nullptr };
		ComPtr<ID3DBlob> errorBlob{ nullptr };


		hr = D3DCompileFromFile(shaderFileName.c_str(), pDefines,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			target.c_str(),
			compileFlag,
			0,
			byteCode.ReleaseAndGetAddressOf(),
			errorBlob.ReleaseAndGetAddressOf());

		if (errorBlob)
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		DX::ThrowIfFailed(hr);

		return byteCode;
	}

	static inline ComPtr<ID3DBlob> ReadBinary(const std::string& filename)
	{
		std::ifstream fin(filename, std::ios::binary);

		fin.seekg(0, std::ios_base::end);
		std::streampos size = static_cast<int>(fin.tellg());
		fin.seekg(0, std::ios_base::beg);

		ComPtr<ID3DBlob> blob{ nullptr };

		DX::ThrowIfFailed(D3DCreateBlob(size, blob.ReleaseAndGetAddressOf()));

		fin.read(reinterpret_cast<char*>(blob->GetBufferPointer()), size);
		fin.close();

		return blob;
	}

	static inline ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList,
		const void* initData, UINT64 byteSize, ComPtr<ID3D12Resource>& uploadBuffer)
	{
		// Create the actual default buffer
		ComPtr<ID3D12Resource> defaultBuffer{ nullptr };

		pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.ReleaseAndGetAddressOf()));

		// In order to copy cpu memory data into our default buffer, we need a intermediate upload buffer

		pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf()));


		// Describe the data we want to copy
		D3D12_SUBRESOURCE_DATA srData = {};
		srData.pData = initData;
		srData.RowPitch = byteSize;
		srData.SlicePitch = srData.RowPitch;

		// Schedule to copy data to default buffer

		pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		
		UpdateSubresources<1>(pCmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &srData);
		
		pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

		// NOTE: UploadBuffer has to be kept alive because command list haven't been executed yet

		return defaultBuffer;
	}

};


struct SubmeshGeometry
{
	UINT IndexCount{ 0 };
	UINT StartIndexLocation{ 0 };
	INT BaseVertexLocation{ 0 };


	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	std::string Name;

	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU{ nullptr };

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader{ nullptr };

	// Data about the buffers
	UINT VertexByteStride{ 0 };
	UINT VertexBufferByteSize{ 0 };
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize{ 0 };


	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	inline D3D12_VERTEX_BUFFER_VIEW	GetVertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	inline void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};