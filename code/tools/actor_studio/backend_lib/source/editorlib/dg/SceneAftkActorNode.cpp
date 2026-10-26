#include "stdafx.h"
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneAftkNode.h>
#include <EditorLib/Dg/SceneAftkActorNode.h>


namespace EditorLib
{

C_SceneAftkActorNode::C_SceneAftkActorNode() : C_SceneAftkNode()
{
	m_aftkUniqueId = 0xffffffff;
}

C_SceneAftkActorNode::~C_SceneAftkActorNode()
{
}

const char* C_SceneAftkActorNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_SceneAftkActorNode::GetStaticTypeName()
{
	static char* p = "sceneAftkActorNode";
	return p;
}

E_ObjType C_SceneAftkActorNode::GetStaticType()
{
	return E_ObjType_SceneAftkActorNode;
}

void* C_SceneAftkActorNode::Create()
{
	return new C_SceneAftkActorNode();
}

E_ObjType C_SceneAftkActorNode::GetType() const
{
	return E_ObjType_SceneAftkActorNode;
}

bool C_SceneAftkActorNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == C_SceneAftkActorNode::GetType()) 
		return true;

	return false;
}

Util::ConstKeyValueArray& C_SceneAftkActorNode::GetAftkProperties() const
{
	return m_attributes;
}

Util::KeyValueArray& C_SceneAftkActorNode::GetAftkProperties()
{
	return m_attributes;
}

U32 C_SceneAftkActorNode::GetAftkUniqueId() const
{
	return m_aftkUniqueId;
}

void C_SceneAftkActorNode::SetAftkUniqueId(U32 uid)
{
	m_aftkUniqueId = uid;
}

}
