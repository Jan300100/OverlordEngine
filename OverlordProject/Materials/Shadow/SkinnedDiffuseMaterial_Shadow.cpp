//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "SkinnedDiffuseMaterial_Shadow.h"
#include "GeneralStructs.h"
#include "Logger.h"
#include "ContentManager.h"
#include "ModelComponent.h"
#include "ModelAnimator.h"
#include "Components.h"

// todo: dx11
#include <GA/DX11/Texture2DDX11.h>

ID3DX11EffectShaderResourceVariable* SkinnedDiffuseMaterial_Shadow::m_pDiffuseSRVvariable = nullptr;
ID3DX11EffectMatrixVariable* SkinnedDiffuseMaterial_Shadow::m_pBoneTransforms = nullptr;
ID3DX11EffectVectorVariable* SkinnedDiffuseMaterial_Shadow::m_pLightDirectionVariable = nullptr;
ID3DX11EffectShaderResourceVariable* SkinnedDiffuseMaterial_Shadow::m_pShadowSRVvariable = nullptr;
ID3DX11EffectMatrixVariable* SkinnedDiffuseMaterial_Shadow::m_pLightWVPvariable = nullptr;

SkinnedDiffuseMaterial_Shadow::SkinnedDiffuseMaterial_Shadow() : Material(L"./Resources/Effects/Shadow/PosNormTex3D_Skinned_Shadow.fx"),
	m_pDiffuseTexture(nullptr)
{}

void SkinnedDiffuseMaterial_Shadow::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<GA::Texture2D>(assetFile).get();

}

void SkinnedDiffuseMaterial_Shadow::SetLightDirection(DirectX::XMFLOAT3 dir)
{
	m_LightDirection = dir;
}

void SkinnedDiffuseMaterial_Shadow::LoadEffectVariables()
{
	//TODO: load all the necessary shader variables
	if (!m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable = GetEffect()->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseSRVvariable->IsValid())
		{
			Logger::LogWarning(L"SkinnedDiffuseMaterial_Shadow::LoadEffectVariables() > \'gDiffuseMap\' variable not found!");
			m_pDiffuseSRVvariable = nullptr;
		}
	}

	if (!m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable = GetEffect()->GetVariableByName("gLightDirection")->AsVector();
		if (!m_pLightDirectionVariable->IsValid())
		{
			Logger::LogWarning(L"SkinnedDiffuseMaterial_Shadow::LoadEffectVariables() > \'gLightDirection\' variable not found!");
			m_pLightDirectionVariable = nullptr;
		}
	}

	if (!m_pLightWVPvariable)
	{
		m_pLightWVPvariable = GetEffect()->GetVariableByName("gWorldViewProj_Light")->AsMatrix();
		if (!m_pLightWVPvariable->IsValid())
		{
			Logger::LogWarning(L"SkinnedDiffuseMaterial_Shadow::LoadEffectVariables() > \'gWorldViewProj_Light\' variable not found!");
			m_pLightWVPvariable = nullptr;
		}
	}

	if (!m_pShadowSRVvariable)
	{
		m_pShadowSRVvariable = GetEffect()->GetVariableByName("gShadowMap")->AsShaderResource();
		if (!m_pShadowSRVvariable->IsValid())
		{
			Logger::LogWarning(L"SkinnedDiffuseMaterial_Shadow::LoadEffectVariables() > \'gShadowMap\' variable not found!");
			m_pShadowSRVvariable = nullptr;
		}
	}

	if (!m_pBoneTransforms)
	{
		m_pBoneTransforms = GetEffect()->GetVariableByName("gBones")->AsMatrix();
		if (!m_pBoneTransforms->IsValid())
		{
			Logger::LogWarning(L"SkinnedDiffuseMaterial_Shadow::LoadEffectVariables() > \'gBones\' variable not found!");
			m_pBoneTransforms = nullptr;
		}
	}
}

void SkinnedDiffuseMaterial_Shadow::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	UNREFERENCED_PARAMETER(gameContext);


	if (m_pDiffuseTexture && m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable->SetResource(GA::DX11::SafeCast(m_pDiffuseTexture)->GetSRV());
	}

	if (m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable->SetFloatVector(&m_LightDirection.x);
	}

	//Set the matrix array (BoneTransforms of the ModelAnimator)}
	if (m_pBoneTransforms)
	{
		std::vector<DirectX::XMFLOAT4X4> transforms = pModelComponent->GetAnimator()->GetBoneTransforms();
		m_pBoneTransforms->SetMatrixArray((const float*)transforms.data(), 0, static_cast<uint32_t>(transforms.size()));
	}

	if ((m_pShadowSRVvariable))
	{
		m_pShadowSRVvariable->SetResource(gameContext.pShadowMapper->GetShadowMap());
	}


	if (m_pLightWVPvariable)
	{
		DirectX::XMFLOAT4X4 lightVP = gameContext.pShadowMapper->GetLightVP();

		DirectX::XMMATRIX w = XMLoadFloat4x4(&pModelComponent->GetTransform()->GetWorld());
		DirectX::XMMATRIX lvp = DirectX::XMLoadFloat4x4(&lightVP);

		DirectX::XMMATRIX wvp = w * lvp;

		m_pLightWVPvariable->SetMatrix(reinterpret_cast<const float*>(&(wvp)));
	}
}