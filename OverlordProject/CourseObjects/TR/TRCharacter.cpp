#include "stdafx.h"
#include "TRCharacter.h"
#include "TRCamera.h"
#include "TransformComponent.h"
#include "ControllerComponent.h"
#include "PhysxManager.h"
#include "PhysxProxy.h"
#include "TRScene.h"
#include "Components.h"
#include "TRHelpers.h"
#include <SoundManager.h>

#include <OverlordGame.h>
#include "ModelAnimator.h"
#include "..\..\Materials\TR\TRSkinnedMaterial.h"

TRCharacter::TRCharacter(float initialSpeed, float height, float radius, float gravity)
	:m_Speed{ initialSpeed }
	, m_Turn{ TURN::Dont }
	, m_RotationY{}
	, m_OriginalRotationY{}
	, m_VerticalVelocity{}
	, m_JumpSpeed{ gravity/2.f }
	, m_Height{ height  }
	, m_Radius{ radius }
	, m_SlideDuration{ 1.0f }
	, m_SlideTimer{}
	, m_Sensitivity{ 2.0f }
	, m_Turning{}
	, m_SpeedIncrease{ 2.00f }
	, m_MaxSpeed{ 100.0f }
	, m_Gravity{gravity}
	, m_RotationSpeed{10.0f}
	, m_CanTurn{true}
	, m_CurrentTile{0}
	, m_State{State::Sprint}
	, m_LeanSpeed{ 1.25f }
	, m_pRunChannel{}
	, m_pSlideChannel{}
{
	for (int i = TILE_HISTORY; i > 0; i--)
	{
		m_TileHistory[TILE_HISTORY -i] = { 0, -(i% TILE_HISTORY) };
	}
}

TRCharacter::~TRCharacter()
{
	m_pRunChannel->stop();
}

XMINT2 TRCharacter::GetCurrentTile() const
{
	 return m_TileHistory[m_CurrentTile%TILE_HISTORY]; 
}

XMINT2 TRCharacter::GetOldestTile() const
{
	return m_TileHistory[(m_CurrentTile + 1) % TILE_HISTORY];
}

void TRCharacter::SetCurrentTile(const XMINT2& tile)
{
	m_TileHistory[(++m_CurrentTile) % TILE_HISTORY] = tile;
}

void TRCharacter::Initialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	//	
	InputAction jumpAction{ Actions::JUMP, InputTriggerState::Pressed, 'W' ,-1 };
	gameContext.pInput->AddInputAction(jumpAction);
	InputAction slideAction{ Actions::SLIDE, InputTriggerState::Pressed, 'S' ,-1 };
	gameContext.pInput->AddInputAction(slideAction);
	InputAction leftAction{ Actions::TURN_L, InputTriggerState::Pressed, 'A' ,-1 };
	gameContext.pInput->AddInputAction(leftAction);
	InputAction rightAction{ Actions::TURN_R, InputTriggerState::Pressed, 'D' ,-1 };
	gameContext.pInput->AddInputAction(rightAction);


	//alternative
	jumpAction = { Actions::JUMP2, InputTriggerState::Pressed, VK_UP ,-1 };
	gameContext.pInput->AddInputAction(jumpAction);
	slideAction = { Actions::SLIDE2, InputTriggerState::Pressed, VK_DOWN ,-1 };
	gameContext.pInput->AddInputAction(slideAction);
	leftAction = { Actions::TURN_L2, InputTriggerState::Pressed, VK_LEFT ,-1 };
	gameContext.pInput->AddInputAction(leftAction);
	rightAction = { Actions::TURN_R2, InputTriggerState::Pressed, VK_RIGHT ,-1 };
	gameContext.pInput->AddInputAction(rightAction);

	//Scene
	TRScene* pScene = reinterpret_cast<TRScene*>(GetScene());
	pScene->CreateRoad(m_TileHistory[m_CurrentTile], { 0,1 }, TRTile::Type::Full);

	//CAMERA
	TRCamera* pCamera = new TRCamera{ DirectX::XMFLOAT3{0, 40, -45}, DirectX::XMFLOAT3{XMConvertToRadians(25.0f) , 0, 0} };

	AddChild(pCamera);

	//Create controller
	physx::PxMaterial* pMaterial = PhysxManager::GetInstance()->GetPhysics()->createMaterial(0.0f, 0.0f, 0.0f);
	m_pController = new ControllerComponent{ pMaterial, m_Radius, m_Height, L"Adventurer"};
	AddComponent(m_pController);

	//Visuals
	if (gameContext.pMaterialManager->GetMaterial(65468) == nullptr)
	{
		auto skinnedDiffuseMaterial = new TRSkinnedMaterial();
		skinnedDiffuseMaterial->SetRDAM(L"Character", 0,0,0,0,1,1,L"png");
		gameContext.pMaterialManager->AddMaterial(skinnedDiffuseMaterial, 65468);
	}


	m_pModel = new ModelComponent(L"./Resources/Meshes/TR/Adventurer.ovm");
	m_pModel->SetMaterial(65468);
	GameObject* pModelHolder = new GameObject{};
	pModelHolder->AddComponent(m_pModel);
	pModelHolder->GetTransform()->Translate(0, -(m_Height/2.0f + m_Radius), 0);
	pModelHolder->GetTransform()->Rotate(0, 180, 0);
	pModelHolder->GetTransform()->Scale(0.65f, 0.65f, 0.65f);
	AddChild(pModelHolder);





	//Audio
	InitializeSounds();
}

