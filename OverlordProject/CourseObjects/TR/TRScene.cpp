#include "stdafx.h"

#include <imgui\imgui.h>

#include <PhysxProxy.h>
#include <OverlordGame.h>
#include <SpriteFont.h>
#include <CmdOptions.h>

#include <ContentManager.h>
#include <SoundManager.h>

#include <TextRenderer.h>
#include <DebugRenderer.h>

#include <TransformComponent.h>
#include <SpriteComponent.h>
#include <ParticleEmitterComponent.h>

#include "TRScene.h"
#include "TRCrossRoads.h"
#include "TRCharacter.h"

#include "../../Materials/TR/TRGroundMaterial.h"
#include "../../Materials/TR/TRMaterial.h"
#include "../../Materials/TR/TRPropsMaterial.h"

#include "../../Materials/Post/PostBlur.h"
#include "../../Materials/Post/PostGrayscale.h"
#include "../../Materials/Post/PostFog.h"
#include "../../Materials/Post/PostFilters.h"

const float TRScene::TILE_SIZE = 40.0f;

TRScene::TRScene()
	:GameScene{L"TRScene"}
	, m_pPlayer{}
	, m_Tiles{}
	, m_GameOver{}
	, m_Score{}
	, m_Multiplier{1.0f}
	, m_PreviousScore{}
{
}

TRScene::~TRScene()
{
	RemoveChild(m_pPlayer);
	auto copy = m_Tiles;
	m_Tiles.clear();
	for (std::pair<XMINT2, TRTile*> tile : copy)
	{
		RemoveChild(tile.second);
	}

}

void TRScene::Initialize()
{

	//GetGameContext().pGameTime->ForceElapsedUpperbound(true);

	srand(unsigned int(time(0)));
	//srand(9);

	InputAction nextSet{(int)GameActions::NextPPSet, InputTriggerState::Released, 'P' ,-1 };
	GetGameContext().pInput->AddInputAction(nextSet);
	InputAction toggleSetCreation{ (int)GameActions::ToggleSetCreation, InputTriggerState::Released, 'N' ,-1 };
	GetGameContext().pInput->AddInputAction(toggleSetCreation);
	InputAction stop{ (int)GameActions::Stop, InputTriggerState::Released, VK_ESCAPE ,-1 };
	GetGameContext().pInput->AddInputAction(stop);
	InputAction restart{ (int)GameActions::Restart, InputTriggerState::Released, VK_TAB ,-1 };
	GetGameContext().pInput->AddInputAction(restart);
	InputAction start{ (int)GameActions::Start, InputTriggerState::Released, VK_RETURN ,-1 };
	GetGameContext().pInput->AddInputAction(start);

	DebugRenderer::ToggleDebugRenderer();

	GetPhysxProxy()->EnablePhysxDebugRendering(true);
	m_pFilterEffect = new PostFilters{};
	m_pFogEffect = new PostFog{ 3 };

	AddPostProcessingEffect(m_pFilterEffect);
	AddPostProcessingEffect(m_pFogEffect);

	//init materials
	InitializeMaterials();
	
	//font
	m_pFont64 = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/Astrantia_64_Bold.fnt");
	m_pFont48 = ContentManager::Load<SpriteFont>(L"./Resources/SpriteFonts/Astrantia_48_Bold.fnt");

	//main menu banner
	if (CmdOptions::Exists(L"showDAEBanner"))
	{
		GameObject* bannerObj = new GameObject{};

		m_pMenuBanner = new SpriteComponent(L"./Resources/Textures/GP2Exam2020_MainMenu.png", DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT4(1, 1, 1, 1.f));
		bannerObj->AddComponent(m_pMenuBanner);

		AddChild(bannerObj);
		bannerObj->GetTransform()->Translate(0, 0, 0);
		bannerObj->GetTransform()->Scale(OverlordGame::GetGameSettings().Window.Width / (1920.0f /*width of the banner texture*/)
			, OverlordGame::GetGameSettings().Window.Height / (1080.f /*width of the banner texture*/), 1);
	}

	InitializePPSets();
	InitializeSounds();
	InitializeGame();
}

