#include <stdafx.h>

using namespace Aftk;

C_SceneInstanceNode::C_SceneInstanceNode(C_Scene* pScene) : C_SceneNode(pScene)
{
	OneTimeInit();
}

C_SceneInstanceNode::C_SceneInstanceNode(const C_SceneInstanceNode& rhs) : C_SceneNode(0)
{
	OneTimeInit();

	*this = rhs;
}

C_Node* C_SceneInstanceNode::Clone(C_Scene* pScene) const
{
	C_SceneInstanceNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneInstanceNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneInstanceNode(pScene);
		(C_SceneInstanceNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

const C_SceneInstanceNode& C_SceneInstanceNode::operator=(const C_SceneInstanceNode& rhs)
{
	C_SceneNode::operator=(rhs);

	ClearThis();

	return *this;
}

C_SceneInstanceNode::~C_SceneInstanceNode()
{
	//Remove association with instance source here.
	if (m_pSourceNode)
	{
		SetSourceNodePtr(0);
	}

	ClearThis();
}

E_ObjType C_SceneInstanceNode::GetType() const
{
	return E_ObjType_SceneInstance;
}

bool C_SceneInstanceNode::HasType(E_ObjType type) const
{
	if (C_SceneNode::HasType(type))
		return true;
	if (type == C_SceneInstanceNode::GetType()) 
		return true;

	return false;
}

void C_SceneInstanceNode::SetSourceNodePtr(C_SceneNode* pSceneNode)
{
	AFTK_ASSERT(pSceneNode != this);
	if (m_pSourceNode)
	{
		//We're no longer referencing the same node, decrease reference count
		m_pSourceNode->DeleteInstanceOfThisNode(this);
	}

	m_pSourceNode = pSceneNode;

	if (m_pSourceNode)
	{
		AFTK_ASSERT(pSceneNode->m_pScene == m_pScene);
		//Increase the reference count of the new node that we're instancing.
		m_pSourceNode->AddInstanceOfThisNode(this);
	}
}

C_SceneNode* C_SceneInstanceNode::GetSourceNodePtr()
{
	return m_pSourceNode;
}

const C_SceneNode* C_SceneInstanceNode::GetSourceNodePtr() const
{
	return m_pSourceNode;
}

bool C_SceneInstanceNode::operator== (const C_SceneInstanceNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneInstanceNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneInstanceNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneInstance)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneInstanceNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(GetType()))
	{
		return false;
	}

	C_SceneInstanceNode& rhs = (C_SceneInstanceNode&) _rhs;


	if (m_pSourceNode != rhs.m_pSourceNode)
		return false;

	return true;
}

U32 C_SceneInstanceNode::Repair()
{
	return ProcessProblems(true);
}

U32 C_SceneInstanceNode::Validate() const
{
	C_SceneInstanceNode* pThis = const_cast<C_SceneInstanceNode*>(this);
	return pThis->ProcessProblems(false);
}

U32 C_SceneInstanceNode::ProcessProblems(bool fix) 
{
	U32 error = 0;
	error += C_SceneNode::ProcessProblems(fix);

	if (!m_pSourceNode)
		AFTK_DO_BEGIN << "Instance " << m_name << " has no source node.\n" AFTK_DO_WARNING_END;

	return error;
}

void C_SceneInstanceNode::Clear()
{
	ClearThis();
	C_SceneNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneInstanceNode::OneTimeInit()
{
	m_pSourceNode = 0;
}

void C_SceneInstanceNode::ClearThis()
{
	m_pSourceNode = 0;
}

void C_SceneInstanceNode::CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const
{
	bounds.Clear();
	if (m_pSourceNode)
		m_pSourceNode->CalculateBounds(bounds, pMatrix);
	else
		C_SceneNode::CalculateBounds(bounds, pMatrix);
}

C_SceneNode* C_SceneInstanceNode::GetTopLevelSourceNodePtr()
{
	if (m_pSourceNode && m_pSourceNode->GetType() == E_ObjType_SceneInstance)
	{
		return ((C_SceneInstanceNode*)m_pSourceNode)->GetTopLevelSourceNodePtr();
	}
	return m_pSourceNode;
}

const C_SceneNode* C_SceneInstanceNode::GetTopLevelSourceNodePtr() const
{
	return const_cast<C_SceneInstanceNode*>(this)->GetTopLevelSourceNodePtr();
}

void C_SceneInstanceNode::DeleteInstanceSource(C_SceneNode* pSource)
{
	if (GetSourceNodePtr() == pSource)
		SetSourceNodePtr(0);
}

#endif

