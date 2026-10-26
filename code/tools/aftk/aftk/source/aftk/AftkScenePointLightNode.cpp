#include <stdafx.h>

using namespace Aftk;


C_ScenePointLightNode::C_ScenePointLightNode(C_Scene* pScene) 
: 
C_SceneNonAmbientLightNode(pScene)
{
	OneTimeInit();
}

C_Node* C_ScenePointLightNode::Clone(C_Scene* pScene) const
{
	C_ScenePointLightNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_ScenePointLightNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_ScenePointLightNode(pScene);
		(C_ScenePointLightNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_ScenePointLightNode::C_ScenePointLightNode(const C_ScenePointLightNode& rhs)
: 
C_SceneNonAmbientLightNode(0)
{
	OneTimeInit();

	*this = rhs;
}

const C_ScenePointLightNode& C_ScenePointLightNode::operator=(const C_ScenePointLightNode& rhs)
{
	C_SceneNonAmbientLightNode::operator=(rhs);

	ClearThis();

	m_cutOffRadius = rhs.m_cutOffRadius;

	return *this;
}

C_ScenePointLightNode::~C_ScenePointLightNode()
{
	ClearThis();
}

void C_ScenePointLightNode::SetCutOffRadius(F32 cutOffRadius)
{
	m_cutOffRadius = cutOffRadius;
}

E_ObjType C_ScenePointLightNode::GetType() const
{
	return E_ObjType_ScenePointLight;
}

bool C_ScenePointLightNode::HasType(E_ObjType type) const
{
	if (C_SceneNonAmbientLightNode::HasType(type))
		return true;
	if (type == C_ScenePointLightNode::GetType()) 
		return true;

	return false;
}

F32 C_ScenePointLightNode::GetCutOffRadius() const
{
	return m_cutOffRadius;
}

bool C_ScenePointLightNode::operator== (const C_ScenePointLightNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_ScenePointLightNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_ScenePointLightNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_ScenePointLight)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneNonAmbientLightNode::IsEqualToAll(rhs, classToAbort, flags));
}


bool C_ScenePointLightNode::IsEqualToThis(const I_Object& _rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_ScenePointLight))
	{
		return false;
	}

	C_ScenePointLightNode& rhs = (C_ScenePointLightNode&) _rhs;

	if (!EqualityTestFScalar(m_cutOffRadius, rhs.m_cutOffRadius))
		return false;

	return true;
}

void C_ScenePointLightNode::Clear()
{
	ClearThis();
	C_SceneNonAmbientLightNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_ScenePointLightNode::OneTimeInit()
{
	m_cutOffRadius = 0;
}

void C_ScenePointLightNode::ClearThis()
{
	m_cutOffRadius = 0;
}

/*
C_ScenePointLightNode::C_ScenePointLightNode()
{
	m_cutOffRadius = 0;
}


C_ScenePointLightNode::C_ScenePointLightNode(
	const C_ScenePointLightNode& rhs)
{
	Reset();
	*this = rhs;
}

const C_ScenePointLightNode& C_ScenePointLightNode::operator =(
	const C_ScenePointLightNode& rhs)
{
	Clear();
	C_SceneNonAmbientLightNode::operator=(rhs);
	return *this;
}
*/


#endif
