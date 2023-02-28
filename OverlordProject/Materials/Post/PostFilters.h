#pragma once
#include "PostProcessingMaterial.h"

class ID3D11EffectShaderResourceVariable;

class PostFilters : public PostProcessingMaterial
{
public:
	PostFilters(float brightness = 0, float contrast = 0, float hue = 0, float saturation = 0);
	PostFilters(const PostFilters& other) = delete;
	PostFilters(PostFilters&& other) noexcept = delete;
	PostFilters& operator=(const PostFilters& other) = delete;
	PostFilters& operator=(PostFilters&& other) noexcept = delete;
	virtual ~PostFilters() = default;
	void SetBrightness(float brightness) { m_Brightness = brightness; }
	void SetContrast(float contr) { m_Contrast = contr; }
	void SetHue(float hue) { m_Hue = hue; }
	void SetSaturation(float satur) { m_Saturation = satur; }
protected:
	void LoadEffectVariables() override;
	void UpdateEffectVariables(const GameContext& gameContext,RenderTarget* pPrevRendertarget, RenderTarget* pOriginalRendertarget) override;
private:
	ID3DX11EffectShaderResourceVariable* m_pTextureMapVariabele;
	ID3DX11EffectScalarVariable* m_pBrightnessVariable;
	ID3DX11EffectScalarVariable* m_pContrastVariable;
	ID3DX11EffectScalarVariable* m_pHueVariable;
	ID3DX11EffectScalarVariable* m_pSaturationVariable;
	float m_Brightness;
	float m_Contrast;
	float m_Hue;
	float m_Saturation;
};
