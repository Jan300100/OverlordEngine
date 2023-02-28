#pragma once
#include "TRTile.h"


class TRRoad : public TRTile
{
public:
	TRRoad(const XMINT2& position,const XMINT2& direction, Type type, int initRoadRef = 0, int initFreeRef = 0);
	virtual ~TRRoad() override;
	virtual void Extend(bool forceFull);
	static const size_t TILES_ON_EACH_SIDE = 5;
protected:
private:
	void Initialize(const GameContext& gameContext) override;
};

