#include "stdafx.h"
#include "DX11Resource.h"

ID3D11Buffer* DX11Resource::GetBuffer()
{
	return m_pBuffer;
}

void DX11Resource::Create(const Params& createParams)
{
	//*************
	//VERTEX BUFFER
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.BindFlags = D3D10_BIND_FLAG::D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = createParams.stride * createParams.width * createParams.height * createParams.depth;
	bufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_FLAG::D3D10_CPU_ACCESS_WRITE;
	bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	bufferDesc.MiscFlags = 0;
	m_pRenderer->GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_pBuffer);
}

void DX11Resource::Destroy()
{
	SafeRelease(m_pBuffer);
}
