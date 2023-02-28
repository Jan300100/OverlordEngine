#include "stdafx.h"
#include "Instance.h"


InstanceBase::InstanceBase(const InstancedRenderer::Key& key, InstancedRenderer* pRenderer)
	: m_Key{ key }
	, m_pRenderer{ pRenderer }
	, m_Initialized{}
{

}

InstanceBase::InstanceBase(const InstanceBase& other)
{
	m_Key = other.m_Key;
	m_pRenderer = other.m_pRenderer;
	m_Initialized = false;
}

InstanceBase& InstanceBase::operator=( const InstanceBase& other)
{
	m_Key = other.m_Key;
	m_pRenderer = other.m_pRenderer;
	m_Initialized = false;
	return *this;
}

InstanceBase::~InstanceBase()
{
	if (m_pRenderer && m_Initialized)
		m_pRenderer->Remove(this);
}

void InstanceBase::Initialize()
{
	if (m_Initialized == false)
	{
		m_pRenderer->Add(m_Key, this);
		m_Initialized = true;
	}
}
