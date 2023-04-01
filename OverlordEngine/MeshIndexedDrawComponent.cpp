#include "stdafx.h"
#include "MeshIndexedDrawComponent.h"
#include "ContentManager.h"
#include "TransformComponent.h"
#include "CameraComponent.h"
#include "GameObject.h"
#include "GameScene.h"
#include <GA/DX11/InterfaceDX11.h>

ID3DX11EffectMatrixVariable* MeshIndexedDrawComponent::m_pWorldVar = nullptr;
ID3DX11EffectMatrixVariable* MeshIndexedDrawComponent::m_pWvpVar = nullptr;

MeshIndexedDrawComponent::MeshIndexedDrawComponent(unsigned int vertexCapacity, unsigned int indexCapacity):
	m_vecVertices(std::vector<VertexPosNormCol>()),
	m_vecIndices(std::vector<unsigned int>()),
	m_pVertexBuffer(nullptr),
	m_pIndexBuffer(nullptr),
	m_VertexCapacity(vertexCapacity),
	m_IndexCapacity(indexCapacity),
	m_pEffect(nullptr),
	m_pTechnique(nullptr),
	m_pInputLayout(nullptr)
{
}

MeshIndexedDrawComponent::~MeshIndexedDrawComponent()
{
	SafeRelease(m_pInputLayout);
	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pIndexBuffer);
}

void MeshIndexedDrawComponent::Initialize(const GameContext& gameContext)
{
	LoadEffect(gameContext);

	InitializeVertexBuffer(gameContext);
	InitializeIndexBuffer(gameContext);

	UpdateVertexBuffer();
	UpdateIndexBuffer();
}

void MeshIndexedDrawComponent::LoadEffect(const GameContext& gameContext)
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
	Logger::LogHResult(result, L"MeshIndexedDrawComponent::LoadEffect(...)");

	if (!m_pWorldVar)
		m_pWorldVar = m_pEffect->GetVariableBySemantic("World")->AsMatrix();

	if (!m_pWvpVar)
		m_pWvpVar = m_pEffect->GetVariableBySemantic("WorldViewProjection")->AsMatrix();
}

void MeshIndexedDrawComponent::InitializeVertexBuffer(const GameContext& gameContext)
{
	if (m_pVertexBuffer)
		SafeRelease(m_pVertexBuffer);

	//*************
	//VERTEX BUFFER
	D3D11_BUFFER_DESC vertexBuffDesc;
	vertexBuffDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	vertexBuffDesc.ByteWidth = sizeof(VertexPosNormCol) * m_VertexCapacity;
	vertexBuffDesc.CPUAccessFlags = D3D10_CPU_ACCESS_FLAG::D3D10_CPU_ACCESS_WRITE;
	vertexBuffDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	vertexBuffDesc.MiscFlags = 0;
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice()->CreateBuffer(&vertexBuffDesc, nullptr, &m_pVertexBuffer);
}

void MeshIndexedDrawComponent::InitializeIndexBuffer(const GameContext& gameContext)
{
	if (m_pIndexBuffer)
		SafeRelease(m_pIndexBuffer);

	//*************
	//INDEX BUFFER
	D3D11_BUFFER_DESC indexBufDesc;
	indexBufDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.ByteWidth = sizeof(unsigned int) * m_IndexCapacity;
	indexBufDesc.CPUAccessFlags = D3D10_CPU_ACCESS_FLAG::D3D10_CPU_ACCESS_WRITE;
	indexBufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	indexBufDesc.MiscFlags = 0;
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDevice()->CreateBuffer(&indexBufDesc, nullptr, &m_pIndexBuffer);
}

