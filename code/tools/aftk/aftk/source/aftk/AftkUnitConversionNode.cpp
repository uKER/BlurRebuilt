#include <stdafx.h>

#define AFTK_ANIMATION_UNIT_CONVERSION_NODE_NUM_STATIC_PLUGS 1

using namespace Aftk;

C_UnitConversionNode::C_UnitConversionNode(C_Scene* pScene) 
: 
I_EvaluatableNode(pScene)
{
	OneTimeInit();
}

C_Node* C_UnitConversionNode::Clone(C_Scene* pScene) const
{
	C_Node* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_UnitConversionNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_UnitConversionNode(pScene);
		(C_UnitConversionNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_UnitConversionNode::~C_UnitConversionNode()
{
	ClearThis();

	delete m_pInputPlug;
}

C_UnitConversionNode::C_UnitConversionNode(const C_UnitConversionNode& rhs) 
: 
I_EvaluatableNode(0)
{
	OneTimeInit();

	*this = rhs;
}

C_UnitConversionNode& C_UnitConversionNode::operator =(const C_UnitConversionNode& rhs)
{
	I_EvaluatableNode::operator=(rhs);
	
	*m_pInputPlug = *rhs.m_pInputPlug;
	m_factor = rhs.m_factor;

	return *this;
}

E_ObjType C_UnitConversionNode::GetType() const
{
	return E_ObjType_UnitConversion;
}

bool C_UnitConversionNode::HasType(E_ObjType type) const
{
	if (I_EvaluatableNode::HasType(type))
		return true;
	if (type == E_ObjType_UnitConversion) 
		return true;

	return false;
}

bool C_UnitConversionNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_UnitConversionNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (!I_EvaluatableNode::IsEqualToAll(rhs, classToAbort, flags))
		return false;

	if (!IsEqualToThis(rhs, flags))
		return false;

	return true;
}

bool C_UnitConversionNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_UnitConversion))
		return false;

	const C_UnitConversionNode& rhs = (const C_UnitConversionNode&)_rhs;

	if (!EqualityTestFScalar(m_factor, rhs.m_factor))
		return false;

	return true;
}

bool C_UnitConversionNode::operator==(const C_UnitConversionNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_UnitConversionNode::operator!=(const C_UnitConversionNode &rhs) 
{
	return !((*this)==rhs);
}

U32 C_UnitConversionNode::GetHashOfThis() const
{
	return (U32)m_factor;
}

U32 C_UnitConversionNode::GetHash(E_ObjType classToAbort) const
{
	return 0;
}

bool C_UnitConversionNode::IsUsed() const
{
	return I_EvaluatableNode::IsUsed();
}

C_NullPlug& C_UnitConversionNode::GetInputPlug()
{
	return *m_pInputPlug;
}

const C_NullPlug& C_UnitConversionNode::GetInputPlug() const
{
	return *m_pInputPlug;
}

void C_UnitConversionNode::SetFactor(F32 factor)
{
	m_factor = factor;
}

F32 C_UnitConversionNode::GetFactor() const
{
	return m_factor;
}

bool C_UnitConversionNode::Evaluate(F32& value) const
{
	F32 x;
	if (m_pInputPlug->GetValue(x))
	{
		value = m_factor * x;
		return true;
	}

	return false;
}

bool C_UnitConversionNode::GetAdjacentKeys(C_BaseKey& key1, C_BaseKey& key2) const
{
	return GetAdjacentKeysT(key1, key2);
}

bool C_UnitConversionNode::GetAdjacentKeys(C_LinearKey& key1, C_LinearKey& key2) const
{
	return GetAdjacentKeysT(key1, key2);
}

bool C_UnitConversionNode::GetAdjacentKeys(C_HermiteKey& key1, C_HermiteKey& key2) const
{
	return GetAdjacentKeysT(key1, key2);
}

const I_Plug* C_UnitConversionNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = I_EvaluatableNode::GetNumStaticPlugs();
	if (instance < staticOffset)
		return I_EvaluatableNode::GetStaticPlugPtr(instance);
	
	instance -= staticOffset;
	if (instance == 0)
		return m_pInputPlug;

	return 0;
}

U32	C_UnitConversionNode::GetNumStaticPlugs() const
{
	return I_EvaluatableNode::GetNumStaticPlugs() + AFTK_ANIMATION_UNIT_CONVERSION_NODE_NUM_STATIC_PLUGS;
}

// Protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_UnitConversionNode::OneTimeInit()
{
	m_pInputPlug = new C_NullPlug(0);
	m_pInputPlug->SetName("input");
	m_pInputPlug->SetLogicalIndex(0);
	m_pInputPlug->SetParentPtr(this);

	ClearThis();
}

void C_UnitConversionNode::ClearThis()
{
	m_factor = 0;
}

void C_UnitConversionNode::Clear()
{
	ClearThis();
	I_EvaluatableNode::Clear();
}

#endif

