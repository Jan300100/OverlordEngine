#include "stdafx.h"
#include "TRSkinnedMaterial.h"
#include "ModelComponent.h"
#include "ModelAnimator.h"

ID3DX11EffectMatrixVariable* TRSkinnedMaterial::m_pBoneTransforms = nullptr;

TRSkinnedMaterial::TRSkinnedMaterial()
	:TRMaterial{ L"./Resources/Effects/TR/TRDefaultSkinned.fx", L"tDefault", false }
{
}

void TRSkinnedMaterial::LoadEffectVariables()
{
	PIX_PROFILE();

	TRMaterial::LoadEffectVariables();

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

void TRSkinnedMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	PIX_PROFILE();

	TRMaterial::UpdateEffectVariables(gameContext, pModelComponent);

	//Set the matrix array (BoneTransforms of the ModelAnimator)}
	if (m_pBoneTransforms)
	{
		std::vector<DirectX::XMFLOAT4X4> transforms = pModelComponent->GetAnimator()->GetBoneTransforms();
		m_pBoneTransforms->SetMatrixArray((const float*)transforms.data(), 0, static_cast<uint32_t>(transforms.size()));
	}

}
