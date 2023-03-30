#include "stdafx.h"
#include "GAResource.h"

const GA::Resource::Params& GA::Resource::GetParams() const
{
	return m_Params;
}

GA::Resource::~Resource()
{
	Release();
}
