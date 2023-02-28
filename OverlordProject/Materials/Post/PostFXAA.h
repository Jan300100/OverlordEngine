#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;

class PostFXAA : public PostProcessingMaterial
{
public:
	PostFXAA();
	virtual ~PostFXAA() = default;

	PostFXAA(const PostFXAA& other) = delete;
	PostFXAA(PostFXAA&& other) noexcept = delete;
	PostFXAA& operator=(const PostFXAA& other) = delete;
	PostFXAA& operator=(PostFXAA&& other) noexcept = delete;
protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(const GameContext& gameContext, RenderTarget* pPrevRendertarget, RenderTarget* pOriginalRendertarget) override;
private:
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariabele;
};
