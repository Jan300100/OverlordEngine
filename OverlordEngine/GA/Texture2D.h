#pragma once
#include <vector>
#include <GA/Resource.h>

namespace GA
{
	class Texture2D : public Resource
	{
	public:

		enum class Format
		{
			R8G8B8A8_UNORM,
			B8G8R8A8_UNORM,
		};

	public:
		struct Params
		{
			Resource::LifeTime lifeTime;
			Resource::CPUUpdateFrequency cpuUpdateFreq;

			Format format;

			uint32_t width;
			uint32_t height;

			uint32_t numSubresources;
			void** subresourceData;
		};

	public:
		Texture2D(Interface* pGAInterface, const Params& params);
		virtual ~Texture2D() = default;

		// Inherited via Resource
		virtual std::any GetInternal() = 0;
	
		virtual void* Map() = 0;
		virtual void Unmap() = 0;

		virtual uint32_t GetWidth() const { return m_Params.width; }
		virtual uint32_t GetHeight() const { return m_Params.height; }
	protected:
		Params m_Params;
	};
}