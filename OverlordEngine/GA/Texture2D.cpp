#include "stdafx.h"
#include "Texture2D.h"

namespace GA
{
	GA::Texture2D::Texture2D(Interface* pGAInterface, const Params& params)
		:Resource(pGAInterface, GA::Resource::Type::Texture2D, params.lifeTime, params.cpuUpdateFreq)
		, m_Params(params)
	{
	}
}
