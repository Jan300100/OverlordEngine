#pragma once
#include <GameObject.h>
#include "InstancedRenderer.h"
#include "Instance.h"

class TRScene;

struct DetailDescription;

class TRTile : public GameObject
{
public:
	TRTile(const XMINT2& position, const XMINT2& direction, int initRoadRef = 0, int initFreeRef = 0, size_t level = 0);
	virtual ~TRTile() override;
	void AddRoadRef() { m_RoadRef++; };
	void AddFreeRef();
	void RemoveRoadRef() { m_RoadRef--; };
	void RemoveFreeRef();
	int GetRoadRef() const { return m_RoadRef; };
	int GetFreeRef() const { return m_FreeRef; };
	enum class Type
	{
		Free, Full,GapStart,GapEnd,FullToRight,Right,RightToFull,FullToLeft,Left,LeftToFull, CrossRoads
	};

	Type GetType() const { return m_Type; }
	XMINT2 GetDirection() const { return m_Direction; }
	XMINT2 GetPosition() const { return m_Position; }
	virtual void Extend(bool forceFull = false) { UNREFERENCED_PARAMETER(forceFull); }
	size_t GetLevel() const { return m_Level; }
protected:
	virtual void Initialize(const GameContext& gameContext) override;
	XMINT2 m_Position;
	Type m_Type;
	std::vector< Instance<XMFLOAT4X4>> m_Details;
	static std::vector<InstancedRenderer::Key> m_Keys;
	enum class KeyIds : size_t
	{
		Grass2_1,Grass2_2,Zont, Nettle, Oduvanchik
		, Rock, Pillar, FireHolder, Brick, Bush,  Conifer1_t, Conifer2_t, Conifer1_l, Conifer2_l, Obstacle, Ground, Road, GapRoad, HalfRoad, CrossRoad
	};
	XMINT2 m_Direction;
private:
	void GenerateProps();
	static size_t NR_TILES;
	int m_FreeRef;
	int m_RoadRef;
	Instance<XMFLOAT2>* m_pGroundQuad;
	
	//
	enum class DetailType : size_t
	{
		Grass, Zont, Nettle, Oduvanchik, Rock, Pillar, FireHolder, Brick, Bush, Conifer, Size
	};

	size_t m_DesiredDetails[size_t(DetailType::Size)];
	bool m_DetailsAdded;
	size_t m_Level;
	void CreateKeys();
};