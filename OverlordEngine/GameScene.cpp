#include "stdafx.h"
#include "GameScene.h"
#include "GameObject.h"
#include "SceneManager.h"
#include "OverlordGame.h"
#include "Prefabs.h"
#include "Components.h"
#include "DebugRenderer.h"
#include "RenderTarget.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "PhysxProxy.h"
#include "SoundManager.h"
#include <algorithm>
#include "PostProcessingMaterial.h"
#include "InstancedRenderer.h"

GameScene::GameScene(std::wstring sceneName):
	m_pChildren(std::vector<GameObject*>()),
	m_GameContext(GameContext()),
	m_IsInitialized(false),
	m_SceneName(std::move(sceneName)),
	m_pDefaultCamera(nullptr),
	m_pActiveCamera(nullptr),
	m_pPhysxProxy(nullptr),
	m_pInstancedRenderer{ new InstancedRenderer{m_GameContext} }
{
}

GameScene::~GameScene()
{
	SafeDelete(m_GameContext.pGameTime);
	SafeDelete(m_GameContext.pInput);
	SafeDelete(m_GameContext.pMaterialManager);
	SafeDelete(m_GameContext.pShadowMapper);

	for (auto pChild : m_pChildren)
	{
		SafeDelete(pChild);
	}
	SafeDelete(m_pPhysxProxy);

	//delete pp stuff
	for (auto pMat : m_pPostProcessingEffects)
	{
		SafeDelete(pMat);
	}

	SafeDelete(m_pInstancedRenderer);

}

void GameScene::AddChild(GameObject* obj)
{
	PIX_PROFILE();

#if _DEBUG
	if (obj->m_pParentScene)
	{
		if (obj->m_pParentScene == this)
			Logger::LogWarning(L"GameScene::AddChild > GameObject is already attached to this GameScene");
		else
			Logger::LogWarning(
				L"GameScene::AddChild > GameObject is already attached to another GameScene. Detach it from it's current scene before attaching it to another one.");

		return;
	}

	if (obj->m_pParentObject)
	{
		Logger::LogWarning(
			L"GameScene::AddChild > GameObject is currently attached to a GameObject. Detach it from it's current parent before attaching it to another one.");
		return;
	}
#endif

	obj->m_pParentScene = this;
	obj->RootInitialize(m_GameContext);
	m_pChildren.push_back(obj);
}

void GameScene::RemoveChild(GameObject* obj, bool deleteObject)
{
	PIX_PROFILE();

	const auto it = find(m_pChildren.begin(), m_pChildren.end(), obj);

#if _DEBUG
	if (it == m_pChildren.end())
	{
		Logger::LogWarning(L"GameScene::RemoveChild > GameObject to remove is not attached to this GameScene!");
		return;
	}
#endif

	m_pChildren.erase(it);
	if (deleteObject)
	{
		delete obj;
		obj = nullptr;
	}
	else
		obj->m_pParentScene = nullptr;
}

void GameScene::RootInitialize(IRenderer* pRenderer)
{
	PIX_PROFILE();

	if (m_IsInitialized)
		return;

	//Create DefaultCamera
	auto freeCam = new FreeCamera();
	freeCam->SetRotation(30, 0);
	freeCam->GetTransform()->Translate(0, 50, -80);
	AddChild(freeCam);
	m_pDefaultCamera = freeCam->GetComponent<CameraComponent>();
	m_pActiveCamera = m_pDefaultCamera;
	m_GameContext.pCamera = m_pDefaultCamera;

	//Create GameContext
	m_GameContext.pGameTime = new GameTime();
	m_GameContext.pGameTime->Reset();
	m_GameContext.pGameTime->Stop();

	m_GameContext.pInput = new InputManager();
	InputManager::Initialize();

	m_GameContext.pMaterialManager = new MaterialManager();
	m_GameContext.pShadowMapper = new ShadowMapRenderer();

	m_GameContext.pRenderer = pRenderer;

	//Initialize ShadowMapper
	m_GameContext.pShadowMapper->Initialize(m_GameContext);

	// Initialize Physx
	m_pPhysxProxy = new PhysxProxy();
	m_pPhysxProxy->Initialize(this);

	//User-Scene Initialize
	Initialize();

	//Root-Scene Initialize
	for (auto pChild : m_pChildren)
	{
		pChild->RootInitialize(m_GameContext);
	}

	for (auto pMat : m_pPostProcessingEffects)
	{
		pMat->Initialize(m_GameContext);
	}

	m_IsInitialized = true;
}

