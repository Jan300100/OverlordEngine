#include "stdafx.h"

#include "MeshFilter.h"
#include "Material.h"

#include <algorithm>
#include <GA/DX11/InterfaceDX11.h>

DirectX::XMFLOAT4 MeshFilter::m_DefaultColor = DirectX::XMFLOAT4(1,0,0,1);
DirectX::XMFLOAT4 MeshFilter::m_DefaultFloat4 = DirectX::XMFLOAT4(0, 0, 0, 0);
DirectX::XMFLOAT3 MeshFilter::m_DefaultFloat3 = DirectX::XMFLOAT3(0, 0, 0);
DirectX::XMFLOAT2 MeshFilter::m_DefaultFloat2 = DirectX::XMFLOAT2(0, 0);

MeshFilter::MeshFilter():
			m_Positions(std::vector<DirectX::XMFLOAT3>()),
			m_Normals(std::vector<DirectX::XMFLOAT3>()),
			m_Tangents(std::vector<DirectX::XMFLOAT3>()),
			m_Binormals(std::vector<DirectX::XMFLOAT3>()),
			m_TexCoords(std::vector<DirectX::XMFLOAT2>()),
			m_Colors(std::vector<DirectX::XMFLOAT4>()),
			m_Indices(std::vector<DWORD>()),
			m_BlendIndices(std::vector<DirectX::XMFLOAT4>()),
			m_BlendWeights(std::vector<DirectX::XMFLOAT4>()),
			m_AnimationClips(std::vector<AnimationClip>()),
			m_pIndexBuffer(nullptr),
			m_VertexBuffers(std::vector<VertexBufferData>()),
			m_VertexCount(0),
			m_IndexCount(0),
			m_TexCoordCount(0),
			m_HasElement(static_cast<UINT>(ILSemantic::NONE)),
			m_HasAnimations(false),
			m_BoneCount(0)
{
}


MeshFilter::~MeshFilter()
{
	m_Positions.clear();
	m_Normals.clear();
	m_TexCoords.clear();
	m_Colors.clear();
	m_Indices.clear();
	m_Tangents.clear();
	m_Binormals.clear();
	m_BlendIndices.clear();
	m_BlendWeights.clear();
	m_AnimationClips.clear();

	for_each(m_VertexBuffers.begin(), m_VertexBuffers.end(), [](VertexBufferData& data)
	{
		data.Destroy();
	});

	m_VertexBuffers.clear();
}

void MeshFilter::BuildIndexBuffer(const GameContext& gameContext)
{
	PIX_PROFILE();

	if (m_pIndexBuffer != nullptr)
		return;

	GA::Buffer::Params params;
	params.lifeTime = GA::Resource::LifeTime::Permanent;
	params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Never;
	params.sizeInBytes = static_cast<uint32_t>(sizeof(DWORD) * m_Indices.size());
	params.initialData = m_Indices.data();
	params.type = GA::Buffer::Type::Index;
	m_pIndexBuffer = gameContext.pRenderer->CreateBuffer(params);
}

int MeshFilter::GetVertexBufferId(UINT inputLayoutId)
{
	PIX_PROFILE();

	for (UINT i = 0; i<m_VertexBuffers.size(); ++i)
	{
		if (m_VertexBuffers[i].InputLayoutID == inputLayoutId)
			return i;
	}

	return -1;
}

void MeshFilter::BuildVertexBuffer(const GameContext& gameContext, Material* pMaterial)
{
	PIX_PROFILE();

	BuildVertexBuffer(gameContext, pMaterial->m_InputLayoutID, pMaterial->m_pInputLayoutSize, pMaterial->m_pInputLayoutDescriptions);
}

