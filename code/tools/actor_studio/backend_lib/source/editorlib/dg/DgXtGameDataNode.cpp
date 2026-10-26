#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Dg/DgXtGameDataNode.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Core.h>


namespace EditorLib
{

//From I_DgNode
bool C_DgXtGameDataNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, E_ObjType_DgXtGameDataNode, pHc);

	//Update superclass.
	if (!__super::Update(&signaller.GetCollection())) return false;

	return true;
}

bool C_DgXtGameDataNode::DirtyDependents(bool setThisDirty)
{
	if (!__super::DirtyDependents(setThisDirty))
		return false;

	m_references.InvokeDependentOnChanged(false);
	return true;
}

//From C_DgXtGameDataNode
const char* C_DgXtGameDataNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_DgXtGameDataNode::GetStaticTypeName()
{
	static char* p = "GameData";
	return p;
}

E_ObjType C_DgXtGameDataNode::GetStaticType()
{
	return E_ObjType_DgXtGameDataNode;
}

void* C_DgXtGameDataNode::Create()
{
	return new C_DgXtGameDataNode();
}

C_NullPlug&	C_DgXtGameDataNode::GetReferencesPlug()
{
	return m_references;
}

const C_NullPlug& C_DgXtGameDataNode::GetReferencesPlug() const
{
	return m_references;
}

C_DgXtGameDataNode::C_DgXtGameDataNode() : C_DgXtNodeBase(),
	m_references(this, E_PlugType_Output, "references")
{
	m_dgXtGameDataNodeDirtyFlags.data = 0xffffffff;
}

C_DgXtGameDataNode::~C_DgXtGameDataNode()
{
}

E_ObjType C_DgXtGameDataNode::GetType() const
{
	return E_ObjType_DgXtGameDataNode;
}

bool C_DgXtGameDataNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgXtGameDataNode::GetType()) 
		return true;

	return false;
}


C_Status C_DgXtGameDataNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;

	return E_Status_Success;
}

void C_DgXtGameDataNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
}

}