void TRScene::Update()
{
	PIX_PROFILE();

	if (CmdOptions::Exists(L"showfps"))
	{
		ImGui::Begin("FPS", 0,
			ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoMove
		);
		ImGui::SetWindowPos(ImVec2(25, 25), ImGuiCond_::ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(100, 25), ImGuiCond_::ImGuiCond_Always);
		ImGui::Text("FPS: %d", GetGameContext().pGameTime->GetFPS());
		ImGui::End();
	}

	static bool create{};
	if (GetGameContext().pInput->IsActionTriggered((int)GameActions::ToggleSetCreation))
	{
		create = !create;
	}
	if (create)
	{
		static float brightness{ 0 }, contrast{ 0 }, hue{ 0 }, saturation{ 1 }, fogFallof{ 1500 }, fogStrength{ 15.f };
		static XMFLOAT3 color{ 235 / 255.0f,255 / 255.0f,255 / 255.0f };
		ImGui::Begin("Filters");
		ImGui::SliderFloat("brightness", &brightness, -0.5, 0.5);
		ImGui::SliderFloat("contrast", &contrast, -1, 1);
		ImGui::SliderFloat("hue", &hue, -40, 40);
		ImGui::SliderFloat("sat", &saturation, 0.f, 2.0f);
		ImGui::SliderFloat("fogdistance", &fogFallof, 0.0f, 5000.0f);
		ImGui::SliderFloat("fogstrength", &fogStrength, 1.0f, 100.0f);
		ImGui::ColorPicker3("fogcolor", &color.x);
		ImGui::End();

		m_pFogEffect->SetFogFalloff(fogFallof);
		m_pFogEffect->SetFogColor(color);
		m_pFogEffect->SetFogStrength(fogStrength);

		m_pFilterEffect->SetBrightness(brightness);
		m_pFilterEffect->SetContrast(contrast);
		m_pFilterEffect->SetHue(hue);
		m_pFilterEffect->SetSaturation(saturation);
	}
	else
	{
		if (GetGameContext().pInput->IsActionTriggered((int)GameActions::NextPPSet))
		{
			m_Sets[(++m_CurrentSet) % m_Sets.size()].Set(m_pFogEffect, m_pFilterEffect);
		}
	}

	if (GetGameContext().pInput->IsActionTriggered((int)GameActions::Stop))
	{
		PostQuitMessage(WM_QUIT);
	}
	if (GetGameContext().pInput->IsActionTriggered((int)GameActions::Restart))
	{
		Restart();
	}

	if (m_Playing)
	{

		if (m_GameOver)
		{
			Restart();
		}
		
		m_Multiplier += 0.01f * GetGameContext().pGameTime->GetElapsed();
		m_Score += int(GetGameContext().pGameTime->GetElapsed() * 100 * m_Multiplier);

		if (GetTile(m_pPlayer->GetTransform()->GetWorldPosition()))
		{
			XMINT2 curPos = GetTile(m_pPlayer->GetTransform()->GetWorldPosition())->GetPosition();
			if (curPos.x != m_pPlayer->GetCurrentTile().x || curPos.y != m_pPlayer->GetCurrentTile().y)
			{
				ExtendTiles();
				GetTile(curPos)->AddRoadRef();
				RemoveRoad(m_pPlayer->GetOldestTile());
				m_pPlayer->SetCurrentTile(curPos);
				m_pPlayer->CanTurn(true);
				m_Score += m_TileBonus;
			}
		}
		else
		{
			GameOver();
		}
	}
	else
	{
		if (GetGameContext().pInput->IsActionTriggered((int)GameActions::Start))
		{
			GetGameContext().pInput->ForceMouseToCenter(false);
			m_Playing = true;
			if (m_pMenuBanner)
				m_pMenuBanner->SetColor({ 1,1,1,0 });
			
			//stop mainmenu audio and start game audio
			unsigned long long dspclock;
			int rate;
			auto sys = SoundManager::GetInstance()->GetSystem();
			sys->getSoftwareFormat(&rate, 0, 0);
			m_pChannel->getDSPClock(0, &dspclock);                  
			m_pChannel->addFadePoint(dspclock, 1.0f);
			m_pChannel->addFadePoint(dspclock + 3 * rate, 0);
			m_pChannel->setDelay(0, dspclock + (rate * 3), true);
			sys->playSound(m_pGameMusic, NULL, false, &m_pChannel);
			m_pChannel->getDSPClock(0, &dspclock);
			m_pChannel->addFadePoint(dspclock, 0.0f);
			m_pChannel->addFadePoint(dspclock + 1 * rate, 1.0f);
		}

	}
	

}


