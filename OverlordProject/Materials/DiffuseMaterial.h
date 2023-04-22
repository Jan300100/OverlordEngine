#pragma once
#include <Material.h>

namespace
{
	class Texture2D;
}

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

	GA::Texture2D* m_pDiffuseTexture;
	static ID3DX11EffectShaderResourceVariable* m_pDiffuseSRVvariable;

};

