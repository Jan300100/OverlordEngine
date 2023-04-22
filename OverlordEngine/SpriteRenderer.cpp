#include "stdafx.h"
#include "SpriteRenderer.h"
#include "ContentManager.h"
#include "EffectHelper.h"
#include <algorithm>
#include "OverlordGame.h"

#include <GA/Buffer.h>

// todo: dx11
#include <GA/DX11/InterfaceDX11.h>
#include <GA/DX11/Texture2DDX11.h>

bool SpriteRenderer::SpriteSortByTexture(const SpriteVertex& v0, const SpriteVertex& v1)
{
	return v0.TextureId < v1.TextureId;
}

bool SpriteRenderer::SpriteSortByDepth(const SpriteVertex& v0, const SpriteVertex& v1)
{
	return v0.TransformData.z < v1.TransformData.z;
}

SpriteRenderer::SpriteRenderer() :
	m_Sprites(std::vector<SpriteVertex>()),
	m_Textures(std::vector<GA::Texture2D*>()),
	m_BufferSize(50),
	m_InputLayoutSize(0),
	m_pEffect(nullptr),
	m_pTechnique(nullptr),
	m_pInputLayout(nullptr),
	m_pVertexBuffer(nullptr),
	m_pImmediateVertexBuffer(nullptr),
	m_pTransfromMatrixV(nullptr),
	m_Transform(DirectX::XMFLOAT4X4()),
	m_pTextureSizeV(nullptr),
	m_pTextureSRV(nullptr)
{
}

SpriteRenderer::~SpriteRenderer()
{
	SafeRelease(m_pInputLayout);

	m_Sprites.clear();
	m_Textures.clear();
}

void SpriteRenderer::InitRenderer(ID3D11Device* pDevice)
{
	PIX_PROFILE();

	//Effect
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/SpriteRenderer.fx").get();

	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	m_pTransfromMatrixV = m_pEffect->GetVariableByName("gTransform")->AsMatrix();
	if (!m_pTransfromMatrixV->IsValid())
	{
		Logger::LogError(L"SpriteRenderer::Initialize() > Shader variable \'gTransform\' not valid!");
		return;
	}

	m_pTextureSizeV = m_pEffect->GetVariableByName("gTextureSize")->AsVector();
	if (!m_pTextureSizeV->IsValid())
	{
		Logger::LogError(L"SpriteRenderer::Initialize() > Shader variable \'gTextureSize\' not valid!");
		return;
	}

	m_pTextureSRV = m_pEffect->GetVariableByName("gSpriteTexture")->AsShaderResource();
	if (!m_pTextureSRV->IsValid())
	{
		Logger::LogError(L"SpriteRenderer::Initialize() > Shader variable \'gSpriteTexture\' not valid!");
		return;
	}

	EffectHelper::BuildInputLayout(pDevice, m_pTechnique, &m_pInputLayout, m_InputLayoutSize);

	//Transform Matrix
	const auto settings = OverlordGame::GetGameSettings();
	const float scaleX = (settings.Window.Width > 0) ? 2.0f / settings.Window.Width : 0;
	const float scaleY = (settings.Window.Height > 0) ? 2.0f / settings.Window.Height : 0;

	m_Transform._11 = scaleX;
	m_Transform._12 = 0;
	m_Transform._13 = 0;
	m_Transform._14 = 0;
	m_Transform._21 = 0;
	m_Transform._22 = -scaleY;
	m_Transform._23 = 0;
	m_Transform._24 = 0;
	m_Transform._31 = 0;
	m_Transform._32 = 0;
	m_Transform._33 = 1;
	m_Transform._34 = 0;
	m_Transform._41 = -1;
	m_Transform._42 = 1;
	m_Transform._43 = 0;
	m_Transform._44 = 1;
}

