#include "stdafx.h"
#include "Resource.h"

namespace GA
{
	Resource::Resource(Type type, LifeTime lifeTime)
		: m_Type(type)
		, m_LifeTime(lifeTime)
	{
	}
}