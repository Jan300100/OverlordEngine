#include "stdafx.h"
#include "DiffuseMaterial.h"
#include "ContentManager.h"

// todo: dx11
#include <GA/DX11/Texture2DDX11.h>

ID3DX11EffectShaderResourceVariable* DiffuseMaterial::m_pDiffuseSRVvariable = nullptr;


DiffuseMaterial::DiffuseMaterial() :Material(L"./Resources/Effects/PosNormTex3D.fx")
	,m_pDiffuseTexture{nullptr}
{
}

DiffuseMaterial::~DiffuseMaterial()
{
}

void DiffuseMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<GA::Texture2D>(assetFile).get();
}

void DiffuseMaterial::LoadEffectVariables()
{
	if (!m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable = GetEffect()->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gDiffuseMap\' variable not found!");
			m_pDiffuseSRVvariable = nullptr;
		}
	}
}

void DiffuseMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(pModelComponent);

	if (m_pDiffuseTexture && m_pDiffuseSRVvariable)
	{
		const GA::DX11::Texture2DDX11* dx11Tex = GA::DX11::SafeCast(m_pDiffuseTexture);
		m_pDiffuseSRVvariable->SetResource(dx11Tex->GetSRV());
	}
}