void MeshIndexedDrawComponent::UpdateVertexBuffer()
{
	const auto scene = m_pGameObject->GetScene();
	if (!scene)
	{
#if _DEBUG
		Logger::LogWarning(
			L"MeshIndexedDrawComponent::UpdateVertexBuffer > Can't update buffer, Component is not part of a scene. (= No DeviceContext)");
#endif
		return;
	}

	auto size = m_vecVertices.size();
	if (size > 0)
	{
		if (size > m_VertexCapacity)
		{
			Logger::LogInfo(L"MeshIndexedDrawComponent::UpdateVertexBuffer > Buffer size clamped. (Increase VertexCapacity)");
			size = m_VertexCapacity;
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		auto gameContext = scene->GetGameContext();
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
		memcpy(mappedResource.pData, m_vecVertices.data(), sizeof(VertexPosNormCol) * size);
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->Unmap(m_pVertexBuffer, 0);
	}
}

void MeshIndexedDrawComponent::UpdateIndexBuffer()
{
	const auto scene = m_pGameObject->GetScene();
	if (!scene)
	{
#if _DEBUG
		Logger::LogWarning(
			L"MeshIndexedDrawComponent::UpdateIndexBuffer > Can't update buffer, Component is not part of a scene. (= No DeviceContext)");
#endif
		return;
	}

	auto size = m_vecIndices.size();
	if (size > 0)
	{
		if (size > m_IndexCapacity)
		{
			Logger::LogInfo(L"MeshIndexedDrawComponent::UpdateIndexBuffer > Buffer size clamped. (Increase IndexCapacity)");
			size = m_IndexCapacity;
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		auto gameContext = scene->GetGameContext();
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
		memcpy(mappedResource.pData, m_vecIndices.data(), sizeof(unsigned int) * size);
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->Unmap(m_pIndexBuffer, 0);
	}
}

void MeshIndexedDrawComponent::Update(const GameContext& )
{
}

void MeshIndexedDrawComponent::Draw(const GameContext& gameContext)
{
	//Set Shader Variables
	if(m_vecIndices.empty())
		return;

	auto world = XMLoadFloat4x4(&GetTransform()->GetWorld());
	const auto viewProjection = XMLoadFloat4x4(&gameContext.pCamera->GetViewProjection());

	m_pWorldVar->SetMatrix(reinterpret_cast<float*>(&world));

	DirectX::XMMATRIX wvp = world * viewProjection;
	m_pWvpVar->SetMatrix(reinterpret_cast<float*>(&wvp));

	//Set Vertexbuffer
	unsigned int offset = 0;
	unsigned int stride = sizeof(VertexPosNormCol);
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set Indexbuffer
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set the input layout
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetInputLayout(m_pInputLayout);

	//Set primitive topology
	GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (unsigned int p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext());
		GA::DX11::SafeCast(gameContext.pRenderer)->GetDeviceContext()->DrawIndexed(static_cast<uint32_t>(m_vecIndices.size()), 0, 0);
	}
}

void MeshIndexedDrawComponent::AddVertex(VertexPosNormCol vertex, bool updateBuffer)
{
	m_vecVertices.push_back(vertex);

	if (updateBuffer)
		UpdateVertexBuffer();
}

void MeshIndexedDrawComponent::AddIndex(unsigned int index, bool updateBuffer)
{
	m_vecIndices.push_back(index);

	if (updateBuffer)
		UpdateIndexBuffer();
}

void MeshIndexedDrawComponent::GenerateNormals()
{
	// see https://stackoverflow.com/questions/21688529/binary-directxxmvector-does-not-define-this-operator-or-a-conversion
	using namespace DirectX;

	const int indexCount = (int)m_vecIndices.size();

	for (auto i = 0; i < indexCount; ++i)
	{
		auto v0 = m_vecVertices[m_vecIndices[i]];
		auto v1 = m_vecVertices[m_vecIndices[i + 1]];
		auto v2 = m_vecVertices[m_vecIndices[i + 2]];

		const auto vecV0 = XMLoadFloat3(&v0.Position);
		const auto vecV1 = XMLoadFloat3(&v1.Position);
		const auto vecV2 = XMLoadFloat3(&v2.Position);

		const auto ax0 = vecV2 - vecV0;
		const auto ax1 = vecV1 - vecV0;
		auto vecNormal = XMVector3Cross(ax0, ax1);
		vecNormal = XMVector3Normalize(vecNormal);

		XMFLOAT3 normal;
		XMStoreFloat3(&normal, vecNormal);

		m_vecVertices[m_vecIndices[i]].Normal = normal;
		m_vecVertices[m_vecIndices[++i]].Normal = normal;
		m_vecVertices[m_vecIndices[++i]].Normal = normal;
	}
}

void MeshIndexedDrawComponent::ClearVertexList()
{
	m_vecVertices.clear();
}

void MeshIndexedDrawComponent::ClearIndexList()
{
	m_vecIndices.clear();
}
