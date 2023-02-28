#include "stdafx.h"
#include "PostGrayscale.h"
#include "RenderTarget.h"

PostGrayscale::PostGrayscale()
	: PostProcessingMaterial(L"./Resources/Effects/Post/Grayscale.fx", 1),
	m_pTextureMapVariabele(nullptr)
{
}

void PostGrayscale::LoadEffectVariables()
{
	//TODO: Bind the 'gTexture' variable with 'm_pTextureMapVariable'
	//Check if valid!
	m_pTextureMapVariabele = GetEffect()->GetVariableByName("gTexture")->AsShaderResource();
	if (!m_pTextureMapVariabele->IsValid())
	{
		Logger::LogWarning(L"PostGrayscale::LoadEffectVariables() > \'gTexture\' variable not found!");
		m_pTextureMapVariabele = nullptr;
	}
}

void PostGrayscale::UpdateEffectVariables(const GameContext& gameContext, RenderTarget* pPrevRendertarget, RenderTarget* pOriginalRendertarget)
{
	UNREFERENCED_PARAMETER(pOriginalRendertarget);
	UNREFERENCED_PARAMETER(gameContext);
	//TODO: Update the TextureMapVariable with the Color ShaderResourceView of the given RenderTarget
	if (m_pTextureMapVariabele)
	{
		m_pTextureMapVariabele->SetResource(pPrevRendertarget->GetShaderResourceView());
	}
}