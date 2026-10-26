#include "stdafx.h"

#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Sphere/DgSpherePropNode.h>

void RegisterSpherePropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgSpherePropNode)
}

namespace EditorLib
{

// From C_XtObject
C_Status C_DgSpherePropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* _pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, _pData);
	if (!status) return status;

	Xt::DomBlock* pData = Xt::AsBlock(_pData);

	Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pData, "Position"));
	if (XtFieldHelper::IsNullVector(pBlock))
	{
		pBlock = Xt::AcquireBlock(pData, "Position");
		Vector3 position(0,0,0);
		XtFieldHelper::SetVector(pBlock, position);
		PlugHelpers::SetCompoundValue(GetPositionPlug(), position, false);
	}

	Xt::DomField* pField = Xt::AsField(Xt::At(pData, "Radius"));
	if (XtFieldHelper::IsNullScalar(pField))
	{
		pField = Xt::AcquireField(pData, "Radius");
		F32 radius(1.f);
		XtFieldHelper::SetScalar(pField, radius);
		GetRadiusPlug().SetValue(radius);
	}

	return E_Status_Success;
}

// From I_DgObject
E_ObjType C_DgSpherePropNode::GetType() const
{
	return E_ObjType_DgSpherePropNode;
}

// From I_DgObject
const char* C_DgSpherePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

// From I_DgObject
bool C_DgSpherePropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgSpherePropNode::GetType()) 
		return true;

	return false;
}

// From I_DgNode
bool C_DgSpherePropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgSpherePropNode::GetStaticType(), pHc);

	Xt::DomBlock* pRoot = Xt::AsBlock(GetXtData());
	if (!pRoot) return false;

	{
		Xt::DomBlock* pBlock;
		XT_HELPERS_AT_BLOCK_CHECKED(pBlock, pRoot, "Position");
		if (pBlock && GetXtDirty().IsDirty(pBlock))
		{
			Vector3 position;
			XtFieldHelper::GetVector(position, pBlock);
			PlugHelpers::SetCompoundValue(GetPositionPlug(), position, false);
		}
	}

	{
		Xt::DomField* pField;
		XT_HELPERS_AT_FIELD_CHECKED(pField, pRoot, "Radius");
		if (pField && GetXtDirty().IsDirty(pField))
		{
			F32 radius;
			XtFieldHelper::GetScalar(radius, pField);
			GetRadiusPlug().SetValue(radius, false);
		}
	}

	return __super::Update(&signaller.GetCollection());
}

// From I_DgNode
void C_DgSpherePropNode::GetPlugs(Btl::Vector<I_Plug*>& plugs)
{
	__super::GetPlugs(plugs);
	for (U32 i = 0; i < E_StaticPlug_Count; ++i)
		plugs.PushBack(m_staticPlugs[i]);
}

// From C_DgXtPropNodeBase
bool C_DgSpherePropNode::IsVisible() const
{
	return true;
}

// From C_DgSpherePropNode
const char* C_DgSpherePropNode::GetStaticTypeName()
{
	//Must match name in Xt schema exactly!
	static char* p = "Sphere";
	return p;
}

const char* C_DgSpherePropNode::GetStaticDisplayName()
{
	static char* p = "Sphere Property";
	return p;
}

// From C_DgSpherePropNode
E_ObjType C_DgSpherePropNode::GetStaticType()
{
	return E_ObjType_DgSpherePropNode;
}

// From C_DgSpherePropNode
void* C_DgSpherePropNode::Create()
{
	return new C_DgSpherePropNode();
}

// From C_DgSpherePropNode
C_NullPlug& C_DgSpherePropNode::GetPositionPlug()
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Position];
}

// From C_DgSpherePropNode
const C_NullPlug& C_DgSpherePropNode::GetPositionPlug() const
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Position];
}

// From C_DgSpherePropNode
C_StaticPlug<F32>& C_DgSpherePropNode::GetRadiusPlug()
{
	return (C_StaticPlug<F32>&)*m_staticPlugs[E_StaticPlug_Radius];
}

// From C_DgSpherePropNode
const C_StaticPlug<F32>& C_DgSpherePropNode::GetRadiusPlug() const
{
	return (const C_StaticPlug<F32>&)*m_staticPlugs[E_StaticPlug_Radius];
}

// From C_XtObject
void C_DgSpherePropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
	Xt::DomBlock* pBlockData = Xt::AsBlock(GetXtData());
	BIZ_ASSERT(pBlockData);

	{
		Xt::DomBlock* pBlock;
		XT_HELPERS_AT_BLOCK_CHECKED(pBlock, pBlockData, "Position");
		if (XtHelpers::AreRelated(pBlock, pEvent->from))
		{
			GetXtDirty().SetDirty(pBlock);
		}
	}

	{
		Xt::DomField* pField;
		XT_HELPERS_AT_FIELD_CHECKED(pField, pBlockData, "Radius");
		if (XtHelpers::AreRelated(pField, pEvent->from))
		{
			GetXtDirty().SetDirty(pField);
		}
	}
}

// From C_DgSpherePropNode
C_DgSpherePropNode::C_DgSpherePropNode()
:
C_DgXtPropNodeBase()
{
	m_staticPlugs[E_StaticPlug_Position] = PlugHelpers::CreateCompoundVec3(this, &C_DgSpherePropNode::OnPlugChanged, E_PlugType_Input, "translate");
	m_staticPlugs[E_StaticPlug_Radius]	= PlugHelpers::CreateScalar(this, &C_DgSpherePropNode::OnPlugChanged, E_PlugType_Input, "radius", 1.f);
}

// From C_DgSpherePropNode
C_DgSpherePropNode::~C_DgSpherePropNode()
{
	for (S32 i = 0; i < E_StaticPlug_Count; i++)
		delete m_staticPlugs[i];
}

//From C_DgSpherePropNode
void C_DgSpherePropNode::OnPlugChanged(I_Plug* pPlug)
{
	C_DgSpherePropNode* pThis = (C_DgSpherePropNode*)PlugHelpers::GetTopLevelParent(*pPlug);
	if (!pThis->GetXtData()) 
		return;

	Xt::DomBlock* pBlockData = Xt::AsBlock(pThis->GetXtData());
	BIZ_ASSERT(pBlockData);
	
	if (pPlug->GetParentPlug() == pThis->m_staticPlugs[E_StaticPlug_Position])
	{
		Xt::DomBlock* pBlock = Xt::AcquireBlock(pBlockData, "Position");
		Vector3 position;
		EditorLib::PlugHelpers::GetCompoundValue(*(C_NullPlug*)pPlug->GetParentPlug(), position);
		XtFieldHelper::SetVector(pBlock, position);
		pThis->DirtyDependents();
	}
	else if (pPlug == pThis->m_staticPlugs[E_StaticPlug_Radius])
	{
		Xt::DomField* pField = Xt::AcquireField(pBlockData, "Radius");
		F32 radius;
		radius = ((C_StaticPlug<F32>*)pPlug)->GetValue();
		XtFieldHelper::SetScalar(pField, radius);
		pThis->DirtyDependents();
	}
}

}
