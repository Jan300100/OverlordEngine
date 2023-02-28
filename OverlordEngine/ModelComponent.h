#pragma once
#include "BaseComponent.h"

class MeshFilter;
class Material;
class ModelAnimator;

class ModelComponent : public BaseComponent
{
protected:
	std::wstring m_AssetFile;
	MeshFilter* m_pMeshFilter = nullptr;
	ModelAnimator* m_pAnimator = nullptr;
	Material* m_pMaterial = nullptr;
	unsigned int m_MaterialId = 0;
	bool m_MaterialSet = false;
	bool m_CastShadows = true;
	void UpdateMaterial(const GameContext& gameContext);

public:
	ModelComponent(const ModelComponent& other) = delete;
	ModelComponent(ModelComponent&& other) noexcept = delete;
	ModelComponent& operator=(const ModelComponent& other) = delete;
	ModelComponent& operator=(ModelComponent&& other) noexcept = delete;
	ModelComponent(std::wstring  assetFile, bool castShadows = true);
	virtual ~ModelComponent();

	virtual void SetMaterial(unsigned int materialId);
	unsigned int GetMaterialID() const;
	MeshFilter* GetMeshFilter() const { return m_pMeshFilter; };
	ModelAnimator* GetAnimator() const { return m_pAnimator; }
	bool HasAnimator() const { return m_pAnimator != nullptr; }

	void Initialize(const GameContext& gameContext) override;
protected:
	void Update(const GameContext& gameContext) override;
	void Draw(const GameContext& gameContext) override;
	void DrawShadowMap(const GameContext& gameContext) override;
};
