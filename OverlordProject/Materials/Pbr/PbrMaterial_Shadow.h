#pragma once

#include "PbrMaterial.h"



class PbrMaterial_Shadow : public PbrMaterial
{
public:
	PbrMaterial_Shadow(const wstring& effectFile, const wstring& techniqueName, bool usesTesselation = false, bool generatesShadows = false);
	PbrMaterial_Shadow();
	~PbrMaterial_Shadow() = default;

protected:
	virtual void LoadEffectVariables();
	virtual void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent);

private:
	ID3DX11EffectShaderResourceVariable* m_pShadowSRVvariable;
	ID3DX11EffectMatrixVariable* m_pLightVPVariable;

	// -------------------------
	// Disabling default copy constructor and default
	// assignment operator.
	// -------------------------
	PbrMaterial_Shadow(const PbrMaterial_Shadow& obj) = delete;
	PbrMaterial_Shadow& operator=(const PbrMaterial_Shadow& obj) = delete;
};