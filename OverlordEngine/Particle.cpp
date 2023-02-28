#include "stdafx.h"
#include "Particle.h"

// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
using namespace DirectX;

Particle::Particle(const ParticleEmitterSettings& emitterSettings):
	m_VertexInfo(ParticleVertex()),
	m_EmitterSettings(emitterSettings),
	m_IsActive(false),
	m_TotalEnergy(0),
	m_CurrentEnergy(0),
	m_SizeGrow(0),
	m_InitSize(0)
{}

void Particle::Update(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (m_IsActive)
	{
		float elapsed = gameContext.pGameTime->GetElapsed();
		m_CurrentEnergy -= elapsed;
		if (m_CurrentEnergy < 0)
		{
			m_IsActive = false;
			return;
		}

		//update position
		DirectX::XMVECTOR vel = DirectX::XMLoadFloat3(&m_EmitterSettings.Velocity);
		vel = DirectX::XMVectorScale(vel, elapsed);
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&m_VertexInfo.Position);
		pos = DirectX::XMVectorAdd(vel, pos);
		DirectX::XMStoreFloat3(&m_VertexInfo.Position, pos);

		//Color
		m_VertexInfo.Color = m_EmitterSettings.Color;
		float particleLifePercent{m_CurrentEnergy / m_TotalEnergy};
		m_VertexInfo.Color.w = particleLifePercent * 2.0f;
		
		//size
		m_VertexInfo.Size = m_InitSize + m_SizeGrow * (1-particleLifePercent);
	}
}

void Particle::Init(XMFLOAT3 initPosition)
{
	PIX_PROFILE();

	m_IsActive = true;
	//energy
	m_TotalEnergy = randF(m_EmitterSettings.MinEnergy, m_EmitterSettings.MaxEnergy);
	m_CurrentEnergy = m_TotalEnergy;
	//pos
	DirectX::XMVECTOR randomDir = DirectX::XMVECTOR{1,0,0};
	DirectX::XMMATRIX randomRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(randF(-DirectX::XM_PI, DirectX::XM_PI), randF(-DirectX::XM_PI, DirectX::XM_PI), randF(-DirectX::XM_PI, DirectX::XM_PI));
	randomDir = DirectX::XMVector3TransformNormal(randomDir, randomRotationMatrix);
	float randomDist = randF(m_EmitterSettings.MinEmitterRange, m_EmitterSettings.MaxEmitterRange);
	DirectX::XMVECTOR initPos = DirectX::XMLoadFloat3( &initPosition );
	initPos = DirectX::XMVectorAdd(initPos, DirectX::XMVectorScale(randomDir, randomDist));
	DirectX::XMStoreFloat3(&m_VertexInfo.Position, initPos);

	//size
	m_InitSize = randF(m_EmitterSettings.MinSize, m_EmitterSettings.MaxSize);
	m_VertexInfo.Size = m_InitSize;

	m_SizeGrow = randF(m_EmitterSettings.MinSizeGrow, m_EmitterSettings.MaxSizeGrow);
	m_VertexInfo.Rotation = randF(-DirectX::XM_PI, DirectX::XM_PI);
}
