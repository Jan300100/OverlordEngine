#include "stdafx.h"
#include "TRRoad.h"
#include "TRScene.h"
#include "TRHelpers.h"
#include <Components.h>
#include "PhysxManager.h"
#include "TRCrossRoads.h"
#include <ContentManager.h>

using namespace DirectX;

TRRoad::TRRoad(const DirectX::XMINT2& position, const DirectX::XMINT2& direction, Type type, int initRoadRef, int initFreeRef)
	:TRTile{ position ,direction, initRoadRef, initFreeRef}
{
	if (type == Type::Free) {
		Logger::LogWarning(L"Passing TRTile::Type::Free as type to TRRoad\n");
		type = Type::Full;
	}
	m_Type = type;
}

TRRoad::~TRRoad()
{
	//call to destroy tiles on the sides and the one in front
	//only if not a crossroads
	if (m_Type != Type::CrossRoads)
	{
		TRScene* pScene = reinterpret_cast<TRScene*>(GetScene());

		//call to construct tiles on the sides
		for (int32_t i = 1; i <= int32_t(TILES_ON_EACH_SIDE); i++)
		{
			pScene->RemoveFreeTile({ m_Position.x + m_Direction.y * i, m_Position.y + m_Direction.x * i });
			pScene->RemoveFreeTile({ m_Position.x - m_Direction.y * i, m_Position.y - m_Direction.x * i });
		}
		pScene->RemoveRoad({ m_Position.x + m_Direction.x, m_Position.y + m_Direction.y });
	}
}

void TRRoad::Extend(bool forceFull)
{
	PIX_PROFILE();

	TRTile* tile = reinterpret_cast<TRScene*>(GetScene())->GetTile({ m_Position.x + m_Direction.x, m_Position.y + m_Direction.y });

	if (tile == nullptr || tile->GetType() == Type::Free)
	{
		if (forceFull)
		{ 
			reinterpret_cast<TRScene*>(GetScene())->CreateRoad({ m_Position.x + m_Direction.x, m_Position.y + m_Direction.y }, m_Direction, Type::Full);
		}
		else
		{
			Type type;
			switch (m_Type)
			{
			case TRTile::Type::GapStart:
				type = Type::GapEnd;
				break;
			case TRTile::Type::FullToRight:
			case TRTile::Type::Right:
				type = (rand() % 2) ? Type::Right : Type::RightToFull;
				break;
			case TRTile::Type::FullToLeft:
			case TRTile::Type::Left:
				type = (rand() % 2) ? Type::Left : Type::LeftToFull;
				break;
			case TRTile::Type::RightToFull:
			case TRTile::Type::LeftToFull:
			case TRTile::Type::GapEnd:
			case TRTile::Type::Full:
			default:
				switch (rand() % 10)
				{
				case 0:
					type = Type::GapStart;
					break;
				case 1:
					type = Type::FullToLeft;
					break;
				case 2:
					type = Type::FullToRight;
					break;
				case 3:
				case 4:
					if (tile == nullptr)
					{
						type = Type::CrossRoads;
					}
					else
					{
						type = Type::Full;
					}
					break;
				default:
					type = Type::Full;
					break;
				}
				break;
			}
			reinterpret_cast<TRScene*>(GetScene())->CreateRoad({ m_Position.x + m_Direction.x, m_Position.y + m_Direction.y }, m_Direction, type);
		}
	}
	else if (abs(tile->GetDirection().x) == abs(m_Direction.y) && abs(tile->GetDirection().y) == abs(m_Direction.x))
	{
		reinterpret_cast<TRScene*>(GetScene())->CreateRoad({ m_Position.x + m_Direction.x, m_Position.y + m_Direction.y }, m_Direction, Type::CrossRoads);
	}
}

