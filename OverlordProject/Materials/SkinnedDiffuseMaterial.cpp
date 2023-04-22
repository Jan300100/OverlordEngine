#include "stdafx.h"

#include "SkinnedDiffuseMaterial.h"

#include "ContentManager.h"
#include "ModelComponent.h"
#include "ModelAnimator.h"

// todo: dx11
#include <GA/DX11/Texture2DDX11.h>

ID3DX11EffectShaderResourceVariable* SkinnedDiffuseMaterial::m_pDiffuseSRVvariable = nullptr;
ID3DX11EffectMatrixVariable* SkinnedDiffuseMaterial::m_pBoneTransforms = nullptr;

SkinnedDiffuseMaterial::SkinnedDiffuseMaterial() : Material(L"./Resources/Effects/PosNormTex3D_Skinned.fx"),
	m_pDiffuseTexture(nullptr)
{}

void SkinnedDiffuseMaterial::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<GA::Texture2D>(assetFile).get();
}

void SkinnedDiffuseMaterial::LoadEffectVariables()
{
	if (!m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable = GetEffect()->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseSRVvariable->IsValid())
		{
			Logger::LogWarning(L"SkinnedDiffuseMaterial::LoadEffectVariables() > \'gDiffuseMap\' variable not found!");
			m_pDiffuseSRVvariable = nullptr;
		}
	}

	//TODO: Create a link to the gBones variable
	if (!m_pBoneTransforms)
	{
		m_pBoneTransforms = GetEffect()->GetVariableByName("gBones")->AsMatrix();
		if (!m_pBoneTransforms->IsValid())
		{
			Logger::LogWarning(L"SkinnedDiffuseMaterial::LoadEffectVariables() > \'gBones\' variable not found!");
			m_pBoneTransforms = nullptr;
		}
	}
}

void SkinnedDiffuseMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(pModelComponent);

	if (m_pDiffuseTexture && m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable->SetResource(GA::DX11::SafeCast(m_pDiffuseTexture)->GetSRV());
	}



	//Set the matrix array (BoneTransforms of the ModelAnimator)}
	if (m_pBoneTransforms)
	{
		std::vector<DirectX::XMFLOAT4X4> transforms = pModelComponent->GetAnimator()->GetBoneTransforms();
		m_pBoneTransforms->SetMatrixArray((const float*)transforms.data(), 0, static_cast<uint32_t>(transforms.size()));
	}
}