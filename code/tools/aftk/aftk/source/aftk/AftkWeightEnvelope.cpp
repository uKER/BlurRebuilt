#include <stdafx.h>

using namespace Aftk;

C_WeightEnvelope::C_WeightEnvelope()
{
	m_pInfluenceNode = 0;
	Clear();
}

C_WeightEnvelope::C_WeightEnvelope(const C_WeightEnvelope& rhs)
{
	*this = rhs;
}

C_WeightEnvelope::~C_WeightEnvelope()
{
}

const C_WeightEnvelope& C_WeightEnvelope::operator=(const C_WeightEnvelope& rhs)
{
	m_pInfluenceNode = rhs.m_pInfluenceNode;
	m_weights = rhs.m_weights;
	m_dataType = rhs.m_dataType;

	return *this;
}

U32	C_WeightEnvelope::AddWeight(const C_Weight<F32>& weight)
{
	m_weights.PushBack(weight);
	m_dataType = E_GenericAttributeType_F32;
	return (U32)m_weights.Size()-1;
}

void C_WeightEnvelope::SetWeightInitialCapacity(U32 capacity)
{
	m_weights.Reserve(capacity);
}

void C_WeightEnvelope::SetWeight(U32 instance, const C_Weight<F32>& weight)
{
	m_weights[instance] = weight;
}

void C_WeightEnvelope::SetInfluenceNodePtr(C_SceneNode* pInfluenceNode)
{
	m_pInfluenceNode = pInfluenceNode;
}

C_SceneNode* C_WeightEnvelope::GetInfluenceNodePtr()
{
	return m_pInfluenceNode;
}

const C_SceneNode* C_WeightEnvelope::GetInfluenceNodePtr() const
{
	return m_pInfluenceNode;
}

U32 C_WeightEnvelope::GetNumWeights() const
{
	return (U32)m_weights.Size();
}

void C_WeightEnvelope::GetWeight(U32 instance, C_Weight<F32>& weight)
{
	weight = m_weights[instance];
}

void C_WeightEnvelope::GetWeight(U32 instance, C_Weight<F32>& weight) const
{
	weight = m_weights[instance];
}

E_GenericAttributeType C_WeightEnvelope::GetDataType() const
{
	return m_dataType;
}

void C_WeightEnvelope::Clear()
{
	m_weights.Clear();
	m_dataType = E_GenericAttributeType_Null;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS


#endif
