#include <stdafx.h>
#include "TRMaterial.h"
#include <ContentManager.h>

// todo: dx11
#include <GA/DX11/Texture2DDX11.h>

DirectX::XMFLOAT3 TRMaterial::m_LightDirection = DirectX::XMFLOAT3{ -0.577f, -0.577f, 0.577f };
float TRMaterial::m_LightIntensity = 5.0f;

TRMaterial::TRMaterial(const wstring& effectPath, const wstring& defaultTechnique, bool usesTesselation, bool generateShadows)
	:Material{effectPath, defaultTechnique, usesTesselation}
	, m_pAlbedoTexture{ nullptr }
	, m_bUseAlbedoTexture{ false }
	, m_AlbedoColor{ 1.f,1.f,1.f }

	, m_pRDAMTexture{ nullptr }

	, m_bUseAOMap{ false }
	, m_AoStrength{1.0f}

	, m_bFlipGreenChannel{ false }
	, m_pNormalMap{ nullptr }
	, m_bUseNormalMap{ false }

	, m_IsMetal{ false }
	, m_bUseMetalnessMap{ false }

	, m_bUseRoughnessMap{ false }
	, m_Roughness{ 0.25f }

	, m_pRDAMSRVvariable{nullptr}
	, m_pNormalMapSRVvariable{nullptr}
	, m_pAlbedoSRVvariable{nullptr}
	, m_pFlipGreenChannelVariable{nullptr}
	, m_pUseAoMapVariable{nullptr}
	, m_pUseAlbedoTextureVariable{nullptr}
	, m_pIsMetalVariable{nullptr}
	, m_pUseMetalnessMapVariable{nullptr}
	, m_pAlbedoColorVariable{ nullptr }
	, m_pLightDirectionVariable{ nullptr }
	, m_pUseNormalMapVariable{ nullptr }
	, m_pUseRoughnessMapVariable{ nullptr }
	, m_pRoughnessVariable{ nullptr }
	, m_pAoStrengthVariable{nullptr}
	, m_pLightIntensityVariable{ nullptr }

	, m_pShadowTechnique{nullptr}
	, m_GenerateShadows{generateShadows}
	, m_pShadowSRVvariable{ nullptr }
	, m_pLightVPVariable{ nullptr }
{
}

TRMaterial::TRMaterial()
	:TRMaterial{L"./Resources/Effects/TR/TRDefault.fx", L"tDefault", false, false}
{
}



void TRMaterial::SetRDAM(const wstring& id, bool roughness, bool displacement, bool ao, bool metalness, bool albedo, bool normal, const wstring& ext)
{
	PIX_PROFILE();

	SetUseAOMap(ao);
	SetUseMetalnessMap(metalness);
	SetUseRoughnessMap(roughness);
	if (albedo)
	{
		SetAlbedoTexture(L"./Resources/Textures/TR/" + id + L"_Albedo." + ext);
	}
	if (normal)
	{
		SetNormalMap(L"./Resources/Textures/TR/" + id + L"_Normal." + ext);
	}
	if (roughness || metalness || ao || displacement)
	{
		m_pRDAMTexture = ContentManager::Load<GA::Texture2D>(L"./Resources/Textures/TR/" + id + L"_RoughDispAO." + ext).get();
	}
}

void TRMaterial::SetLightDirection(XMFLOAT3 direction)
{
	m_LightDirection = direction;
}

void TRMaterial::SetLightIntensity(float intensity)
{
	m_LightIntensity = intensity;
}

void TRMaterial::SetAlbedoTexture(const wstring& assetFile)
{
	SetUseAlbedo(true);
	m_pAlbedoTexture = ContentManager::Load<GA::Texture2D>(assetFile).get();
}


void TRMaterial::SetUseAOMap(bool useAOMap)
{
	m_bUseAOMap = useAOMap;
}

void TRMaterial::SetAoStrength(float strength)
{
	m_AoStrength = strength;
}


void TRMaterial::SetMetal(bool metal)
{
	m_IsMetal = metal;
}

void TRMaterial::SetAlbedoColor(XMFLOAT3 color)
{
	m_AlbedoColor = color;
}

void TRMaterial::SetUseAlbedo(bool enable)
{
	m_bUseAlbedoTexture = enable;
}

void TRMaterial::SetUseMetalnessMap(bool useMetalnessMap)
{
	m_bUseMetalnessMap = useMetalnessMap;
}

void TRMaterial::SetUseRoughnessMap(bool use)
{
	m_bUseRoughnessMap = use;
}

void TRMaterial::SetRoughness(float roughness)
{
	m_Roughness = roughness;
}

void TRMaterial::FlipNormalGreenChannel(bool flip)
{
	m_bFlipGreenChannel = flip;
}

