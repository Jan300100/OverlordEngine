#pragma once
#include "GameTime.h"
#include "CameraComponent.h"
#include "InputManager.h"
#include "MaterialManager.h"
#include "ShadowMapRenderer.h"
#include "Renderer/GARenderer.h"

class CameraComponent;

struct GameSettings
{
	GameSettings():
		Window(WindowSettings())
	{}

#pragma region
	struct WindowSettings
	{
		WindowSettings():
			Width(1280),
			Height(720),
			AspectRatio(Width/static_cast<float>(Height)),
			Title(L"Overlord Engine - TEMPLE RUN (DX11)"),
			WindowHandle(nullptr)
		{
		}

		int Width;
		int Height;
		float AspectRatio;
		std::wstring Title;
		HWND WindowHandle;
	}Window;
#pragma endregion WINDOW_SETTINGS

};

struct GameContext
{
	GameTime* pGameTime;
	CameraComponent* pCamera;
	GA::Renderer* pRenderer;
	InputManager* pInput;
	MaterialManager* pMaterialManager;
	ShadowMapRenderer* pShadowMapper;
};
