#include <stdafx.h>

using namespace Aftk;

C_DistanceDimNode::C_DistanceDimNode(C_Scene* pScene) : I_EvaluatableNode(pScene)
{
	OneTimeInit();
}

C_Node* C_DistanceDimNode::Clone(C_Scene* pScene) const
{
	C_Node* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_DistanceDimNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_DistanceDimNode(pScene);
		(C_DistanceDimNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_DistanceDimNode::~C_DistanceDimNode()
{
	ClearThis();
}

C_DistanceDimNode::C_DistanceDimNode(const C_DistanceDimNode& rhs) : I_EvaluatableNode(0)
{
	OneTimeInit();

	*this = rhs;
}

C_DistanceDimNode& C_DistanceDimNode::operator =(const C_DistanceDimNode& rhs)
{
	I_EvaluatableNode::operator=(rhs);
	
	ClearThis();

	//Copy plug proporties.
	for (U32 i = 0; i < 2; i++)
		(*m_inputs[i]) = (*rhs.m_inputs[i]);

	return *this;
}

E_ObjType C_DistanceDimNode::GetType() const
{
	return E_ObjType_DistanceDim;
}

bool C_DistanceDimNode::HasType(E_ObjType type) const
{
	if (I_EvaluatableNode::HasType(type))
		return true;
	if (type == E_ObjType_DistanceDim) 
		return true;

	return false;
}

bool C_DistanceDimNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_DistanceDimNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (!I_EvaluatableNode::IsEqualToAll(rhs, classToAbort, flags))
		return false;

	if (!IsEqualToThis(rhs, flags))
		return false;

	return true;
}

bool C_DistanceDimNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_DistanceDim))
		return false;

	const C_DistanceDimNode& rhs = (const C_DistanceDimNode&)_rhs;

	for (U32 i = 0; i < 2; i++)
	{
		if (*m_inputs[i] != *rhs.m_inputs[i])
			return false;
	}

	return true;
}

bool C_DistanceDimNode::operator==(const C_DistanceDimNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_DistanceDimNode::operator!=(const C_DistanceDimNode &rhs) 
{
	return !((*this)==rhs);
}

U32 C_DistanceDimNode::GetHashOfThis() const
{
	return 0;
}

U32 C_DistanceDimNode::GetHash(E_ObjType classToAbort) const
{
	return I_EvaluatableNode::GetHash(classToAbort);
}

bool C_DistanceDimNode::IsUsed() const
{
	return I_EvaluatableNode::IsUsed();
}

void C_DistanceDimNode::SetInput(U32 index, I_Object* pObj)
{
	m_inputs[index]->SetSourceConnectionPtr(pObj);
}

I_Object* C_DistanceDimNode::GetInput(U32 index)
{
	return (I_Object*)m_inputs[index]->GetSourceConnectionPtr();
}

const I_Object* C_DistanceDimNode::GetInput(U32 index) const
{
	return (I_Object*)m_inputs[index]->GetSourceConnectionPtr();
}

bool C_DistanceDimNode::Evaluate(F32& value) const
{
	Vector3F32 p0(0,0,0);
	Vector3F32 p1(0,0,0);
	m_inputs[0]->GetCompoundValue(p0);
	m_inputs[1]->GetCompoundValue(p1);
	Vector3F32 delta = p1 - p0;
	value = sqrtf(delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2]);

	return true;
}

bool C_DistanceDimNode::GetAdjacentKeys(C_BaseKey& key1, C_BaseKey& key2) const
{
	return GetAdjacentKeysT(key1, key2);
}

bool C_DistanceDimNode::GetAdjacentKeys(C_LinearKey& key1, C_LinearKey& key2) const
{
	return GetAdjacentKeysT(key1, key2);
}

bool C_DistanceDimNode::GetAdjacentKeys(C_HermiteKey& key1, C_HermiteKey& key2) const
{
	return GetAdjacentKeysT(key1, key2);
}

const I_Plug* C_DistanceDimNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = I_EvaluatableNode::GetNumStaticPlugs();
	if (instance < staticOffset)
		return I_EvaluatableNode::GetStaticPlugPtr(instance);
	
	instance -= staticOffset;
	if (instance < 2)
		return m_inputs[instance];

	return 0;
}

U32	C_DistanceDimNode::GetNumStaticPlugs() const
{
	return I_EvaluatableNode::GetNumStaticPlugs() + 2;
}

// Protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_DistanceDimNode::OneTimeInit()
{
	for (U32 i = 0; i < 2; i++)
	{
		m_inputs[i] = new C_NullPlug(OnInputPlugChanged);
		m_inputs[i]->SetLogicalIndex(0);
		m_inputs[i]->SetParentPtr(this);

		m_inputs[i]->m_children.Reserve(3);
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_inputs[i]->AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_inputs[i]->AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_inputs[i]->AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");

		m_inputs[i]->SetChildrenLocked(true);
	}

	m_inputs[0]->SetName("startPoint");
	m_inputs[1]->SetName("endPoint");
}

void C_DistanceDimNode::OnInputPlugChanged(void* _pPlug)
{
	I_Plug* pPlug = (I_Plug*)_pPlug;

	C_DistanceDimNode* p;
	if (pPlug->IsPlugChild())
		p = (C_DistanceDimNode*)((I_Plug*)pPlug->GetParentPtr())->GetParentPtr();
	else
		p = (C_DistanceDimNode*)pPlug->GetTopLevelParentPtr();

	AFTK_ASSERT(p && p->HasType(E_ObjType_DistanceDim));

	//Force an update of nodes evaluating us.
	for (U32 i = 0; i < (U32)p->m_connectedPlugs.Size(); i++)
	{
		p->m_connectedPlugs[i]->OnChanged();
	}
}

void C_DistanceDimNode::ClearThis()
{
	for (U32 i = 0; i < 2; i++)
	{
		SAFE_DELETE_PTR(m_inputs[i])
	}
}

void C_DistanceDimNode::Clear()
{
	ClearThis();
	I_EvaluatableNode::Clear();
}

#endif