void TRRoad::Initialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	TRTile::Initialize(gameContext);

	TRScene* pScene = reinterpret_cast<TRScene*>(GetScene());

	int rotation = 0;
	if (m_Direction.x == 0)
	{
		if (m_Direction.y == 1)
		{
			rotation = 2;
		}
		else
		{
			rotation = 0;
		}
	}
	else
	{
		if (m_Direction.x == 1)
		{
			rotation = 3;
		}
		else
		{
			rotation = 1;
		}
	}

	XMFLOAT3 pos{ TRScene::GetPosition(m_Position).x,13,TRScene::GetPosition(m_Position).y };

	XMFLOAT3 scale{ 1.0f,1.0f,1.0f };
	physx::PxMaterial* pMaterial = PhysxManager::GetInstance()->GetPhysics()->createMaterial(0.f, 0.f, 0.f);
	std::shared_ptr<physx::PxGeometry> pGeometry{};
	XMFLOAT4 quat;
	RigidBodyComponent* rb = new RigidBodyComponent{ true };
	AddComponent(rb);

	float randomRotation = XMConvertToRadians(float(rand() % 3) - 1.5f);
	float randomHeightOffset = float(rand() % 20) / 100.0f;
	XMFLOAT3 visualPos{ pos.x, pos.y + randomHeightOffset, pos.z };
	switch (m_Type)
	{
	case TRTile::Type::Full:
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::Road], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * rotation + randomRotation,0 }, scale));
		//collider
		pGeometry = std::shared_ptr<physx::PxGeometry>{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/Road.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * rotation,0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		//chance for an obstacle
		if (rand() % 10 == 0 && (m_Position.x != 0 || m_Position.y != 0))
		{
			m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::Obstacle], pScene->GetInstancedRenderer() });
			m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * rotation + randomRotation,0 }, scale));
			//trigger
			pGeometry = std::shared_ptr<physx::PxGeometry>{
				new physx::PxBoxGeometry{12,2.f,2.f} };
			ColliderComponent* pObstacleTrigger = new ColliderComponent{ pGeometry, *pMaterial
			,physx::PxTransform{{pos.x,pos.y + 11, pos.z},ToPxQuat(quat) } };
			pObstacleTrigger->EnableTrigger(true);
			AddComponent(pObstacleTrigger);
			
			auto lmbda = [this, pScene](GameObject* trigger, GameObject* otherObject, GameObject::TriggerAction action)
			{
				if (action == GameObject::TriggerAction::ENTER)
				{
					if (trigger && otherObject)
					{
						if (otherObject != this)
						{
							pScene->GameOver();
						}
					}
				}
			};
			SetOnTriggerCallBack(lmbda);
		}

		break;
	case TRTile::Type::GapStart:
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::GapRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * rotation + randomRotation,0 }, scale));
		//collider
		pGeometry = std::shared_ptr<physx::PxGeometry>{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/GapRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * rotation,0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		break;
	case TRTile::Type::GapEnd:
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::GapRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * (rotation+2) + randomRotation,0 }, scale));
		//collider
		pGeometry = std::shared_ptr<physx::PxGeometry>{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/GapRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * (rotation+2),0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		break;
	case TRTile::Type::FullToRight:
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::GapRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * rotation + randomRotation,0 }, scale));
		//
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::HalfRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * (rotation + 2) + randomRotation,0 }, scale));
		//collider
		pGeometry = std::shared_ptr<physx::PxGeometry>{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/HalfRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * (rotation + 2),0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		//
		pGeometry = std::shared_ptr<physx::PxGeometry>{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/GapRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * rotation,0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		break;
	case TRTile::Type::Right:
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::HalfRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * (rotation + 2) + randomRotation,0 }, scale));
		//collider
		pGeometry = std::shared_ptr<physx::PxGeometry>{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/HalfRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * (rotation + 2),0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		break;
	case TRTile::Type::RightToFull:
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::HalfRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * (rotation + 2) + randomRotation,0 }, scale));
		//
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::GapRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * (rotation + 2) + randomRotation,0 }, scale));
		//collider
		pGeometry = std::shared_ptr<physx::PxGeometry>{
					new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/HalfRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * (rotation + 2),0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		//
		pGeometry = std::shared_ptr<physx::PxGeometry>{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/GapRoad.ovpc") } };	
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * (rotation + 2),0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		break;
	case TRTile::Type::FullToLeft:
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::GapRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * rotation + randomRotation,0 }, scale));
		//
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::HalfRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * rotation + randomRotation,0 }, scale));
		
		//collider
		pGeometry = std::shared_ptr<physx::PxGeometry>{
					new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/HalfRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * rotation,0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		//
		pGeometry = std::shared_ptr<physx::PxGeometry>{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/GapRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * rotation,0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		break;
	case TRTile::Type::Left:
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::HalfRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * rotation + randomRotation,0 }, scale));
		//collider
		pGeometry = std::shared_ptr<physx::PxGeometry>{
					new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/HalfRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * rotation,0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		break;
	case TRTile::Type::LeftToFull:
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::HalfRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * rotation + randomRotation,0 }, scale));
		//
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::GapRoad], pScene->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(visualPos, { 0,XM_PIDIV2 * (rotation + 2) + randomRotation,0 }, scale));
		
		//collider
		pGeometry = std::shared_ptr<physx::PxGeometry>{
					new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/HalfRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * rotation,0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		//
		pGeometry = std::shared_ptr<physx::PxGeometry>{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/GapRoad.ovpc") } };
		XMStoreFloat4(&quat, XMQuaternionRotationRollPitchYawFromVector(XMVECTOR{ 0, XM_PIDIV2 * (rotation + 2),0 }));
		AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos),ToPxQuat(quat) } });
		break;
	default:
		break;
	}



	//call to construct tiles on the sides
	for (int32_t i = 1; i <= int32_t(TILES_ON_EACH_SIDE); i++)
	{
		pScene->CreateFreeTile({ m_Position.x + m_Direction.y * i, m_Position.y + m_Direction.x * i },i);
		pScene->CreateFreeTile({ m_Position.x - m_Direction.y * i, m_Position.y - m_Direction.x * i },i);
	}

}
