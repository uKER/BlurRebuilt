#include <stdafx.h>

using namespace Aftk;

C_BlendWeightedNode::C_Target::C_Target(C_BlendWeightedNode* pParent) : 
	inputPlug(OnInputPlugChanged),
	weightPlug(E_GenericAttributeType_F32, OnInputPlugChanged)
{
	inputPlug.SetLogicalIndex(0);
	inputPlug.SetParentPtr(pParent);
	inputPlug.SetName("input");
	inputPlug.SetChildrenLocked(true);

	weightPlug.SetLogicalIndex(1);
	weightPlug.SetParentPtr(pParent);
	weightPlug.SetName("weight");
	weightPlug.SetChildrenLocked(true);

	weightPlug.SetValue(1.f, false);
}


C_BlendWeightedNode::C_BlendWeightedNode(C_Scene* pScene) : I_EvaluatableNode(pScene)
{
	OneTimeInit();
}

C_Node* C_BlendWeightedNode::Clone(C_Scene* pScene) const
{
	C_Node* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_BlendWeightedNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_BlendWeightedNode(pScene);
		(C_BlendWeightedNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_BlendWeightedNode::~C_BlendWeightedNode()
{
	ClearThis();
}

C_BlendWeightedNode::C_BlendWeightedNode(const C_BlendWeightedNode& rhs) : I_EvaluatableNode(0)
{
	OneTimeInit();

	*this = rhs;
}

C_BlendWeightedNode& C_BlendWeightedNode::operator =(const C_BlendWeightedNode& rhs)
{
	I_EvaluatableNode::operator=(rhs);
	
	ClearThis();

	//Allocate plugs.
	SetNumInputs(rhs.GetNumInputs());

	//Copy plug proporties.
	for (U32 i = 0; i < (U32)m_inputs.Size(); i++)
		(*m_inputs[i]) = (*rhs.m_inputs[i]);

	return *this;
}

E_ObjType C_BlendWeightedNode::GetType() const
{
	return E_ObjType_BlendWeighted;
}

bool C_BlendWeightedNode::HasType(E_ObjType type) const
{
	if (I_EvaluatableNode::HasType(type))
		return true;
	if (type == E_ObjType_BlendWeighted) 
		return true;

	return false;
}

bool C_BlendWeightedNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_BlendWeightedNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (!I_EvaluatableNode::IsEqualToAll(rhs, classToAbort, flags))
		return false;

	if (!IsEqualToThis(rhs, flags))
		return false;

	return true;
}

bool C_BlendWeightedNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_BlendWeighted))
		return false;

	const C_BlendWeightedNode& rhs = (const C_BlendWeightedNode&)_rhs;

	if (m_inputs.Size() != rhs.m_inputs.Size())
		return false;
	for (U32 i = 0; i < (U32)m_inputs.Size(); i++)
	{
		if (m_inputs[i]->inputPlug != rhs.m_inputs[i]->inputPlug ||
			m_inputs[i]->weightPlug != rhs.m_inputs[i]->weightPlug)
			return false;
	}

	return true;
}

