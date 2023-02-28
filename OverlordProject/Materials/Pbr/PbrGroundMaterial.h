#pragma once
#include "PbrMaterial_Shadow.h"

class TextureData;
using namespace DirectX;
using namespace std;

class PbrGroundMaterial :
	public PbrMaterial_Shadow
{
public:

	PbrGroundMaterial();
	~PbrGroundMaterial() = default;

	//world
	void SetWorldUvScale(float scale);
	
	//noise
	void SetNoiseTexture(const wstring& assetFile);
	void SetNoiseHeight(float height);
	void SetNoiseUvScale(float scale);
		
	//DISPLACEMENT
	//************
	void SetDisplacementAmount(float amount);
	void SetTessFactor(float maxFactor);
	
protected:
	virtual void LoadEffectVariables();
	virtual void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent);

private:
	
	//NOISE HEIGHT
	float m_NoiseHeight;
	static ID3DX11EffectScalarVariable* m_pNoiseHeightVariable;
	float m_NoiseUVScale;
	static ID3DX11EffectScalarVariable* m_pNoiseUvScaleVariable;
	TextureData* m_pNoiseTexture;
	static ID3DX11EffectShaderResourceVariable* m_pNoiseTextureVariable;

	//WORLDUVS
	float m_WorldUvScale;
	static ID3DX11EffectScalarVariable* m_pWorldUvScaleVariable;

	
	//DISPLACEMENT
	//************
	float m_DisplacementAmount;
	static ID3DX11EffectScalarVariable* m_pDisplacementAmountVariable;
	float m_TessFactor;
	static ID3DX11EffectScalarVariable* m_pMaxTessFactorVariable;
	
private:
	// -------------------------
	// Disabling default copy constructor and default
	// assignment operator.
	// -------------------------
	PbrGroundMaterial(const PbrGroundMaterial& obj) = delete;
	PbrGroundMaterial& operator=(const PbrGroundMaterial& obj) = delete;
};