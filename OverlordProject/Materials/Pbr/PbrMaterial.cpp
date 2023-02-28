#include <stdafx.h>
#include "PbrMaterial.h"
#include <ContentManager.h>
#include "TextureData.h"


DirectX::XMFLOAT3 PbrMaterial::m_LightDirection = DirectX::XMFLOAT3{ -0.577f, -0.577f, 0.577f };
float PbrMaterial::m_LightIntensity = 5.0f;
TextureData* PbrMaterial::m_pEnvironmentCube = nullptr;

PbrMaterial::PbrMaterial(const wstring& effectPath, const wstring& defaultTechnique, bool usesTesselation, bool generateShadows)
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

	, m_RefractionIndex{ 0.3f }
	, m_bEnvironmentMapping{true}


	, m_pEnvironmentSRVvariable{nullptr}
	, m_pRDAMSRVvariable{nullptr}
	, m_pNormalMapSRVvariable{nullptr}
	, m_pAlbedoSRVvariable{nullptr}
	, m_pFlipGreenChannelVariable{nullptr}
	, m_pUseAoMapVariable{nullptr}
	, m_pUseAlbedoTextureVariable{nullptr}
	, m_pRefractionIndexVariable{nullptr}
	, m_pUseEnvironmentMappingVariable{nullptr}
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
{
}

PbrMaterial::PbrMaterial()
	:PbrMaterial{L"./Resources/Effects/Pbr/PbrDefault.fx", L"tDefault", false, false}
{
}


void PbrMaterial::SetRDAM(const wstring& id, bool roughness, bool displacement, bool ao, bool metalness, bool albedo, bool normal, const wstring& ext)
{
	SetUseAOMap(ao);
	SetUseMetalnessMap(metalness);
	SetUseRoughnessMap(roughness);
	if (albedo)
	{
		SetAlbedoTexture(L"./Resources/Textures/PBR/" + id + L"_Albedo." + ext);
	}
	if (normal)
	{
		SetNormalMap(L"./Resources/Textures/PBR/" + id + L"_Normal." + ext);
	}
	if (roughness || metalness || ao || displacement)
	{
		m_pRDAMTexture = ContentManager::Load<TextureData>(L"./Resources/Textures/PBR/" + id + L"_RoughDispAO." + ext);
	}
}

void PbrMaterial::SetLightDirection(XMFLOAT3 direction)
{
	m_LightDirection = direction;
}

void PbrMaterial::SetLightIntensity(float intensity)
{
	m_LightIntensity = intensity;
}

void PbrMaterial::SetAlbedoTexture(const wstring& assetFile)
{
	SetUseAlbedo(true);
	m_pAlbedoTexture = ContentManager::Load<TextureData>(assetFile);
}


void PbrMaterial::SetUseAOMap(bool useAOMap)
{
	m_bUseAOMap = useAOMap;
}

void PbrMaterial::SetAoStrength(float strength)
{
	m_AoStrength = strength;
}


void PbrMaterial::SetMetal(bool metal)
{
	m_IsMetal = metal;
}

void PbrMaterial::SetAlbedoColor(XMFLOAT3 color)
{
	m_AlbedoColor = color;
}

void PbrMaterial::SetUseAlbedo(bool enable)
{
	m_bUseAlbedoTexture = enable;
}

void PbrMaterial::SetUseMetalnessMap(bool useMetalnessMap)
{
	m_bUseMetalnessMap = useMetalnessMap;
}

void PbrMaterial::SetUseRoughnessMap(bool use)
{
	m_bUseRoughnessMap = use;
}

void PbrMaterial::SetRoughness(float roughness)
{
	m_Roughness = roughness;
}

void PbrMaterial::EnableEnvironmentMapping(bool enable)
{
	m_bEnvironmentMapping = enable;
}

void PbrMaterial::FlipNormalGreenChannel(bool flip)
{
	m_bFlipGreenChannel = flip;
}

void PbrMaterial::SetUseNormalMap(bool use)
{
	m_bUseNormalMap = use;
}

