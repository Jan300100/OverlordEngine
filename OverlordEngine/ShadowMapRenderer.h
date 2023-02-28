#pragma once

class MeshFilter;
class RenderTarget;
class ShadowMapMaterial;
class ShadowMapMaterial_Skinned;
class ModelComponent;

class ShadowMapRenderer final
{
public:
	ShadowMapRenderer() = default;
	~ShadowMapRenderer();

	ShadowMapRenderer(const ShadowMapRenderer& other) = delete;
	ShadowMapRenderer(ShadowMapRenderer&& other) noexcept = delete;
	ShadowMapRenderer& operator=(const ShadowMapRenderer& other) = delete;
	ShadowMapRenderer& operator=(ShadowMapRenderer&& other) noexcept = delete;

	void Initialize(const GameContext& gameContext);

	float GetSize() const { return m_Size; };
	void SetSize(float size);
	void SetLight(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 direction);

	void SetLightBasedOnCamera(const GameContext& gameContext);
	void SetLightBasedOnCameraSphere(const GameContext& gameContext);

	void SetLightPosition(DirectX::XMFLOAT3 position);
	void SetLightDirection(DirectX::XMFLOAT3 direction);
	DirectX::XMFLOAT3 GetLightDirection() const { return m_LightDirection; }
	DirectX::XMFLOAT4X4 GetLightVP() const { return m_LightVP; }

	void Begin(const GameContext& gameContext);
	void End(const GameContext& gameContext) const;

	void ImGuiUpdate();

	void Draw(const GameContext& gameContext, MeshFilter* pMeshFilter, DirectX::XMFLOAT4X4 world, const std::vector<DirectX::XMFLOAT4X4>& bones = std::vector<DirectX::XMFLOAT4X4>()) const;
	void DrawCustom(const GameContext& gameContext, ModelComponent* pModelComponent);
	void UpdateMeshFilter(const GameContext& gameContext, MeshFilter* pMeshFilter);
	ShadowMapMaterial* GetMaterial() const { return m_pShadowMat; }
	ID3D11ShaderResourceView* GetShadowMap() const;

private:
	ShadowMapMaterial* m_pShadowMat = nullptr;
	RenderTarget* m_pShadowRT = nullptr;
	bool m_IsInitialized = false;
	float m_Near = 0.01f, m_Far = 500.0f;

	//LIGHT
	DirectX::XMFLOAT3 m_LightPosition = {}, m_LightDirection = {};
	DirectX::XMFLOAT4X4 m_LightVP = {};
	float m_Size = 100.0f;
	D3D11_VIEWPORT m_Viewport{};
};

