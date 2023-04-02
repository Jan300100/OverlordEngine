#include "stdafx.h"
#include "Resource.h"

namespace GA
{
	Resource::Resource(Interface* pGAInterface, Type type, LifeTime lifeTime, CPUUpdateFrequency updateFreq)
		: m_Type(type)
		, m_LifeTime(lifeTime)
		, m_pInterface(pGAInterface)
		, m_UpdateFreq(updateFreq)
	{
	}
}