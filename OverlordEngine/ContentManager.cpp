#include "stdafx.h"

#include "ContentManager.h"
#include "EffectLoader.h"
#include "MeshFilterLoader.h"
#include "TextureDataLoader.h"
#include "PxTriangleMeshLoader.h"
#include "PxConvexMeshLoader.h"
#include "SpriteFontLoader.h"

std::vector<BaseLoader*> ContentManager::m_Loaders = std::vector<BaseLoader*>();
GA::Interface* ContentManager::m_pGAInterface = nullptr;
bool ContentManager::m_IsInitialized = false;

void ContentManager::Release()
{
	for(BaseLoader *ldr:m_Loaders)
	{	
		ldr->Unload();
		SafeDelete(ldr);
	}

	m_Loaders.clear();
}

void ContentManager::Initialize(GA::Interface* pGAInterface)
{
	if(!m_IsInitialized)
	{
		m_pGAInterface = pGAInterface;
		m_IsInitialized = true;
		AddLoader(new EffectLoader());
		AddLoader(new MeshFilterLoader());
		AddLoader(new TextureDataLoader());
		AddLoader(new PxTriangleMeshLoader());
		AddLoader(new PxConvexMeshLoader());
		AddLoader(new SpriteFontLoader());
	}
}

void ContentManager::AddLoader(BaseLoader* loader)
{ 
	for(BaseLoader *ldr:m_Loaders)
	{	
		if(ldr->GetType()==loader->GetType())
		{
			SafeDelete(loader);
			break;
		}
	}

	m_Loaders.push_back(loader);
	loader->SetGAInterface(m_pGAInterface);
}
