#pragma once
#include "GameObject.h"

using namespace DirectX;

class TRCamera final : public GameObject
{
public:
	TRCamera(const XMFLOAT3& inGamePosition, const XMFLOAT3& inGameRotation);
protected:
	void Initialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;
private:
	XMFLOAT3 m_PosGame;
	XMFLOAT3 m_RotGame;
};
