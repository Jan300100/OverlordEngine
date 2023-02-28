#include "stdafx.h"
#include "PostBlur.h"
#include "RenderTarget.h"

PostBlur::PostBlur()
	: PostProcessingMaterial(L"./Resources/Effects/Post/Blur.fx", 2),
	m_pTextureMapVariabele(nullptr)
{
}

void PostBlur::LoadEffectVariables()
{
	//TODO: Bind the 'gTexture' variable with 'm_pTextureMapVariable'
	//Check if valid!
	m_pTextureMapVariabele = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();
	if (!m_pTextureMapVariabele->IsValid())
	{
		Logger::LogWarning(L"PostBlur:LoadEffectVariables() > \'gTexture\' variable not found!");
		m_pTextureMapVariabele = nullptr;
	}
}

void PostBlur::UpdateEffectVariables(const GameContext& gameContext,RenderTarget* pPrevRendertarget, RenderTarget* pOriginalRendertarget)
{
	UNREFERENCED_PARAMETER(pOriginalRendertarget);
	UNREFERENCED_PARAMETER(gameContext);

	//TODO: Update the TextureMapVariable with the Color ShaderResourceView of the given RenderTarget
	if (m_pTextureMapVariabele)
	{
		m_pTextureMapVariabele->SetResource(pPrevRendertarget->GetShaderResourceView());
	}
}