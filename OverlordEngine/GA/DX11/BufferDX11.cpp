#include "stdafx.h"
#include "BufferDX11.h"
#include "GA/DX11/InterfaceDX11.h"

GA::DX11::BufferDX11::BufferDX11(GA::DX11::InterfaceDX11* i, const GA::Buffer::Params& params)
    : GA::Buffer(i, params)
{

	D3D11_BUFFER_DESC desc;

	switch (params.type)
	{
	case Buffer::Type::Vertex:
		desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		break;
	case Buffer::Type::Unknown:
		Logger::LogWarning(L"Buffer type is unknown");
		desc.BindFlags = NULL;
	default:
		break;
	}

	desc.ByteWidth = params.sizeInBytes;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;

	// D3D11_USAGE_DEFAULT : CPU : None, GPU : ReadWrite
	// D3D11_USAGE_IMMUTABLE : CPU : None, GPU : ReadOnly
	// D3D11_USAGE_DYNAMIC : CPU : WriteOnly, GPU : ReadOnly
	// D3D11_USAGE_STAGING : CPU : ReadWrite, GPU : None

	desc.MiscFlags = 0;

	HRESULT hr = i->GetDevice()->CreateBuffer(&desc, nullptr, &m_pInternalBuffer);
	Logger::LogHResult(hr, L"BufferDX11::BufferDX11() CreateBuffer Failed!");
}

GA::DX11::BufferDX11::~BufferDX11()
{
	m_pInternalBuffer->Release();
}

std::any GA::DX11::BufferDX11::GetInternal()
{
    return m_pInternalBuffer;
}

void* GA::DX11::BufferDX11::Map()
{
	// only possible if GPUMappable : D3D11_USAGE_DYNAMIC | D3D11_USAGE_STAGING
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	QuickCast(m_pInterface)->GetDeviceContext()->Map(m_pInternalBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
    return mappedResource.pData;
}

void GA::DX11::BufferDX11::Unmap()
{
	// only possible if GPUMappable : D3D11_USAGE_DYNAMIC | D3D11_USAGE_STAGING
	QuickCast(m_pInterface)->GetDeviceContext()->Unmap(m_pInternalBuffer, 0);
}
