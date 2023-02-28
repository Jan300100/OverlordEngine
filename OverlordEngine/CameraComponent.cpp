#include "stdafx.h"
#include "CameraComponent.h"
#include "OverlordGame.h"
#include "TransformComponent.h"
#include "PhysxProxy.h"
#include "GameObject.h"

#include "GameScene.h"

CameraComponent::CameraComponent() :
	m_FarPlane(1000.0f),
	m_NearPlane(0.1f),
	m_FOV(DirectX::XM_PIDIV4),
	m_Size(25.0f),
	m_IsActive(true),
	m_PerspectiveProjection(true)
{
	XMStoreFloat4x4(&m_Projection, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_View, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewInverse, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjection, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&m_ViewProjectionInverse, DirectX::XMMatrixIdentity());
}

void CameraComponent::Initialize(const GameContext&) {}

void CameraComponent::Update(const GameContext&)
{
	// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
	using namespace DirectX;

	const auto windowSettings = OverlordGame::GetGameSettings().Window;
	DirectX::XMMATRIX projection;

	if (m_PerspectiveProjection)
	{
		projection = DirectX::XMMatrixPerspectiveFovLH(m_FOV, windowSettings.AspectRatio, m_NearPlane, m_FarPlane);
	}
	else
	{
		const float viewWidth = (m_Size > 0) ? m_Size * windowSettings.AspectRatio : windowSettings.Width;
		const float viewHeight = (m_Size > 0) ? m_Size : windowSettings.Height;
		projection = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, m_NearPlane, m_FarPlane);
	}
	DirectX::XMMATRIX projectionInv = XMMatrixInverse(nullptr, projection);

	const DirectX::XMVECTOR worldPosition = XMLoadFloat3(&GetTransform()->GetWorldPosition());
	const DirectX::XMVECTOR lookAt = XMLoadFloat3(&GetTransform()->GetForward());
	const DirectX::XMVECTOR upVec = XMLoadFloat3(&GetTransform()->GetUp());

	const DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(worldPosition, worldPosition + lookAt, upVec);
	const DirectX::XMMATRIX viewInv = XMMatrixInverse(nullptr, view);
	const DirectX::XMMATRIX viewProjectionInv = XMMatrixInverse(nullptr, view * projection);

	XMStoreFloat4x4(&m_Projection, projection);
	XMStoreFloat4x4(&m_ProjectionInverse, projectionInv);
	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_ViewInverse, viewInv);
	XMStoreFloat4x4(&m_ViewProjection, view * projection);
	XMStoreFloat4x4(&m_ViewProjectionInverse, viewProjectionInv);
}

void CameraComponent::Draw(const GameContext&) {}

void CameraComponent::SetActive()
{
	auto gameObject = GetGameObject();
	if (gameObject == nullptr)
	{
		Logger::LogError(L"[CameraComponent] Failed to set active camera. Parent game object is null");
		return;
	}

	auto gameScene = gameObject->GetScene();
	if (gameScene == nullptr)
	{
		Logger::LogError(L"[CameraComponent] Failed to set active camera. Parent game scene is null");
		return;
	}

	gameScene->SetActiveCamera(this);
}

GameObject* CameraComponent::PickMouse(const GameContext& gameContext, CollisionGroupFlag ignoreGroups) const
{
	POINT mousePos = gameContext.pInput->GetMousePosition();
	//coordinates to NDCcoordinates
	float halfWidth = OverlordGame::GetGameSettings().Window.Width / 2.0f;
	float halfHeight = OverlordGame::GetGameSettings().Window.Height / 2.0f;
	float xNdc = (mousePos.x - halfWidth) / halfWidth;
	float yNdc = (halfHeight - mousePos.y) / halfHeight;

	auto hit = Pick(DirectX::XMFLOAT2{ xNdc,yNdc }, ignoreGroups);
	if (hit.hasAnyHits())
	{
		GameObject* go = static_cast<BaseComponent*>(hit.block.actor->userData)->GetGameObject();
		return go;
	}
	return nullptr;
}

physx::PxRaycastBuffer CameraComponent::Pick(DirectX::XMFLOAT2 ndc,  CollisionGroupFlag ignoreGroups) const
{
	//near point and far point
	DirectX::XMMATRIX VPi = DirectX::XMLoadFloat4x4(&m_ViewProjectionInverse);
	DirectX::XMVECTOR n = DirectX::XMVectorSet(ndc.x, ndc.y, 0.0f, 0.0f);
	DirectX::XMVECTOR f = DirectX::XMVectorSet(ndc.x, ndc.y, 1.0f, 0.0f);
	n = DirectX::XMVector3TransformCoord(n, VPi);
	f = DirectX::XMVector3TransformCoord(f, VPi);

	DirectX::XMFLOAT4 nearPoint{};
	DirectX::XMFLOAT4 farPoint{};
	DirectX::XMStoreFloat4(&nearPoint, n);
	DirectX::XMStoreFloat4(&farPoint, f);

	//setup raycast
	physx::PxQueryFilterData filterData;
	filterData.data.word0 = ~physx::PxU32(ignoreGroups);

	physx::PxRaycastBuffer hit;
	physx::PxVec3 rayStart{ ToPxVec4(nearPoint).getXYZ() };
	physx::PxVec3 rayDir{ (ToPxVec4(farPoint).getXYZ() - rayStart).getNormalized() };

	GetGameObject()->GetScene()->GetPhysxProxy()->Raycast(rayStart, rayDir, PX_MAX_F32, hit, physx::PxHitFlag::eDEFAULT, filterData);
	return hit;
}