void SpriteRenderer::UpdateBuffer(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (m_pVertexBuffer)
	{
		if (m_Sprites.size() > m_pVertexBuffer->GetSizeInBytes() / sizeof(SpriteVertex))
		{
			// needs to be recreated
			m_pVertexBuffer = nullptr;
		}
	}

	if (!m_pVertexBuffer)
	{
		GA::Buffer::Params params;
		params.sizeInBytes = (uint32_t)m_Sprites.size() * sizeof(SpriteVertex);
		params.lifeTime = GA::Resource::LifeTime::Permanent;
		params.type = GA::Buffer::Type::Vertex;
		params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Frequent;

		m_pVertexBuffer = gameContext.pRenderer->CreateBuffer(params);
	}

	//------------------------
	//Sort Sprites
	//SORT BY TEXTURE
	sort(m_Sprites.begin(), m_Sprites.end(), [](const SpriteVertex& v0, const SpriteVertex& v1)
	{
		return v0.TextureId < v1.TextureId;
	});

	//SORT BY DEPTH
	sort(m_Sprites.begin(), m_Sprites.end(), SpriteSortByDepth);
	sort(m_Sprites.begin(), m_Sprites.end(), [](const SpriteVertex& v0, const SpriteVertex& v1)
	{
		if (v0.TextureId == v1.TextureId)
		{
			return v0.TransformData.z < v1.TransformData.z;
		}

		return false;
	});
	//------------------------

	if (m_pVertexBuffer != nullptr)
	{
		void* pData = m_pVertexBuffer->Map();
		// use memcpy to copy all our sprites to the mapped resource
		memcpy(pData, m_Sprites.data(), (uint32_t)m_Sprites.size() * sizeof(SpriteVertex));
		// unmap the vertex buffer
		m_pVertexBuffer->Unmap();
	}
}

void SpriteRenderer::Draw(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (m_Sprites.empty())
		return;

	UpdateBuffer(gameContext);

	//Set Render Pipeline
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	unsigned int stride = sizeof(SpriteVertex);
	unsigned int offset = 0;

	ID3D11Buffer* internalBuf = std::any_cast<ID3D11Buffer*>(m_pVertexBuffer->GetInternal());
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetVertexBuffers(0, 1, &internalBuf, &stride, &offset);
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetInputLayout(m_pInputLayout);

	unsigned int batchSize = 1;
	unsigned int batchOffset = 0;
	const size_t spriteCount = m_Sprites.size();
	for (size_t i = 0; i < spriteCount; ++i)
	{
		if (i < (spriteCount - 1) && m_Sprites[i].TextureId == m_Sprites[i + 1].TextureId)
		{
			++batchSize;
			continue;
		}

		//Set Texture
		const GA::Texture2D* texData = m_Textures[m_Sprites[i].TextureId];
		const GA::DX11::Texture2DDX11* dx11TexData = GA::DX11::SafeCast(texData);
		m_pTextureSRV->SetResource(dx11TexData->GetSRV());

		//Set Texture Size
		DirectX::XMFLOAT2 texSize = {static_cast<float>(texData->GetWidth()), static_cast<float>(texData->GetHeight()) };
		m_pTextureSizeV->SetFloatVector(reinterpret_cast<float*>(&texSize));

		//Set Transform
		m_pTransfromMatrixV->SetMatrix(reinterpret_cast<float*>(&m_Transform));

		D3DX11_TECHNIQUE_DESC techDesc;
		m_pTechnique->GetDesc(&techDesc);
		for (unsigned int j = 0; j < techDesc.Passes; ++j)
		{
			m_pTechnique->GetPassByIndex(j)->Apply(0, GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext());
			GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->Draw(batchSize, batchOffset);
		}

		batchOffset += batchSize;
		batchSize = 1;
	}

	m_Sprites.clear();
	m_Textures.clear();
}

