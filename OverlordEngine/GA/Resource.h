#pragma once
#include <cstdint>

namespace GA
{
	class Resource
	{
	protected:
		enum class Type
		{
			Buffer,
			Texture2D,
		} m_Type;

		enum class LifeTime
		{
			SingleFrame,
			MultiFrame,
		} m_LifeTime;

		Resource(Type type, LifeTime lifeTime);
		virtual ~Resource() = default;
	};
}