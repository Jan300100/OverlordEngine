#pragma once
#include <map>
#include <set>

using namespace DirectX;
using namespace std;


class InstanceBase;
typedef std::pair<const char*, uint32_t> TypeInfo;
typedef pair<ID3D11Buffer*, pair<void*, vector<InstanceBase*>>> InstanceBuffers;

class InstancedRenderer
{
public:
	struct Key
	{
		TypeInfo typeInfo;
		uint32_t materialID;
		uint32_t maxInstances;
		MeshFilter* pMeshfilter;
		bool operator<(const Key& other) const;
		bool operator==(const Key& other) const;
	};

private:
	 std::map<Key, InstanceBuffers> m_pDataMap;
	 std::set<Key> m_DirtyBuffers;
	 const GameContext& m_GameContext;
	 
	 bool CreateInstanceBuffer(const Key& key);
	 void UpdateInstanceBuffers();
	 bool Register(const Key& key);
	 void DrawInstanced(uint32_t materialID, MeshFilter* pMeshFilter, uint32_t instances, uint32_t stride, ID3D11Buffer* pBuffer);
	 void DrawInstancedShadowMap(uint32_t materialID, MeshFilter* pMeshFilter, uint32_t instances, uint32_t stride, ID3D11Buffer* pBuffer);
	 //stats
	 void ImGuiInfo();
	 int m_VerticesDrawn;
public:
	static const uint32_t DEFAULT_INSTANCES = 15'000;
	void Draw();
	void DrawShadowMap();
	void Update();
	Key Add(const TypeInfo& info, uint32_t matId, MeshFilter* pMeshfilter, uint32_t maxInstances, InstanceBase* pInstance);
	Key Add(const Key& key, InstanceBase* pInstance);
	void Remove(InstanceBase* pInstance);
	InstancedRenderer(const GameContext& gameContext);
	~InstancedRenderer();

private:

};