XMFLOAT2 TRScene::GetPosition(const XMINT2& position)
{
	return XMFLOAT2(position.x * TILE_SIZE, position.y * TILE_SIZE);
}

void TRScene::CreateFreeTile(const DirectX::XMINT2& at, size_t level)
{
	PIX_PROFILE();

	if (m_Tiles.find(at) == m_Tiles.end())
	{
		m_Tiles[at] = new TRTile{ at , {}, 0,0, level };
		AddChild(m_Tiles[at]);
	}
	else if (m_Tiles[at]->GetLevel() > level)
	{
		TRTile* pOld = m_Tiles[at];
		m_Tiles[at] = new TRTile{ at,pOld->GetDirection(), pOld->GetRoadRef(), pOld->GetFreeRef(), level };
		AddChild(m_Tiles[at]);
		RemoveChild(pOld);
	}
	m_Tiles[at]->AddFreeRef();
}

void TRScene::RemoveFreeTile(const XMINT2& at)
{
	PIX_PROFILE();

	if (m_Tiles.find(at) != m_Tiles.end())
	{
		m_Tiles[at]->RemoveFreeRef();
		if (m_Tiles[at]->GetRoadRef() <= 0 && m_Tiles[at]->GetFreeRef() <= 0)
		{
			TRTile* pOld = m_Tiles[at];
			m_Tiles.erase(at);
			RemoveChild(pOld);
		}
	}
}

void TRScene::RemoveRoad(const XMINT2& at)
{
	PIX_PROFILE();

	if (m_Tiles.find(at) != m_Tiles.end())
	{
		m_Tiles[at]->RemoveRoadRef();
		if (m_Tiles[at]->GetRoadRef() <= 0)
		{
			if (m_Tiles[at]->GetFreeRef() <= 0)
			{
				TRTile* pOld = m_Tiles[at];
				m_Tiles.erase(at);
				RemoveChild(pOld);
			}
			else
			{
				TRTile* pOld = m_Tiles[at];
				m_Tiles[at] = new TRTile{ at,pOld->GetDirection(), 0, pOld->GetFreeRef()};
				AddChild(m_Tiles[at]);
				RemoveChild(pOld);
			}
		}
	}
}

int TRScene::GetPlayerDirection()
{
	return ((m_pPlayer->GetRotationY() / 90) + 1) % 4; //0 - 4
}

DirectX::XMINT2 TRScene::GetPlayerPosition()
{
	return GetTile(m_pPlayer->GetTransform()->GetPosition())->GetPosition();
}

