#include "stdafx.h"
#include "BufferDX11.h"
#include "GA/DX11/InterfaceDX11.h"

GA::DX11::BufferDX11::BufferDX11(GA::DX11::InterfaceDX11* pGAInterface, const GA::Buffer::Params& params)
    : GA::Buffer(pGAInterface, params)
{

	D3D11_BUFFER_DESC desc;

	switch (params.type)
	{
	case Type::Vertex:
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		break;
	case Type::Index:
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		break;
	case Type::Unknown:
		Logger::LogError(L"Buffer type is unknown");
		desc.BindFlags = NULL;
	default:
		break;
	}

	// D3D11_USAGE_DEFAULT : CPU : None, GPU : ReadWrite
	// D3D11_USAGE_IMMUTABLE : CPU : None, GPU : ReadOnly
	// D3D11_USAGE_DYNAMIC : CPU : WriteOnly, GPU : ReadOnly
	// D3D11_USAGE_STAGING : CPU : ReadWrite, GPU : None
	switch (params.cpuUpdateFreq)
	{
	case CPUUpdateFrequency::Never:
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
		break;
	
	case CPUUpdateFrequency::Frequent:
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		break;

	case CPUUpdateFrequency::Possible:
	default:
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
		break;
	}

	desc.ByteWidth = params.sizeInBytes;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA* initDataPtr = nullptr;
	D3D11_SUBRESOURCE_DATA initData;
	if (params.initialData)
	{
		initData.pSysMem = params.initialData;
		initDataPtr = &initData;
	}

	HRESULT hr = pGAInterface->GetDevice()->CreateBuffer(&desc, initDataPtr, &m_pInternalBuffer);
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
	// else create temp staging resource, remove the staging resource in unmap and queue a copy
	if (m_UpdateFreq == CPUUpdateFrequency::Possible)
	{
		Logger::LogError(L"Mapping has not been implemented for resources with type D3D11_USAGE_DEFAULT!");
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	QuickCast(m_pInterface)->GetDeviceContext()->Map(m_pInternalBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
    return mappedResource.pData;

}

void GA::DX11::BufferDX11::Unmap()
{
	// only possible if GPUMappable : D3D11_USAGE_DYNAMIC | D3D11_USAGE_STAGING
	QuickCast(m_pInterface)->GetDeviceContext()->Unmap(m_pInternalBuffer, 0);
}