void TRCharacter::PostInitialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);
	//Activate Camera
	m_pCamera = GetChild<TRCamera>()->GetComponent<CameraComponent>();
	//
	m_pCamera->SetFarClippingPlane(550.0f);
	//
	GetScene()->SetActiveCamera(m_pCamera);
}

void TRCharacter::Update(const GameContext& gameContext)
{
	PIX_PROFILE();

	TRScene* pScene = reinterpret_cast<TRScene*>(GetScene());


	if (pScene->IsPlaying())
	{

		if (GetTransform()->GetPosition().y <= 0) pScene->GameOver();


#pragma region Turning
		//TURNING
		//*******
		if (m_CanTurn)
		{
			if (gameContext.pInput->IsActionTriggered(Actions::TURN_R) || gameContext.pInput->IsActionTriggered(Actions::TURN_R2))
			{
				m_Turning = true;
				m_Turn = TURN::Right;
				m_CanTurn = false;
			}
			else if (gameContext.pInput->IsActionTriggered(Actions::TURN_L) || gameContext.pInput->IsActionTriggered(Actions::TURN_L2))
			{
				m_Turning = true;
				m_Turn = TURN::Left;
				m_CanTurn = false;
			}
		}
		//TURN INTERPOLATION
		//******************
		if (m_Turning)
		{
			float rotationSpeed = m_RotationSpeed * m_Speed;
			//we are close enough to actually turn
			m_RotationY += (int(m_Turn) * rotationSpeed * gameContext.pGameTime->GetElapsed());

			bool doneRotating = false;
			switch (m_Turn)
			{
			case TRCharacter::Right:
				doneRotating = (m_RotationY > m_OriginalRotationY + (int(m_Turn) * 90.0f));
				break;
			case TRCharacter::Left:
				doneRotating = (m_RotationY < m_OriginalRotationY + (int(m_Turn) * 90.0f));
				break;
			}
			if (doneRotating)
			{
				//reset desire to turn.
				m_RotationY = m_OriginalRotationY + (int(m_Turn) * 90.0f);
				m_OriginalRotationY = m_OriginalRotationY + (int(m_Turn) * 90);
				m_Turn = TURN::Dont;
				m_Turning = false;
			}
			GetTransform()->Rotate(0, m_RotationY, 0);
		}
#pragma endregion

		//if we are standing on the ground
		if (m_pController->GetCollisionFlags().isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN))
		{
			//JUMPING
			//*******
			if (gameContext.pInput->IsActionTriggered(Actions::JUMP) || gameContext.pInput->IsActionTriggered(Actions::JUMP2))
			{
				//sound
				auto sys = SoundManager::GetInstance()->GetSystem();
				sys->playSound(m_pJumpSound, NULL, false, NULL);
				if (m_pRunChannel) m_pRunChannel->setPaused(true);
				//

				m_State = State::Jump;
				m_VerticalVelocity = m_JumpSpeed;
				//stop sliding
				m_SlideTimer = 0.0f;
			}
			//SLIDING
			//*******
			else if (gameContext.pInput->IsActionTriggered(Actions::SLIDE) || gameContext.pInput->IsActionTriggered(Actions::SLIDE2))
			{
				bool playing = false;
				if (m_pSlideChannel)
				{
					m_pSlideChannel->isPlaying(&playing);
				}
				if (!playing)
				{
					if (m_pRunChannel) m_pRunChannel->setPaused(true);
					SoundManager::GetInstance()->GetSystem()->playSound(m_pSlideSound, NULL, false, &m_pSlideChannel);
					m_pSlideChannel->setVolume(2.0f);
				}
				
				m_State = State::Slide;
				m_pController->Resize(m_Height / 3.0f);
				m_pModel->GetTransform()->Translate(0, -((m_Height / 3.0f) / 2.0f + m_Radius), 0);
				m_pController->Move(DirectX::XMFLOAT3{ 0,-m_Height / 3.0f , 0 }); //stick to the ground
				m_SlideTimer = m_SlideDuration;
			}
			else if (m_SlideTimer <= 0)
			{
				m_State = State::Sprint;
			}

		}
		else
		{
			//apply gravity
			m_VerticalVelocity -= m_Gravity * gameContext.pGameTime->GetElapsed();
		}

		//Stop sliding after the duration is over.
		m_SlideTimer -= gameContext.pGameTime->GetElapsed();
		if (m_SlideTimer < 0.0f)
		{
			m_pController->Resize(m_Height);
			m_pModel->GetTransform()->Translate(0, -(m_Height / 2.0f + m_Radius), 0);

		}

		//MOUSE MOVE
		//**********
		float xNdc = (float(gameContext.pInput->GetMousePosition().x) / OverlordGame::GetGameSettings().Window.Width) * 2 - 1;
		float desiredHorizontalPosition = xNdc * TRScene::TILE_SIZE/4.0f;
		float delta = (desiredHorizontalPosition) * gameContext.pGameTime->GetElapsed() * m_LeanSpeed;
		DirectX::XMVECTOR horizontalVel = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&GetTransform()->GetRight()), delta);


		//SLOWLY INCREASE SPEED
		if (m_Speed < m_MaxSpeed)
		{
			m_Speed += m_SpeedIncrease * gameContext.pGameTime->GetElapsed();
		}


		//FINAL MOVE
		//**********
		DirectX::XMVECTOR fw = DirectX::XMLoadFloat3(&GetTransform()->GetForward());
		fw = DirectX::XMVectorScale(fw, m_Speed * gameContext.pGameTime->GetElapsed());
		fw = DirectX::XMVectorAdd(fw, horizontalVel);
		DirectX::XMFLOAT3 velocity;
		DirectX::XMStoreFloat3(&velocity, fw);
		velocity.y = m_VerticalVelocity * gameContext.pGameTime->GetElapsed();
		m_pController->Move(velocity);
	}
	else
	{
		m_State = State::Idle;
	}
	UpdateAnimations();

}

