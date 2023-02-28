#include "stdafx.h"
#include "TRCamera.h"
#include "TRScene.h"
#include "TransformComponent.h"
#include <imgui\imgui.h>

TRCamera::TRCamera(const XMFLOAT3& inGamePosition, const XMFLOAT3& inGameRotation)
	:m_PosGame{ inGamePosition }
	, m_RotGame{ inGameRotation }
{
}

void TRCamera::Initialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);
	AddComponent(new CameraComponent());

	//camera to better position;
	int randomSign = rand()%2 * 2 -1;



	switch (rand()%3)
	{
	case 0:
		GetTransform()->Translate(randomSign * 75.0f, 97, -37);
		GetTransform()->Rotate(36, randomSign * -29.0f, 0);
		break;
	case 1:
		GetTransform()->Translate(randomSign * 9.0f, 25, -49);
		GetTransform()->Rotate(13.6f, randomSign * -0.06f, 0);
		break;
	case 2:
	default:
		GetTransform()->Translate(randomSign * 55.0f, 0, -20);
		GetTransform()->Rotate(-5, randomSign * -40.0f, 5);
		break;
	}

}

void TRCamera::Update(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (reinterpret_cast<TRScene*>(GetScene())->IsPlaying())
	{
		//Position
		XMVECTOR current = XMLoadFloat3(&GetTransform()->GetPosition());
		XMVECTOR goal = XMLoadFloat3(&m_PosGame);
		current = DirectX::XMVectorLerp(current, goal, 3.0f * gameContext.pGameTime->GetElapsed());
		GetTransform()->Translate(current);

		//Rotation
		current = XMLoadFloat4(&GetTransform()->GetRotation());
		goal = XMQuaternionRotationRollPitchYaw(m_RotGame.x, m_RotGame.y, m_RotGame.z);
		current = DirectX::XMQuaternionSlerp(current, goal, 3.0f * gameContext.pGameTime->GetElapsed());
		GetTransform()->Rotate(current);
	}
}
