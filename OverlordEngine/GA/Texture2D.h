#pragma once
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

			size_t numMips;
			size_t width;
			size_t height;

			// multisampling only makes sense on RenderTargets, and I probably want another class for that

			// per subResource? some kind of subresourceData/Params
			// slicepitch
			// rowpitch
			// void* initialData = nullptr;
		};
	public:
		Texture2D(Interface* pGAInterface, const Params& params);
		virtual ~Texture2D() = default;

		// Inherited via Resource
		virtual std::any GetInternal() = 0;
		virtual void* Map() = 0;
		virtual void Unmap() = 0;
	protected:
		Params m_Params;
	};
}