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
			Unknown
		};

		struct Params
		{
			Type type;
			uint32_t sizeInBytes;
			Resource::LifeTime lifeTime;
			Resource::CPUUpdateFrequency cpuUpdateFreq;
			void* initialData = nullptr;
		};
	public:
		Buffer(Interface* pGAInterface, const Params& params);
		virtual ~Buffer();

		uint32_t GetSizeInBytes() const
		{
			return m_SizeInBytes;
		}

		virtual std::any GetInternal() = 0;

		virtual void* Map() = 0;
		virtual void Unmap() = 0;

	protected:
		uint32_t m_SizeInBytes;
		Type m_BufferType;
	};
}