#pragma once

class IRenderer;
class RenderTarget;

class OverlordGame
{
public:
	OverlordGame(const OverlordGame& other) = delete;
	OverlordGame(OverlordGame&& other) noexcept = delete;
	OverlordGame& operator=(const OverlordGame& other) = delete;
	OverlordGame& operator=(OverlordGame&& other) noexcept = delete;
	OverlordGame();
	virtual ~OverlordGame();

	static LRESULT CALLBACK WindowsProcedureStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static const GameSettings& GetGameSettings() { return m_GameSettings; }

	HRESULT Run(HINSTANCE hInstance);

	IRenderer* GetRenderer() const;

protected:
	virtual void OnGamePreparing(GameSettings& gameSettings){ UNREFERENCED_PARAMETER(gameSettings); }
	virtual LRESULT WindowProcedureHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual void Initialize() = 0;
	bool m_IsActive;

private:

	//FUNCTIONS
	//Initializations
	HRESULT InitializeWindow();
	HRESULT InitializeRenderer();
	HRESULT InitializeImGui();
	HRESULT InitializePhysX() const;
	HRESULT InitializeGame();

	void GameLoop() const;

	//Windows Proc
	void StateChanged(int state, bool active);
	LRESULT WindowsProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	//MEMBERS
	static GameSettings m_GameSettings;

	HINSTANCE m_hInstance;
	HWND m_WindowHandle;	

	IRenderer* m_pRenderer;
};

