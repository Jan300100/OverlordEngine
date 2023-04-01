#pragma once
#include <GA/Resource.h>
#include <memory>

namespace GA
{

	class Buffer : public Resource
	{
	public:
		enum class Type
		{
			Vertex,
			Index,
			Structured,
			Constant,
		};

		struct Params
		{
			Type type;
			uint32_t sizeInBytes;
			Resource::LifeTime lifeTime;
		};

	protected:
		Buffer(const Params& params);
		virtual ~Buffer();
		uint32_t m_SizeInBytes;
		Type m_BufferType;
	};
}