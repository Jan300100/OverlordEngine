#include "stdafx.h"
#include "TRTile.h"
#include <ContentManager.h>
#include "MeshFilter.h"
#include "TRScene.h"
#include "TRHelpers.h"
#include "TRRoad.h"

std::vector<InstancedRenderer::Key> TRTile::m_Keys{};
size_t TRTile::NR_TILES = 0;

TRTile::TRTile(const XMINT2& position, const XMINT2& direction, int initRoadRef, int initFreeRef, size_t level)
	: m_Position{ position }, m_Direction{ direction }, m_FreeRef{ initFreeRef }, m_RoadRef{initRoadRef}, m_pGroundQuad{}
	, m_Type{Type::Free}
	, m_Details{}
	, m_Level{level}
	, m_DetailsAdded{ false }
{
	NR_TILES++;

	m_Details.reserve(1000);

	if (m_Keys.size() == 0)
	{
		CreateKeys();
	}
}

TRTile::~TRTile()
{
	SafeDelete(m_pGroundQuad);
	NR_TILES--;
}

void TRTile::AddFreeRef()
{
	m_FreeRef++;
}

void TRTile::RemoveFreeRef()
{
	m_FreeRef--;
}

void TRTile::GenerateProps()
{
	PIX_PROFILE();

	//Rock
	for (size_t i = 0; i < m_DesiredDetails[size_t(DetailType::Rock)]; i++)
	{
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[int(KeyIds::Rock)], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), 0 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		float scale = randF(0.6f, 0.9f);
		m_Details.back().Initialize(CreateTransform(pos, { randF(0, 6.2830f),randF(0, 6.2830f),randF(0, 6.2830f) }, { scale,scale,scale }));
	}

	//Bush
	for (size_t i = 0; i < m_DesiredDetails[size_t(DetailType::Bush)]; i++)
	{
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[int(KeyIds::Bush)], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), 0 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		float scale = randF(0.5f, 1.4f);
		m_Details.back().Initialize(CreateTransform(pos, { 0,randF(0, 6.2830f), 0 }, { scale,scale,scale }));
	}

	//Pillar
	for (size_t i = 0; i < m_DesiredDetails[size_t(DetailType::Pillar)]; i++)
	{
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(size_t)KeyIds::Pillar], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), 0 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		float width = randF(0.9f, 1.1f);
		float height = randF(0.9f, 1.1f);
		XMFLOAT3 rot = { randF(-0.1f, 0.1f),randF(0, 6.2830f),randF(-0.1f, 0.1f) };
		m_Details.back().Initialize(CreateTransform(pos, rot, { width,height,width }));
	}

	//FireHolder
	for (size_t i = 0; i < m_DesiredDetails[size_t(DetailType::FireHolder)]; i++)
	{
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(size_t)KeyIds::FireHolder], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), 0 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		float scale = randF(0.9f, 1.1f);
		m_Details.back().Initialize(CreateTransform(pos, { randF(-0.1f, 0.1f),randF(0, 6.2830f),randF(-0.1f, 0.1f) }, { scale,scale,scale }));
	}

	//Brick
	for (size_t i = 0; i < m_DesiredDetails[size_t(DetailType::Brick)]; i++)
	{
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(size_t)KeyIds::Brick], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), -1 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		m_Details.back().Initialize(CreateTransform(pos, { randF(-0.1f, 0.1f),randF(0, 6.2830f),randF(-0.1f, 0.1f) }, { randF(0.6f, 1.0f),randF(0.3f, 0.5f),randF(0.6f, 1.0f) }));
	}

	//conifer
	for (size_t i =0; i < m_DesiredDetails[size_t(DetailType::Conifer)]; i++)
	{
		int key = int(KeyIds::Conifer1_t) + (rand() % 3 == 0);
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[key], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), 0 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		float scale = randF(0.4f, 0.6f);
		XMFLOAT3 rot = { randF(-0.05f, 0.05f),randF(0, 6.2830f),randF(-0.05f, 0.05f) };
		m_Details.back().Initialize(CreateTransform(pos, rot, { scale,scale,scale }));
		////leaves
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[key + 2], GetScene()->GetInstancedRenderer() });
		m_Details.back().Initialize(CreateTransform(pos, rot, { scale,scale,scale }));
	}

	//grass
	for (size_t i = 0; i < m_DesiredDetails[size_t(DetailType::Grass)]; i++)
	{
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[int(KeyIds::Grass2_1) + rand()%2], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), 0 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		float scale = randF(0.35f, 0.6f);
		m_Details.back().Initialize(CreateTransform(pos, { 0,randF(0, 6.2830f),0 }, { scale,scale,scale }));
	}

	//zont
	for (size_t i = 0; i < m_DesiredDetails[size_t(DetailType::Zont)]; i++)
	{
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(size_t)KeyIds::Zont], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), 0 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		float scale = randF(0.35f, 0.6f);
		m_Details.back().Initialize(CreateTransform(pos, { 0,randF(0, 6.2830f),0 }, { scale,scale,scale }));
	}

	//nettle
	for (size_t i = 0; i < m_DesiredDetails[size_t(DetailType::Nettle)]; i++)
	{
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(size_t)KeyIds::Nettle], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), 0 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		float scale = randF(0.35f, 0.6f);
		m_Details.back().Initialize(CreateTransform(pos, { 0,randF(0, 6.2830f),0 }, { scale,scale,scale }));
	}

	//oduvanchik
	for (size_t i = 0; i < m_DesiredDetails[size_t(DetailType::Oduvanchik)]; i++)
	{
		m_Details.push_back(Instance<XMFLOAT4X4>{ m_Keys[(size_t)KeyIds::Oduvanchik], GetScene()->GetInstancedRenderer() });
		XMFLOAT3 pos = { randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f), 0 , randF(-TRScene::TILE_SIZE / 2.0f, TRScene::TILE_SIZE / 2.0f) };
		pos.x += TRScene::GetPosition(m_Position).x;
		pos.z += TRScene::GetPosition(m_Position).y;
		float scale = randF(0.35f, 0.6f);
		m_Details.back().Initialize(CreateTransform(pos, { 0,randF(0, 6.2830f),0 }, { scale,scale,scale }));
	}

}

