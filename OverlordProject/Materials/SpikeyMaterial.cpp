
#include "stdafx.h"

#include "SpikeyMaterial.h"

#include "ContentManager.h"

// todo: dx11
#include <GA/DX11/Texture2DDX11.h>

ID3DX11EffectShaderResourceVariable* SpikeyMaterial::m_pDiffuseSRVvariable = nullptr;

SpikeyMaterial::SpikeyMaterial() : Material(L"./Resources/Effects/SpikeyShader.fx"),
	m_pDiffuseTexture(nullptr)
{}

SpikeyMaterial::~SpikeyMaterial()
{}

void SpikeyMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<GA::Texture2D>(assetFile).get();
}

void SpikeyMaterial::LoadEffectVariables()
{
	if (!m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable = GetEffect()->GetVariableByName("m_TextureDiffuse")->AsShaderResource();
		if (!m_pDiffuseSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'m_TextureDiffuse\' variable not found!");
			m_pDiffuseSRVvariable = nullptr;
		}
	}
}

void SpikeyMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(pModelComponent);

	if (m_pDiffuseTexture && m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable->SetResource(GA::DX11::SafeCast(m_pDiffuseTexture)->GetSRV());
	}
}