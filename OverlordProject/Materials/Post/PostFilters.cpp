#include "stdafx.h"
#include "PostFilters.h"
#include "RenderTarget.h"

PostFilters::PostFilters(float brightness, float contrast, float hue, float saturation)
	: PostProcessingMaterial(L"./Resources/Effects/Post/Filters.fx", 1),
	m_pTextureMapVariabele(nullptr)
	, m_pBrightnessVariable{}
	, m_Brightness{brightness}

	, m_pContrastVariable{}
	,m_Contrast{contrast}
	, m_pHueVariable{}
	,m_Hue{hue}
	, m_pSaturationVariable{}
	,m_Saturation{saturation}
{
}

void PostFilters::LoadEffectVariables()
{
	//TODO: Bind the 'gTexture' variable with 'm_pTextureMapVariable'
	//Check if valid!
	if (!m_pTextureMapVariabele)
	{
		m_pTextureMapVariabele = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();
		if (!m_pTextureMapVariabele->IsValid())
		{
			Logger::LogWarning(L"PostFilters::LoadEffectVariables() > \'gTexture\' variable not found!");
			m_pTextureMapVariabele = nullptr;
		}
	}

	if (!m_pBrightnessVariable)
	{
		m_pBrightnessVariable = GetEffect()->GetVariableByName("gBrightness")->AsScalar();
		if (!m_pBrightnessVariable->IsValid())
		{
			Logger::LogWarning(L"PostFilters::LoadEffectVariables() > \'gBrightness\' variable not found!");
			m_pBrightnessVariable = nullptr;
		}
	}

	if (!m_pContrastVariable)
	{
		m_pContrastVariable = GetEffect()->GetVariableByName("gContrast")->AsScalar();
		if (!m_pContrastVariable->IsValid())
		{
			Logger::LogWarning(L"PostFilters::LoadEffectVariables() > \'gContrast\' variable not found!");
			m_pContrastVariable = nullptr;
		}
	}

	if (!m_pHueVariable)
	{
		m_pHueVariable = GetEffect()->GetVariableByName("gHue")->AsScalar();
		if (!m_pHueVariable->IsValid())
		{
			Logger::LogWarning(L"PostFilters::LoadEffectVariables() > \'gHue\' variable not found!");
			m_pHueVariable = nullptr;
		}
	}

	if (!m_pSaturationVariable)
	{
		m_pSaturationVariable = GetEffect()->GetVariableByName("gSaturation")->AsScalar();
		if (!m_pSaturationVariable->IsValid())
		{
			Logger::LogWarning(L"PostFilters::LoadEffectVariables() > \'gSaturation\' variable not found!");
			m_pSaturationVariable = nullptr;
		}
	}



}

void PostFilters::UpdateEffectVariables(const GameContext& gameContext, RenderTarget* pPrevRendertarget, RenderTarget* pOriginalRendertarget)
{
	UNREFERENCED_PARAMETER(pOriginalRendertarget);
	UNREFERENCED_PARAMETER(gameContext);
	//TODO: Update the TextureMapVariable with the Color ShaderResourceView of the given RenderTarget
	if (m_pTextureMapVariabele)
	{
		m_pTextureMapVariabele->SetResource(pPrevRendertarget->GetShaderResourceView());
	}

	if (m_pBrightnessVariable)
	{
		m_pBrightnessVariable->SetFloat(m_Brightness);
	}

	if (m_pContrastVariable)
	{
		m_pContrastVariable->SetFloat(m_Contrast);
	}

	if (m_pHueVariable)
	{
		m_pHueVariable->SetFloat(m_Hue);
	}

	if (m_pSaturationVariable)
	{
		m_pSaturationVariable->SetFloat(m_Saturation);
	}
}