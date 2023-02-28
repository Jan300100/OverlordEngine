#pragma once
#include "TRMaterial.h"

using namespace DirectX;

class TRPropsMaterial : public TRMaterial
{
public:
private:

	//NOISE HEIGHT
	static float m_NoiseHeight;
	static float m_NoiseUvScale;
	static ID3DX11EffectScalarVariable* m_pNoiseHeightVariable;
	static ID3DX11EffectScalarVariable* m_pNoiseUvScaleVariable;
	static TextureData* m_pNoiseTexture;
	static ID3DX11EffectShaderResourceVariable* m_pNoiseTextureVariable;
	//noise, scale, height

	//Foliage
	static XMFLOAT2 m_WindDirection;

	static ID3DX11EffectScalarVariable* m_pWindForceVariable;
	static ID3DX11EffectVectorVariable* m_pWindDirectionVariable;
	static ID3DX11EffectScalarVariable* m_pInfluenceVariable;
	static ID3DX11EffectScalarVariable* m_pDistanceInfluence;
	static ID3DX11EffectScalarVariable* m_pTimePassedVariable;

	//unique
	float m_WindForce;
	float m_Influence; //how much the foliage is affected by wind
	float m_DistanceInfluence; //at the origin of the prop, there is no wind movement, the further away from the origin, the more the wind grasps the prop.
	bool m_IsFoliage;

public:
	//noise
	static void SetNoiseTexture(const wstring& assetFile);
	static void SetNoiseHeight(float height);
	static void SetNoiseUvScale(float scale);

	TRPropsMaterial(bool isFoliage = false, bool generatesShadows = true);
	~TRPropsMaterial() = default;

	void SetWindForce(float force);
	static void SetWindDirection(XMFLOAT2 dir);
	void SetInfluence(float influence);
	void SetDistanceInfluence(float influence);

	virtual ID3DX11EffectTechnique* GetShadowTechnique() override;

protected:
	virtual void LoadEffectVariables();
	virtual void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent);

};