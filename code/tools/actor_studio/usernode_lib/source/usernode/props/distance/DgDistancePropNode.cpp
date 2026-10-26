#include "stdafx.h"

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Distance/DgDistancePropNode.h>

void RegisterDistancePropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgDistancePropNode)
}

namespace EditorLib
{

//From I_DgNode
bool C_DgDistancePropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgDistancePropNode::GetStaticType(), pHc);

	if (GetXtDirty().IsDirty())
	{
		Xt::DomBlock* pRoot = Xt::AsBlock(GetXtData());
		{
			Xt::DomRef* pField;
			XT_HELPERS_AT_REF_CHECKED(pField, pRoot, "NodeA");
			if (pField && GetXtDirty().IsDirty(pField))
			{
				XtId id = Xt::AsId(pField);
				I_DgNode* pDgNode = id ? XtNodeHelpers::FindNodeByXtId(id) : 0;
				if (pDgNode && pDgNode->IsA(E_ObjType_SceneXtNodeBase))
				{
					C_SceneXtNodeBase* pNode = (C_SceneXtNodeBase*)pDgNode;
					m_plugs[E_StaticPlug_WorldMatrixA]->
						SetSourceConnection(&pNode->GetWorldMatrixPlug(), false);
				}
				else 
					m_plugs[E_StaticPlug_WorldMatrixA]->SetSourceConnection(0, false);
			}
		}
		{
			Xt::DomRef* pField;
			XT_HELPERS_AT_REF_CHECKED(pField, pRoot, "NodeB");
			if (pField && GetXtDirty().IsDirty(pField))
			{
				XtId id = Xt::AsId(pField);
				I_DgNode* pDgNode = id ? XtNodeHelpers::FindNodeByXtId(id) : 0;
				if (pDgNode && pDgNode->IsA(E_ObjType_SceneXtNodeBase))
				{
					C_SceneXtNodeBase* pNode = (C_SceneXtNodeBase*)pDgNode;
					m_plugs[E_StaticPlug_WorldMatrixB]->
						SetSourceConnection(&pNode->GetWorldMatrixPlug(), false);
				}
				else 
					m_plugs[E_StaticPlug_WorldMatrixB]->SetSourceConnection(0, false);
			}
		}
	}

	//We don't actually use this flag for the moment.
	m_distanceNodeDirtyFlags.targetChanged = false;
	return __super::Update(&signaller.GetCollection());
}

bool C_DgDistancePropNode::DirtyDependents(bool setThisDirty)
{
	if (!__super::DirtyDependents(setThisDirty))
		return false;

	if (setThisDirty)
		m_distanceNodeDirtyFlags.data = 0xffffffff;
	return true;
}

bool C_DgDistancePropNode::IsNonXtDirty() const
{
	return __super::IsNonXtDirty();
}

const char* C_DgDistancePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_DgDistancePropNode::GetStaticTypeName()
{
	static char* p = "AsDistance";
	return p;
}

E_ObjType C_DgDistancePropNode::GetStaticType()
{
	return E_ObjType_DgDistancePropNode;
}

void* C_DgDistancePropNode::Create()
{
	return new C_DgDistancePropNode();
}

C_DgDistancePropNode::C_DgDistancePropNode()
: C_DgXtPropNodeBase()
{
	m_plugs[E_StaticPlug_WorldMatrixA] = new C_StaticPlug<MathsStorage::Matrix44>(
		this, &C_DgDistancePropNode::OnTargetPlugChanged, E_PlugType_Input, "targetAWorldMatrix");
	m_plugs[E_StaticPlug_WorldMatrixB] = new C_StaticPlug<MathsStorage::Matrix44>(
		this, &C_DgDistancePropNode::OnTargetPlugChanged, E_PlugType_Input, "targetBWorldMatrix");
	m_distanceNodeDirtyFlags.data = 0;
}

C_DgDistancePropNode::~C_DgDistancePropNode()
{
	for (S32 i = 0; i < E_StaticPlug_Count; ++i)
		SAFE_DELETE_PTR(m_plugs[i]);
}

E_ObjType C_DgDistancePropNode::GetType() const
{
	return E_ObjType_DgDistancePropNode;
}

bool C_DgDistancePropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgDistancePropNode::GetType()) 
		return true;

	return false;
}

bool C_DgDistancePropNode::IsVisible() const
{
	return true;
}

bool C_DgDistancePropNode::IsDirty() const
{
	return __super::IsDirty() || m_distanceNodeDirtyFlags.data != 0;
}

void C_DgDistancePropNode::ResetDirty()
{
	__super::ResetDirty();
	m_distanceNodeDirtyFlags.data = 0;
}

C_Status C_DgDistancePropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;

	return status;
}

const I_DgNode* C_DgDistancePropNode::GetLinkedNodeA() const
{
	I_Plug* pTargetPlug = m_plugs[E_StaticPlug_WorldMatrixA]->GetSourceConnection();
	return pTargetPlug ? (I_DgNode*)PlugHelpers::GetTopLevelParent(*pTargetPlug) : 0;
}

const I_DgNode* C_DgDistancePropNode::GetLinkedNodeB() const
{
	I_Plug* pTargetPlug = m_plugs[E_StaticPlug_WorldMatrixB]->GetSourceConnection();
	return pTargetPlug ? (I_DgNode*)PlugHelpers::GetTopLevelParent(*pTargetPlug) : 0;
}

C_StaticPlug<MathsStorage::Matrix44>& C_DgDistancePropNode::GetTargetAWorldMatrixPlug()
{
	return *(C_StaticPlug<MathsStorage::Matrix44>*)m_plugs[E_StaticPlug_WorldMatrixA];
}

const C_StaticPlug<MathsStorage::Matrix44>& C_DgDistancePropNode::GetTargetAWorldMatrixPlug() const
{
	return *(const C_StaticPlug<MathsStorage::Matrix44>*)m_plugs[E_StaticPlug_WorldMatrixA];
}

C_StaticPlug<MathsStorage::Matrix44>& C_DgDistancePropNode::GetTargetBWorldMatrixPlug()
{
	return *(C_StaticPlug<MathsStorage::Matrix44>*)m_plugs[E_StaticPlug_WorldMatrixB];
}

const C_StaticPlug<MathsStorage::Matrix44>& C_DgDistancePropNode::GetTargetBWorldMatrixPlug() const
{
	return *(const C_StaticPlug<MathsStorage::Matrix44>*)m_plugs[E_StaticPlug_WorldMatrixB];
}

void C_DgDistancePropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);

	if (pEvent->code = Xt::kDomRefChanged)
		GetXtDirty().SetDirty(GetXtData());
}

void C_DgDistancePropNode::OnTargetPlugChanged(I_Plug* pPlug)
{
	BIZ_ASSERT(pPlug);
	//Set as dirty due to something interesting happening to the node we're looking at.
	I_DgNode* pParent = PlugHelpers::GetTopLevelParent(*pPlug);
	BIZ_ASSERT(pParent->IsA(E_ObjType_DgDistancePropNode));
	((C_DgDistancePropNode*)pParent)->m_distanceNodeDirtyFlags.targetChanged = true;
	pParent->DirtyDependents(false);
}

}
