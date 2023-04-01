#include "stdafx.h"
#include "GA/Buffer.h"

GA::Buffer::Buffer(Interface* i, const Params& params)
	:Resource(i, GA::Resource::Type::Buffer, params.lifeTime)
	, m_BufferType(params.type)
	, m_SizeInBytes(params.sizeInBytes)
{
}

GA::Buffer::~Buffer()
{
}
