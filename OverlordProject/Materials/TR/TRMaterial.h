#pragma once
#include <Material.h>

class GA::Texture2D;
using namespace DirectX;
using namespace std;

class TRMaterial :
public Material
{
protected:
	ID3DX11EffectTechnique* m_pShadowTechnique;
	bool m_GenerateShadows;
public:

	TRMaterial(const wstring& effectPath, const wstring& defaultTechnique = L"", bool usesTesselation = false, bool generateShadows = false);
	TRMaterial();
	~TRMaterial() = default;

	void SetRDAM(const wstring& id, bool roughness, bool displacement, bool ao, bool metalness, bool albedo, bool normal, const wstring& ext = L"png");

	//LIGHT
	//*****
	static void SetLightDirection(DirectX::XMFLOAT3 direction);
	static void SetLightIntensity(float intensity);
	//Albedo
	//*******
	void SetAlbedoTexture(const wstring& assetFile);
	void SetAlbedoColor(XMFLOAT3 color);
	void SetUseAlbedo(bool enable);
	//Ambient Occlusion
	//*****************
	void SetUseAOMap(bool useAOMap);
	void SetAoStrength(float strength);
	//SPECULAR
	//********
	void SetMetal(bool metal);
	void SetUseMetalnessMap(bool useMetalnessMap);
	void SetUseRoughnessMap(bool use);
	void SetRoughness(float roughness);
	//NORMAL MAPPING
	//**************
	void FlipNormalGreenChannel(bool flip);
	void SetUseNormalMap(bool use);
	void SetNormalMap(const wstring& assetFile);

	virtual void LoadEffectVariables();
	virtual void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent);

	virtual ID3DX11EffectTechnique* GetShadowTechnique() override;

private:
	//LIGHT
	//*****
	static XMFLOAT3 m_LightDirection;
	ID3DX11EffectVectorVariable* m_pLightDirectionVariable;
	static float m_LightIntensity;
	ID3DX11EffectScalarVariable* m_pLightIntensityVariable;
	//Albedo
	//*******
	bool m_bUseAlbedoTexture;
	ID3DX11EffectScalarVariable* m_pUseAlbedoTextureVariable;
	GA::Texture2D* m_pAlbedoTexture;
	ID3DX11EffectShaderResourceVariable* m_pAlbedoSRVvariable;
	XMFLOAT3 m_AlbedoColor;
	ID3DX11EffectVectorVariable* m_pAlbedoColorVariable;

	//RDAM packed texture
	GA::Texture2D* m_pRDAMTexture;
	ID3DX11EffectShaderResourceVariable* m_pRDAMSRVvariable;
	//ROUGHNESS
	//********
	float m_Roughness;
	ID3DX11EffectScalarVariable* m_pRoughnessVariable;
	bool m_bUseRoughnessMap;
	ID3DX11EffectScalarVariable* m_pUseRoughnessMapVariable;

	//Ambient Occlusion
	//*****************
	bool m_bUseAOMap;
	ID3DX11EffectScalarVariable* m_pUseAoMapVariable;
	float m_AoStrength;
	ID3DX11EffectScalarVariable* m_pAoStrengthVariable;

	//METAL
	//********
	bool m_IsMetal;
	ID3DX11EffectScalarVariable* m_pIsMetalVariable;
	bool m_bUseMetalnessMap;
	ID3DX11EffectScalarVariable* m_pUseMetalnessMapVariable;

	//NORMAL MAPPING
	//**************
	bool m_bFlipGreenChannel;
	ID3DX11EffectScalarVariable* m_pFlipGreenChannelVariable;
	bool m_bUseNormalMap;
	ID3DX11EffectScalarVariable* m_pUseNormalMapVariable;
	GA::Texture2D* m_pNormalMap;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapSRVvariable;

	//shadows
	ID3DX11EffectShaderResourceVariable* m_pShadowSRVvariable;
	ID3DX11EffectMatrixVariable* m_pLightVPVariable;

private:
	// -------------------------
	// Disabling default copy constructor and default
	// assignment operator.
	// -------------------------
	TRMaterial(const TRMaterial& obj) = delete;
	TRMaterial& operator=(const TRMaterial& obj) = delete;
};