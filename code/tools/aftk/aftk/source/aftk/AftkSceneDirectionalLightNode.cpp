#include <stdafx.h>

using namespace Aftk;

C_SceneDirectionalLightNode::C_SceneDirectionalLightNode(C_Scene* pScene) 
: 
C_SceneNonAmbientLightNode(pScene)
{
}

C_SceneDirectionalLightNode::~C_SceneDirectionalLightNode()
{
}

C_SceneDirectionalLightNode::C_SceneDirectionalLightNode(const C_SceneDirectionalLightNode& rhs)
: 
C_SceneNonAmbientLightNode(0)
{
	OneTimeInit();

	*this = rhs;
}

C_Node* C_SceneDirectionalLightNode::Clone(C_Scene* pScene) const
{
	C_SceneDirectionalLightNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneDirectionalLightNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneDirectionalLightNode(pScene);
		(C_SceneDirectionalLightNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

E_ObjType C_SceneDirectionalLightNode::GetType() const
{
	return E_ObjType_SceneDirectionalLight;
}

bool C_SceneDirectionalLightNode::HasType(E_ObjType type) const
{
	if (C_SceneNonAmbientLightNode::HasType(type))
		return true;
	if (type == C_SceneDirectionalLightNode::GetType()) 
		return true;

	return false;
}

bool C_SceneDirectionalLightNode::operator== (const C_SceneDirectionalLightNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneDirectionalLightNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneDirectionalLightNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneDirectionalLight)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneNonAmbientLightNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneDirectionalLightNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	return true;
}

void C_SceneDirectionalLightNode::Clear()
{
	ClearThis();
	C_SceneNonAmbientLightNode::Clear();
}

void C_SceneDirectionalLightNode::ClearThis()
{
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

/*
C_SceneDirectionalLightNode::C_SceneDirectionalLightNode()
{
}

void C_SceneDirectionalLightNode::Reset()
{
	C_SceneNonAmbientLightNode::Reset();
}

C_SceneDirectionalLightNode::C_SceneDirectionalLightNode(
	const C_SceneDirectionalLightNode& rhs)
{
	Reset();
	*this = rhs;
}
*/
const C_SceneDirectionalLightNode& C_SceneDirectionalLightNode::operator =(
	const C_SceneDirectionalLightNode& rhs)
{
	C_SceneNonAmbientLightNode::operator=(rhs);
	ClearThis();
	return *this;
}

#endif
