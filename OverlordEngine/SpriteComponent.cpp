#include "stdafx.h"

#include "SpriteComponent.h"
 #include <utility>

#include "GameObject.h"
#include "ContentManager.h"
#include "SpriteRenderer.h"
#include "TransformComponent.h"

// todo: dx11
#include <GA/DX11/Texture2DDX11.h>

SpriteComponent::SpriteComponent(std::wstring spriteAsset, DirectX::XMFLOAT2 pivot, DirectX::XMFLOAT4 color):
	m_pTexture(nullptr),
	m_SpriteAsset(std::move(spriteAsset)),
	m_Pivot(pivot),
	m_Color(color)
{
}

void SpriteComponent::Initialize(const GameContext& )
{
	m_pTexture = ContentManager::Load<GA::Texture2D>(m_SpriteAsset).get();
}

void SpriteComponent::SetTexture(const std::wstring& spriteAsset)
{
	m_SpriteAsset = spriteAsset;
	m_pTexture = ContentManager::Load<GA::Texture2D>(m_SpriteAsset).get();
}

void SpriteComponent::Update(const GameContext& )
{
}

void SpriteComponent::Draw(const GameContext& )
{
	if (!m_pTexture)
		return;

	SpriteRenderer::GetInstance()->Draw(m_pTexture
		, DirectX::XMFLOAT2{ this->GetTransform()->GetPosition().x,this->GetTransform()->GetPosition().y }
		, m_Color
		, m_Pivot
		, DirectX::XMFLOAT2{ GetTransform()->GetScale().x, GetTransform()->GetScale().y }, QuaternionToEuler(GetTransform()->GetRotation()).z, GetTransform()->GetPosition().z);
}
