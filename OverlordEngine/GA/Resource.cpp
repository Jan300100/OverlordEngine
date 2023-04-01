#include "stdafx.h"
#include "Resource.h"

namespace GA
{
	Resource::Resource(Interface* i, Type type, LifeTime lifeTime)
		: m_Type(type)
		, m_LifeTime(lifeTime)
		, m_pInterface(i)
	{
	}
}