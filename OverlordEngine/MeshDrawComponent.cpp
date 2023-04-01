#include "stdafx.h"
#include "MeshDrawComponent.h"
#include "ContentManager.h"
#include "TransformComponent.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "GameScene.h"
#include <GA/Buffer.h>
#include <GA/DX11/InterfaceDX11.h>

ID3DX11EffectMatrixVariable* MeshDrawComponent::m_pWorldVar = nullptr;
ID3DX11EffectMatrixVariable* MeshDrawComponent::m_pWvpVar = nullptr;

MeshDrawComponent::MeshDrawComponent(UINT triangleCapacity):
	m_pVertexBuffer(nullptr),
	m_TriangleCapacity(triangleCapacity),
	m_pEffect(nullptr),
	m_pTechnique(nullptr),
	m_pInputLayout(nullptr)
{}

MeshDrawComponent::~MeshDrawComponent()
{
	SafeRelease(m_pInputLayout);
}

void MeshDrawComponent::Initialize(const GameContext& gameContext)
{
	LoadEffect(gameContext);
	InitializeBuffer(gameContext);
	UpdateBuffer();
}

void MeshDrawComponent::LoadEffect(const GameContext& gameContext)
{
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/PosNormCol3D.fx");
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	//*****************
	//Load Input Layout (TODO: EffectUtils::CreateInputLayout(...) +> use @ Material)
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	const auto numElements = sizeof layout / sizeof layout[0];

	D3DX11_PASS_DESC PassDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	const auto result = GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice()->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
	                                                     PassDesc.IAInputSignatureSize, &m_pInputLayout);
	Logger::LogHResult(result, L"MeshDrawComponent::LoadEffect(...)");

	if (!m_pWorldVar)
		m_pWorldVar = m_pEffect->GetVariableBySemantic("World")->AsMatrix();

	if (!m_pWvpVar)
		m_pWvpVar = m_pEffect->GetVariableBySemantic("WorldViewProjection")->AsMatrix();
}

void MeshDrawComponent::InitializeBuffer(const GameContext& gameContext)
{
	GA::Buffer::Params params;
	params.lifeTime = GA::Resource::LifeTime::Permanent;
	params.sizeInBytes = sizeof(TrianglePosNormCol) * m_TriangleCapacity;
	params.type = GA::Buffer::Type::Vertex;
	m_pVertexBuffer = gameContext.pRenderer->CreateBuffer(params);
}

void MeshDrawComponent::UpdateBuffer()
{
	const auto scene = m_pGameObject->GetScene();
	if (!scene)
	{
#if _DEBUG
		Logger::LogWarning(
			L"MeshDrawComponent::UpdateBuffer > Can't update buffer, Component is not part of a scene. (= No DeviceContext)");
#endif
		return;
	}

	auto gameContext = scene->GetGameContext();
	auto size = m_vecTriangles.size();

	if (size > 0)
	{
		if (size > m_TriangleCapacity)
		{
			Logger::LogInfo(L"MeshDrawComponent::UpdateBuffer > Buffer size clamped. (Increase TriangleCapacity)");
			size = m_TriangleCapacity;
		}

		void* pData = m_pVertexBuffer->Map();
		memcpy(pData, m_vecTriangles.data(), sizeof(TrianglePosNormCol) * size);
		m_pVertexBuffer->Unmap();
	}
}

void MeshDrawComponent::Update(const GameContext& )
{
}

void MeshDrawComponent::Draw(const GameContext& gameContext)
{
	//Set Shader Variables
	if (m_vecTriangles.empty())
		return;

	auto world = XMLoadFloat4x4(&GetTransform()->GetWorld());
	const auto viewProjection = XMLoadFloat4x4(&gameContext.pCamera->GetViewProjection());

	m_pWorldVar->SetMatrix(reinterpret_cast<float*>(&world));
	auto wvp = world * viewProjection;
	m_pWvpVar->SetMatrix(reinterpret_cast<float*>(&wvp));

	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetInputLayout(m_pInputLayout);

	unsigned int offset = 0;
	unsigned int stride = sizeof(VertexPosNormCol);

	ID3D11Buffer* internalBuf = std::any_cast<ID3D11Buffer*>(m_pVertexBuffer->GetInternal());
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetVertexBuffers(0, 1, &internalBuf, &stride, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext());
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->Draw(static_cast<uint32_t>(m_vecTriangles.size() * 3), 0);
	}
}

void MeshDrawComponent::AddQuad(VertexPosNormCol vertex1, VertexPosNormCol vertex2, VertexPosNormCol vertex3,
                                VertexPosNormCol vertex4, bool updateBuffer)
{
	AddTriangle(TrianglePosNormCol(vertex1, vertex2, vertex3), false);
	AddTriangle(TrianglePosNormCol(vertex3, vertex4, vertex1), updateBuffer);
}

void MeshDrawComponent::AddQuad(QuadPosNormCol quad, bool updateBuffer)
{
	AddTriangle(TrianglePosNormCol(quad.Vertex1, quad.Vertex2, quad.Vertex3), false);
	AddTriangle(TrianglePosNormCol(quad.Vertex3, quad.Vertex4, quad.Vertex1), updateBuffer);
}

void MeshDrawComponent::AddTriangle(VertexPosNormCol vertex1, VertexPosNormCol vertex2, VertexPosNormCol vertex3,
                                    bool updateBuffer)
{
	AddTriangle(TrianglePosNormCol(vertex1, vertex2, vertex3), updateBuffer);
}

void MeshDrawComponent::AddTriangle(TrianglePosNormCol triangle, bool updateBuffer)
{
	m_vecTriangles.push_back(triangle);

	if (updateBuffer && m_IsInitialized)
		UpdateBuffer();
}

void MeshDrawComponent::RemoveTriangles()
{
	m_vecTriangles.clear();
}
