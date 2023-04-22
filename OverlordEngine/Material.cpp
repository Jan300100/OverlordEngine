#include "stdafx.h"
#include "Material.h"
#include "ContentManager.h"
#include "CameraComponent.h"
#include "ModelComponent.h"
#include "TransformComponent.h"
#include <GA/DX11/InterfaceDX11.h>

#include <StringHelper.h>

Material::Material(std::wstring effectFile, std::wstring technique, bool usesTesselation) :
	m_UsesTesselation{usesTesselation},
	m_effectFile(std::move(effectFile)), 
	m_pEffect(nullptr),
	m_TechniqueName(std::move(technique)),
	m_pTechnique(nullptr),
	m_pWorldMatrixVariable(nullptr),
	m_pViewMatrixVariable(nullptr),
	m_pViewInverseMatrixVariable(nullptr),
	m_pWvpMatrixVariable(nullptr),
	m_pProjectionMatrixVariable{nullptr},
	m_pInputLayout(nullptr),
	m_pInputLayoutSize(0),
	m_InputLayoutID(0),
	m_IsInitialized(false)
{
}

Material::~Material()
{
	SafeRelease(m_pInputLayout);
	m_pInputLayoutDescriptions.clear();
}

void Material::Initialize(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (!m_IsInitialized)
	{
		auto pos = m_effectFile.rfind('.', m_effectFile.length());
		if(pos != std::string::npos)
		{
			std::wstring const extension = m_effectFile.substr(pos + 1, m_effectFile.length() - pos);
			if (extension == L"fx")
			{
				LoadEffect(gameContext);
				m_IsInitialized = true;
			}
			else if(extension == L"fxc")
			{
				Logger::LogInfo(L"Loading precompiled shader");
				LoadCompiledEffect(gameContext);
				m_IsInitialized = true;
			}
		}
	}
}

bool Material::LoadEffect(const GameContext& gameContext)
{
	PIX_PROFILE();

	//Load Effect
	m_pEffect = ContentManager::Load<ID3DX11Effect>(m_effectFile).get();

	if (!m_TechniqueName.empty())
	{
		m_pTechnique = m_pEffect->GetTechniqueByName(StringHelpers::WStringToString(m_TechniqueName).c_str());
	}
	else
	{
		m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
	}

	//Build InputLayout
	EffectHelper::BuildInputLayout(GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice(), m_pTechnique, &m_pInputLayout, m_pInputLayoutDescriptions,
	                               m_pInputLayoutSize, m_InputLayoutID);

	auto effectVar = m_pEffect->GetVariableBySemantic("World");
	m_pWorldMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
	effectVar = m_pEffect->GetVariableBySemantic("View");
	m_pViewMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
	effectVar = m_pEffect->GetVariableBySemantic("ViewInverse");
	m_pViewInverseMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
	effectVar = m_pEffect->GetVariableBySemantic("WorldViewProjection");
	m_pWvpMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;
	effectVar = m_pEffect->GetVariableBySemantic("Projection");
	m_pProjectionMatrixVariable = (effectVar->IsValid()) ? effectVar->AsMatrix() : nullptr;

	LoadEffectVariables();

	return true;
}

bool Material::LoadCompiledEffect(const GameContext& gameContext)
{
	PIX_PROFILE();

	ID3DX11Effect* pEffect;
	D3DX11CreateEffectFromFile(m_effectFile.c_str(),
		0, GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice(), &pEffect);
	return pEffect;
}

void Material::SetEffectVariables(const GameContext& gameContext, ModelComponent* pModelComponent, bool matIdChanged)
{
	PIX_PROFILE();

	if (m_IsInitialized)
	{
		auto world = DirectX::XMMatrixIdentity();
		if (pModelComponent)
			world = XMLoadFloat4x4(&pModelComponent->GetTransform()->GetWorld());

		auto view = XMLoadFloat4x4(&gameContext.pCamera->GetView());
		auto projection = XMLoadFloat4x4(&gameContext.pCamera->GetProjection());

		if (m_pWorldMatrixVariable)
			m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<float*>(&world));

		if (m_pViewMatrixVariable)
			m_pViewMatrixVariable->SetMatrix(reinterpret_cast<float*>(&view));

		if (m_pProjectionMatrixVariable)
			m_pProjectionMatrixVariable->SetMatrix(reinterpret_cast<float*>(&projection));

		if (m_pWvpMatrixVariable)
		{
			auto wvp = world * view * projection;
			m_pWvpMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&(wvp)));
		}

		if (m_pViewInverseMatrixVariable)
		{
			auto viewInv = XMLoadFloat4x4(&gameContext.pCamera->GetViewInverse());
			m_pViewInverseMatrixVariable->SetMatrix(reinterpret_cast<float*>(&viewInv));
		}

		//in best case : only updates if the materialID changed
		if (matIdChanged)
			UpdateEffectVariables(gameContext, pModelComponent);
	}
}


bool Material::UsesTesselation() const
{
	return m_UsesTesselation;
}

void Material::SetTechnique(unsigned int id)
{
	PIX_PROFILE();

	auto t = m_pEffect->GetTechniqueByIndex(id);
	if (t->IsValid())
	{
		m_pTechnique = t;
	}
}

void Material::SetTechnique(const std::string& name)
{
	PIX_PROFILE();

	auto t = m_pEffect->GetTechniqueByName(name.c_str());
	if (t->IsValid())
	{
		m_pTechnique = t;
	}
}

ID3DX11EffectTechnique* Material::GetTechnique(const std::string& name) const
{
	PIX_PROFILE();

	auto t = m_pEffect->GetTechniqueByName(name.c_str());
	if (t->IsValid())
	{
		return t;
	}
	else return nullptr;
}