#pragma once
#include <GameScene.h>
#include <unordered_map>
#include <map>
#include "TRTile.h"
#include "TRHelpers.h"


using namespace std;


enum class MatIds
{
	Road = 1, Base, Props, Bark, Rock, Ground, Grass, Nettle, Oduvanchik, Ramashka, Zont, Moss, Bush, Conifer
};

enum class GameActions
{
	Stop = 100,Start, Restart, NextPPSet, ToggleSetCreation
};

class PostFog;
class PostFilters;
struct PPVariableSet
{
	void Set(PostFog* pFog, PostFilters* pFilters);
	float brightness, contrast, hue, saturation;
	DirectX::XMFLOAT3 fogColor;
	float fogFalloff, fogStrength;
};

class PostBlur;
class PostGrayscale;

class TRCharacter;
class TRRoad;
class SpriteComponent;
class SpriteFont;

class TRScene :
	public GameScene
{
public:
	TRScene();
	virtual ~TRScene();

	TRScene(const TRScene& other) = delete;
	TRScene(TRScene&& other) noexcept = delete;
	TRScene& operator=(const TRScene& other) = delete;
	TRScene& operator=(TRScene&& other) noexcept = delete;

	static const float TILE_SIZE;
	static DirectX::XMFLOAT2 GetPosition(const DirectX::XMINT2& position);
	void CreateFreeTile(const DirectX::XMINT2& at, size_t level);
	void RemoveFreeTile(const DirectX::XMINT2& at);
	void RemoveRoad(const DirectX::XMINT2& at);
	int GetPlayerDirection();
	DirectX::XMINT2 GetPlayerPosition();
	void CreateRoad(const DirectX::XMINT2& at, const DirectX::XMINT2& direction, TRTile::Type type);
	TRTile* GetTile(const DirectX::XMINT2& at);
	TRTile* GetTile(const DirectX::XMFLOAT3& pos);
	void ExtendTiles(bool forceFull = false);
	void GameOver();
	bool IsPlaying() const { return m_Playing; }
private:
	//
	bool m_Playing = false;
	int m_TileBonus = 20;
	float m_Multiplier;
	int m_Score;
	int m_PreviousScore;
	//
	static const size_t TILES_AHEAD = 12;
	TRCharacter* m_pPlayer;
	map<DirectX::XMINT2, TRTile*, XMINT2Less> m_Tiles;
	void InitializeMaterials() const;
	void InitializeGame();
	void Initialize() override;
	void Update() override;
	void Draw() override;

	bool m_GameOver;
	void Restart();

	//PP
	int m_CurrentSet = 0;
	PostFilters* m_pFilterEffect;
	PostFog* m_pFogEffect;
	void InitializePPSets();
	std::vector<PPVariableSet> m_Sets;

	//text
	SpriteFont* m_pFont64;
	SpriteFont* m_pFont48;

	//menu
	SpriteComponent* m_pMenuBanner = nullptr;


	//Audio
	FMOD::Sound* m_pGameMusic;
	FMOD::Sound* m_pMenuMusic;
	FMOD::Channel* m_pChannel;
	void InitializeSounds();


};