void TRScene::CreateRoad(const XMINT2& at, const XMINT2& direction, TRTile::Type type)
{
	PIX_PROFILE();

	//if no tile here, create a road tile
	if (m_Tiles.find(at) == m_Tiles.end())
	{
		if (type != TRTile::Type::CrossRoads)
		{
			m_Tiles[at] = ( new TRRoad{ at, direction, type });
		}
		else
		{
			m_Tiles[at] = (new TRCrossRoads{ at });
		}
		AddChild(m_Tiles[at]);
		m_Tiles[at]->AddRoadRef();
	}
	//if there is a tile, and it is not a road, change its type so it's a road.
	else if (m_Tiles[at]->GetType() == TRTile::Type::Free)
	{
		auto pOld = m_Tiles[at];
		if ((type != TRTile::Type::CrossRoads)) m_Tiles[at] = (new TRRoad{ at, direction, type, m_Tiles[at]->GetRoadRef(), m_Tiles[at]->GetFreeRef() });
		else m_Tiles[at] = (new TRCrossRoads{ at, m_Tiles[at]->GetRoadRef(), m_Tiles[at]->GetFreeRef() });
		RemoveChild(pOld);
		AddChild(m_Tiles[at]);
		m_Tiles[at]->AddRoadRef();
	}
	//else if it IS a road, and NOT a crossroads already, or we want to make a crossroads, change it to a crossroads AND update any roads pointing towards it.
	else if (m_Tiles[at]->GetType() != TRTile::Type::CrossRoads )
	{
		auto pOld = m_Tiles[at];
		m_Tiles[at] = (new TRCrossRoads{ at, m_Tiles[at]->GetRoadRef(), m_Tiles[at]->GetFreeRef() });
		RemoveChild(pOld);
		AddChild(m_Tiles[at]);
		m_Tiles[at]->AddRoadRef();
	}

}

TRTile* TRScene::GetTile(const XMINT2& at)
{
	if (m_Tiles.find(at) != m_Tiles.end())
	{
		return m_Tiles[at];
	}
	return nullptr;
}

TRTile* TRScene::GetTile(const DirectX::XMFLOAT3& pos)
{
	XMINT2 at = { int32_t((pos.x + (((signbit(pos.x) * -2) + 1) * TILE_SIZE/2.0f)) / (TILE_SIZE)) 
		, int32_t((pos.z + (((signbit(pos.z) * -2) + 1) * TILE_SIZE / 2.0f)) / (TILE_SIZE)) };
	return GetTile(at);
}

void TRScene::ExtendTiles(bool forceFull)
{
	PIX_PROFILE();

	std::vector<XMINT2> toExtend{};
	for (std::pair<XMINT2, TRTile*> tile : m_Tiles)
	{
		if (tile.second->GetType() != TRTile::Type::Free)
			toExtend.push_back(tile.first);
	}

	constexpr uint32_t largeAmount = 40;
	if (toExtend.size() > largeAmount)
	{
		Logger::LogInfo(L"Large Amount of tiles generated during ExtendTiles: " + std::to_wstring(toExtend.size()));
	}

	for (XMINT2 tileLocation : toExtend)
	{
		m_Tiles[tileLocation]->Extend(forceFull);
	}
}

void TRScene::GameOver()
{
	m_GameOver = true;
}