void MeshFilter::BuildVertexBuffer(const GameContext& gameContext, UINT inputLayoutID, UINT inputLayoutSize, const std::vector<ILDescription>& inputLayoutDescriptions)
{
	PIX_PROFILE();

	//Check if VertexBufferInfo already exists with requested InputLayout
	if (GetVertexBufferId(inputLayoutID) >= 0)
		return;


	VertexBufferData data;
	data.VertexStride = inputLayoutSize;
	data.BufferSize = data.VertexStride * m_VertexCount;
	data.VertexCount = m_VertexCount;
	data.IndexCount = m_IndexCount;

	void *pDataLocation = malloc(data.BufferSize);
	if (pDataLocation == nullptr)
	{
		Logger::LogWarning(L"MeshFilter::BuildVertexBuffer() > Failed to allocate the required memory!");
		return;
	}

	data.pDataStart = pDataLocation;
	data.InputLayoutID = inputLayoutID;

	for (UINT i = 0; i < m_VertexCount; ++i)
	{
		for (UINT j = 0; j < inputLayoutDescriptions.size(); ++j)
		{
			auto ilDescription = inputLayoutDescriptions[j];

			if (ilDescription.Classification == D3D11_INPUT_PER_VERTEX_DATA)
			{

				if (i == 0 && !HasElement(ilDescription.SemanticType))
				{
					std::wstring name = EffectHelper::GetIlSemanticName(ilDescription.SemanticType);
					Logger::LogFormat(LogLevel::Warning, L"MeshFilter::BuildVertexBuffer > Mesh \"%s\" has no vertex %s data, using a default value!", m_MeshName.c_str(), name.c_str());
				}

				switch (ilDescription.SemanticType)
				{
				case ILSemantic::POSITION:
					memcpy(pDataLocation, HasElement(ilDescription.SemanticType) ? &m_Positions[i] : &m_DefaultFloat3, ilDescription.Offset);
					break;
				case ILSemantic::NORMAL:
					memcpy(pDataLocation, HasElement(ilDescription.SemanticType) ? &m_Normals[i] : &m_DefaultFloat3, ilDescription.Offset);

					break;
				case ILSemantic::COLOR:
					memcpy(pDataLocation, HasElement(ilDescription.SemanticType) ? &m_Colors[i] : &m_DefaultColor, ilDescription.Offset);

					break;
				case ILSemantic::TEXCOORD:
					memcpy(pDataLocation, HasElement(ilDescription.SemanticType) ? &m_TexCoords[i] : &m_DefaultFloat2, ilDescription.Offset);

					break;
				case ILSemantic::TANGENT:
					memcpy(pDataLocation, HasElement(ilDescription.SemanticType) ? &m_Tangents[i] : &m_DefaultFloat3, ilDescription.Offset);

					break;
				case ILSemantic::BINORMAL:
					memcpy(pDataLocation, HasElement(ilDescription.SemanticType) ? &m_Binormals[i] : &m_DefaultFloat3, ilDescription.Offset);

					break;
				case ILSemantic::BLENDINDICES:
					memcpy(pDataLocation, HasElement(ilDescription.SemanticType) ? &m_BlendIndices[i] : &m_DefaultFloat4, ilDescription.Offset);

					break;
				case ILSemantic::BLENDWEIGHTS:
					memcpy(pDataLocation, HasElement(ilDescription.SemanticType) ? &m_BlendWeights[i] : &m_DefaultFloat4, ilDescription.Offset);

					break;
				default:
					Logger::LogError(L"MeshFilter::BuildVertexBuffer() > Unsupported SemanticType!");
					break;
				}

				pDataLocation = (char*)pDataLocation + ilDescription.Offset;
			}
		}
	}

	//fill a buffer description to copy the vertexdata into graphics memory
	GA::Buffer::Params params;
	params.cpuUpdateFreq = GA::Resource::CPUUpdateFrequency::Possible;
	params.lifeTime = GA::Resource::LifeTime::Permanent;
	params.sizeInBytes = data.BufferSize;
	params.type = GA::Buffer::Type::Vertex;
	params.initialData = data.pDataStart;

	data.pVertexBuffer = gameContext.pRenderer->CreateBuffer(params).release();

	m_VertexBuffers.push_back(data);
}

const VertexBufferData& MeshFilter::GetVertexBufferData(const GameContext& gameContext, Material* pMaterial)
{
	PIX_PROFILE();

	int possibleBuffer = GetVertexBufferId(pMaterial->m_InputLayoutID);

	if (possibleBuffer < 0)
	{
		Logger::LogWarning(L"MeshFilter::GetVertexBufferInformation(...) => No VertexBufferInformation for this material found! Building matching VertexBufferInformation (Performance Issue).");
		BuildVertexBuffer(gameContext, pMaterial);

		//Return last created vertexbufferinformation
		return m_VertexBuffers.back();
	}

	return m_VertexBuffers[possibleBuffer];
}

const VertexBufferData& MeshFilter::GetVertexBufferData(const GameContext& gameContext, UINT inputLayoutId)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);

	int possibleBuffer = GetVertexBufferId(inputLayoutId);
	if (possibleBuffer < 0)
	{
		Logger::LogError(L"MeshFilter::GetVertexBufferInformation(INPUTLAYOUT_ID) => No VertexBufferInformation for this InputLayoutID found! Initialize before retrieving.");

		//Return last created vertexbufferinformation
		return m_VertexBuffers.back();
	}
	return m_VertexBuffers[possibleBuffer];
}