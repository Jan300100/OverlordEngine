#pragma once
#include "GameObject.h"

class ControllerComponent;
class CameraComponent;
class TRTile;

class TRCharacter final : public GameObject
{
public:

	enum Actions : UINT
	{
		JUMP, SLIDE, TURN_R, TURN_L, JUMP2, SLIDE2, TURN_R2, TURN_L2
	};
	enum TURN
	{
		Dont = 0, Right = 1, Left = -1
	};

	TRCharacter(float initialSpeed, float height, float radius, float gravity);
	virtual ~TRCharacter();

	TRCharacter(const TRCharacter& other) = delete;
	TRCharacter(TRCharacter&& other) noexcept = delete;
	TRCharacter& operator=(const TRCharacter& other) = delete;
	TRCharacter& operator=(TRCharacter&& other) noexcept = delete;

	int GetRotationY() const { return m_OriginalRotationY; }
	float GetExactRotationY() const { return m_RotationY; }

	DirectX::XMINT2 GetOldestTile() const;
	DirectX::XMINT2 GetCurrentTile() const;
	void SetCurrentTile(const DirectX::XMINT2& tile);
	void CanTurn(bool can) { m_CanTurn = can; }
private:


	void Initialize(const GameContext& gameContext) override;
	void PostInitialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;


	//leaning
	float m_LeanSpeed;

	//ROTATING
	bool m_Turning;
	int m_OriginalRotationY;
	TURN m_Turn; //value that says what will happen at next node.
	float m_RotationY; //int because i want to be sure there are no floating point errors.
	//
	CameraComponent* m_pCamera;
	ControllerComponent* m_pController;

	//SLIDING
	float m_SlideDuration, m_SlideTimer;

	//HorizontalMouse
	float m_Sensitivity;

	float m_VerticalVelocity;
	float m_Height;
	float m_Radius;
	float m_JumpSpeed;
	float m_Speed;
	float m_SpeedIncrease;
	float m_RotationSpeed;
	float m_MaxSpeed;

	float m_Gravity;

	//

	int m_CurrentTile = 0;
	static const int TILE_HISTORY = 6;
	DirectX::XMINT2 m_TileHistory[TILE_HISTORY];
	bool m_CanTurn;


	enum class State
	{
		Idle, Sprint, Slide, Jump
	};
	//animations
	State m_State;
	void UpdateAnimations();
	ModelComponent* m_pModel;

	//Audio
	void InitializeSounds();
	FMOD::Sound* m_pJumpSound;
	FMOD::Sound* m_pRunningSound;
	FMOD::Sound* m_pSlideSound;
	FMOD::Channel* m_pSlideChannel;
	FMOD::Channel* m_pRunChannel;

};