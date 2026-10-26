#include "stdafx.h"

#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtTypeDatabase.h>

#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>

#include <EditorLib/Collision/Collision.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorLib/Misc/GuidManager.h>

namespace EditorLib
{

//From I_DgNode
bool C_SceneXtActorNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_SceneXtActorNode::GetStaticType(), pHc);
	if (!__super::Update(&signaller.GetCollection())) return false;

	return true;
}

bool C_SceneXtActorNode::DirtyDependents(bool setThisDirty)
{
	if (!__super::DirtyDependents(setThisDirty))
		return false;

	return true;
}

bool C_SceneXtActorNode::IsNonXtDirty() const
{
	return __super::IsNonXtDirty();
}


//From C_SceneXtNodeBase
void C_SceneXtActorNode::ImmediatePropagate(const C_ImmediateHintCollection& hc, bool dirtyDependents)
{
	__super::ImmediatePropagate(hc, dirtyDependents);
}


//From C_SceneXtActorNode
C_SceneXtActorNode::C_SceneXtActorNode() : C_SceneXtNodeBase()
{
}

C_SceneXtActorNode::~C_SceneXtActorNode()
{
}

const char* C_SceneXtActorNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_SceneXtActorNode::GetStaticTypeName()
{
	static char* p = "sceneActorNode";
	return p;
}

E_ObjType C_SceneXtActorNode::GetStaticType()
{
	return E_ObjType_SceneXtActorNode;
}

void* C_SceneXtActorNode::Create()
{
	return new C_SceneXtActorNode();
}

E_ObjType C_SceneXtActorNode::GetType() const
{
	return E_ObjType_SceneXtActorNode;
}

bool C_SceneXtActorNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_SceneXtActorNode::GetType()) 
		return true;

	return false;
}

C_Status C_SceneXtActorNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status;
	Xt::DomComponent* pComponent = Xt::AsComponent(pData);
	Xt::DomBlock* pBlockData = &pComponent->data;

	status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;

	// If we're creating a null actor, skip initialisation
	if (pLarvaType)
	{
		InitialiseExtActorProperties();
	}

	return E_Status_Success;
}

void C_SceneXtActorNode::InitialiseExtActorProperties()
{
	if (XtNodeHelpers::IsExternalActor(Xt::AsBlock(GetXtData())))
	{
		m_nodeFlags.readOnly = true;
		m_sceneNodeFlags.canReparent = false;
		m_nodeFlags.userDeletable = false;
		m_nodeFlags.external = true;

		for (DgActorComponentArray::ConstIterator it = 
			m_components.Begin();
			it != m_components.End(); ++it)
			(*it)->GetNodeFlags() = m_nodeFlags;
	}
}

void C_SceneXtActorNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
}

}
