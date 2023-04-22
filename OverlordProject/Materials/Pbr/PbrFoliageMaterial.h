#pragma once

#include "PbrMaterial_Shadow.h"

namespace GA
{
	class Texture2D;
}
using namespace DirectX;
using namespace std;

class PbrFoliageMaterial :
	public PbrMaterial_Shadow
{
public:

	PbrFoliageMaterial();
	~PbrFoliageMaterial() = default;

	static void SetNoiseTexture(const wstring& assetFile);
	static void SetNoiseUvScale(float scale);
	static void SetWindForce(float force);
	static void SetWindDirection(XMFLOAT2 dir);
	void SetInfluence(float influence);

protected:
	virtual void LoadEffectVariables();
	virtual void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent);


private:
	//statics
	static GA::Texture2D* m_pNoiseTexture;
	static float m_WindForce;
	static XMFLOAT2 m_WindDirection;
	static float m_NoiseUvScale;

	static ID3DX11EffectShaderResourceVariable* m_pNoiseTextureVariable;
	static ID3DX11EffectScalarVariable* m_pNoiseUvScaleVariable;
	static ID3DX11EffectScalarVariable* m_pWindForceVariable;
	static ID3DX11EffectVectorVariable* m_pWindDirectionVariable;
	static ID3DX11EffectScalarVariable* m_pInfluenceVariable;
	static ID3DX11EffectScalarVariable* m_pTimePassedVariable;
	
	//unique
	float m_Influence; //how much the foliage is affected by wind

	// -------------------------
	// Disabling default copy constructor and default
	// assignment operator.
	// -------------------------
	PbrFoliageMaterial(const PbrFoliageMaterial& obj) = delete;
	PbrFoliageMaterial& operator=(const PbrFoliageMaterial& obj) = delete;
};