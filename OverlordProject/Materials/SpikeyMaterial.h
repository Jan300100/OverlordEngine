#pragma once
#include "Material.h"

class GA::Texture2D;

class SpikeyMaterial final: public Material
{
public:
	SpikeyMaterial();
	~SpikeyMaterial();

	void SetDiffuseTexture(const std::wstring& assetFile);

protected:
	virtual void LoadEffectVariables();
	virtual void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent);

	GA::Texture2D* m_pDiffuseTexture;
	static ID3DX11EffectShaderResourceVariable* m_pDiffuseSRVvariable;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	SpikeyMaterial(const SpikeyMaterial &obj);
	SpikeyMaterial& operator=(const SpikeyMaterial& obj);
};

