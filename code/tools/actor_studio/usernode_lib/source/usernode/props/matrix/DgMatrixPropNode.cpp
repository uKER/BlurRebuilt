#include "stdafx.h"

#include <EditorLib/Misc/Status.h>

#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Matrix/DgMatrixPropNode.h>

void RegisterMatrixPropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgMatrixPropNode)
}

namespace EditorLib
{

// From C_XtObject
C_Status C_DgMatrixPropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* _pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, _pData);
	Xt::DomBlock* pData = Xt::AsBlock(_pData);
	if (XtFieldHelper::IsNullMatrix(pData))
	{
		Matrix44 matrix;
		UtMaths::SetIdentity(&matrix);
		XtFieldHelper::SetMatrix(pData, matrix);

		PlugHelpers::SetMatrixComponents(
			*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Scale],
			*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Shear],
			*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Rotate],
			*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Translate],
			E_RotationOrder_XYZ,
			matrix,
			false);
	}

	return E_Status_Success;
}

// From I_DgObject
E_ObjType C_DgMatrixPropNode::GetType() const
{
	return E_ObjType_DgMatrixPropNode;
}

// From I_DgObject
const char* C_DgMatrixPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

// From I_DgObject
bool C_DgMatrixPropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgMatrixPropNode::GetType()) 
		return true;

	return false;
}

//From I_DgNode
bool C_DgMatrixPropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgMatrixPropNode::GetStaticType(), pHc);

	if (GetXtDirty().IsDirty())
	{
		Xt::DomBlock* pRoot = Xt::AsBlock(GetXtData());

		{
			if (pRoot && GetXtDirty().IsDirty(pRoot))
			{
				XtFieldHelper::GetMatrix(m_localMatrix, pRoot);

				PlugHelpers::SetMatrixComponents(
					*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Scale],
					*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Shear],
					*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Rotate],
					*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Translate],
					E_RotationOrder_XYZ,
					m_localMatrix,
					false);
			}
		}
	}

	return __super::Update(&signaller.GetCollection());
}

//From I_DgNode
void C_DgMatrixPropNode::GetPlugs(Btl::Vector<I_Plug*>& plugs)
{
	__super::GetPlugs(plugs);
	for (U32 i = 0; i < E_StaticPlug_Count; ++i)
		plugs.PushBack(m_staticPlugs[i]);
}

// From C_DgXtPropNodeBase
bool C_DgMatrixPropNode::IsVisible() const
{
	return true;
}

// From C_DgMatrixPropNode
const char* C_DgMatrixPropNode::GetStaticTypeName()
{
	//Must match name in Xt schema exactly!
	static char* p = "Matrix";
	return p;
}

const char* C_DgMatrixPropNode::GetStaticDisplayName()
{
	static char* p = "Matrix Property";
	return p;
}

// From C_DgMatrixPropNode
E_ObjType C_DgMatrixPropNode::GetStaticType()
{
	return E_ObjType_DgMatrixPropNode;
}

// From C_DgMatrixPropNode
void* C_DgMatrixPropNode::Create()
{
	return new C_DgMatrixPropNode();
}

// From C_DgMatrixPropNode
void C_DgMatrixPropNode::SetLocalMatrix(const Matrix44& matrix)
{
	C_XtObject::C_Unbinder unbinder(this);
	XtFieldHelper::SetMatrix(Xt::AsBlock(GetXtData()), matrix);
	GetXtDirty().SetDirty(GetXtData());
}

// From C_DgMatrixPropNode
const Matrix44& C_DgMatrixPropNode::GetLocalMatrix() const
{
	return m_localMatrix;
}

// From C_DgMatrixPropNode
C_NullPlug& C_DgMatrixPropNode::GetTranslatePlug()
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Translate];
}

// From C_DgMatrixPropNode
C_NullPlug& C_DgMatrixPropNode::GetRotatePlug()
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Rotate];
}

// From C_DgMatrixPropNode
C_NullPlug& C_DgMatrixPropNode::GetScalePlug()
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Scale];
}

// From C_DgMatrixPropNode
C_NullPlug& C_DgMatrixPropNode::GetShearPlug()
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Shear];
}

// From C_DgMatrixPropNode
const C_NullPlug& C_DgMatrixPropNode::GetTranslatePlug() const
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Translate];
}

// From C_DgMatrixPropNode
const C_NullPlug& C_DgMatrixPropNode::GetRotatePlug() const
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Rotate];
}

// From C_DgMatrixPropNode
const C_NullPlug& C_DgMatrixPropNode::GetScalePlug() const
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Scale];
}

// From C_DgMatrixPropNode
const C_NullPlug& C_DgMatrixPropNode::GetShearPlug() const
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Shear];
}

// From C_XtObject
void C_DgMatrixPropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);

	Xt::DomBlock* pBlockData = Xt::AsBlock(GetXtData());
	BIZ_ASSERT(pBlockData);

	GetXtDirty().SetDirty(pBlockData);
}

// From C_DgMatrixPropNode
C_DgMatrixPropNode::C_DgMatrixPropNode()
:
C_DgXtPropNodeBase()
{
	//Initialise transform plugs.
	m_staticPlugs[E_StaticPlug_Translate] = PlugHelpers::CreateCompoundVec3(this, C_DgMatrixPropNode::OnPlugChanged, E_PlugType_Input, "translate");
	m_staticPlugs[E_StaticPlug_Rotate] = PlugHelpers::CreateCompoundVec3(this, C_DgMatrixPropNode::OnPlugChanged, E_PlugType_Input, "rotate");
	m_staticPlugs[E_StaticPlug_Scale] = PlugHelpers::CreateCompoundVec3(this, C_DgMatrixPropNode::OnPlugChanged, E_PlugType_Input, "scale", 1.f, 1.f, 1.f);
	m_staticPlugs[E_StaticPlug_Shear] = PlugHelpers::CreateCompoundVec3(this, C_DgMatrixPropNode::OnPlugChanged, E_PlugType_Input, "shear", 0.f, 0.f, 0.f);
}

// From C_DgMatrixPropNode
C_DgMatrixPropNode::~C_DgMatrixPropNode()
{
	for (S32 i = 0; i < E_StaticPlug_Count; i++)
		delete m_staticPlugs[i];
}

// From C_DgMatrixPropNode
void C_DgMatrixPropNode::OnPlugChanged(I_Plug* pPlug)
{
	C_DgMatrixPropNode* pThis = (C_DgMatrixPropNode*)PlugHelpers::GetTopLevelParent(*pPlug);
	if (!pThis->GetXtData()) 
		return;

	Xt::DomBlock* pBlockData = Xt::AsBlock(pThis->GetXtData());
	BIZ_ASSERT(pBlockData);

	{
		Matrix44 matrix;
		C_XtObject::C_Unbinder unbinder(pThis);
		EditorLib::PlugHelpers::SetMatrix(
			matrix,
			pThis->GetScalePlug(),
			pThis->GetShearPlug(),
			pThis->GetRotatePlug(),
			pThis->GetTranslatePlug(),
			E_RotationOrder_XYZ,
			false);
		XtFieldHelper::SetMatrix(pBlockData, matrix);
		pThis->GetXtDirty().SetDirty(pBlockData);
	}
}

}
