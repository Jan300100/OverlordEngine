#include "stdafx.h"
#include "GAResource.h"

namespace GA
{
	const Resource::Params& Resource::GetParams() const
	{
		return m_Params;
	}

	Resource::~Resource()
	{
		Release();
	}

	namespace Memory
	{
		Access operator|(Access lhs, Access rhs)
		{
			return static_cast<Access>(static_cast<int>(lhs) | static_cast<int>(rhs));
		}
	}
}