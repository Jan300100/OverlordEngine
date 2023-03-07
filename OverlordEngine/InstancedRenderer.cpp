#include "stdafx.h"
#include "InstancedRenderer.h"
#include "Material.h"
#include "MeshFilter.h"
#include "Instance.h"
#include "ContentManager.h"


InstancedRenderer::InstancedRenderer(const GameContext& gameContext)
	:m_GameContext{ gameContext }
	, m_DirtyBuffers{}
	, m_pDataMap{}
	, m_VerticesDrawn{}
{

}

InstancedRenderer::~InstancedRenderer()
{
	for (pair<Key, InstanceBuffers> bufs : m_pDataMap)
	{
		SafeRelease(bufs.second.first);
		free(bufs.second.second.first);
	}
}

bool InstancedRenderer::CreateInstanceBuffer(const Key& key)
{
	PIX_PROFILE();

	//build instance buffer
	D3D11_BUFFER_DESC iBufferDesc{};
	iBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	iBufferDesc.ByteWidth = key.typeInfo.second * key.maxInstances;
	iBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	iBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	iBufferDesc.MiscFlags = 0;
	iBufferDesc.StructureByteStride = 0;

	ID3D11Buffer* d3dBuffer;
	HRESULT result = m_GameContext.pRenderer->GetDevice()->CreateBuffer(&iBufferDesc, nullptr, &d3dBuffer);
	if (Logger::LogHResult(result, L"Failed to create instanceBUFFER in InstancedRenderer::createInstanceBuffer"))
		return false;

	void* instanceData = malloc(key.typeInfo.second * key.maxInstances);
	InstanceBuffers iBuffer{ d3dBuffer, make_pair(instanceData, vector<InstanceBase*>{}) };
	iBuffer.second.second.reserve(key.maxInstances);
	return m_pDataMap.emplace(make_pair(key, iBuffer)).second;
}