void TRTile::Initialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);
	//*********

	//create groundQuad
	m_pGroundQuad = new Instance<XMFLOAT2>{ m_Keys[(int)KeyIds::Ground], GetScene()->GetInstancedRenderer() };
	m_pGroundQuad->Initialize( TRScene::GetPosition(m_Position));

	for (size_t i = 0; i < size_t(DetailType::Size); i++)
	{
		m_DesiredDetails[i] = 0;
	}
	if (m_Level < 4)
	{
		m_DesiredDetails[size_t(DetailType::Grass)] = (m_Level > 0) ? max(200 - 100 * int(m_Level - 1), 0) : 100;
		m_DesiredDetails[size_t(DetailType::Zont)] = (m_Level > 0) ? size_t(randF(0.0f, 100) / (m_Level * 3)) : rand() % 5;
		m_DesiredDetails[size_t(DetailType::Nettle)] = (m_Level > 0) ? size_t(randF(0.0f, 100) / (m_Level * 3)) : rand() % 5;
		m_DesiredDetails[size_t(DetailType::Oduvanchik)] = (m_Level > 0) ? size_t(randF(0.0f, 100) / (m_Level * 3)) : rand() % 5;
		m_DesiredDetails[size_t(DetailType::Rock)] = size_t(((m_Level + 1) / 2.0f) * (rand() % 2));
		m_DesiredDetails[size_t(DetailType::Pillar)] = (m_Level == 1) ? rand() % 4 == 0 : (rand() % 15 == 0 && m_Level != 0);
		m_DesiredDetails[size_t(DetailType::FireHolder)] = (m_Level == 1) ? rand() % 3 : (rand() % 15 == 0);
		m_DesiredDetails[size_t(DetailType::Brick)] = (m_Level <= 1) ? rand() % 3 : (rand() % 7 == 0);
		//m_DesiredDetails[size_t(DetailType::Bush)] = (m_Level > 0) ? (rand() % 3 == 0) : 0;
		m_DesiredDetails[size_t(DetailType::Conifer)] = (m_Level > 1) ? m_Level * 2 : (rand() % 15 == 0 && m_Level != 0);
		GenerateProps();
	}

}

void TRTile::CreateKeys()
{
	PIX_PROFILE();

	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Grass, 30000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Grass2_2.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Grass, 30000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Grass2_4.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Zont,10000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Zont.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Nettle,10000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Nettle.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Oduvanchik,10000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Oduvanchik.ovm") });

	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Rock,1000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Rock3.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Props,1000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Pillar.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Props,1000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/FireHolder.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Props,1000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Brick.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Bush,1000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Bush1.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Bark,10000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Conifer1_Trunk.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Bark,10000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Conifer3_Trunk.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Conifer,10000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Conifer1_Leaves.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Conifer,10000, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Conifer3_Leaves.ovm") });

	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Road,150, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Obstacle.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT2>::m_TypeInfo, (int)MatIds::Ground,1500, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/TileQuad.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Road,150, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Road.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Road,150, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/GapRoad.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Road,150, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/HalfRoad.ovm") });
	m_Keys.push_back(InstancedRenderer::Key{ Instance<XMFLOAT4X4>::m_TypeInfo, (int)MatIds::Base,150, ContentManager::Load<MeshFilter>(L"./Resources/Meshes/TR/Base.ovm") });
}
