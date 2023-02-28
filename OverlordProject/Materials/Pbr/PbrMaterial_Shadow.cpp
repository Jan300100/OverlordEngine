#include "stdafx.h"
#include "Components.h"
#include "PbrMaterial_Shadow.h"

PbrMaterial_Shadow::PbrMaterial_Shadow(const wstring& effectFile, const wstring& techniqueName, bool usesTesselation, bool generatesShadows)
	:PbrMaterial{effectFile, techniqueName, usesTesselation, generatesShadows}
	, m_pShadowSRVvariable{nullptr}
	, m_pLightVPVariable{nullptr}
{
}

PbrMaterial_Shadow::PbrMaterial_Shadow()
	: PbrMaterial_Shadow{ L"./Resources/Effects/Pbr/PbrDefault_Shadow.fx", L"tDefault", false }
{
}

void PbrMaterial_Shadow::LoadEffectVariables()
{
	PbrMaterial::LoadEffectVariables();

	if (!m_pLightVPVariable)
	{
		m_pLightVPVariable = GetEffect()->GetVariableByName("gViewProj_Light")->AsMatrix();
		if (!m_pLightVPVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial_Shadow::LoadEffectVariables() > \'gViewProj_Light\' variable not found!");
			m_pLightVPVariable = nullptr;
		}
	}

	if (!m_pShadowSRVvariable)
	{
		m_pShadowSRVvariable = GetEffect()->GetVariableByName("gShadowMap")->AsShaderResource();
		if (!m_pShadowSRVvariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial_Shadow::LoadEffectVariables() > \'gShadowMap\' variable not found!");
			m_pShadowSRVvariable = nullptr;
		}
	}
}

void PbrMaterial_Shadow::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	PbrMaterial::UpdateEffectVariables(gameContext, pModelComponent);

	if ((m_pShadowSRVvariable))
	{
		m_pShadowSRVvariable->SetResource(gameContext.pShadowMapper->GetShadowMap());
	}

	if (m_pLightVPVariable)
	{
		DirectX::XMFLOAT4X4 lightVP = gameContext.pShadowMapper->GetLightVP();
		DirectX::XMMATRIX lvp = DirectX::XMLoadFloat4x4(&lightVP);
		m_pLightVPVariable->SetMatrix(reinterpret_cast<const float*>(&(lvp)));
	}

}
