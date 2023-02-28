#include "stdafx.h"
#include "PbrPropsMaterial.h"
#include "TextureData.h"
#include "ContentManager.h"

ID3DX11EffectScalarVariable* PbrPropsMaterial::m_pNoiseHeightVariable = nullptr;
ID3DX11EffectScalarVariable* PbrPropsMaterial::m_pNoiseUvScaleVariable = nullptr;
ID3DX11EffectShaderResourceVariable* PbrPropsMaterial::m_pNoiseTextureVariable = nullptr;
ID3DX11EffectScalarVariable* PbrPropsMaterial::m_pWindForceVariable = nullptr;
ID3DX11EffectVectorVariable* PbrPropsMaterial::m_pWindDirectionVariable = nullptr;
ID3DX11EffectScalarVariable* PbrPropsMaterial::m_pInfluenceVariable = nullptr;
ID3DX11EffectScalarVariable* PbrPropsMaterial::m_pHeightInfluenceVariable = nullptr;
ID3DX11EffectScalarVariable* PbrPropsMaterial::m_pTimePassedVariable = nullptr;


TextureData* PbrPropsMaterial::m_pNoiseTexture = nullptr;
float PbrPropsMaterial::m_WindForce = 0.01f;
XMFLOAT2 PbrPropsMaterial::m_WindDirection = XMFLOAT2{ 1,1 };
float PbrPropsMaterial::m_NoiseUvScale = 100.f;
float PbrPropsMaterial::m_NoiseHeight = 10.0f;



void PbrPropsMaterial::SetNoiseTexture(const wstring& assetFile)
{
	m_pNoiseTexture = ContentManager::Load<TextureData>(assetFile);
}

void PbrPropsMaterial::SetNoiseHeight(float height)
{
	m_NoiseHeight = height;
}

void PbrPropsMaterial::SetNoiseUvScale(float scale)
{
	m_NoiseUvScale = scale;
}

PbrPropsMaterial::PbrPropsMaterial(bool isFoliage, bool generatesShadows)
	:PbrMaterial_Shadow{ L"./Resources/Effects/Pbr/PbrInstancedProps.fx", isFoliage?L"tFoliage":L"tDefault", false, generatesShadows }
	, m_IsFoliage{ isFoliage }
	, m_Influence{ 50.0f }
	, m_HeightInfluence{10.0f}
{

}

void PbrPropsMaterial::SetWindForce(float force)
{
	m_WindForce = force;
}

void PbrPropsMaterial::SetWindDirection(XMFLOAT2 dir)
{
	m_WindDirection = dir;
}

void PbrPropsMaterial::SetInfluence(float influence)
{
	m_Influence = influence;
}

ID3DX11EffectTechnique* PbrPropsMaterial::GetShadowTechnique()
{
	if (!m_GenerateShadows) return nullptr;
	if (m_pShadowTechnique == nullptr)
	{
		if (m_IsFoliage)
		{
			m_pShadowTechnique = GetTechnique("tGenerateShadowsFoliage");
		}
		else
		{
			m_pShadowTechnique = GetTechnique("tGenerateShadowsStatic");
		}
	}
	return m_pShadowTechnique;
}

void PbrPropsMaterial::LoadEffectVariables()
{
	PbrMaterial_Shadow::LoadEffectVariables();

	//noise
	if (!m_pNoiseHeightVariable)
	{
		m_pNoiseHeightVariable = GetEffect()->GetVariableByName("gNoiseHeight")->AsScalar();
		if (!m_pNoiseHeightVariable->IsValid())
		{
			Logger::LogWarning(L"PbrGroundMaterial::LoadEffectVariables() > \'gNoiseHeight\' variable not found!");
			m_pNoiseHeightVariable = nullptr;
		}
	}
	if (!m_pNoiseUvScaleVariable)
	{
		m_pNoiseUvScaleVariable = GetEffect()->GetVariableByName("gNoiseUVScale")->AsScalar();
		if (!m_pNoiseUvScaleVariable->IsValid())
		{
			Logger::LogWarning(L"PbrGroundMaterial::LoadEffectVariables() > \'gNoiseUVScale\' variable not found!");
			m_pNoiseUvScaleVariable = nullptr;
		}
	}
	if (!m_pNoiseTextureVariable)
	{
		m_pNoiseTextureVariable = GetEffect()->GetVariableByName("gNoiseTexture")->AsShaderResource();
		if (!m_pNoiseTextureVariable->IsValid())
		{
			Logger::LogWarning(L"PbrGroundMaterial::LoadEffectVariables() > \'gNoiseTexture\' variable not found!");
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
	if (!m_pHeightInfluenceVariable)
	{
		m_pHeightInfluenceVariable = GetEffect()->GetVariableByName("gDistanceInfluence")->AsScalar();
		if (!m_pHeightInfluenceVariable->IsValid())
		{
			Logger::LogWarning(L"PbrFoliageMaterial::LoadEffectVariables() > \'gDistanceInfluence\' variable not found!");
			m_pHeightInfluenceVariable = nullptr;
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

void PbrPropsMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	PbrMaterial_Shadow::UpdateEffectVariables(gameContext, pModelComponent);
	//NOISE
	if (m_pNoiseHeightVariable)
	{
		m_pNoiseHeightVariable->SetFloat(m_NoiseHeight);
	}
	if (m_pNoiseUvScaleVariable)
	{
		m_pNoiseUvScaleVariable->SetFloat(m_NoiseUvScale);
	}
	if (m_pNoiseTexture && m_pNoiseTextureVariable)
	{
		m_pNoiseTextureVariable->SetResource(m_pNoiseTexture->GetShaderResourceView());
	}

	if (m_pTimePassedVariable)
	{
		m_pTimePassedVariable->SetFloat(gameContext.pGameTime->GetTotal());
	}
	if (m_pInfluenceVariable)
	{
		m_pInfluenceVariable->SetFloat(m_Influence);
	}
	if (m_pHeightInfluenceVariable)
	{
		m_pHeightInfluenceVariable->SetFloat(m_HeightInfluence);
	}
	if (m_pWindForceVariable)
	{
		m_pWindForceVariable->SetFloat(m_WindForce);
	}
	if (m_pWindDirectionVariable)
	{
		m_pWindDirectionVariable->SetFloatVector(&m_WindDirection.x);
	}
}

