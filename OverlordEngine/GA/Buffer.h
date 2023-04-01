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
			Unknown
		};

		struct Params
		{
			Type type;
			uint32_t sizeInBytes;
			Resource::LifeTime lifeTime;
		};
	public:
		Buffer(Interface* i, const Params& params);
		virtual ~Buffer();

		virtual void* GetInternal() = 0;

		virtual void* Map() = 0;
		virtual void Unmap() = 0;

	protected:
		uint32_t m_SizeInBytes;
		Type m_BufferType;
	};
}