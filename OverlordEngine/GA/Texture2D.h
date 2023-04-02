#pragma once
#include <GA/Resource.h>

namespace GA
{
	class Texture2D : public Resource
	{
	public:
		struct Params
		{
			Resource::LifeTime lifeTime;
			Resource::CPUUpdateFrequency cpuUpdateFreq;

			// per subresource?
			uint32_t sizeInBytes;
			void* initialData = nullptr;
		};
	public:
		// Inherited via Resource
		virtual std::any GetInternal() = 0;
		virtual void* Map() = 0;
		virtual void Unmap() = 0;
	};
}