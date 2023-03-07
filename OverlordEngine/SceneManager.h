#pragma once
#include "Singleton.h"

class GameScene;
class OverlordGame;

class SceneManager final: public Singleton<SceneManager>
{
public:
	~SceneManager();
	SceneManager(const SceneManager& other) = delete;
	SceneManager(SceneManager&& other) noexcept = delete;
	SceneManager& operator=(const SceneManager& other) = delete;
	SceneManager& operator=(SceneManager&& other) noexcept = delete;

	void AddGameScene(GameScene* pScene);
	void RemoveGameScene(GameScene* pScene);
	void SetActiveGameScene(const std::wstring& sceneName);
	void NextScene();
	void PreviousScene();
	GameScene* GetActiveScene() const { return m_ActiveScene; }
	OverlordGame* GetGame() const { return m_pGame; }
	IRenderer* GetRenderer() const { return m_pRenderer; }
private:
	// TODO: get rid of this nasty dependency
	friend class OverlordGame;
	friend class Singleton<SceneManager>;

	SceneManager();	

	void Initialize(IRenderer* pRenderer, OverlordGame* pGame);
	void WindowStateChanged(int state, bool active) const;
	void Update();
	void Draw() const;

	std::vector<GameScene*> m_pScenes;
	bool m_IsInitialized;
	GameScene* m_ActiveScene, *m_NewActiveScene;

	IRenderer* m_pRenderer;
	OverlordGame* m_pGame;

};