void TRScene::Draw()
{
	PIX_PROFILE();

	if (m_Playing)
	{
		//draw Score
		float fontSize = 64;
		wstring text = L"Score: " + to_wstring(m_Score);
		TextRenderer::GetInstance()->DrawText(m_pFont64, text
			, DirectX::XMFLOAT2( fontSize, fontSize), DirectX::XMFLOAT4({ 1,1,1,1 }));
	}
	else
	{


		//draw menu info
		
		//quit
		float fontSize = 64;
		wstring text = L"'Escape' To Quit";
		TextRenderer::GetInstance()->DrawText(m_pFont64, text
			, DirectX::XMFLOAT2(((OverlordGame::GetGameSettings().Window.Width) / 2.0f - ((text.size()) * fontSize / 8.0f)), (OverlordGame::GetGameSettings().Window.Height) - fontSize*2), DirectX::XMFLOAT4({ 1, 1, 1, 1 }));

		//back to main menu
		fontSize = 48;
		text = L"'TAB' to Restart";
		TextRenderer::GetInstance()->DrawText(m_pFont48, text
			, DirectX::XMFLOAT2(((OverlordGame::GetGameSettings().Window.Width) / 2.0f - ((text.size()) * fontSize / 8.0f)), (OverlordGame::GetGameSettings().Window.Height) - fontSize * 4), DirectX::XMFLOAT4({ 1, 1, 1, 1 }));


		//play
		fontSize = 64;
		text = L"Press 'Enter' To Play";
		float pulseSpeed = 1.75f;
		float pulse = abs(sin(GetGameContext().pGameTime->GetTotal()* pulseSpeed));
		TextRenderer::GetInstance()->DrawText(m_pFont64, text
			, DirectX::XMFLOAT2(((OverlordGame::GetGameSettings().Window.Width) / 2.0f - ((text.size()) * fontSize / 8.0f)), OverlordGame::GetGameSettings().Window.Height / 3.f), DirectX::XMFLOAT4({ pulse, pulse, pulse, pulse }));
		
		//controls
		text = L"'WASD' + Mouse";
		fontSize = 48;
		TextRenderer::GetInstance()->DrawText(m_pFont48, text
			, DirectX::XMFLOAT2(((OverlordGame::GetGameSettings().Window.Width) / 2.0f - ((text.size()) * fontSize / 8.0f))  , OverlordGame::GetGameSettings().Window.Height / 3.f + fontSize*1.5f) , DirectX::XMFLOAT4({ 1, 1, 1, 1 }));
	
		//previous score
		if ((m_PreviousScore) != 0)
		{
			text = L"Previous Score: " + std::to_wstring((m_PreviousScore));
			fontSize = 48;
			TextRenderer::GetInstance()->DrawText(m_pFont48, text
				, DirectX::XMFLOAT2(((OverlordGame::GetGameSettings().Window.Width) / 2.0f - ((text.size()) * fontSize / 8.0f)), OverlordGame::GetGameSettings().Window.Height / 3.f + fontSize * 3.f), DirectX::XMFLOAT4({ 1, 1, 1, 1 }));
		}
	}
}

void TRScene::Restart()
{
	//stop game audio and start menu audio
	unsigned long long dspclock;
	int rate;
	auto sys = SoundManager::GetInstance()->GetSystem();
	sys->getSoftwareFormat(&rate, 0, 0);
	m_pChannel->getDSPClock(0, &dspclock);
	m_pChannel->addFadePoint(dspclock, 1.0f);
	m_pChannel->addFadePoint(dspclock + 3 * rate, 0);
	m_pChannel->setDelay(0, dspclock + (rate * 3), true);
	sys->playSound(m_pMenuMusic, NULL, false, &m_pChannel);
	m_pChannel->getDSPClock(0, &dspclock);
	m_pChannel->addFadePoint(dspclock, 0.0f);
	m_pChannel->addFadePoint(dspclock + 1 * rate, 1.0f);

	//cleanup previous game
	RemoveChild(m_pPlayer);
	auto copy = m_Tiles;
	m_Tiles.clear();
	for (std::pair<XMINT2, TRTile*> tile : copy)
	{
		RemoveChild(tile.second);
	}
	m_PreviousScore = m_Score;
	m_Score = 0;
	m_Multiplier = 1.0f;

	InitializeGame();
}

void TRScene::InitializePPSets()
{
	m_Sets.push_back({ 0,0.1f,-25.0f,1.3f,{205/255.f,255 / 255.f,255 / 255.f},1400,8.0f });
	m_Sets.push_back({ -0.2f,0.1f,-2.8f, 0.8f,{124 / 255.f,124 / 255.f,124.0f / 255.f},500,2.3f });
	m_Sets.push_back({ 0,-0.1f,1, 1,{148 / 255.f,193 / 255.f,255.0f / 255.f},1500,6 });
	m_Sets[m_CurrentSet].Set(m_pFogEffect, m_pFilterEffect);
}

