#include "stdafx.h"
#include "OverlordGame.h"
#include "RenderTarget.h"
#include "SceneManager.h"
#include "ContentManager.h"
#include "PhysxManager.h"
#include "DebugRenderer.h"
#include "SoundManager.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "GameScene.h"

#include "Renderer/IRenderer.h"
#include "Renderer/DX11/DX11Renderer.h"
#include <CmdOptions.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


GameSettings OverlordGame::m_GameSettings = GameSettings();

//FOR NVIDIA GPUS ONLY
//Force NVIDIA Optimus to use the NVIDIA GPU
//extern "C" {
//	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//}

OverlordGame::OverlordGame():
	m_IsActive(true),
	m_hInstance(nullptr),
	m_WindowHandle(nullptr)
{
	Logger::Initialize();
}


OverlordGame::~OverlordGame()
{
	// Cleanup IMGUI
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//Game Cleanup
	DebugRenderer::Release();
	SpriteRenderer::DestroyInstance();
	TextRenderer::DestroyInstance();
	SceneManager::DestroyInstance();
	ContentManager::Release();
	PhysxManager::DestroyInstance();
	SoundManager::DestroyInstance();
	Logger::Release();

	m_pRenderer->Destroy();
	delete m_pRenderer;
}

HRESULT OverlordGame::Run(HINSTANCE hInstance)
{
	m_hInstance = hInstance;

	//PREPARATION
	//***********
	OnGamePreparing(m_GameSettings);

	//INITIALIZE
	//**********

	HRESULT hr = InitializeWindow();
	if(Logger::LogHResult(hr, L"OverlordGame::InitializeWindow")) return hr;

	hr = InitializeRenderer();
	if(Logger::LogHResult(hr, L"OverlordGame::InitializeDirectX")) return hr;

	hr = InitializeImGui();
	if (Logger::LogHResult(hr, L"OverlordGame::InitializeImGui")) return hr;

	hr = InitializePhysX();
	if(Logger::LogHResult(hr, L"OverlordGame::InitializePhysX")) return hr;

	hr = InitializeGame();
	if(Logger::LogHResult(hr, L"OverlordGame::InitializeGame")) return hr;

	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while(msg.message != WM_QUIT)
	{
		while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				break;
		}

		GameLoop();
	}

	//TODO: should return 'msg.wParam'
	return S_OK;
}

HRESULT OverlordGame::InitializeWindow()
{
	//1. Create Windowclass
	//*********************
	const auto className = L"OverlordWindowClass";
	WNDCLASS windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASS));
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.hIcon = nullptr;
	windowClass.hbrBackground = nullptr;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowsProcedureStatic;
	windowClass.hInstance = m_hInstance;
	windowClass.lpszClassName = className;

	if(!RegisterClass(&windowClass))
	{
		const auto error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	//2. Create Window
	//****************

	RECT r = {0, 0, m_GameSettings.Window.Width, m_GameSettings.Window.Height};
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
	const auto winWidth = r.right - r.left;
	const auto winHeight = r.bottom - r.top;

	const int x = 0;
	const int y = 0;

	m_WindowHandle = CreateWindow(className,
									m_GameSettings.Window.Title.c_str(), 
									WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
									x, 
									y, 
									winWidth, 
									winHeight, 
									NULL,
									nullptr, 
									m_hInstance, 
									this);

	if(!m_WindowHandle)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	m_GameSettings.Window.WindowHandle = m_WindowHandle;

	//3. Show The Window
	//******************
	ShowWindow(m_WindowHandle, SW_SHOWDEFAULT);

	return S_OK;
}

HRESULT OverlordGame::InitializeRenderer()
{
	if (CmdOptions::Exists(L"dx11"))
	{
		m_pRenderer = new DX11Renderer();
	}
	else
	{
		Logger::LogInfo(L"No Render api specified, falling back to directx 11");
		m_pRenderer = new DX11Renderer();
	}

	m_pRenderer->Initialize();
	return S_OK;
}

