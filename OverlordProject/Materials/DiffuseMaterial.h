#pragma once
#include <Material.h>


class TextureData;

class DiffuseMaterial :
	public Material
{
public:
	DiffuseMaterial();
	DiffuseMaterial(const DiffuseMaterial&) = delete;
	DiffuseMaterial& operator=(const DiffuseMaterial&) = delete;
	~DiffuseMaterial();

	void SetDiffuseTexture(const std::wstring& assetFile);
private:
	virtual void LoadEffectVariables();
	virtual void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent);

	TextureData* m_pDiffuseTexture;
	static ID3DX11EffectShaderResourceVariable* m_pDiffuseSRVvariable;

};