void TRScene::InitializeSounds()
{
	FMOD::Sound* pAmbient;
	FMOD::Channel* pChannel;

	SoundManager::GetInstance()->GetSystem()->createStream("./Resources/Audio/AmbientBirds.mp3",
		FMOD_2D
		| FMOD_LOOP_NORMAL
		, NULL, &pAmbient);
	SoundManager::GetInstance()->GetSystem()->playSound(pAmbient, NULL, false, &pChannel);
	pChannel->setVolume(0.85f);
	SoundManager::GetInstance()->GetSystem()->createStream("./Resources/Audio/AmbientWind.mp3",
		FMOD_2D
		| FMOD_LOOP_NORMAL
		, NULL, &pAmbient);
	SoundManager::GetInstance()->GetSystem()->playSound(pAmbient, NULL, false, &pChannel);
	pChannel->setVolume(0.85f);
	SoundManager::GetInstance()->GetSystem()->createStream("./Resources/Audio/DanceoftheMammoths.mp3",
		FMOD_2D
		| FMOD_LOOP_NORMAL
		, NULL, &m_pMenuMusic);
	SoundManager::GetInstance()->GetSystem()->createStream("./Resources/Audio/Dawn_of_Man.mp3",
		FMOD_2D
		| FMOD_LOOP_NORMAL
		, NULL, &m_pGameMusic);
	SoundManager::GetInstance()->GetSystem()->playSound(m_pMenuMusic, NULL, false, &m_pChannel);
}



void TRScene::InitializeMaterials() const
{
	TRMaterial* pMat;

	//shadowStuff
	GetGameContext().pShadowMapper->SetLightDirection({ -0.577f,-0.577f,0.577f });

	//static stuff, the same for all materials
	//PbrMaterial::SetLightDirection(gameContext.pShadowMapper->GetLightDirection());
	TRMaterial::SetLightDirection(XMFLOAT3{ -0.577f,-0.577f,0.577f });
	TRMaterial::SetLightIntensity(6.0f);
	TRPropsMaterial::SetNoiseTexture(L"./Resources/Textures/Perlin.png");
	TRPropsMaterial::SetNoiseUvScale(500.0f);
	TRPropsMaterial::SetNoiseHeight(50.0f);

	//RoadMaterial
	pMat = new TRMaterial{ L"./Resources/Effects/TR/TRInstanced.fx", L"tDefault", false, true };
	pMat->SetRDAM(L"Road", 1, 0, 1, 0, 1, 1, L"png");
	pMat->SetAoStrength(0.5f);
	reinterpret_cast<TRPropsMaterial*>(pMat)->FlipNormalGreenChannel(true);

	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Road));

	//BaseMaterial
	pMat = new TRMaterial{ L"./Resources/Effects/TR/TRInstanced.fx", L"tDefault", false, true };
	pMat->SetRDAM(L"Base", 1, 0, 1, 0, 1, 1, L"png");
	pMat->SetAoStrength(0.5f);
	reinterpret_cast<TRPropsMaterial*>(pMat)->FlipNormalGreenChannel(true);

	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Base));

	//props material (= fireholder, pillar, brick)
	pMat = new TRPropsMaterial{false, true};
	pMat->SetRDAM(L"Props", 1, 0, 1, 0, 1, 1, L"png");
	pMat->SetAoStrength(0.5f);
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetInfluence(0.0f);
	reinterpret_cast<TRPropsMaterial*>(pMat)->FlipNormalGreenChannel(true);
	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Props));

	//rock
	pMat = new TRPropsMaterial{false, true};
	pMat->SetRDAM(L"Rock", 0, 0, 0, 0, 1, 1, L"png");
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetInfluence(0.0f);
	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Rock));

	//bark
	pMat = new TRPropsMaterial(false, true);
	pMat->SetRDAM(L"ConiferBark", 0, 0, 0, 0, 1, 1, L"png");
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetDistanceInfluence(0.05f);
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetWindForce(0.01f);

	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Bark));

	//Conifer leaves
	pMat = new TRPropsMaterial{ false, true };
	pMat->SetRDAM(L"Conifer", 0, 0, 0, 0, 1, 1, L"tga");

	reinterpret_cast<TRPropsMaterial*>(pMat)->SetDistanceInfluence(0.05f);
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetWindForce(0.01f);

	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Conifer));

	//bush
	pMat = new TRPropsMaterial{ true, true };
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetDistanceInfluence(0.5f);
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetInfluence(0.25f);
	pMat->SetRDAM(L"Bush", 0, 0, 0, 0, 1, 1, L"tga");
	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Bush));

	//grass
	pMat = new TRPropsMaterial{ true };
	pMat->SetRDAM(L"Grass", 0, 0, 0, 0, 1, 1, L"tga");
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetDistanceInfluence(1.0f);
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetInfluence(0.5f);
	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Grass));

	//Nettle
	pMat = new TRPropsMaterial{ true };
	pMat->SetRDAM(L"Nettle", 0, 0, 0, 0, 1, 1, L"tga");
	reinterpret_cast<TRPropsMaterial*>(pMat)->SetInfluence(0.15f);

	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Nettle));

	//Oduvanchik
	pMat = new TRPropsMaterial{ true };
	pMat->SetRDAM(L"Oduvanchik", 0, 0, 0, 0, 1, 1, L"tga");
	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Oduvanchik));

	//Zont
	pMat = new TRPropsMaterial{ true };
	pMat->SetRDAM(L"Zont", 0, 0, 0, 0, 1, 1, L"tga");
	GetGameContext().pMaterialManager->AddMaterial(pMat, int(MatIds::Zont));

	//ground
	TRGroundMaterial* pgMat = new TRGroundMaterial{};
	pgMat->SetRDAM(L"Floor", 1, 1, 1, 0, 1, 1, L"png");
	pgMat->SetWorldUvScale(25);
	pgMat->SetTessFactor(8);
	pgMat->SetDisplacementAmount(0.8f);
	pgMat->SetNoiseTexture(L"./Resources/Textures/Perlin.png");
	pgMat->SetNoiseHeight(50.0f);
	pgMat->SetNoiseUvScale(500.0f);
	GetGameContext().pMaterialManager->AddMaterial(pgMat, int(MatIds::Ground));
}

