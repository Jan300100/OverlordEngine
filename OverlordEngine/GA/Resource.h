#pragma once
#include <cstdint>

namespace GA
{
	class Interface;

	class Resource
	{
	public:
		enum class LifeTime
		{
			// SingleFrame,
			Permanent,
		} m_LifeTime;

	public:
		virtual void* GetInternal() = 0;

		virtual void* Map() = 0;
		virtual void Unmap() = 0;
	protected:
		enum class Type
		{
			Buffer,
		} m_Type;

		Resource(Interface* i, Type type, LifeTime lifeTime);
		virtual ~Resource() = default;

		Interface* m_pInterface;
	};
}