void GameScene::RootUpdate()
{
	PIX_PROFILE();

	m_GameContext.pGameTime->Update();
	m_GameContext.pInput->Update();
	m_GameContext.pCamera = m_pActiveCamera;


	SoundManager::GetInstance()->GetSystem()->update();

	//User-Scene Update
	Update();

	//Root-Scene Update
	for (auto pChild : m_pChildren)
	{
		pChild->RootUpdate(m_GameContext);
	}

	m_pInstancedRenderer->Update();

	m_GameContext.pShadowMapper->SetLightBasedOnCameraSphere(m_GameContext);


	//sort effects based on index
	std::sort(m_pPostProcessingEffects.begin(), m_pPostProcessingEffects.end(), [](PostProcessingMaterial* m1
		, PostProcessingMaterial* m2)->bool {return (m1->GetRenderIndex() < m2->GetRenderIndex()); });

	m_pPhysxProxy->Update(m_GameContext);
}

void GameScene::RootDraw()
{
	PIX_PROFILE();

	//object-Scene SHADOW_PASS
	m_GameContext.pShadowMapper->Begin(m_GameContext);
	
	//Object-Scene Draw
	for (auto pChild : m_pChildren)
	{
		pChild->RootDrawShadowMap(m_GameContext);
	}

	m_pInstancedRenderer->DrawShadowMap();

	m_GameContext.pShadowMapper->End(m_GameContext);


	//User-Scene Draw
	Draw();

	//Object-Scene Draw
	for (auto pChild : m_pChildren)
	{
		pChild->RootDraw(m_GameContext);
	}

	//
	m_pInstancedRenderer->Draw();

	//Object-Scene Post-Draw
	for (auto pChild : m_pChildren)
	{
		pChild->RootPostDraw(m_GameContext);
	}

	//Draw PhysX
	m_pPhysxProxy->Draw(m_GameContext);




	//Draw Debug Stuff
	DebugRenderer::Draw(m_GameContext);


	//Post Processing
	if (!m_pPostProcessingEffects.empty())
	{
		RenderTarget* original_rt = m_GameContext.pRenderer->GetRenderTarget(); //first effect uses the first render
		RenderTarget* prev_rt = original_rt;
		RenderTarget* temp_rt;
		for (auto pMat : m_pPostProcessingEffects)
		{
			temp_rt = pMat->GetRenderTarget();
			m_GameContext.pRenderer->SetRenderTarget(temp_rt);
			pMat->Draw(m_GameContext,prev_rt, original_rt);
			prev_rt = temp_rt;
		}

		//unbind srv to bind as rt again?
		ID3D11ShaderResourceView* const pSRV[1] = { nullptr };
		m_GameContext.pRenderer->GetDeviceContext()->PSSetShaderResources(0, 1	, pSRV);


		m_GameContext.pRenderer->SetRenderTarget(original_rt); //reset rt to original
		SpriteRenderer::GetInstance()->DrawImmediate(m_GameContext, prev_rt->GetShaderResourceView(), DirectX::XMFLOAT2{ 0.0f,0.0f });

	}

	SpriteRenderer::GetInstance()->Draw(m_GameContext);
	TextRenderer::GetInstance()->Draw(m_GameContext);

}

void GameScene::RootSceneActivated()
{
	//Start Timer
	m_GameContext.pGameTime->Start();
	SceneActivated();
}

void GameScene::RootSceneDeactivated()
{
	//Stop Timer
	m_GameContext.pGameTime->Stop();
	SceneDeactivated();
}

void GameScene::RootWindowStateChanged(int state, bool active) const
{
	//TIMER
	if (state == 0)
	{
		if (active)m_GameContext.pGameTime->Start();
		else m_GameContext.pGameTime->Stop();
	}
}

void GameScene::AddPostProcessingEffect(PostProcessingMaterial* pMat)
{
	//only add it if it't not there yet
	if (std::find(m_pPostProcessingEffects.begin(), m_pPostProcessingEffects.end(), pMat) == m_pPostProcessingEffects.end())
	{
		m_pPostProcessingEffects.push_back(pMat);
		pMat->Initialize(m_GameContext);
	}
}

void GameScene::RemoveostProcessingEffect(PostProcessingMaterial* pMat)
{
	for (size_t i = 0; i < m_pPostProcessingEffects.size(); i++)
	{
		if (m_pPostProcessingEffects[i] == pMat)
		{
			SafeDelete(m_pPostProcessingEffects[i]);
			m_pPostProcessingEffects[i] = m_pPostProcessingEffects.back();
			m_pPostProcessingEffects.pop_back();
		}
	}
}

void GameScene::SetActiveCamera(CameraComponent* pCameraComponent)
{
	if (m_pActiveCamera != nullptr)
		m_pActiveCamera->m_IsActive = false;

	m_pActiveCamera = (pCameraComponent) ? pCameraComponent : m_pDefaultCamera;
	m_pActiveCamera->m_IsActive = true;
	m_GameContext.pCamera = m_pActiveCamera;
}