void TRCharacter::UpdateAnimations()
{
	PIX_PROFILE();

	ModelAnimator* animator = m_pModel->GetAnimator();
	switch (m_State)
	{
	case TRCharacter::State::Sprint:
		if (animator->IsPlaying() == false || animator->GetClipName() == L"Idle")
		{
			if (m_pRunChannel)
				m_pRunChannel->setPaused(false);
			else
			{
				SoundManager::GetInstance()->GetSystem()->playSound(m_pRunningSound, NULL, false, &m_pRunChannel);
				m_pRunChannel->setVolume(1.5f);
			}
			animator->SetAnimation(int(m_State), true);
			animator->Play();
		}
		if (animator->GetClipName() == L"Sprint")
		{
			animator->SetAnimationSpeed(m_Speed * 0.01f + 0.3f);
			if (!m_pController->GetCollisionFlags().isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN))
			{
				//if not standing on anything, we are probably falling
				animator->SetAnimationSpeed(0);
			}
		}

		break;
	case TRCharacter::State::Slide:
		if (animator->GetClipName() != L"Slide")
		{

			animator->SetAnimation(int(m_State), false);
			animator->SetAnimationSpeed(1.0f);
			animator->Play();
		}
		else if (m_SlideTimer > 0)
		{
			if (animator->GetClipProgression() > 0.35f)
			{
				animator->SetAnimationSpeed(0);
				animator->SetAnimationAt(0.35f);
			}
		}
		else
		{
			animator->SetAnimationSpeed(1);
		}
		//else
		//{
		//	animator->SetAnimationAt(0.5f);
		//}
		break;
	case TRCharacter::State::Jump:

		if (animator->GetClipName() != L"Jump" || animator->IsPlaying() == false)
		{
			animator->SetAnimation(int(m_State), false);
			animator->Play();
		}
		animator->SetAnimationSpeed((abs((m_VerticalVelocity) / m_JumpSpeed) + 1.0f) / 2.0f);


		break;
	case TRCharacter::State::Idle:
	default:
		if (animator->GetClipName() != L"Idle" || animator->IsPlaying() == false)
		{
			animator->SetAnimation(int(m_State));
			animator->Play();
		}
		break;
	}
}

void TRCharacter::InitializeSounds()
{
	PIX_PROFILE();

	FMOD::System* sys = SoundManager::GetInstance()->GetSystem();

	sys->createStream("./Resources/Audio/Jump.mp3",
		FMOD_2D
		| FMOD_LOOP_OFF
		, NULL, &m_pJumpSound);
	sys->createStream("./Resources/Audio/Running.mp3",
		FMOD_2D
		| FMOD_LOOP_NORMAL
		, NULL, &m_pRunningSound);

	sys->createStream("./Resources/Audio/SlideHard.mp3",
		FMOD_2D
		| FMOD_LOOP_OFF
		, NULL, &m_pSlideSound);
}
