#include <stdafx.h>

using namespace Aftk;

C_SceneAreaLightNode::C_SceneAreaLightNode(C_Scene* pScene) 
: 
C_SceneNonAmbientLightNode(pScene)
{
	OneTimeInit();
}

C_Node* C_SceneAreaLightNode::Clone(C_Scene* pScene) const
{
	C_SceneAreaLightNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneAreaLightNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneAreaLightNode(pScene);
		(C_SceneAreaLightNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_SceneAreaLightNode::C_SceneAreaLightNode(const C_SceneAreaLightNode& rhs)
: 
C_SceneNonAmbientLightNode(0)
{
	OneTimeInit();

	*this = rhs;
}

const C_SceneAreaLightNode& C_SceneAreaLightNode::operator=(const C_SceneAreaLightNode& rhs)
{
	C_SceneNonAmbientLightNode::operator=(rhs);

	ClearThis();

	return *this;
}

C_SceneAreaLightNode::~C_SceneAreaLightNode()
{
	ClearThis();
}

E_ObjType C_SceneAreaLightNode::GetType() const
{
	return E_ObjType_SceneAreaLight;
}

bool C_SceneAreaLightNode::HasType(E_ObjType type) const
{
	if (C_SceneNonAmbientLightNode::HasType(type))
		return true;
	if (type == C_SceneAreaLightNode::GetType()) 
		return true;

	return false;
}

bool C_SceneAreaLightNode::operator== (const C_SceneAreaLightNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneAreaLightNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneAreaLightNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	//Nothing to test!
	return true;
}

bool C_SceneAreaLightNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneAreaLight)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneNonAmbientLightNode::IsEqualToAll(rhs, classToAbort, flags));
}

void C_SceneAreaLightNode::Clear()
{
	ClearThis();
	C_SceneNonAmbientLightNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneAreaLightNode::OneTimeInit()
{
}

void C_SceneAreaLightNode::ClearThis()
{
}

#endif