HRESULT OverlordGame::InitializeImGui()
{
	// Setup Dear ImGui context
	//IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(m_WindowHandle);

	if (CmdOptions::Exists(L"dx11"))
	{
		DX11Renderer* pRenderer = static_cast<DX11Renderer*>(m_pRenderer);
		ImGui_ImplDX11_Init(pRenderer->GetDevice(), pRenderer->GetDeviceContext());
	}
	else
	{
		Logger::LogInfo(L"No Render api specified: imgui falling back to directx11");

		DX11Renderer* pRenderer = static_cast<DX11Renderer*>(m_pRenderer);
		ImGui_ImplDX11_Init(pRenderer->GetDevice(), pRenderer->GetDeviceContext());
	}

	return S_OK;
}

HRESULT OverlordGame::InitializePhysX() const
{
	PhysxManager::GetInstance()->Init();
	return S_OK;
}

HRESULT OverlordGame::InitializeGame()
{
	//******************
	//MANAGER INITIALIZE
	ContentManager::Initialize(m_pRenderer->GetDevice());
	DebugRenderer::InitRenderer(m_pRenderer->GetDevice());
	SpriteRenderer::GetInstance()->InitRenderer(m_pRenderer->GetDevice());
	TextRenderer::GetInstance()->InitRenderer(m_pRenderer->GetDevice());
	SoundManager::GetInstance(); //Constructor calls Initialize

	// Update PP
	SceneManager::GetInstance()->Initialize(m_pRenderer,this);

	//***************
	//GAME INITIALIZE
	Initialize();

	return S_OK;
}
#pragma endregion Initializations

#pragma region
void OverlordGame::StateChanged(int state, bool active)
{
	switch (state)
	{
		//WINDOW ACTIVE/INACTIVE
	case 0:
		if (m_IsActive != active)
		{
			m_IsActive = active;
			SceneManager::GetInstance()->WindowStateChanged(state, active);
		}
		break;
		//INPUT ACTIVE/INACTIVE
	case 1:
		InputManager::SetEnabled(active);
		break;
	default: ;
	}
}

LRESULT CALLBACK OverlordGame::WindowsProcedureStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_CREATE)
	{
		CREATESTRUCT *pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCS->lpCreateParams));
	}
	else
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
			return true;
		
		OverlordGame* pThisGame = reinterpret_cast<OverlordGame*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		if(pThisGame) return pThisGame->WindowsProcedure(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT OverlordGame::WindowsProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)StateChanged(1, true);
			else StateChanged(1, false);

			return 0;
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED) StateChanged(0, false);
			else if (wParam == SIZE_RESTORED) StateChanged(0, true);
			return 0;
		case WM_SETFOCUS:
			if (HWND(wParam) == m_WindowHandle)
			{
				StateChanged(1, true);
				return 0;
			}
			break;
		case WM_KILLFOCUS:
			if (HWND(wParam) == m_WindowHandle)
			{
				StateChanged(1, false);
				return 0;
			}
			break;
		case WM_ENTERSIZEMOVE:
			StateChanged(0, false);
			StateChanged(1, false);
			break;
		case WM_EXITSIZEMOVE:
			StateChanged(0, true);
			StateChanged(1, true);
			break;
	default: ;
	}

	if(m_IsActive && WindowProcedureHook(hWnd, message, wParam, lParam) == 0)
		return 0;

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT OverlordGame::WindowProcedureHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(message);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	return -1;
}
#pragma endregion Windows Procedures

#pragma region

IRenderer* OverlordGame::GetRenderer() const
{
	return m_pRenderer;
}

void OverlordGame::GameLoop() const
{
	PIX_PROFILE();

	m_pRenderer->ClearBackBuffer();

	//***********
	//IMGUI FRAME
	{	
		PIX_PROFILE_NAME("ImGui::NewFrame");
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	//******
	//UPDATE
	SceneManager::GetInstance()->Update();

	//****
	//DRAW
	SceneManager::GetInstance()->Draw();

	
	//**********
	//IMGUI DRAW
	{
		PIX_PROFILE_NAME("ImGui::Render");
		ImGui::Render();

		if (CmdOptions::Exists(L"dx11"))
		{
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
		else 
		{
			// Logger::LogInfo(L"No Render api specified: imgui falling back to directx11");
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
	}

	m_pRenderer->Present();
}

#pragma endregion METHODS
