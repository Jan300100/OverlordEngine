#include "stdafx.h"
#include "TRCrossRoads.h"
#include "TRScene.h"
#include <Components.h>
#include "PhysxManager.h"
#include <ContentManager.h>

using namespace DirectX;

TRCrossRoads::TRCrossRoads(const XMINT2& position, int initRoadRef, int initFreeRef)
	:TRRoad{ position, {}, Type::CrossRoads, initRoadRef, initFreeRef }
	, m_Extensions{ -2,-2,-2,-2 }
{
}

TRCrossRoads::~TRCrossRoads()
{
	////call to remove tiles in an area around the crossroads the sides
	TRScene* pScene = reinterpret_cast<TRScene*>(GetScene());
	for (int32_t i = 1; i <= int32_t(TILES_ON_EACH_SIDE); i++)
	{
		for (int32_t j = 1; j <= int32_t(TILES_ON_EACH_SIDE); j++)
		{
			pScene->RemoveFreeTile({ m_Position.x + i, m_Position.y + j });
			pScene->RemoveFreeTile({ m_Position.x - i, m_Position.y - j });
			pScene->RemoveFreeTile({ m_Position.x + i, m_Position.y - j });
			pScene->RemoveFreeTile({ m_Position.x - i, m_Position.y + j });
		}
		pScene->RemoveFreeTile({ m_Position.x + i, m_Position.y  });
		pScene->RemoveFreeTile({ m_Position.x - i, m_Position.y });
		pScene->RemoveFreeTile({ m_Position.x, m_Position.y - i });
		pScene->RemoveFreeTile({ m_Position.x, m_Position.y + i });
	}
	for (int i : m_Extensions)
	{
		if (i != -1)
		{
			XMINT2 dir{ (i - 2) % 2 , (i - 1) % 2 };
			pScene->RemoveRoad({ m_Position.x + dir.x, m_Position.y + dir.y });
		}
	}

}

void TRCrossRoads::Extend(bool forceFull)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(forceFull);

	TRScene* pScene = reinterpret_cast<TRScene*>(GetScene());
	for (int i : m_Extensions)
	{
		if (i != -1)
		{
			XMINT2 dir{(i - 2) % 2, (i - 1) % 2 };
			TRTile* tile = pScene->GetTile({ m_Position.x + dir.x, m_Position.y + dir.y });
			if (tile == nullptr || tile->GetType() == Type::Free)
			{
				pScene->CreateRoad({ m_Position.x + dir.x, m_Position.y + dir.y }, dir, Type::Full);
			}
			else if (abs(tile->GetDirection().x) == abs(dir.y) && abs(tile->GetDirection().y) == abs(dir.x))
			{
				pScene->CreateRoad({ m_Position.x + dir.x, m_Position.y + dir.y }, dir, Type::CrossRoads);
			}
		}
	}
}

void TRCrossRoads::Initialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);

	TRScene* pScene = reinterpret_cast<TRScene*>(GetScene());

	//construct this road
	float zFightOffset = 0.2f;
	m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(int)KeyIds::CrossRoad], pScene->GetInstancedRenderer() });
	XMFLOAT3 pos{ TRScene::GetPosition(m_Position).x,14 + zFightOffset,TRScene::GetPosition(m_Position).y };
	m_Details.back().Initialize(CreateTransform(pos, { 0,XMConvertToRadians(float(rand() % 15) - 7.5f),0 }, { 1.0f,1.0f,1.0f }));

	RigidBodyComponent* rb = new RigidBodyComponent{ true };
	AddComponent(rb);

	physx::PxMaterial* pMaterial = PhysxManager::GetInstance()->GetPhysics()->createMaterial(0.f, 0.f, 0.f);
	std::shared_ptr<physx::PxGeometry> pGeometry{
			new physx::PxConvexMeshGeometry{ContentManager::Load<physx::PxConvexMesh>(L"Resources/Meshes/TR/Base.ovpc") } };
	AddComponent(new ColliderComponent{ pGeometry, *pMaterial , physx::PxTransform{ToPxVec3(pos)} });


	//call to construct tiles in an area around the crossroads the sides
	for (int32_t i = 1; i <= int32_t(TILES_ON_EACH_SIDE); i++)
	{
		for (int32_t j = 1; j <= int32_t(TILES_ON_EACH_SIDE); j++)
		{
			pScene->CreateFreeTile({ m_Position.x + i, m_Position.y + j }, max(i,j));
			pScene->CreateFreeTile({ m_Position.x - i, m_Position.y - j }, max(i, j));
			pScene->CreateFreeTile({ m_Position.x + i, m_Position.y - j }, max(i, j));
			pScene->CreateFreeTile({ m_Position.x - i, m_Position.y + j }, max(i, j));
		}
		pScene->CreateFreeTile({ m_Position.x + i, m_Position.y },i);
		pScene->CreateFreeTile({ m_Position.x - i, m_Position.y },i);
		pScene->CreateFreeTile({ m_Position.x, m_Position.y - i },i);
		pScene->CreateFreeTile({ m_Position.x, m_Position.y + i },i);
	}

	//extend in some directions
	int outGoingRoads = rand()%2 + 1; //1-2 additional outgoing roads
	while (m_Extensions[0] == -2 || m_Extensions[1] == -2 || m_Extensions[2] == -2 || m_Extensions[3] == -2)
	{
		int i = rand() % 4;
		if (m_Extensions[i] == -2)
		{
			XMINT2 dir{ (i - 2) % 2, (i - 1) % 2 };

			TRTile* tile = pScene->GetTile({ m_Position.x + dir.x, m_Position.y + dir.y });
			if ((tile && tile->GetDirection().x == -dir.x && tile->GetDirection().y == -dir.y) || outGoingRoads <= 0)
			{
				m_Extensions[i] = -1; //there already is a road here in this direction.
			}
			else if (outGoingRoads > 0)
			{
				m_Extensions[i] = i;
				outGoingRoads--;
			}
		}
	}
}
