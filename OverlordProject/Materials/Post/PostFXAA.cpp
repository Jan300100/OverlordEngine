#include "stdafx.h"
#include "PostFXAA.h"
#include "RenderTarget.h"

PostFXAA::PostFXAA()
	: PostProcessingMaterial(L"./Resources/Effects/Post/Fxaa.fx", 2),
	m_pTextureMapVariabele(nullptr) 
{
}

void PostFXAA::LoadEffectVariables()
{
	//TODO: Bind the 'gTexture' variable with 'm_pTextureMapVariable'
	//Check if valid!
	m_pTextureMapVariabele = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();
	if (!m_pTextureMapVariabele->IsValid())
	{
		Logger::LogWarning(L"PostFXAA::LoadEffectVariables() > \'gTexture\' variable not found!");
		m_pTextureMapVariabele = nullptr;
	}
}

void PostFXAA::UpdateEffectVariables(const GameContext& gameContext, RenderTarget* pPrevRendertarget, RenderTarget* pOriginalRendertarget)
{
	UNREFERENCED_PARAMETER(pOriginalRendertarget);
	UNREFERENCED_PARAMETER(gameContext);

	//TODO: Update the TextureMapVariable with the Color ShaderResourceView of the given RenderTarget
	if (m_pTextureMapVariabele)
	{
		m_pTextureMapVariabele->SetResource(pPrevRendertarget->GetShaderResourceView());
	}
}
