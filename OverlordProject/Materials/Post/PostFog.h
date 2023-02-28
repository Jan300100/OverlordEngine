#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;

class PostFog : public PostProcessingMaterial
{
public:
	PostFog(float falloff, const DirectX::XMFLOAT3& color = {1,1,1});
	virtual ~PostFog() = default;
	void SetFogFalloff(float falloff) { m_FogFalloff = falloff; }
	void SetFogStrength(float str) { m_FogStrength = str; }
	void SetFogColor(const DirectX::XMFLOAT3& col) { m_FogColor = col; }

	PostFog(const PostFog& other) = delete;
	PostFog(PostFog&& other) noexcept = delete;
	PostFog& operator=(const PostFog& other) = delete;
	PostFog& operator=(PostFog&& other) noexcept = delete;
protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(const GameContext& gameContext, RenderTarget* pPrevRendertarget, RenderTarget* pOriginalRendertarget) override;
private:
	ID3DX11EffectMatrixVariable* m_pProjInvMatrixVariable;

	ID3DX11EffectShaderResourceVariable* m_pDepthMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariabele;
	ID3DX11EffectScalarVariable* m_pFogFalloffVariable;
	ID3DX11EffectScalarVariable* m_pFogStrengthVariable;
	ID3DX11EffectVectorVariable* m_pFogColorVariable;
	DirectX::XMFLOAT3 m_FogColor;
	float m_FogFalloff;
	float m_FogStrength;

};