bool C_BlendWeightedNode::operator==(const C_BlendWeightedNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_BlendWeightedNode::operator!=(const C_BlendWeightedNode &rhs) 
{
	return !((*this)==rhs);
}

U32 C_BlendWeightedNode::GetHashOfThis() const
{
	return 0;
}

U32 C_BlendWeightedNode::GetHash(E_ObjType classToAbort) const
{
	return I_EvaluatableNode::GetHash(classToAbort);
}

bool C_BlendWeightedNode::IsUsed() const
{
	return I_EvaluatableNode::IsUsed();
}

void C_BlendWeightedNode::SetNumInputs(U32 nInputs)
{
	//Clip inputs
	for (U32 i = nInputs; i < (U32)m_inputs.Size(); i++)
		delete m_inputs[i];

	U32 oldSize = (U32)m_inputs.Size();
	m_inputs.Resize(nInputs);
	for (U32 i = oldSize; i < (U32)m_inputs.Size(); i++)
		m_inputs[i] = new C_Target(this);
}

U32 C_BlendWeightedNode::GetNumInputs() const
{
	return (U32)m_inputs.Size();
}

C_StaticPlug<F32>& C_BlendWeightedNode::GetWeightPlug(U32 index)
{
	return m_inputs[index]->weightPlug;
}

const C_StaticPlug<F32>& C_BlendWeightedNode::GetWeightPlug(U32 index) const
{
	return m_inputs[index]->weightPlug;
}

C_NullPlug& C_BlendWeightedNode::GetInputPlug(U32 index)
{
	return m_inputs[index]->inputPlug;
}

const C_NullPlug& C_BlendWeightedNode::GetInputPlug(U32 index) const
{
	return m_inputs[index]->inputPlug;
}

bool C_BlendWeightedNode::Evaluate(F32& value) const
{
	value = 0;
	bool evaluated = false;
	for (U32 i = 0; i < (U32)m_inputs.Size(); i++)
	{
		F32 weight;
		m_inputs[i]->weightPlug.GetValue(weight);
		const I_Object* pObj = m_inputs[i]->inputPlug.GetSourceConnectionPtr();
		if (pObj->HasType(Aftk::E_ObjType_IEvaluatable))
		{
			F32 temp;
			if (!((const I_EvaluatableNode*)pObj)->Evaluate(temp))
				return false;

			value += temp * weight;
			evaluated = true;			
		}
		else if (pObj->HasType(E_ObjType_IPlug))
		{
			//Attempt to evaluate plug.
			F32 temp;
			if (!((const I_Plug*)pObj)->GetValue(temp))
				return false;

			value += temp * weight;
			evaluated = true;
		}
	}

	return evaluated;
}

bool C_BlendWeightedNode::GetAdjacentKeys(C_BaseKey& key1, C_BaseKey& key2) const
{
	return GetAdjacentKeysT(key1, key2);
}

bool C_BlendWeightedNode::GetAdjacentKeys(C_LinearKey& key1, C_LinearKey& key2) const
{
	return GetAdjacentKeysT(key1, key2);
}

bool C_BlendWeightedNode::GetAdjacentKeys(C_HermiteKey& key1, C_HermiteKey& key2) const
{
	return GetAdjacentKeysT(key1, key2);
}

const I_Plug* C_BlendWeightedNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = I_EvaluatableNode::GetNumStaticPlugs();
	if (instance < staticOffset)
		return I_EvaluatableNode::GetStaticPlugPtr(instance);
	
	return 0;
}

U32	C_BlendWeightedNode::GetNumStaticPlugs() const
{
	return I_EvaluatableNode::GetNumStaticPlugs() + m_inputs.Size();
}

// Protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_BlendWeightedNode::OneTimeInit()
{
}

void C_BlendWeightedNode::ClearThis()
{
	for (U32 i = 0; i < (U32)m_inputs.Size(); i++)
		delete m_inputs[i];
	m_inputs.Clear();
}

void C_BlendWeightedNode::OnInputPlugChanged(void* _pPlug)
{
	I_Plug* pPlug = (I_Plug*)_pPlug;

	C_BlendWeightedNode* p;
	if (pPlug->IsPlugChild())
		p = (C_BlendWeightedNode*)((I_Plug*)pPlug->GetParentPtr())->GetParentPtr();
	else
		p = (C_BlendWeightedNode*)pPlug->GetTopLevelParentPtr();

	AFTK_ASSERT(p && p->HasType(E_ObjType_BlendWeighted));

	//Force an update of nodes evaluating us.
	for (U32 i = 0; i < (U32)p->m_connectedPlugs.Size(); i++)
	{
		p->m_connectedPlugs[i]->OnChanged();
	}
}

void C_BlendWeightedNode::Clear()
{
	ClearThis();
	I_EvaluatableNode::Clear();
}

#endif

