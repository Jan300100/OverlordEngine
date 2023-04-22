#include "stdafx.h"

#include "DiffuseMaterial_Shadow.h"
#include "GeneralStructs.h"
#include "Logger.h"
#include "ContentManager.h"
#include "Components.h"

// todo: dx11
#include <GA/DX11/Texture2DDX11.h>

ID3DX11EffectShaderResourceVariable* DiffuseMaterial_Shadow::m_pDiffuseSRVvariable = nullptr;
ID3DX11EffectShaderResourceVariable* DiffuseMaterial_Shadow::m_pShadowSRVvariable = nullptr;
ID3DX11EffectVectorVariable* DiffuseMaterial_Shadow::m_pLightDirectionVariable = nullptr;
ID3DX11EffectMatrixVariable* DiffuseMaterial_Shadow::m_pLightWVPvariable = nullptr;

DiffuseMaterial_Shadow::DiffuseMaterial_Shadow() : Material(L"./Resources/Effects/Shadow/PosNormTex3D_Shadow.fx"),
	m_pDiffuseTexture(nullptr)
{}

void DiffuseMaterial_Shadow::SetDiffuseTexture(const std::wstring& assetFile)
{
	m_pDiffuseTexture = ContentManager::Load<GA::Texture2D>(assetFile).get();

}

void DiffuseMaterial_Shadow::SetLightDirection(DirectX::XMFLOAT3 dir)
{
	m_LightDirection = dir;
}

void DiffuseMaterial_Shadow::LoadEffectVariables()
{
	//TODO: load all the necessary shader variables
	if (!m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable = GetEffect()->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial::LoadEffectVariables() > \'gDiffuseMap\' variable not found!");
			m_pDiffuseSRVvariable = nullptr;
		}
	}

	if (!m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable = GetEffect()->GetVariableByName("gLightDirection")->AsVector();
		if (!m_pLightDirectionVariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial_Shadow::LoadEffectVariables() > \'gLightDirection\' variable not found!");
			m_pLightDirectionVariable = nullptr;
		}
	}

	if (!m_pLightWVPvariable)
	{
		m_pLightWVPvariable = GetEffect()->GetVariableByName("gWorldViewProj_Light")->AsMatrix();
		if (!m_pLightWVPvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial_Shadow::LoadEffectVariables() > \'gWorldViewProj_Light\' variable not found!");
			m_pLightWVPvariable = nullptr;
		}
	}

	if (!m_pShadowSRVvariable)
	{
		m_pShadowSRVvariable = GetEffect()->GetVariableByName("gShadowMap")->AsShaderResource();
		if (!m_pShadowSRVvariable->IsValid())
		{
			Logger::LogWarning(L"DiffuseMaterial_Shadow::LoadEffectVariables() > \'gShadowMap\' variable not found!");
			m_pShadowSRVvariable = nullptr;
		}
	}

}

void DiffuseMaterial_Shadow::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	UNREFERENCED_PARAMETER(pModelComponent);
	UNREFERENCED_PARAMETER(gameContext);
	if (m_pDiffuseTexture && m_pDiffuseSRVvariable)
	{
		m_pDiffuseSRVvariable->SetResource(GA::DX11::SafeCast(m_pDiffuseTexture)->GetSRV());
	}

	if (m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable->SetFloatVector(&m_LightDirection.x);
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