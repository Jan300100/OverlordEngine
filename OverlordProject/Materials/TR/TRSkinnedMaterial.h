#pragma once

#pragma once
#include "TRMaterial.h"

class TextureData;

class TRSkinnedMaterial final : public TRMaterial
{
public:
	TRSkinnedMaterial();
	~TRSkinnedMaterial() = default;

	TRSkinnedMaterial(const TRSkinnedMaterial& other) = delete;
	TRSkinnedMaterial(TRSkinnedMaterial&& other) noexcept = delete;
	TRSkinnedMaterial& operator=(const TRSkinnedMaterial& other) = delete;
	TRSkinnedMaterial& operator=(TRSkinnedMaterial&& other) noexcept = delete;

protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent) override;

private:
	static ID3DX11EffectMatrixVariable* m_pBoneTransforms;
};