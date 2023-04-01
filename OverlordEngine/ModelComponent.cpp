#include "stdafx.h"
#include "ModelComponent.h"
#include "ContentManager.h"
#include "MeshFilter.h"
#include "Material.h"
#include "ModelAnimator.h"
#include "TransformComponent.h"
#include <GA/DX11/InterfaceDX11.h>

ModelComponent::ModelComponent(std::wstring assetFile, bool castShadows):
	m_AssetFile(std::move(assetFile)),
	m_CastShadows(castShadows)
{
}

ModelComponent::~ModelComponent()
{
	SafeDelete(m_pAnimator);
}

void ModelComponent::Initialize(const GameContext& gameContext)
{
	m_pMeshFilter = ContentManager::Load<MeshFilter>(m_AssetFile);
	m_pMeshFilter->BuildIndexBuffer(gameContext);
	m_pMeshFilter->m_MeshName = m_AssetFile;

	if (m_pMeshFilter->m_HasAnimations)
		m_pAnimator = new ModelAnimator(m_pMeshFilter);

	if (m_CastShadows)
	{
		gameContext.pShadowMapper->UpdateMeshFilter(gameContext, m_pMeshFilter);
	}

	UpdateMaterial(gameContext);
};

inline void ModelComponent::UpdateMaterial(const GameContext& gameContext)
{
	if (m_MaterialSet)
	{
		m_MaterialSet = false;

		//FORWARD
		Material* mat = gameContext.pMaterialManager->GetMaterial(m_MaterialId);
		if (mat == nullptr)
		{
			Logger::LogFormat(LogLevel::Warning, L"ModelComponent::UpdateMaterial > Material with ID \"%i\" doesn't exist!",
			                  m_MaterialId);
			return;
		}

		m_pMaterial = mat;
		m_pMaterial->Initialize(gameContext);
		m_pMeshFilter->BuildVertexBuffer(gameContext, m_pMaterial);
	}
}

void ModelComponent::Update(const GameContext& gameContext)
{
	UpdateMaterial(gameContext);

	if (m_pAnimator)
		m_pAnimator->Update(gameContext);
};

void ModelComponent::DrawShadowMap(const GameContext& gameContext)
{

	if (m_pAnimator)
		gameContext.pShadowMapper->Draw(gameContext, m_pMeshFilter, GetTransform()->GetWorld(), m_pAnimator->GetBoneTransforms());
	else
		gameContext.pShadowMapper->Draw(gameContext, m_pMeshFilter, GetTransform()->GetWorld());
	
};

void ModelComponent::Draw(const GameContext& gameContext)
{
	if (!m_pMaterial)
	{
		Logger::LogWarning(L"ModelComponent::Draw() > No Material!");
		return;
	}

	m_pMaterial = gameContext.pMaterialManager->GetMaterial(m_MaterialId);

	m_pMaterial->SetEffectVariables(gameContext, this);
	

	//Set Inputlayout
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetInputLayout(m_pMaterial->GetInputLayout());

	//Set Vertex Buffer
	UINT offset = 0;
	auto vertexBufferData = m_pMeshFilter->GetVertexBufferData(gameContext, m_pMaterial);
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBufferData.pVertexBuffer, &vertexBufferData.VertexStride,
	                                               &offset);

	//Set Index Buffer
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetIndexBuffer(m_pMeshFilter->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set Primitive Topology
	if (!m_pMaterial->UsesTesselation())
	{
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else
	{
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}

	//DRAW
	auto tech = m_pMaterial->GetTechnique();
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext());
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->DrawIndexed(m_pMeshFilter->m_IndexCount,0, 0);
	}

};

void ModelComponent::SetMaterial(unsigned int materialId)
{
	m_MaterialSet = true;
	m_MaterialId = materialId;
}

unsigned int ModelComponent::GetMaterialID() const
{
	return m_MaterialId;
}