void TRScene::InitializeGame()
{
	//initialize new Game
	m_pPlayer = new TRCharacter{ 40.0f, 8.0f, 1.0f, 36.0f };
	m_pPlayer->GetTransform()->Translate(0, 19.5f, 0);

	//particles
	GameObject* particleSpawner = new GameObject{};
	particleSpawner->GetTransform()->Translate(0, -5, 0);
	ParticleEmitterComponent* particles = new ParticleEmitterComponent(L"./Resources/Textures/TR/Particles/Particle.png", 500);
	particles->SetVelocity(DirectX::XMFLOAT3(0, 0.5f, 0));
	particles->SetMinSize(0.05f);
	particles->SetMaxSize(0.05f);
	particles->SetMinEnergy(3.0f);
	particles->SetMaxEnergy(6.0f);
	particles->SetMinSizeGrow(1.0f);
	particles->SetMaxSizeGrow(1.0f);
	particles->SetMinEmitterRange(10.0f);
	particles->SetMaxEmitterRange(150.0f);
	particles->SetColor(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f));
	particleSpawner->AddComponent(particles);
	m_pPlayer->AddChild(particleSpawner);

	AddChild(m_pPlayer);
	//generated the beginning
	for (size_t i = 0; i < TILES_AHEAD; i++)
	{
		ExtendTiles();
	}

	m_Playing = false;
	m_GameOver = false;
	//enable main menu banner
	if (m_pMenuBanner)
		m_pMenuBanner->SetColor({ 1,1,1,1 });
}

void PPVariableSet::Set(PostFog* pFog, PostFilters* pFilters)
{
	//fog
	pFog->SetFogColor(fogColor);
	pFog->SetFogFalloff(fogFalloff);
	pFog->SetFogStrength(fogStrength);
	//filters
	pFilters->SetBrightness(brightness);
	pFilters->SetContrast(contrast);
	pFilters->SetHue(hue);
	pFilters->SetSaturation(saturation);
}