void PbrMaterial::SetNormalMap(const wstring& assetFile)
{
	SetUseNormalMap(true);
	m_pNormalMap = ContentManager::Load<TextureData>(assetFile);
}

void PbrMaterial::SetEnvironmentCube(const wstring& assetFile)
{
	m_pEnvironmentCube = ContentManager::Load<TextureData>(assetFile);
}

void PbrMaterial::SetRefractionIndex(float index)
{
	m_RefractionIndex = index;
}

void PbrMaterial::LoadEffectVariables()
{

	//lights
	if (!m_pLightDirectionVariable)
	{
		m_pLightDirectionVariable = GetEffect()->GetVariableByName("gLightDirection")->AsVector();
		if (!m_pLightDirectionVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gLightDirection\' variable not found!");
			m_pLightDirectionVariable = nullptr;
		}
	}
	if (!m_pLightIntensityVariable)
	{
		m_pLightIntensityVariable = GetEffect()->GetVariableByName("gLightIntensity")->AsScalar();
		if (!m_pLightIntensityVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gLightIntensity\' variable not found!");
			m_pLightIntensityVariable = nullptr;
		}
	}
	//Albedo
	if (!m_pAlbedoSRVvariable)
	{
		m_pAlbedoSRVvariable = GetEffect()->GetVariableByName("gAlbedoTexture")->AsShaderResource();
		if (!m_pAlbedoSRVvariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gAlbedoTexture\' variable not found!");
			m_pAlbedoSRVvariable = nullptr;
		}
	}
	if (!m_pAlbedoColorVariable)
	{
		m_pAlbedoColorVariable = GetEffect()->GetVariableByName("gAlbedoColor")->AsVector();
		if (!m_pAlbedoColorVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gAlbedoColor\' variable not found!");
			m_pAlbedoColorVariable = nullptr;
		}
	}
	if (!m_pUseAlbedoTextureVariable)
	{
		m_pUseAlbedoTextureVariable = GetEffect()->GetVariableByName("gUseAlbedoTexture")->AsScalar();
		if (!m_pUseAlbedoTextureVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gUseAlbedoTexture\' variable not found!");
			m_pUseAlbedoTextureVariable = nullptr;
		}
	}

	//RDAM
	if (!m_pRDAMSRVvariable)
	{
		m_pRDAMSRVvariable = GetEffect()->GetVariableByName("gRDAM")->AsShaderResource();
		if (!m_pRDAMSRVvariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gRDAM\' variable not found!");
			m_pRDAMSRVvariable = nullptr;
		}
	}

	//Ambient Occlusion
	if (!m_pUseAoMapVariable)
	{
		m_pUseAoMapVariable = GetEffect()->GetVariableByName("gUseAO")->AsScalar();
		if (!m_pUseAoMapVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gUseAO\' variable not found!");
			m_pUseAoMapVariable = nullptr;
		}
	}
	if (!m_pAoStrengthVariable)
	{
		m_pAoStrengthVariable = GetEffect()->GetVariableByName("gAoStrength")->AsScalar();
		if (!m_pAoStrengthVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gAoStrength\' variable not found!");
			m_pAoStrengthVariable = nullptr;
		}
	}
	
	//ROUGHNESS
	if (!m_pUseRoughnessMapVariable)
	{
		m_pUseRoughnessMapVariable = GetEffect()->GetVariableByName("gUseRoughnessMap")->AsScalar();
		if (!m_pUseRoughnessMapVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gUseRoughnessMap\' variable not found!");
			m_pUseRoughnessMapVariable = nullptr;
		}
	}
	if (!m_pRoughnessVariable)
	{
		m_pRoughnessVariable = GetEffect()->GetVariableByName("gRoughness")->AsScalar();
		if (!m_pRoughnessVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gRoughness\' variable not found!");
			m_pRoughnessVariable = nullptr;
		}
	}

	//METAL
	if (!m_pIsMetalVariable)
	{
		m_pIsMetalVariable = GetEffect()->GetVariableByName("gIsMetal")->AsScalar();
		if (!m_pIsMetalVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gIsMetal\' variable not found!");
			m_pIsMetalVariable = nullptr;
		}
	}
	if (!m_pUseMetalnessMapVariable)
	{
		m_pUseMetalnessMapVariable = GetEffect()->GetVariableByName("gUseMetalnessMap")->AsScalar();
		if (!m_pUseMetalnessMapVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gUseMetalnessMap\' variable not found!");
			m_pUseMetalnessMapVariable = nullptr;
		}
	}

	//normal
	if (!m_pFlipGreenChannelVariable)
	{
		m_pFlipGreenChannelVariable = GetEffect()->GetVariableByName("gFlipGreenChannel")->AsScalar();
		if (!m_pFlipGreenChannelVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gFlipGreenChannel\' variable not found!");
			m_pFlipGreenChannelVariable = nullptr;
		}
	}
	if (!m_pUseNormalMapVariable)
	{
		m_pUseNormalMapVariable = GetEffect()->GetVariableByName("gUseNormalMap")->AsScalar();
		if (!m_pUseNormalMapVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gUseNormalMap\' variable not found!");
			m_pUseNormalMapVariable = nullptr;
		}
	}

	if (!m_pNormalMapSRVvariable)
	{
		m_pNormalMapSRVvariable = GetEffect()->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapSRVvariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gNormalMap\' variable not found!");
			m_pNormalMapSRVvariable = nullptr;
		}
	}

	//environment
	if (!m_pUseEnvironmentMappingVariable)
	{
		m_pUseEnvironmentMappingVariable = GetEffect()->GetVariableByName("gUseTextureEnvironment")->AsScalar();
		if (!m_pUseEnvironmentMappingVariable->IsValid())
		{
			Logger::LogWarning(L"UberMaterial::LoadEffectVariables() > \'gUseTextureEnvironment\' variable not found!");
			m_pUseEnvironmentMappingVariable = nullptr;
		}
	}
	if (!m_pEnvironmentSRVvariable)
	{
		m_pEnvironmentSRVvariable = GetEffect()->GetVariableByName("gCubeEnvironment")->AsShaderResource();
		if (!m_pEnvironmentSRVvariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gCubeEnvironment\' variable not found!");
			m_pEnvironmentSRVvariable = nullptr;
		}
	}
	if (!m_pRefractionIndexVariable)
	{
		m_pRefractionIndexVariable = GetEffect()->GetVariableByName("gRefractionIndex")->AsScalar();
		if (!m_pRefractionIndexVariable->IsValid())
		{
			Logger::LogWarning(L"PbrMaterial::LoadEffectVariables() > \'gRefractionIndex\' variable not found!");
			m_pRefractionIndexVariable = nullptr;
		}
	}
}

void PbrMaterial::UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent)
{
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
		m_pAlbedoSRVvariable->SetResource(m_pAlbedoTexture->GetShaderResourceView());
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
		m_pRDAMSRVvariable->SetResource(m_pRDAMTexture->GetShaderResourceView());
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
		m_pNormalMapSRVvariable->SetResource(m_pNormalMap->GetShaderResourceView());
	}

	//environment
	if (m_pUseEnvironmentMappingVariable)
	{
		m_pUseEnvironmentMappingVariable->SetBool(m_bEnvironmentMapping);
	}
	if (m_pEnvironmentCube && m_pEnvironmentSRVvariable)
	{
		m_pEnvironmentSRVvariable->SetResource(m_pEnvironmentCube->GetShaderResourceView());
	}
	if (m_pRefractionIndexVariable)
	{
		m_pRefractionIndexVariable->SetFloat(m_RefractionIndex);
	}
}

ID3DX11EffectTechnique* PbrMaterial::GetShadowTechnique()
{
	if (m_GenerateShadows == false) return nullptr;
	if (m_pShadowTechnique == nullptr)
	{
		m_pShadowTechnique = GetTechnique("tGenerateShadowsTechnique");
	}
	return 	m_pShadowTechnique;
}



