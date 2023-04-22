#pragma once
#include <vector>
#include <typeinfo>
#include "ContentLoader.h"

namespace GA
{
	class Interface;
}

class ContentManager
{
public:
	static void Initialize(GA::Interface* pGAInterface);
	static void AddLoader(BaseLoader* loader);

	template<class T> 
	static std::shared_ptr<T> Load(const std::wstring& assetFile)
	{
		const type_info& ti = typeid(T);
		for(BaseLoader* loader:m_Loaders)
		{
			const type_info& loadertype = loader->GetType();
			if(loadertype == ti)
			{
				return (static_cast<ContentLoader<T>*>(loader))->GetContent(assetFile);
			}
		}

		return nullptr;
	}

	static void Release();

private:
	ContentManager() = default;
	~ContentManager() = default;

	static std::vector<BaseLoader*> m_Loaders;
	static GA::Interface* m_pGAInterface;
	static bool m_IsInitialized;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ContentManager(const ContentManager& t);
	ContentManager& operator=(const ContentManager& t);
};