void SpriteRenderer::Draw(GA::Texture2D* pTexture, DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color,
                          DirectX::XMFLOAT2 pivot, DirectX::XMFLOAT2 scale, float rotation, float depth)
{
	PIX_PROFILE();

	SpriteVertex vertex{};

	const auto it = find(m_Textures.begin(), m_Textures.end(), pTexture);

	if (it == m_Textures.end())
	{
		m_Textures.push_back(pTexture);
		vertex.TextureId = static_cast<uint32_t>(m_Textures.size() - 1);
	}
	else
	{
		vertex.TextureId = static_cast<uint32_t>(it - m_Textures.begin());
	}

	vertex.TransformData = DirectX::XMFLOAT4(position.x, position.y, depth, rotation);
	vertex.TransformData2 = DirectX::XMFLOAT4(pivot.x, pivot.y, scale.x, scale.y);
	vertex.Color = color;

	m_Sprites.push_back(vertex);
}

void SpriteRenderer::DrawImmediate(const GameContext& gameContext, ID3D11ShaderResourceView* pSrv,
                                   DirectX::XMFLOAT2 position, DirectX::XMFLOAT4 color, DirectX::XMFLOAT2 pivot,
                                   DirectX::XMFLOAT2 scale, float rotation)
{
	PIX_PROFILE();

	//Create Immediate VB
	if (!m_pImmediateVertexBuffer)
	{
		GA::Buffer::Params params;
		params.sizeInBytes = sizeof(SpriteVertex);
		params.type = GA::Buffer::Type::Vertex;
		params.lifeTime = GA::Resource::LifeTime::Permanent;
		params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Frequent;

		m_pImmediateVertexBuffer = gameContext.pRenderer->CreateBuffer(params);
	}

	//Map Vertex
	SpriteVertex vertex;
	vertex.TextureId = 0;
	vertex.TransformData = DirectX::XMFLOAT4(position.x, position.y, 0, rotation);
	vertex.TransformData2 = DirectX::XMFLOAT4(pivot.x, pivot.y, scale.x, scale.y);
	vertex.Color = color;

	if (!m_ImmediateVertex.Equals(vertex))
	{
		void* pData = m_pImmediateVertexBuffer->Map();
		memcpy(pData, &vertex, sizeof(SpriteVertex));
		m_pImmediateVertexBuffer->Unmap();

		m_ImmediateVertex = vertex;
	}

	//Set Render Pipeline
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	unsigned int stride = sizeof(SpriteVertex);
	unsigned int offset = 0;

	ID3D11Buffer* internalBuf = std::any_cast<ID3D11Buffer*>(m_pImmediateVertexBuffer->GetInternal());
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetVertexBuffers(0, 1, &internalBuf, &stride, &offset);
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetInputLayout(m_pInputLayout);

	//Set Texture
	m_pTextureSRV->SetResource(pSrv);

	ID3D11Resource* pResource;
	pSrv->GetResource(&pResource);
	D3D11_TEXTURE2D_DESC texDesc;
	auto texResource = reinterpret_cast<ID3D11Texture2D*>(pResource);
	texResource->GetDesc(&texDesc);
	texResource->Release();

	//Set Texture Size
	auto texSize = DirectX::XMFLOAT2(static_cast<float>(texDesc.Width), static_cast<float>(texDesc.Height));
	m_pTextureSizeV->SetFloatVector(reinterpret_cast<float*>(&texSize));

	//Set Transform
	m_pTransfromMatrixV->SetMatrix(reinterpret_cast<float*>(&m_Transform));

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (unsigned int i = 0; i < techDesc.Passes; ++i)
	{
		m_pTechnique->GetPassByIndex(i)->Apply(0, GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext());
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->Draw(1, 0);
	}
}

void SpriteRenderer::Draw(GA::Texture2D* pTexture, SpriteVertex& v) const
{
	PIX_PROFILE();

#ifndef SPRITE_EFFECT_SUPPORT
	pTexture;
	v;
	/*Logger::LogWarning(L"SPRITE_EFFECT_SUPPORT is disabled, SpriteExComponent can't be used.");*/
#else
	auto it = find(m_Textures.begin(), m_Textures.end(), pTexture);

	if (it == m_Textures.end())
	{
		m_Textures.push_back(pTexture);
		v.TextureId = m_Textures.size() - 1;
	}
	else
	{
		v.TextureId = it - m_Textures.begin();
	}

	m_Sprites.push_back(v);
#endif
}
