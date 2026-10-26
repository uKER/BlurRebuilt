#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtHelpers.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/SceneXtNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorLib/Misc/GuidManager.h>

namespace EditorLib
{

//From I_DgNode
bool C_SceneXtNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_SceneXtNode::GetStaticType(), pHc);
	if (!__super::Update(&signaller.GetCollection())) return false;

	return true;
}

//From C_SceneXtNode
C_SceneXtNode::C_SceneXtNode() : 
C_SceneXtNodeBase()
{
}

C_SceneXtNode::~C_SceneXtNode()
{
}

const char* C_SceneXtNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_SceneXtNode::GetStaticTypeName()
{
	static char* p = "sceneXtNode";
	return p;
}

E_ObjType C_SceneXtNode::GetStaticType()
{
	return E_ObjType_SceneXtNode;
}

void* C_SceneXtNode::Create()
{
	return new C_SceneXtNode();
}

E_ObjType C_SceneXtNode::GetType() const
{
	return E_ObjType_SceneXtNode;
}

bool C_SceneXtNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_SceneXtNode::GetType()) 
		return true;

	return false;
}

C_Status C_SceneXtNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status;
	status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;

	return E_Status_Success;
}

bool C_SceneXtNode::VetoParentChange(const C_SceneNode* pCandidateParent) const
{
	if (__super::VetoParentChange(pCandidateParent))
		return true;

	if (!pCandidateParent)
		//Don't veto
		return false;

	I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(m_id);
	BIZ_ASSERT(pLevel);

	//May be parented to root or another xt node.
	if (pCandidateParent->IsA(EditorLib::E_ObjType_SceneXtNode) || 
		pCandidateParent == pLevel->GetRootSceneNode())
		//Don't veto
		return false;

	//Veto
	return true;
}

}
