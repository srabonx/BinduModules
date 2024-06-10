#pragma once
#include <wrl.h>
#include "d3dx12.h"
#include <Bindu_Util.h>
#include "../Include/D3D12Util.h"

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* pD3dDevice, UINT elementCount, bool isConstantBuffer = false)
	{
		m_elementByteSize = sizeof(T);

		if (isConstantBuffer)
			m_elementByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(T));

		pD3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadBuffer));

		DX::ThrowIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator = (const UploadBuffer& rhs) = delete;
	~UploadBuffer()
	{
		if (m_uploadBuffer)
			m_uploadBuffer->Unmap(0, nullptr);

		m_mappedData = nullptr;
	}

	inline ID3D12Resource* Resource() const
	{
		return m_uploadBuffer.Get();
	}

	inline void CopyData(int elementIndex, const T& data)
	{
		memcpy(&m_mappedData[elementIndex * m_elementByteSize], &data, sizeof(T));
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource>	m_uploadBuffer{ nullptr };
	BYTE* m_mappedData{ nullptr };

	UINT m_elementByteSize{ 0 };
	bool m_isConstantBuffer{ false };
};
