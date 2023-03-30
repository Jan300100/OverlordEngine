#pragma once
#include <cstdint>

namespace GA
{
	namespace Memory
	{
		enum class Type
		{
			Upload,
			Default,
		};

		enum class Access
		{
			None = 0,
			GpuRead = 1,
			GpuWrite = 2,
			CpuRead = 3,
			CpuWrite = 4,
		};

		Access operator|(Access lhs, Access rhs);
	};

	class Resource
	{
	public:
		enum class Type
		{
			VertexBuffer,
			IndexBuffer,
			Buffer,
			Texture2D,
		};

		struct Params
		{
			Type type;

			// do I want to expose these? could hide this in the implementation, when mapping use uploadresource, and queue copy.
			Memory::Type memType;
			Memory::Access accessFlags;

			// Resource lifetime ?

			uint32_t stride; //the distance (in bytes) between the starting points of two consecutive elements. (size of element + padding)
			uint32_t width;
			uint32_t height;
			uint32_t depth;
			bool useMips;
		};

		virtual void Create(const Params& createParams) = 0;
		virtual void Release() = 0;

		const Params& GetParams() const;

		virtual ~Resource();
	private:
		Params m_Params;
	};
}