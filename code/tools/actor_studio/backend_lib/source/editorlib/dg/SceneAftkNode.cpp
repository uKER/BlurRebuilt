#include "stdafx.h"
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneAftkNode.h>

#include <EditorLib/Aftk/AftkRef.h>

namespace EditorLib
{

C_SceneAftkNode::C_SceneAftkNode() : C_SceneNode(), m_pRef(NULL)
{
	m_nodeFlags.readOnly = 1;
	m_nodeFlags.userDeletable = 0;
}

C_SceneAftkNode::~C_SceneAftkNode()
{
}

const C_AftkRef& C_SceneAftkNode::GetRef() const
{
	return *m_pRef;
}

void C_SceneAftkNode::SetRef(const C_AftkRef& ref)
{
	m_pRef = &ref;
}

const char* C_SceneAftkNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_SceneAftkNode::GetStaticTypeName()
{
	static char* p = "sceneAftkNode";
	return p;
}

E_ObjType C_SceneAftkNode::GetStaticType()
{
	return E_ObjType_SceneAftkNode;
}

void* C_SceneAftkNode::Create()
{
	return new C_SceneAftkNode();
}

E_ObjType C_SceneAftkNode::GetType() const
{
	return E_ObjType_SceneAftkNode;
}

bool C_SceneAftkNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == C_SceneAftkNode::GetType()) 
		return true;

	return false;
}

void C_SceneAftkNode::GetAftkPath(Btl::String& path) const
{
	if (m_pParent)
	{
		if (m_pParent->IsA(E_ObjType_SceneAftkNode))
		{
			((C_SceneAftkNode*)m_pParent)->GetAftkPath(path);
			path += "|";
		}
		else
		{
			path = m_pRef->GetOriginalFilePath() + ":";
		}
	}

	path += m_name.GetValue();
}

}
