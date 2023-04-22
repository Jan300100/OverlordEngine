#include "stdafx.h"
#include "Texture2DDX11.h"
#include "GA/DX11/InterfaceDX11.h"
#include "GA/Helpers.h"

GA::DX11::Texture2DDX11::Texture2DDX11(GA::DX11::InterfaceDX11* pGAInterface, const GA::Texture2D::Params& params)
    :GA::Texture2D(pGAInterface, params)
{
    D3D11_TEXTURE2D_DESC desc;

    desc.Width = m_Params.width;
    desc.Height = m_Params.height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = GA::HELP::GAFormatToDXGIFormat(m_Params.format);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    switch (m_Params.cpuUpdateFreq)
    {
    case Resource::CPUUpdateFrequency::Never:
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;
        break;

    case Resource::CPUUpdateFrequency::Frequent:
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        break;

    case Resource::CPUUpdateFrequency::Possible:
    default:
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;
        break;
    }

    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = 0;

    std::vector<D3D11_SUBRESOURCE_DATA> initData;
    if (m_Params.subresourceData != nullptr)
    {
        desc.MipLevels = static_cast<uint32_t>(m_Params.numSubresources);
        for (size_t i = 0; i < m_Params.numSubresources; i++)
        {
            initData.emplace_back();
            initData[i].pSysMem = m_Params.subresourceData[i];
            initData[i].SysMemPitch = static_cast<UINT>((m_Params.width >> i) * GA::HELP::GetFormatSize(m_Params.format)); // just 1 row
            initData[i].SysMemSlicePitch = 0;
        }
    }

    HRESULT hr = pGAInterface->GetDevice()->CreateTexture2D(&desc, initData.data(), &m_pInternalTexture);
    Logger::LogHResult(hr, L"Texture2DDX11::Texture2DDX11() CreateTexture2D Failed!");

    // create srv
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = GA::HELP::GAFormatToDXGIFormat(m_Params.format);
    srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    hr = pGAInterface->GetDevice()->CreateShaderResourceView(m_pInternalTexture, &srvDesc, &m_pSRV);
    Logger::LogHResult(hr, L"Texture2DDX11::Texture2DDX11() CreateShaderResourceView Failed!");
}

GA::DX11::Texture2DDX11::~Texture2DDX11()
{
    m_pSRV->Release();
    m_pInternalTexture->Release();
}

ID3D11ShaderResourceView* GA::DX11::Texture2DDX11::GetSRV() const
{
    return m_pSRV;
}

std::any GA::DX11::Texture2DDX11::GetInternal()
{
    return m_pInternalTexture;
}

void* GA::DX11::Texture2DDX11::Map()
{
    Logger::LogError(L"Not implemented");
    return nullptr;
}

void GA::DX11::Texture2DDX11::Unmap()
{
    Logger::LogError(L"Not implemented");
}

const GA::DX11::Texture2DDX11* GA::DX11::QuickCast(const GA::Texture2D* i)
{
    return static_cast<const GA::DX11::Texture2DDX11*>(i);
}

const GA::DX11::Texture2DDX11* GA::DX11::SafeCast(const GA::Texture2D* i)
{
    return dynamic_cast<const GA::DX11::Texture2DDX11*>(i);
}
