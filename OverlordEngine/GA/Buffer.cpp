#include "stdafx.h"
#include "GA/Buffer.h"

GA::Buffer::Buffer(const Params& params)
	:Resource(GA::Resource::Type::Buffer, params.lifeTime)
	, m_BufferType(params.type)
	, m_SizeInBytes(params.sizeInBytes)
{
}

GA::Buffer::~Buffer()
{
}
