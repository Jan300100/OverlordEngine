#include "stdafx.h"
#include "GA/Buffer.h"

GA::Buffer::Buffer(Interface* pGAInterface, const Params& params)
	:Resource(pGAInterface, GA::Resource::Type::Buffer, params.lifeTime, params.cpuUpdateFreq)
	, m_BufferType(params.type)
	, m_SizeInBytes(params.sizeInBytes)
{
}

GA::Buffer::~Buffer()
{
}
