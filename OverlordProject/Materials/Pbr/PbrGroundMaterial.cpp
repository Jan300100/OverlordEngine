#include "stdafx.h"
#include "PbrGroundMaterial.h"
#include "ContentManager.h"
#include "TextureData.h"

ID3DX11EffectScalarVariable* PbrGroundMaterial::m_pWorldUvScaleVariable = nullptr;

ID3DX11EffectScalarVariable* PbrGroundMaterial::m_pNoiseHeightVariable = nullptr;
ID3DX11EffectScalarVariable* PbrGroundMaterial::m_pNoiseUvScaleVariable = nullptr;
ID3DX11EffectShaderResourceVariable* PbrGroundMaterial::m_pNoiseTextureVariable = nullptr;

ID3DX11EffectScalarVariable* PbrGroundMaterial::m_pMaxTessFactorVariable = nullptr;
ID3DX11EffectScalarVariable* PbrGroundMaterial::m_pDisplacementAmountVariable = nullptr;


PbrGroundMaterial::PbrGroundMaterial()
	:PbrMaterial_Shadow{ L"./Resources/Effects/Pbr/PbrGround_Shadow.fx", L"tDefault", true}
	, m_WorldUvScale{100.0f}

	, m_NoiseHeight{30.0f}
	, m_NoiseUVScale{1000.0f}
	, m_pNoiseTexture{nullptr}

	, m_DisplacementAmount{ 0 }
	, m_TessFactor{ 1 }
{
}

void PbrGroundMaterial::SetWorldUvScale(float scale)
{
	m_WorldUvScale = scale;
}


void PbrGroundMaterial::SetNoiseTexture(const wstring& assetFile)
{
	m_pNoiseTexture = ContentManager::Load<TextureData>(assetFile);
}

void PbrGroundMaterial::SetNoiseHeight(float height)
{
	m_NoiseHeight = height;
}

void PbrGroundMaterial::SetNoiseUvScale(float scale)
{
	m_NoiseUVScale = scale;
}



void PbrGroundMaterial::SetDisplacementAmount(float amount)
{
	m_DisplacementAmount = amount;
}

void PbrGroundMaterial::SetTessFactor(float maxFactor)
{
	m_TessFactor = maxFactor;
}

void PbrGroundMaterial::LoadEffectVariables()
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

	//WorldUvs
	if (!m_pWorldUvScaleVariable)
	{
		m_pWorldUvScaleVariable = GetEffect()->GetVariableByName("gWorldUVScale")->AsScalar();
		if (!m_pWorldUvScaleVariable->IsValid())
		{
			Logger::LogWarning(L"PbrGroundMaterial::LoadEffectVariables() > \'gWorldUVScale\' variable not found!");
			m_pWorldUvScaleVariable = nullptr;
		}
	}


	//displacement
	if (!m_pDisplacementAmountVariable)
	{
		m_pDisplacementAmountVariable = GetEffect()->GetVariableByName("gDisplacementAmount")->AsScalar();
		if (!m_pDisplacementAmountVariable->IsValid())
		{
			Logger::LogWarning(L"PbrGroundMaterial::LoadEffectVariables() > \'gDisplacementAmount\' variable not found!");
			m_pDisplacementAmountVariable = nullptr;
		}
	}
	if (!m_pMaxTessFactorVariable)
	{
		m_pMaxTessFactorVariable = GetEffect()->GetVariableByName("gTessFactor")->AsScalar();
		if (!m_pMaxTessFactorVariable->IsValid())
		{
			Logger::LogWarning(L"PbrGroundMaterial::LoadEffectVariables() > \'gTessFactor\' variable not found!");
			m_pMaxTessFactorVariable = nullptr;
		}
	}

}

void PbrGroundMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{

	PbrMaterial_Shadow::UpdateEffectVariables(gameContext, pModelComponent);

	//NOISE
	if (m_pNoiseHeightVariable)
	{
		m_pNoiseHeightVariable->SetFloat(m_NoiseHeight);
	}
	if (m_pNoiseUvScaleVariable)
	{
		m_pNoiseUvScaleVariable->SetFloat(m_NoiseUVScale);
	}
	if (m_pNoiseTexture && m_pNoiseTextureVariable)
	{
		m_pNoiseTextureVariable->SetResource(m_pNoiseTexture->GetShaderResourceView());
	}

	//worldUV
	if (m_pWorldUvScaleVariable)
	{
		m_pWorldUvScaleVariable->SetFloat(m_WorldUvScale);
	}
	
	
	//displacement
	if (m_pDisplacementAmountVariable)
	{
		m_pDisplacementAmountVariable->SetFloat(m_DisplacementAmount);
	}
	if (m_pMaxTessFactorVariable)
	{
		m_pMaxTessFactorVariable->SetFloat(m_TessFactor);
	}
}