void InstancedRenderer::UpdateInstanceBuffers()
{
	PIX_PROFILE();

	for (Key key : m_DirtyBuffers)
	{
		if (m_pDataMap.find(key) != m_pDataMap.end())
		{
			InstanceBuffers& bufs = m_pDataMap[key];

			void* instanceData = bufs.second.first;

			D3D11_MAPPED_SUBRESOURCE data;
			ZeroMemory(&data, sizeof(D3D11_MAPPED_SUBRESOURCE));
			m_GameContext.pRenderer->GetDeviceContext()->Map(bufs.first, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
			memcpy(data.pData, instanceData, bufs.second.second.size() * key.typeInfo.second);
			m_GameContext.pRenderer->GetDeviceContext()->Unmap(bufs.first, 0);
		}
	}
	
	m_DirtyBuffers.clear();
}

bool InstancedRenderer::Register(const Key& key)
{
	PIX_PROFILE();

	//initalize key meshfilter and material
	key.pMeshfilter->BuildIndexBuffer(m_GameContext);

	//FOR SHADOWMAPPING
	m_GameContext.pShadowMapper->UpdateMeshFilter(m_GameContext, key.pMeshfilter);
	
	//FORWARD
	const auto mat = m_GameContext.pMaterialManager->GetMaterial(key.materialID);
	if (mat == nullptr)
	{
		Logger::LogFormat(LogLevel::Warning, L"InstancedRenderer::Register > Material with ID \"%i\" doesn't exist!",
			key.materialID);
		return false;
	}

	mat->Initialize(m_GameContext);
	key.pMeshfilter->BuildVertexBuffer(m_GameContext, mat);


	if (!CreateInstanceBuffer(key))
	{
		Logger::LogError(L"Failed to create instance buffer");
		return false;
	}
	return true;
}

void InstancedRenderer::DrawInstanced(uint32_t materialID, MeshFilter* pMeshFilter, uint32_t instances, uint32_t stride, ID3D11Buffer* pBuffer)
{
	PIX_PROFILE();

	Material* pMat = m_GameContext.pMaterialManager->GetMaterial(materialID);
	if (!pMat)
	{
		Logger::LogWarning(L"InstancedRenderer::DrawInstanced() > No Material!");
		return;
	}

	pMat->SetEffectVariables(m_GameContext, nullptr);

	//Set Inputlayout
	m_GameContext.pRenderer->GetDeviceContext()->IASetInputLayout(pMat->GetInputLayout());

	//Set Vertex Buffer
	const VertexBufferData& vBuffer = pMeshFilter->GetVertexBufferData(m_GameContext, pMat);
	UINT offsets[2] = { 0,0 };
	UINT strides[2] = { vBuffer.VertexStride, stride };
	ID3D11Buffer* buffers[2] = { vBuffer.pVertexBuffer, pBuffer };

	m_GameContext.pRenderer->GetDeviceContext()->IASetVertexBuffers(0, 2, buffers, strides, offsets);

	//Set Index Buffer
	m_GameContext.pRenderer->GetDeviceContext()->IASetIndexBuffer(pMeshFilter->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set Primitive Topology
	if (!pMat->UsesTesselation())
	{
		m_GameContext.pRenderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else
	{
		m_GameContext.pRenderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}

	//DRAW
	auto tech = pMat->GetTechnique();
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, m_GameContext.pRenderer->GetDeviceContext());
		m_GameContext.pRenderer->GetDeviceContext()->DrawIndexedInstanced(pMeshFilter->m_IndexCount, instances, 0, 0, 0);
	}
}

void InstancedRenderer::DrawInstancedShadowMap(uint32_t materialID, MeshFilter* pMeshFilter, uint32_t instances, uint32_t stride, ID3D11Buffer* pBuffer)
{
	PIX_PROFILE();

	//update shader variables in material
	Material* pMat = m_GameContext.pMaterialManager->GetMaterial(materialID);
	if (!pMat)
	{
		Logger::LogWarning(L"InstancedRenderer::DrawInstanced() > No Material!");
		return;
	}
	//early out
	if (pMat->GetShadowTechnique() == nullptr) return;

	pMat->SetEffectVariables(m_GameContext, nullptr);
	//Set Inputlayout
	m_GameContext.pRenderer->GetDeviceContext()->IASetInputLayout(pMat->GetInputLayout());

	//Set Vertex Buffer
	const VertexBufferData& vBuffer = pMeshFilter->GetVertexBufferData(m_GameContext, pMat);
	UINT offsets[2] = { 0,0 };
	UINT strides[2] = { vBuffer.VertexStride, stride };
	ID3D11Buffer* buffers[2] = { vBuffer.pVertexBuffer, pBuffer };

	m_GameContext.pRenderer->GetDeviceContext()->IASetVertexBuffers(0, 2, buffers, strides, offsets);

	//Set Index Buffer
	m_GameContext.pRenderer->GetDeviceContext()->IASetIndexBuffer(pMeshFilter->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set Primitive Topology
	if (!pMat->UsesTesselation())
	{
		m_GameContext.pRenderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else
	{
		m_GameContext.pRenderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}

	//DRAW
	ID3DX11EffectTechnique* tech = pMat->GetShadowTechnique();
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, m_GameContext.pRenderer->GetDeviceContext());
		m_GameContext.pRenderer->GetDeviceContext()->DrawIndexedInstanced(pMeshFilter->m_IndexCount, instances, 0, 0, 0);
	}

}

void InstancedRenderer::ImGuiInfo()
{
	PIX_PROFILE();

	static bool show = false;
	if (show)
	{
		ImGui::Begin("InstancedRenderer::Info", &show);
		ImGui::Text((std::to_string(m_VerticesDrawn / 3) + " Triangles").c_str());
		ImGui::Spacing();
		ImGui::Spacing();
		wstring_convert < std::codecvt_utf8<wchar_t>> conv;
		for (std::pair<Key, InstanceBuffers> bufs : m_pDataMap)
		{
			ImGui::Text((conv.to_bytes(bufs.first.pMeshfilter->m_AssetFile) + " : " + to_string(bufs.second.second.second.size())).c_str());
		}

		ImGui::End();
	}
}

void InstancedRenderer::Draw()
{
	PIX_PROFILE();

	m_VerticesDrawn = 0;
	for (pair<Key, InstanceBuffers> bufs : m_pDataMap)
	{
		uint32_t size = uint32_t(bufs.second.second.second.size());
		uint32_t stride = bufs.first.typeInfo.second;
		ID3D11Buffer* pBuffer = bufs.second.first;
		if (size > 0)
		{
			DrawInstanced(bufs.first.materialID, bufs.first.pMeshfilter, size, stride, pBuffer);
			m_VerticesDrawn += size * bufs.first.pMeshfilter->GetIndexCount();
		}
	}
}

void InstancedRenderer::DrawShadowMap()
{
	PIX_PROFILE();

	for (pair<Key, InstanceBuffers> bufs : m_pDataMap)
	{
		uint32_t size = static_cast<uint32_t>(bufs.second.second.second.size());
		uint32_t stride = bufs.first.typeInfo.second;
		ID3D11Buffer* pBuffer = bufs.second.first;
		if (size > 0)
		{
			DrawInstancedShadowMap(bufs.first.materialID, bufs.first.pMeshfilter, size, stride, pBuffer);
		}
	}
}

void InstancedRenderer::Update()
{
	PIX_PROFILE();

	UpdateInstanceBuffers();
	ImGuiInfo();
}

InstancedRenderer::Key InstancedRenderer::Add(const TypeInfo& info, uint32_t matId, MeshFilter* pMeshfilter, uint32_t maxInstances, InstanceBase* pInstance)
{
	Key key = Key{ info, matId,maxInstances, pMeshfilter };
	return Add(key, pInstance);
}

InstancedRenderer::Key InstancedRenderer::Add(const InstancedRenderer::Key& key, InstanceBase* pInstance)
{
	if (m_pDataMap.find(key) == m_pDataMap.end())
	{
		if (!Register(key))
		{
			return key;
		}
	}
	InstanceBuffers& buf = m_pDataMap[key];
	size_t id = buf.second.second.size();
	buf.second.second.push_back(pInstance);
	pInstance->m_pData = ((char*)buf.second.first + id * key.typeInfo.second); //data of this instance
	m_DirtyBuffers.insert(key); //flag for update if not already flagged
	return key;
}


void InstancedRenderer::Remove(InstanceBase* pInstance)
{
	Key key = pInstance->m_Key;
	if (m_pDataMap.find(key) == m_pDataMap.end()) return;
	
	InstanceBuffers& bufs = m_pDataMap[key];

	void* pOld = pInstance->m_pData;
	ptrdiff_t diff = (((const char*)pInstance->m_pData) - ((const char*)bufs.second.first));
	size_t id = diff / key.typeInfo.second;

	//replace it with the last item
	if (id < bufs.second.second.size())
	{
		bufs.second.second[id] = bufs.second.second.back();
		memcpy(pOld, bufs.second.second[id]->m_pData, key.typeInfo.second);
		bufs.second.second[id]->m_pData = pOld;
	}
	bufs.second.second.pop_back();
	m_DirtyBuffers.insert(key); //flag for update if not already flagged
}

bool InstancedRenderer::Key::operator<(const Key& other) const
{
	if (materialID == other.materialID)
	{
		if (typeInfo.second == other.typeInfo.second)
		{
			if (maxInstances == other.maxInstances)
			{
				if (pMeshfilter->GetIndexCount() == other.pMeshfilter->GetIndexCount())
				{
					return (typeInfo.first < other.typeInfo.first);
				}
				else return (pMeshfilter->GetIndexCount() < other.pMeshfilter->GetIndexCount());
			}
			else return (maxInstances < other.maxInstances);
		}
		else return (typeInfo.second < other.typeInfo.second);
	}
	else return(materialID < other.materialID);
}

bool InstancedRenderer::Key::operator==(const Key& other) const
{
	return (((*this) < other) == false && (other < (*this)) == false);
}
