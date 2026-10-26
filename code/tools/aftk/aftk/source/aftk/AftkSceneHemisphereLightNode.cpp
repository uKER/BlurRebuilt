#include <stdafx.h>

using namespace Aftk;

C_SceneHemisphereLightNode::C_SceneHemisphereLightNode(C_Scene* pScene)
:
C_SceneLightNode(pScene)
{
	OneTimeInit();
}

C_SceneHemisphereLightNode::C_SceneHemisphereLightNode(const C_SceneHemisphereLightNode& rhs)
:
C_SceneLightNode(0)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneHemisphereLightNode& C_SceneHemisphereLightNode::operator=(const C_SceneHemisphereLightNode& rhs)
{
	C_SceneLightNode::operator=(rhs);

	ClearThis();

	return *this;
}

C_SceneHemisphereLightNode::~C_SceneHemisphereLightNode()
{
	ClearThis();
}

bool C_SceneHemisphereLightNode::operator== (const C_SceneHemisphereLightNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneHemisphereLightNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneHemisphereLightNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneHemisphereLight)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneLightNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneHemisphereLightNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	return true;
}

C_Node* C_SceneHemisphereLightNode::Clone(C_Scene* pScene) const
{
	C_SceneHemisphereLightNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneHemisphereLightNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneHemisphereLightNode(pScene);
		(C_SceneHemisphereLightNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

void C_SceneHemisphereLightNode::Clear()
{
	ClearThis();
	C_SceneLightNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneHemisphereLightNode::OneTimeInit()
{
}

void C_SceneHemisphereLightNode::ClearThis()
{
}

#endif
