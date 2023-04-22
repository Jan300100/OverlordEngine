#include "stdafx.h"
#include "TextRenderer.h"
#include "ContentManager.h"
#include "EffectHelper.h"
#include "OverlordGame.h"
#include "SpriteFont.h"
#include <GA/Buffer.h>
#include <Logger.h>

// todo: dx11
#include <GA/DX11/InterfaceDX11.h>
#include <GA/DX11/Texture2DDX11.h>

TextRenderer::TextRenderer() :
	m_BufferSize(500),
	m_InputLayoutSize(0),
	m_NumCharacters(0),
	m_Transform(DirectX::XMFLOAT4X4()),
	m_pEffect(nullptr),
	m_pTechnique(nullptr),
	m_pTransfromMatrixV(nullptr),
	m_pTextureSizeV(nullptr),
	m_pTextureSRV(nullptr),
	m_pInputLayout(nullptr),
	m_pVertexBuffer(nullptr),
	m_SpriteFonts(std::vector<SpriteFont*>())
{
}

TextRenderer::~TextRenderer()
{
	SafeRelease(m_pInputLayout);
}

void TextRenderer::InitRenderer(GA::Interface* pGAInterface)
{
	PIX_PROFILE();

	//Effect
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/TextRenderer.fx").get();
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

	EffectHelper::BuildInputLayout(GA::DX11::SafeCast(pGAInterface)->GetDevice(), m_pTechnique, &m_pInputLayout, m_InputLayoutSize);

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

void TextRenderer::DrawText(SpriteFont* pFont, const std::wstring& text, DirectX::XMFLOAT2 position,
                            DirectX::XMFLOAT4 color)
{
	PIX_PROFILE();

	m_NumCharacters += static_cast<uint32_t>(wcslen(text.c_str()));
	pFont->AddToTextCache(TextCache(text, position, color));
	if (!pFont->IsAddedToRenderer())
	{
		m_SpriteFonts.push_back(pFont);
		pFont->SetAddedToRenderer(true);
	}
}

void TextRenderer::Draw(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (m_SpriteFonts.empty())
		return;

	UpdateBuffer(gameContext);

	//Set Render Pipeline
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	unsigned int stride = sizeof(TextVertex);
	unsigned int offset = 0;
	
	ID3D11Buffer* internalBuffer = std::any_cast<ID3D11Buffer*>(m_pVertexBuffer->GetInternal());
	if (!internalBuffer)
	{
		Logger::LogError(L"Cast failed: VertexBuffer was not a ID3D11Buffer");
	}

	GA::DX11::QuickCast(gameContext.pRenderer)->GetDeviceContext()->IASetVertexBuffers(0, 1, &internalBuffer, &stride, &offset);
	GA::DX11::QuickCast(gameContext.pRenderer)->GetDeviceContext()->IASetInputLayout(m_pInputLayout);

	for each (SpriteFont* pFont in m_SpriteFonts)
	{
		//Set Texture
		const GA::DX11::Texture2DDX11* dx11Tex = GA::DX11::SafeCast(pFont->GetTexture());
		m_pTextureSRV->SetResource(dx11Tex->GetSRV());

		//Set Texture Size
		DirectX::XMFLOAT2 texSize = { static_cast<float>(pFont->GetTexture()->GetWidth()), static_cast<float>(pFont->GetTexture()->GetHeight()) };
		m_pTextureSizeV->SetFloatVector(reinterpret_cast<float*>(&texSize));

		//Set Transform
		m_pTransfromMatrixV->SetMatrix(reinterpret_cast<float*>(&m_Transform));

		D3DX11_TECHNIQUE_DESC techDesc;
		m_pTechnique->GetDesc(&techDesc);
		for (unsigned int i = 0; i < techDesc.Passes; ++i)
		{
			m_pTechnique->GetPassByIndex(i)->Apply(0, GA::DX11::QuickCast(gameContext.pRenderer)->GetDeviceContext());
			GA::DX11::QuickCast(gameContext.pRenderer)->GetDeviceContext()->Draw(pFont->GetBufferSize(), pFont->GetBufferStart());
		}

		pFont->SetAddedToRenderer(false);
	}

	m_SpriteFonts.clear();
}

void TextRenderer::UpdateBuffer(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (!m_pVertexBuffer || m_NumCharacters > m_BufferSize)
	{
		//Set buffersize if needed
		if (m_NumCharacters > m_BufferSize)
			m_BufferSize = m_NumCharacters;

		GA::Buffer::Params params;
		params.lifeTime = GA::Resource::LifeTime::Permanent;
		params.sizeInBytes = sizeof(TextVertex) * m_BufferSize;
		params.type = GA::Buffer::Type::Vertex;
		params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Frequent;

		m_pVertexBuffer = gameContext.pRenderer->CreateBuffer(params);
	}

	//Fill Buffer
	void* pData = m_pVertexBuffer->Map();
	int bufferPosition = 0;
	for each (SpriteFont* pFont in m_SpriteFonts)
	{
		const auto& cache = pFont->GetTextCache();
		pFont->SetBufferStart(bufferPosition);
		for (unsigned int i = 0; i < cache.size(); ++i)
		{
			CreateTextVertices(pFont, cache[i], static_cast<TextVertex*>(pData), bufferPosition);
		}

		pFont->SetBufferSize(bufferPosition - pFont->GetBufferStart());
		pFont->ClearCache();
	}
	m_pVertexBuffer->Unmap();

	m_NumCharacters = 0;
}

void TextRenderer::CreateTextVertices(const SpriteFont* pFont, const TextCache& info, TextVertex* pBuffer,
                                      int& bufferPosition)
{
	PIX_PROFILE();

	int totalAdvanceX = 0;
	for (wchar_t charId : info.Text)
	{
		if (SpriteFont::IsCharValid(charId) && pFont->GetMetric(charId).IsValid)
		{
			const auto metric = pFont->GetMetric(charId);

			if (charId == ' ')
			{
				totalAdvanceX += metric.AdvanceX;
				continue;
			}

			TextVertex vertexText;
			vertexText.Position.x = info.Position.x + totalAdvanceX + metric.OffsetX;
			vertexText.Position.y = info.Position.y + metric.OffsetY;
			vertexText.Position.z = 0.9f;
			vertexText.Color = info.Color;
			vertexText.TexCoord = metric.TexCoord;
			vertexText.CharacterDimension = DirectX::XMFLOAT2(metric.Width, metric.Height);
			vertexText.ChannelId = metric.Channel;

			pBuffer[bufferPosition] = vertexText;

			++bufferPosition;
			totalAdvanceX += metric.AdvanceX;
		}
		else
		{
			Logger::LogFormat(
				LogLevel::Warning, L"TextRenderer::CreateTextVertices > Char not supported for current font.\nCHARACTER: %c (%i)\nFONT: %s",
				charId, static_cast<int>(charId), pFont->GetFontName().c_str());
		}
	}
}
