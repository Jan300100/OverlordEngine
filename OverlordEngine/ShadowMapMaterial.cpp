//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ShadowMapMaterial.h"
#include "ContentManager.h"
#include <GA/DX11/InterfaceDX11.h>

ShadowMapMaterial::~ShadowMapMaterial()
{
	m_pInputLayouts[ShadowGenType::Static]->Release();
	m_pInputLayouts[ShadowGenType::Skinned]->Release();
}

void ShadowMapMaterial::Initialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (!m_IsInitialized)
	{
		//Load Effect
		m_pShadowEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/Shadow/ShadowMapGenerator.fx");


		//effects
		m_pShadowTechs[ShadowGenType::Static] = m_pShadowEffect->GetTechniqueByName("GenerateShadows");
		m_pShadowTechs[ShadowGenType::Skinned] = m_pShadowEffect->GetTechniqueByName("GenerateShadows_Skinned");


		//input layouts
		EffectHelper::BuildInputLayout(GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice(), m_pShadowTechs[ShadowGenType::Static], &m_pInputLayouts[ShadowGenType::Static], m_InputLayoutDescriptions[ShadowGenType::Static], m_InputLayoutSizes[ShadowGenType::Static], m_InputLayoutIds[ShadowGenType::Static]);
		EffectHelper::BuildInputLayout(GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice(), m_pShadowTechs[ShadowGenType::Skinned], &m_pInputLayouts[ShadowGenType::Skinned], m_InputLayoutDescriptions[ShadowGenType::Skinned], m_InputLayoutSizes[ShadowGenType::Skinned], m_InputLayoutIds[ShadowGenType::Skinned]);
	
		//shader variables
		auto effectVar = m_pShadowEffect->GetVariableByName("gWorld");
		m_pWorldMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
		effectVar = m_pShadowEffect->GetVariableByName("gLightViewProj");
		m_pLightVPMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
		effectVar = m_pShadowEffect->GetVariableByName("gBones");
		m_pBoneTransforms = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
	
	}
}

void ShadowMapMaterial::SetLightVP(DirectX::XMFLOAT4X4 lightVP) const
{
	if (m_pLightVPMatrixVariable)
	{
		m_pLightVPMatrixVariable->SetMatrix(reinterpret_cast<float*>(&lightVP));
	}
}

void ShadowMapMaterial::SetWorld(DirectX::XMFLOAT4X4 world) const
{
	if (m_pWorldMatrixVariable)
	{
		m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<float*>(&world));
	}
}

void ShadowMapMaterial::SetBones(const float* pData, int count) const
{
	if (m_pBoneTransforms)
	{
		m_pBoneTransforms->SetMatrixArray(pData, 0, count);
	}
}
