#include "stdafx.h"
#include "PostFog.h"
#include "RenderTarget.h"
#include "OverlordGame.h"

PostFog::PostFog(float falloff, const DirectX::XMFLOAT3& color)
	: PostProcessingMaterial(L"./Resources/Effects/Post/DepthFog.fx", 3),
	m_pTextureMapVariabele(nullptr)
	, m_pDepthMapVariable{nullptr}
	, m_pFogColorVariable{nullptr}
	, m_pFogFalloffVariable{nullptr}
	, m_pProjInvMatrixVariable{nullptr}
	, m_FogColor{color}
	, m_FogFalloff{falloff}
	, m_pFogStrengthVariable{}
	, m_FogStrength{1.0f}
{
}

void PostFog::LoadEffectVariables()
{
	//TODO: Bind the 'gTexture' variable with 'm_pTextureMapVariable'
	//Check if valid!
	if (!m_pTextureMapVariabele)
	{
		m_pTextureMapVariabele = GetEffect()->GetVariableByName("gColorSRV")->AsShaderResource();
		if (!m_pTextureMapVariabele->IsValid())
		{
			Logger::LogWarning(L"PostFog::LoadEffectVariables() > \'gColorSRV\' variable not found!");
			m_pTextureMapVariabele = nullptr;
		}
	}

	if (!m_pDepthMapVariable)
	{
		m_pDepthMapVariable = GetEffect()->GetVariableByName("gDepthSRV")->AsShaderResource();
		if (!m_pDepthMapVariable->IsValid())
		{
			Logger::LogWarning(L"PostFog::LoadEffectVariables() > \'gDepthSRV\' variable not found!");
			m_pDepthMapVariable = nullptr;
		}
	}

	if (!m_pFogColorVariable)
	{
		m_pFogColorVariable = GetEffect()->GetVariableByName("gFogColor")->AsVector();
		if (!m_pFogColorVariable->IsValid())
		{
			Logger::LogWarning(L"PostFog::LoadEffectVariables() > \'gFogColor\' variable not found!");
			m_pFogColorVariable = nullptr;
		}
	}

	if (!m_pFogFalloffVariable)
	{
		m_pFogFalloffVariable = GetEffect()->GetVariableByName("gFogFalloff")->AsScalar();
		if (!m_pFogFalloffVariable->IsValid())
		{
			Logger::LogWarning(L"PostFog::LoadEffectVariables() > \'gFogFalloff\' variable not found!");
			m_pFogFalloffVariable = nullptr;
		}
	}

	if (!m_pFogStrengthVariable)
	{
		m_pFogStrengthVariable = GetEffect()->GetVariableByName("gFogStrength")->AsScalar();
		if (!m_pFogStrengthVariable->IsValid())
		{
			Logger::LogWarning(L"PostFog::LoadEffectVariables() > \'gFogStrength\' variable not found!");
			m_pFogStrengthVariable = nullptr;
		}
	}

	if (!m_pProjInvMatrixVariable)
	{
		m_pProjInvMatrixVariable = GetEffect()->GetVariableByName("gMatrixProjInverse")->AsMatrix();
		if (!m_pProjInvMatrixVariable->IsValid())
		{
			Logger::LogWarning(L"PostFog::LoadEffectVariables() > \'gMatrixProjInverse\' variable not found!");
			m_pProjInvMatrixVariable = nullptr;
		}
	}

	

}

void PostFog::UpdateEffectVariables(const GameContext& gameContext, RenderTarget* pPrevRendertarget, RenderTarget* pOriginalRendertarget)
{
	//TODO: Update the TextureMapVariable with the Color ShaderResourceView of the given RenderTarget
	if (m_pTextureMapVariabele)
	{
		m_pTextureMapVariabele->SetResource(pPrevRendertarget->GetShaderResourceView());
	}
	if (m_pDepthMapVariable)
	{
		m_pDepthMapVariable->SetResource(pOriginalRendertarget->GetDepthShaderResourceView());
	}
	if (m_pFogColorVariable)
	{
		m_pFogColorVariable->SetFloatVector(&m_FogColor.x);
	}
	if (m_pFogFalloffVariable)
	{
		m_pFogFalloffVariable->SetFloat(m_FogFalloff);
	}
	if (m_pFogStrengthVariable)
	{
		m_pFogStrengthVariable->SetFloat(m_FogStrength);
	}
	if (m_pProjInvMatrixVariable)
	{
		auto projectionInv = DirectX::XMLoadFloat4x4(&gameContext.pCamera->GetProjectionInverse());
		m_pProjInvMatrixVariable->SetMatrix(reinterpret_cast<float*>(&projectionInv));
	}
}