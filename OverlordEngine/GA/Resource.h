#pragma once
#include <cstdint>
#include <any>

namespace GA
{
	class Interface;

	class Resource
	{
	public:

		enum class CPUUpdateFrequency
		{
			Never,
			Possible,
			Frequent,
		};

		enum class LifeTime
		{
			// Transient,
			Permanent,
		};

	public:
		virtual std::any GetInternal() = 0;

		virtual void* Map() = 0;
		virtual void Unmap() = 0;

	protected:
		enum class Type
		{
			Buffer,
			Texture2D,
		} m_Type;

		LifeTime m_LifeTime;
		CPUUpdateFrequency m_UpdateFreq;

		Interface* m_pInterface;
	protected:
		Resource(Interface* pGAInterface, Type type, LifeTime lifeTime, CPUUpdateFrequency updateFreq);
		virtual ~Resource() = default;
	};
}