#pragma once
#include <string>
#include "InstancedRenderer.h"


class InstancedRenderer;
class MeshFilter;
typedef std::pair<const char*, uint32_t> TypeInfo;

class InstanceBase
{
public:
	InstanceBase(const InstancedRenderer::Key& key, InstancedRenderer* pRenderer);
	
	InstanceBase(const InstanceBase&);
	InstanceBase(InstanceBase&&) = delete;
	InstanceBase& operator=(const InstanceBase&);
	InstanceBase& operator=(InstanceBase&&) = delete;

	virtual ~InstanceBase();
	InstancedRenderer::Key GetKey() const { return m_Key; }
protected:
	InstancedRenderer::Key m_Key;
	friend class InstancedRenderer;
	InstancedRenderer* m_pRenderer;
	void* m_pData;
	void Initialize();
	bool m_Initialized;
};


template <typename T>
class Instance : public InstanceBase
{
public:
	Instance(const InstancedRenderer::Key& key, InstancedRenderer* pRenderer);
	Instance(const Instance& other);
	virtual ~Instance() override{};
	Instance& operator=(const Instance& other);
	void Initialize(const T& initialData);
	T* Data() { return (T*)(m_pData); }
	static TypeInfo m_TypeInfo;
};

template<typename T>
TypeInfo Instance<T>::m_TypeInfo{ make_pair(typeid(T).name() , static_cast<uint32_t>(sizeof(T))) };


template<typename T>
inline Instance<T>::Instance(const InstancedRenderer::Key& key, InstancedRenderer* pRenderer)
	: InstanceBase{ key, pRenderer }
{
}

template<typename T>
inline Instance<T>::Instance(const Instance& other)
	: InstanceBase(other)
{
	
}

template<typename T>
inline Instance<T>& Instance<T>::operator=(const Instance<T>& other)
{
	m_Key = other.GetKey();
	m_pRenderer = other.m_pRenderer;
	m_Initialized = false;
	return *this;
}

template<typename T>
inline void Instance<T>::Initialize(const T& initialData)
{
	InstanceBase::Initialize();
	(*Data()) = initialData;
}