void TRMaterial::SetUseNormalMap(bool use)
{
	m_bUseNormalMap = use;
}

void TRMaterial::SetNormalMap(const wstring& assetFile)
{
	SetUseNormalMap(true);
	m_pNormalMap = ContentManager::Load<GA::Texture2D>(assetFile).get();
}

void TRMaterial::LoadEffectVariables()
{
	PIX_PROFILE();

	//lights
	if (!m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable = GetEffect()->GetVariableByName("gLightDirection")->AsVector();
		if (!m_pLightDirectionVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gLightDirection\' variable not found!");
			m_pLightDirectionVariable = nullptr;
		}
	}
	if (!m_pLightIntensityVariable)
	{
		m_pLightIntensityVariable = GetEffect()->GetVariableByName("gLightIntensity")->AsScalar();
		if (!m_pLightIntensityVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gLightIntensity\' variable not found!");
			m_pLightIntensityVariable = nullptr;
		}
	}
	//Albedo
	if (!m_pAlbedoSRVvariable)
	{
		m_pAlbedoSRVvariable = GetEffect()->GetVariableByName("gAlbedoTexture")->AsShaderResource();
		if (!m_pAlbedoSRVvariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gAlbedoTexture\' variable not found!");
			m_pAlbedoSRVvariable = nullptr;
		}
	}
	if (!m_pAlbedoColorVariable)
	{
		m_pAlbedoColorVariable = GetEffect()->GetVariableByName("gAlbedoColor")->AsVector();
		if (!m_pAlbedoColorVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gAlbedoColor\' variable not found!");
			m_pAlbedoColorVariable = nullptr;
		}
	}
	if (!m_pUseAlbedoTextureVariable)
	{
		m_pUseAlbedoTextureVariable = GetEffect()->GetVariableByName("gUseAlbedoTexture")->AsScalar();
		if (!m_pUseAlbedoTextureVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gUseAlbedoTexture\' variable not found!");
			m_pUseAlbedoTextureVariable = nullptr;
		}
	}

	//RDAM
	if (!m_pRDAMSRVvariable)
	{
		m_pRDAMSRVvariable = GetEffect()->GetVariableByName("gRDAM")->AsShaderResource();
		if (!m_pRDAMSRVvariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gRDAM\' variable not found!");
			m_pRDAMSRVvariable = nullptr;
		}
	}

	//Ambient Occlusion
	if (!m_pUseAoMapVariable)
	{
		m_pUseAoMapVariable = GetEffect()->GetVariableByName("gUseAO")->AsScalar();
		if (!m_pUseAoMapVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gUseAO\' variable not found!");
			m_pUseAoMapVariable = nullptr;
		}
	}
	if (!m_pAoStrengthVariable)
	{
		m_pAoStrengthVariable = GetEffect()->GetVariableByName("gAoStrength")->AsScalar();
		if (!m_pAoStrengthVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gAoStrength\' variable not found!");
			m_pAoStrengthVariable = nullptr;
		}
	}
	
	//ROUGHNESS
	if (!m_pUseRoughnessMapVariable)
	{
		m_pUseRoughnessMapVariable = GetEffect()->GetVariableByName("gUseRoughnessMap")->AsScalar();
		if (!m_pUseRoughnessMapVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gUseRoughnessMap\' variable not found!");
			m_pUseRoughnessMapVariable = nullptr;
		}
	}
	if (!m_pRoughnessVariable)
	{
		m_pRoughnessVariable = GetEffect()->GetVariableByName("gRoughness")->AsScalar();
		if (!m_pRoughnessVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gRoughness\' variable not found!");
			m_pRoughnessVariable = nullptr;
		}
	}

	//METAL
	if (!m_pIsMetalVariable)
	{
		m_pIsMetalVariable = GetEffect()->GetVariableByName("gIsMetal")->AsScalar();
		if (!m_pIsMetalVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gIsMetal\' variable not found!");
			m_pIsMetalVariable = nullptr;
		}
	}
	if (!m_pUseMetalnessMapVariable)
	{
		m_pUseMetalnessMapVariable = GetEffect()->GetVariableByName("gUseMetalnessMap")->AsScalar();
		if (!m_pUseMetalnessMapVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gUseMetalnessMap\' variable not found!");
			m_pUseMetalnessMapVariable = nullptr;
		}
	}

	//normal
	if (!m_pFlipGreenChannelVariable)
	{
		m_pFlipGreenChannelVariable = GetEffect()->GetVariableByName("gFlipGreenChannel")->AsScalar();
		if (!m_pFlipGreenChannelVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gFlipGreenChannel\' variable not found!");
			m_pFlipGreenChannelVariable = nullptr;
		}
	}
	if (!m_pUseNormalMapVariable)
	{
		m_pUseNormalMapVariable = GetEffect()->GetVariableByName("gUseNormalMap")->AsScalar();
		if (!m_pUseNormalMapVariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gUseNormalMap\' variable not found!");
			m_pUseNormalMapVariable = nullptr;
		}
	}

	if (!m_pNormalMapSRVvariable)
	{
		m_pNormalMapSRVvariable = GetEffect()->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapSRVvariable->IsValid())
		{
			Logger::LogWarning(L"TRMaterial::LoadEffectVariables() > \'gNormalMap\' variable not found!");
			m_pNormalMapSRVvariable = nullptr;
		}
	}

	if (!m_pLightVPVariable)
	{
		m_pLightVPVariable = GetEffect()->GetVariableByName("gViewProj_Light")->AsMatrix();
		if (!m_pLightVPVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial_Shadow::LoadEffectVariables() > \'gViewProj_Light\' variable not found!");
			m_pLightVPVariable = nullptr;
		}
	}

	if (!m_pShadowSRVvariable)
	{
		m_pShadowSRVvariable = GetEffect()->GetVariableByName("gShadowMap")->AsShaderResource();
		if (!m_pShadowSRVvariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial_Shadow::LoadEffectVariables() > \'gShadowMap\' variable not found!");
			m_pShadowSRVvariable = nullptr;
		}
	}
}

void TRMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);
	UNREFERENCED_PARAMETER(pModelComponent);

	//lights
	if (m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable->SetFloatVector(&m_LightDirection.x);
	}
	if (m_pLightIntensityVariable)
	{
		m_pLightIntensityVariable->SetFloat(m_LightIntensity);
	}

	//Albedo
	if (m_pAlbedoTexture && m_pAlbedoSRVvariable)
	{
		m_pAlbedoSRVvariable->SetResource(GA::DX11::SafeCast(m_pAlbedoTexture)->GetSRV());
	}
	if (m_pAlbedoColorVariable)
	{
		m_pAlbedoColorVariable->SetFloatVector(&m_AlbedoColor.x);
	}
	if (m_pUseAlbedoTextureVariable)
	{
		m_pUseAlbedoTextureVariable->SetBool(m_bUseAlbedoTexture);
	}
	//AO
	if (m_pUseAoMapVariable)
	{
		m_pUseAoMapVariable->SetBool(m_bUseAOMap);
	}
	if (m_pAoStrengthVariable)
	{
		m_pAoStrengthVariable->SetFloat(m_AoStrength);
	}
	//rdam
	if (m_pRDAMSRVvariable && m_pRDAMTexture)
	{
		m_pRDAMSRVvariable->SetResource(GA::DX11::SafeCast(m_pRDAMTexture)->GetSRV());
	}


	//Metal

	if (m_pIsMetalVariable)
	{
		m_pIsMetalVariable->SetBool(m_IsMetal);
	}
	if (m_pUseMetalnessMapVariable)
	{
		m_pUseMetalnessMapVariable->SetBool(m_bUseMetalnessMap);
	}

	//roughness
	if (m_pRoughnessVariable)
	{
		m_pRoughnessVariable->SetFloat(m_Roughness);
	}
	if (m_pUseRoughnessMapVariable)
	{
		m_pUseRoughnessMapVariable->SetBool(m_bUseRoughnessMap);
	}

	//normal
	if (m_pFlipGreenChannelVariable)
	{
		m_pFlipGreenChannelVariable->SetBool(m_bFlipGreenChannel);
	}
	if (m_pUseNormalMapVariable)
	{
		m_pUseNormalMapVariable->SetBool(m_bUseNormalMap);
	}
	if (m_pNormalMap && m_pNormalMapSRVvariable)
	{
		m_pNormalMapSRVvariable->SetResource(GA::DX11::SafeCast(m_pNormalMap)->GetSRV());
	}

	//shadows
	if ((m_pShadowSRVvariable))
	{
		m_pShadowSRVvariable->SetResource(gameContext.pShadowMapper->GetShadowMap());
	}

	if (m_pLightVPVariable)
	{
		DirectX::XMFLOAT4X4 lightVP = gameContext.pShadowMapper->GetLightVP();
		DirectX::XMMATRIX lvp = DirectX::XMLoadFloat4x4(&lightVP);
		m_pLightVPVariable->SetMatrix(reinterpret_cast<const float*>(&(lvp)));
	}

}

ID3DX11EffectTechnique* TRMaterial::GetShadowTechnique()
{
	PIX_PROFILE();

	if (m_GenerateShadows == false) return nullptr;
	if (m_pShadowTechnique == nullptr)
	{
		m_pShadowTechnique = GetTechnique("tGenerateShadowsTechnique");
	}
	return 	m_pShadowTechnique;
}



