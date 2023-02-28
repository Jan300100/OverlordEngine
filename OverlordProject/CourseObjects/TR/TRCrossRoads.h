#pragma once
#include "TRRoad.h"


class TRCrossRoads : public TRRoad
{
public:
	TRCrossRoads(const XMINT2& position, int initRoadRef = 0, int initFreeRef = 0);
	virtual ~TRCrossRoads() override;
	virtual void Extend(bool forceFull = false) override;
private:
	void Initialize(const GameContext& gameContext) override;
	int m_Extensions[4];
};

