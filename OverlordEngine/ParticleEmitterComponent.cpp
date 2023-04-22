#include "stdafx.h"
#include "ParticleEmitterComponent.h"
 #include <utility>
#include "EffectHelper.h"
#include "ContentManager.h"
#include "TextureDataLoader.h"
#include "Particle.h"
#include "TransformComponent.h"
#include <GA/Buffer.h>

// TODO: dx11
#include <GA/DX11/InterfaceDX11.h>
#include <GA/DX11/Texture2DDX11.h>

ParticleEmitterComponent::ParticleEmitterComponent(std::wstring  assetFile, int particleCount) :
	m_pVertexBuffer(nullptr),
	m_pEffect(nullptr),
	m_pParticleTexture(nullptr),
	m_pInputLayout(nullptr),
	m_pInputLayoutSize(0),
	m_Settings(ParticleEmitterSettings()),
	m_ParticleCount(particleCount),
	m_ActiveParticles(0),
	m_LastParticleInit(0.0f),
	m_AssetFile(std::move(assetFile))
{
	m_Particles = std::vector<Particle*>{ (const unsigned int)m_ParticleCount };
	for (size_t i = 0; i < m_Particles.size(); i++)
	{
		m_Particles[i] = new Particle{m_Settings};
	}
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
	for (size_t i = 0; i < m_Particles.size(); i++)
	{
		SafeDelete(m_Particles[i]);
	}
	m_Particles.clear();
	SafeRelease(m_pInputLayout);
}

void ParticleEmitterComponent::Initialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	LoadEffect(gameContext);
	CreateVertexBuffer(gameContext);
	m_pParticleTexture = ContentManager::Load<GA::Texture2D>(m_AssetFile).get();
}

void ParticleEmitterComponent::LoadEffect(const GameContext& gameContext)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/ParticleRenderer.fx").get();
	m_pDefaultTechnique = m_pEffect->GetTechniqueByIndex(0);
	m_pWvpVariable = m_pEffect->GetVariableByName("matWvp")->AsMatrix();
	if (!m_pWvpVariable->IsValid())
	{
		Logger::LogError(L"ParticleEmitterComponent::LoadEffect() > Shader variable \'matWvp\' not valid!");
		return;
	}
	m_pViewInverseVariable = m_pEffect->GetVariableByName("matViewInverse")->AsMatrix();
	if (!m_pViewInverseVariable->IsValid())
	{
		Logger::LogError(L"ParticleEmitterComponent::LoadEffect() > Shader variable \'matViewInverse\' not valid!");
		return;
	}
	m_pTextureVariable = m_pEffect->GetVariableByName("particleTexture")->AsShaderResource();
	if (!m_pTextureVariable->IsValid())
	{
		Logger::LogError(L"ParticleEmitterComponent::LoadEffect() > Shader variable \'particleTexture\' not valid!");
		return;
	}

	EffectHelper::BuildInputLayout(GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice(), m_pDefaultTechnique, &m_pInputLayout, m_pInputLayoutSize);
}

void ParticleEmitterComponent::CreateVertexBuffer(const GameContext& gameContext)
{
	PIX_PROFILE();

	GA::Buffer::Params params;
	params.type = GA::Buffer::Type::Vertex;
	params.lifeTime = GA::Resource::LifeTime::Permanent;
	params.sizeInBytes= sizeof(ParticleVertex) * m_ParticleCount;
	params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Frequent;

	m_pVertexBuffer = gameContext.pRenderer->CreateBuffer(params);
}

void ParticleEmitterComponent::Update(const GameContext& gameContext)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);
	float averageEnergy{ (m_Settings.MaxEnergy + m_Settings.MinEnergy) / 2.0f };
	float particleInterval{ averageEnergy/m_ParticleCount };
	//2.
	m_LastParticleInit += gameContext.pGameTime->GetElapsed();
	//3.
	m_ActiveParticles = 0;


	//BUFFER MAPPING CODE [PARTIAL :)]
	void* pData = m_pVertexBuffer->Map();

	ParticleVertex* pBuffer = (ParticleVertex*)pData;
	//d
	for (size_t i = 0; i < m_Particles.size(); i++)
	{
		m_Particles[i]->Update(gameContext);
		if (m_Particles[i]->IsActive())
		{
			pBuffer[m_ActiveParticles++] = m_Particles[i]->GetVertexInfo();
		}
		else if (m_LastParticleInit >= particleInterval)
		{
			m_Particles[i]->Init(GetTransform()->GetWorldPosition());
			pBuffer[m_ActiveParticles++] = m_Particles[i]->GetVertexInfo();
			m_LastParticleInit = 0.0f;
		}
	}
	m_pVertexBuffer->Unmap();
}

void ParticleEmitterComponent::Draw(const GameContext& )
{}

void ParticleEmitterComponent::PostDraw(const GameContext& gameContext)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);
	
	DirectX::XMMATRIX w = DirectX::XMLoadFloat4x4(&GetTransform()->GetWorld());
	DirectX::XMMATRIX vp = DirectX::XMLoadFloat4x4(&gameContext.pCamera->GetViewProjection());
	DirectX::XMMATRIX viewInv = DirectX::XMLoadFloat4x4(&gameContext.pCamera->GetViewInverse());
	DirectX::XMMATRIX wvp = DirectX::XMMatrixMultiply(w, vp);

	m_pWvpVariable->SetMatrix(reinterpret_cast<float*>(&vp));
	m_pViewInverseVariable->SetMatrix(reinterpret_cast<float*>(&viewInv));

	if (m_pParticleTexture && m_pTextureVariable)
	{
		m_pTextureVariable->SetResource(GA::DX11::SafeCast(m_pParticleTexture)->GetSRV());
	}


	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetInputLayout(m_pInputLayout);
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT offset = 0;
	UINT stride = sizeof(ParticleVertex);

	ID3D11Buffer* internalBuf = std::any_cast<ID3D11Buffer*>(m_pVertexBuffer->GetInternal());
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetVertexBuffers(0, 1, &internalBuf, &stride, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pDefaultTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pDefaultTechnique->GetPassByIndex(p)->Apply(0, GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext());
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->DrawIndexed(m_ActiveParticles,0, 0);
	}

}
