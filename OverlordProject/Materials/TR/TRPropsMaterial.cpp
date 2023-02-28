#include "stdafx.h"
#include "TRPropsMaterial.h"
#include "TextureData.h"
#include "ContentManager.h"

ID3DX11EffectScalarVariable* TRPropsMaterial::m_pNoiseHeightVariable = nullptr;
ID3DX11EffectScalarVariable* TRPropsMaterial::m_pNoiseUvScaleVariable = nullptr;
ID3DX11EffectShaderResourceVariable* TRPropsMaterial::m_pNoiseTextureVariable = nullptr;
ID3DX11EffectScalarVariable* TRPropsMaterial::m_pWindForceVariable = nullptr;
ID3DX11EffectVectorVariable* TRPropsMaterial::m_pWindDirectionVariable = nullptr;
ID3DX11EffectScalarVariable* TRPropsMaterial::m_pInfluenceVariable = nullptr;
ID3DX11EffectScalarVariable* TRPropsMaterial::m_pDistanceInfluence = nullptr;
ID3DX11EffectScalarVariable* TRPropsMaterial::m_pTimePassedVariable = nullptr;


TextureData* TRPropsMaterial::m_pNoiseTexture = nullptr;
XMFLOAT2 TRPropsMaterial::m_WindDirection = XMFLOAT2{ 1,1 };
float TRPropsMaterial::m_NoiseUvScale = 100.f;
float TRPropsMaterial::m_NoiseHeight = 10.0f;



void TRPropsMaterial::SetNoiseTexture(const wstring& assetFile)
{
	m_pNoiseTexture = ContentManager::Load<TextureData>(assetFile);
}

void TRPropsMaterial::SetNoiseHeight(float height)
{
	m_NoiseHeight = height;
}

void TRPropsMaterial::SetNoiseUvScale(float scale)
{
	m_NoiseUvScale = scale;
}

TRPropsMaterial::TRPropsMaterial(bool isFoliage, bool generatesShadows)
	:TRMaterial{ L"./Resources/Effects/TR/TRInstancedProps.fx", isFoliage?L"tFoliage":L"tDefault", false, generatesShadows }
	, m_IsFoliage{ isFoliage }
	, m_Influence{ 1.0f }
	, m_DistanceInfluence{1.0f}
	, m_WindForce{0.05f}
{

}

void TRPropsMaterial::SetWindForce(float force)
{
	m_WindForce = force;
}

void TRPropsMaterial::SetWindDirection(XMFLOAT2 dir)
{
	m_WindDirection = dir;
}

void TRPropsMaterial::SetInfluence(float influence)
{
	m_Influence = influence;
}

void TRPropsMaterial::SetDistanceInfluence(float influence)
{
	m_DistanceInfluence = influence;
}

ID3DX11EffectTechnique* TRPropsMaterial::GetShadowTechnique()
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

void TRPropsMaterial::LoadEffectVariables()
{
	PIX_PROFILE();

	TRMaterial::LoadEffectVariables();

	//noise
	if (!m_pNoiseHeightVariable)
	{
		m_pNoiseHeightVariable = GetEffect()->GetVariableByName("gNoiseHeight")->AsScalar();
		if (!m_pNoiseHeightVariable->IsValid())
		{
			Logger::LogWarning(L"TRGroundMaterial::LoadEffectVariables() > \'gNoiseHeight\' variable not found!");
			m_pNoiseHeightVariable = nullptr;
		}
	}
	if (!m_pNoiseUvScaleVariable)
	{
		m_pNoiseUvScaleVariable = GetEffect()->GetVariableByName("gNoiseUVScale")->AsScalar();
		if (!m_pNoiseUvScaleVariable->IsValid())
		{
			Logger::LogWarning(L"TRGroundMaterial::LoadEffectVariables() > \'gNoiseUVScale\' variable not found!");
			m_pNoiseUvScaleVariable = nullptr;
		}
	}
	if (!m_pNoiseTextureVariable)
	{
		m_pNoiseTextureVariable = GetEffect()->GetVariableByName("gNoiseTexture")->AsShaderResource();
		if (!m_pNoiseTextureVariable->IsValid())
		{
			Logger::LogWarning(L"TRGroundMaterial::LoadEffectVariables() > \'gNoiseTexture\' variable not found!");
			m_pNoiseTextureVariable = nullptr;
		}
	}

	if (!m_pTimePassedVariable)
	{
		m_pTimePassedVariable = GetEffect()->GetVariableByName("gTimePassed")->AsScalar();
		if (!m_pTimePassedVariable->IsValid())
		{
			Logger::LogWarning(L"TRFoliageMaterial::LoadEffectVariables() > \'gTimePassed\' variable not found!");
			m_pTimePassedVariable = nullptr;
		}
	}
	if (!m_pInfluenceVariable)
	{
		m_pInfluenceVariable = GetEffect()->GetVariableByName("gInfluence")->AsScalar();
		if (!m_pInfluenceVariable->IsValid())
		{
			Logger::LogWarning(L"TRFoliageMaterial::LoadEffectVariables() > \'gInfluence\' variable not found!");
			m_pInfluenceVariable = nullptr;
		}
	}
	if (!m_pDistanceInfluence)
	{
		m_pDistanceInfluence = GetEffect()->GetVariableByName("gDistanceInfluence")->AsScalar();
		if (!m_pDistanceInfluence->IsValid())
		{
			Logger::LogWarning(L"TRFoliageMaterial::LoadEffectVariables() > \'gDistanceInfluence\' variable not found!");
			m_pDistanceInfluence = nullptr;
		}
	}
	if (!m_pWindForceVariable)
	{
		m_pWindForceVariable = GetEffect()->GetVariableByName("gWindForce")->AsScalar();
		if (!m_pWindForceVariable->IsValid())
		{
			Logger::LogWarning(L"TRFoliageMaterial::LoadEffectVariables() > \'gWindForce\' variable not found!");
			m_pWindForceVariable = nullptr;
		}
	}
	if (!m_pWindDirectionVariable)
	{
		m_pWindDirectionVariable = GetEffect()->GetVariableByName("gWindDirection")->AsVector();
		if (!m_pWindDirectionVariable->IsValid())
		{
			Logger::LogWarning(L"TRFoliageMaterial::LoadEffectVariables() > \'gWindDirection\' variable not found!");
			m_pWindDirectionVariable = nullptr;
		}
	}

}

void TRPropsMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	PIX_PROFILE();

	TRMaterial::UpdateEffectVariables(gameContext, pModelComponent);
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
	if (m_pDistanceInfluence)
	{
		m_pDistanceInfluence->SetFloat(m_DistanceInfluence);
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

