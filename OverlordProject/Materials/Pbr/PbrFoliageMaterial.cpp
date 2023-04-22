#include "stdafx.h"
#include "PbrFoliageMaterial.h"
#include <ContentManager.h>

// todo: dx11
#include <GA/DX11/Texture2DDX11.h>

ID3DX11EffectShaderResourceVariable* PbrFoliageMaterial::m_pNoiseTextureVariable = nullptr;
ID3DX11EffectScalarVariable* PbrFoliageMaterial::m_pNoiseUvScaleVariable = nullptr;
ID3DX11EffectScalarVariable* PbrFoliageMaterial::m_pWindForceVariable = nullptr;
ID3DX11EffectVectorVariable* PbrFoliageMaterial::m_pWindDirectionVariable = nullptr;
ID3DX11EffectScalarVariable* PbrFoliageMaterial::m_pInfluenceVariable = nullptr;
ID3DX11EffectScalarVariable* PbrFoliageMaterial::m_pTimePassedVariable = nullptr;

GA::Texture2D* PbrFoliageMaterial::m_pNoiseTexture = nullptr;
float PbrFoliageMaterial::m_WindForce = 0.2f;
XMFLOAT2 PbrFoliageMaterial::m_WindDirection = XMFLOAT2{0,1};
float PbrFoliageMaterial::m_NoiseUvScale = 0.01f;

PbrFoliageMaterial::PbrFoliageMaterial()
	:PbrMaterial_Shadow{ L"./Resources/Effects/Pbr/PbrFoliage_Shadow.fx", L"tDefault", false }
	, m_Influence{1.0f}
{
}

void PbrFoliageMaterial::LoadEffectVariables()
{
	PbrMaterial_Shadow::LoadEffectVariables();

	if (!m_pNoiseTextureVariable)
	{
		m_pNoiseTextureVariable = GetEffect()->GetVariableByName("gNoiseMap")->AsShaderResource();
		if (!m_pNoiseTextureVariable->IsValid())
		{
			Logger::LogWarning(L"PbrFoliageMaterial::LoadEffectVariables() > \'gNoiseMap\' variable not found!");
			m_pNoiseTextureVariable = nullptr;
		}
	}
	if (!m_pTimePassedVariable)
	{
		m_pTimePassedVariable = GetEffect()->GetVariableByName("gTimePassed")->AsScalar();
		if (!m_pTimePassedVariable->IsValid())
		{
			Logger::LogWarning(L"PbrFoliageMaterial::LoadEffectVariables() > \'gTimePassed\' variable not found!");
			m_pTimePassedVariable = nullptr;
		}
	}
	if (!m_pInfluenceVariable)
	{
		m_pInfluenceVariable = GetEffect()->GetVariableByName("gInfluence")->AsScalar();
		if (!m_pInfluenceVariable->IsValid())
		{
			Logger::LogWarning(L"PbrFoliageMaterial::LoadEffectVariables() > \'gInfluence\' variable not found!");
			m_pInfluenceVariable = nullptr;
		}
	}
	if (!m_pWindForceVariable)
	{
		m_pWindForceVariable = GetEffect()->GetVariableByName("gWindForce")->AsScalar();
		if (!m_pWindForceVariable->IsValid())
		{
			Logger::LogWarning(L"PbrFoliageMaterial::LoadEffectVariables() > \'gWindForce\' variable not found!");
			m_pWindForceVariable = nullptr;
		}
	}
	if (!m_pNoiseUvScaleVariable)
	{
		m_pNoiseUvScaleVariable = GetEffect()->GetVariableByName("gNoiseUvScale")->AsScalar();
		if (!m_pNoiseUvScaleVariable->IsValid())
		{
			Logger::LogWarning(L"PbrFoliageMaterial::LoadEffectVariables() > \'gNoiseUvScale\' variable not found!");
			m_pNoiseUvScaleVariable = nullptr;
		}
	}
	if (!m_pWindDirectionVariable)
	{
		m_pWindDirectionVariable = GetEffect()->GetVariableByName("gWindDirection")->AsVector();
		if (!m_pWindDirectionVariable->IsValid())
		{
			Logger::LogWarning(L"PbrFoliageMaterial::LoadEffectVariables() > \'gWindDirection\' variable not found!");
			m_pWindDirectionVariable = nullptr;
		}
	}
}

void PbrFoliageMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{

	PbrMaterial_Shadow::UpdateEffectVariables(gameContext, pModelComponent);

	if (m_pNoiseTextureVariable && m_pNoiseTexture)
	{
		m_pNoiseTextureVariable->SetResource(GA::DX11::SafeCast(m_pNoiseTexture)->GetSRV());
	}

	if (m_pTimePassedVariable)
	{
		m_pTimePassedVariable->SetFloat(gameContext.pGameTime->GetTotal());
	}
	if (m_pInfluenceVariable)
	{
		m_pInfluenceVariable->SetFloat(m_Influence);
	}
	if (m_pWindForceVariable)
	{
		m_pWindForceVariable->SetFloat(m_WindForce);
	}
	if (m_pNoiseUvScaleVariable)
	{
		m_pNoiseUvScaleVariable->SetFloat(m_NoiseUvScale);
	}
	if (m_pWindDirectionVariable)
	{
		m_pWindDirectionVariable->SetFloatVector(&m_WindDirection.x);
	}
}

void PbrFoliageMaterial::SetNoiseTexture(const wstring& assetFile)
{
	m_pNoiseTexture = ContentManager::Load<GA::Texture2D>(assetFile).get();
}

void PbrFoliageMaterial::SetNoiseUvScale(float scale)
{
	m_NoiseUvScale = scale;
}

void PbrFoliageMaterial::SetWindForce(float force)
{
	m_WindForce = force;
}

void PbrFoliageMaterial::SetWindDirection(XMFLOAT2 dir)
{
	m_WindDirection = dir;
}

void PbrFoliageMaterial::SetInfluence(float influence)
{
	m_Influence = influence;
}
