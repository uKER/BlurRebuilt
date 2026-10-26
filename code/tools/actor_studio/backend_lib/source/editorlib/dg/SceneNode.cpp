#include "stdafx.h"

#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkEnum.h>
#include <Aftk/AftkBounds.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/Hints/UpdateTransformHint.h>
#include <EditorLib/Dg/Hints/ImmediateReparentHint.h>
#include <EditorLib/Dg/Hints/ImmediateTransformHint.h>
#include <EditorLib/Dg/Hints/ImmediatePostInitialiseHint.h>
#include <EditorLib/Dg/DgNodeBase.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/UpdateSignaller.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

//From I_DgNode
void C_SceneNode::GetPlugs(Btl::Vector<I_Plug*>& plugs)
{
	__super::GetPlugs(plugs);

	for (U32 i = 0; i < E_StaticPlug_Count; ++i)
		plugs.PushBack(m_staticPlugs[i]);
}

bool C_SceneNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_SceneNode::GetStaticType(), pHc);
	if (!C_DgNodeBase::Update(&signaller.GetCollection())) return false;

	//Transform changed...
	if (m_sceneNodeDirtyFlags.transformViaImmediateDirty)
	{
		//Add hint
		signaller.GetCollection().hints.PushBack(new C_UpdateTransformHint());
		m_sceneNodeDirtyFlags.transformViaImmediateDirty = false;
	}

	return true;
}

bool C_SceneNode::DirtyDependents(bool setThisDirty)
{
	if (!__super::DirtyDependents(setThisDirty))
		return false;

	if (setThisDirty)
	{
		//Our only two dirty flags are immediate update ones which aren't to be set here.
	}

	C_SceneNode *pCurrentChild = m_pChild;
	while (pCurrentChild)
	{
		//False arg ensures we call dirtydependents only at the top level.
		//would like to find a nicer way of doing this.
		pCurrentChild->DirtyDependents();
		pCurrentChild = pCurrentChild->m_pNext;
	}

	return true;
}

bool C_SceneNode::IsNonXtDirty() const
{
	return __super::IsNonXtDirty() || m_sceneNodeDirtyFlags.data != 0;
}

void C_SceneNode::ResetNonXtDirty()
{
	C_DgNodeBase::ResetNonXtDirty();
	m_sceneNodeDirtyFlags.data = 0;
}

//From C_SceneNode
C_SceneNode::C_SceneNode() : C_DgNodeBase()
{
	memset(&m_staticPlugs, 0, sizeof(m_staticPlugs));
	//Initialise transform plugs.
	m_staticPlugs[E_StaticPlug_Translate]	= PlugHelpers::CreateCompoundVec3(
		this, &C_SceneNode::OnSceneNodePlugChanged, E_PlugType_Input, "translate");
	m_staticPlugs[E_StaticPlug_Rotate]		= PlugHelpers::CreateCompoundVec3(
		this, &C_SceneNode::OnSceneNodePlugChanged, E_PlugType_Input, "rotate");
	m_staticPlugs[E_StaticPlug_Scale]		= PlugHelpers::CreateCompoundVec3(
		this, &C_SceneNode::OnSceneNodePlugChanged, E_PlugType_Input, "scale", 1.f, 1.f, 1.f);
	m_staticPlugs[E_StaticPlug_Shear]		= PlugHelpers::CreateCompoundVec3(
		this, &C_SceneNode::OnSceneNodePlugChanged, E_PlugType_Input, "shear");
	
	m_staticPlugs[E_StaticPlug_LocalMatrix] = new C_StaticPlug<MathsStorage::Matrix44>(
		this, NULL, E_PlugType_Output, "localMatrix");
	m_staticPlugs[E_StaticPlug_WorldMatrix] = new C_StaticPlug<MathsStorage::Matrix44>(
		this, NULL, E_PlugType_Output, "worldMatrix");

	Matrix44 identity;
	UtMaths::SetIdentity(&identity);
	GetLocalMatrixPlug().SetValue(
		(MathsStorage::Matrix44&)identity);
	GetWorldMatrixPlug().SetValue(
		(MathsStorage::Matrix44&)identity);

	m_sceneNodeFlags.canAddChildren = 1;
	m_sceneNodeFlags.canReparent = 1;
	//Flag everything as dirty.
	m_sceneNodeDirtyFlags.data = 0xffffffff;
	//Apart from out transform, which is nicely initialised.
	m_sceneNodeDirtyFlags.transformViaImmediateDirty = false;

	m_pParent = 0;
	m_pChild = 0;
	m_pPrevious = 0;
	m_pNext = 0;
}

C_SceneNode::~C_SceneNode()
{
	for (S32 i = 0; i < E_StaticPlug_Count; i++)
		SAFE_DELETE_PTR(m_staticPlugs[i]);
}

const char* C_SceneNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_SceneNode::GetStaticTypeName()
{
	static char* p = "sceneNode";
	return p;
}

E_ObjType C_SceneNode::GetStaticType()
{
	return E_ObjType_SceneNode;
}

void* C_SceneNode::Create()
{
	return new C_SceneNode();
}

C_NullPlug& C_SceneNode::GetTranslatePlug()
{
	return *(C_NullPlug*)m_staticPlugs[E_StaticPlug_Translate];
}

C_NullPlug& C_SceneNode::GetRotatePlug()
{
	return *(C_NullPlug*)m_staticPlugs[E_StaticPlug_Rotate];
}

C_NullPlug& C_SceneNode::GetScalePlug()
{
	return *(C_NullPlug*)m_staticPlugs[E_StaticPlug_Scale];
}

C_NullPlug& C_SceneNode::GetShearPlug()
{
	return *(C_NullPlug*)m_staticPlugs[E_StaticPlug_Shear];
}

C_StaticPlug<MathsStorage::Matrix44>& C_SceneNode::GetLocalMatrixPlug()
{
	return *(C_StaticPlug<MathsStorage::Matrix44>*)m_staticPlugs[E_StaticPlug_LocalMatrix];
}

const C_StaticPlug<MathsStorage::Matrix44>& C_SceneNode::GetLocalMatrixPlug() const
{
	return *(C_StaticPlug<MathsStorage::Matrix44>*)m_staticPlugs[E_StaticPlug_LocalMatrix];
}

C_StaticPlug<MathsStorage::Matrix44>& C_SceneNode::GetWorldMatrixPlug()
{
	return *(C_StaticPlug<MathsStorage::Matrix44>*)m_staticPlugs[E_StaticPlug_WorldMatrix];
}

const C_StaticPlug<MathsStorage::Matrix44>& C_SceneNode::GetWorldMatrixPlug() const
{
	return *(C_StaticPlug<MathsStorage::Matrix44>*)m_staticPlugs[E_StaticPlug_WorldMatrix];
}

const C_NullPlug& C_SceneNode::GetTranslatePlug() const
{
	return *(C_NullPlug*)m_staticPlugs[E_StaticPlug_Translate];
}

const C_NullPlug& C_SceneNode::GetRotatePlug() const
{
	return *(C_NullPlug*)m_staticPlugs[E_StaticPlug_Rotate];
}

const C_NullPlug& C_SceneNode::GetScalePlug() const
{
	return *(C_NullPlug*)m_staticPlugs[E_StaticPlug_Scale];
}

const C_NullPlug& C_SceneNode::GetShearPlug() const
{
	return *(C_NullPlug*)m_staticPlugs[E_StaticPlug_Shear];
}

void C_SceneNode::AddChild(C_SceneNode* pChild)
{
	if (pChild->GetParent() == this)
		return;

	if (pChild->GetNextSibling() || pChild->GetPreviousSibling())
	{
		LOGERROR(C_Core::Instance().GetLogger(), 
			"Cannot add a child which has siblings.");
		return;
	}

	if (pChild->VetoParentChange(this))
		return;

	pChild->m_pParent = this;

	if (m_pChild)
	{
		C_SceneNode *pCurrent = m_pChild;
		while (pCurrent->m_pNext)
			pCurrent = pCurrent->m_pNext;

		pCurrent->m_pNext = pChild;
		pChild->m_pPrevious = pCurrent;
	}
	else
		m_pChild = pChild;

	C_ImmediateHintCollection hc(pChild);
	hc.hints.PushBack(new C_ImmediateReparentHint(true)); 
	pChild->ImmediatePropagate(hc);
}

C_SceneNode* C_SceneNode::GetParent()
{
	//No need to check if update required due to parenting being resolved immediately.
	return m_pParent;
}

const C_SceneNode* C_SceneNode::GetParent() const
{
	//No need to check if update required due to parenting being resolved immediately.
	return m_pParent;
}

C_SceneNode* C_SceneNode::GetChild()
{
	//No need to check if update required due to parenting being resolved immediately.
	return m_pChild;
}

const C_SceneNode* C_SceneNode::GetChild() const
{
	//No need to check if update required due to parenting being resolved immediately.
	return m_pChild;
}

C_SceneNode* C_SceneNode::GetPreviousSibling()
{
	//No need to check if update required due to parenting being resolved immediately.
	return m_pPrevious;
}

const C_SceneNode* C_SceneNode::GetPreviousSibling() const
{
	//No need to check if update required due to parenting being resolved immediately.
	return m_pPrevious;
}

C_SceneNode* C_SceneNode::GetNextSibling()
{
	//No need to check if update required due to parenting being resolved immediately.
	return m_pNext;
}

const C_SceneNode* C_SceneNode::GetNextSibling() const
{
	//No need to check if update required due to parenting being resolved immediately.
	return m_pNext;
}

E_ObjType C_SceneNode::GetType() const
{
	return E_ObjType_SceneNode;
}

bool C_SceneNode::IsA(E_ObjType type) const
{
	if (C_DgNodeBase::IsA(type) || type == E_ObjType_ISceneNode)
		return true;
	
	if (type == C_SceneNode::GetType()) 
		return true;

	return false;
}

void C_SceneNode::SetLocalMatrix(const Matrix44& matrix, bool compensateChildren)
{
	Aftk::Matrix44 oldLocalMatrix;
	if (compensateChildren)
		oldLocalMatrix = GetLocalMatrix();

	{
		PlugHelpers::SetMatrixComponents(
			*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Scale],
			*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Shear],
			*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Rotate],
			*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Translate],
			E_RotationOrder_XYZ,
			matrix,
			false); //Defer transform change call.

		//Okay, invoke transform change node.
		OnTransformPlugsChanged();
	}

	if (compensateChildren)
	{
		Aftk::Matrix44 newLocalMatrixInverse;
		UtMaths::Invert4x4(&newLocalMatrixInverse, matrix);

		C_SceneNode *pCurrentChild = m_pChild;
		while (pCurrentChild)
		{
			pCurrentChild->SetLocalMatrix(pCurrentChild->GetLocalMatrix() * oldLocalMatrix * newLocalMatrixInverse);
			pCurrentChild = pCurrentChild->m_pNext;
		}
	}
	else
	{
		C_SceneNode *pCurrentChild = m_pChild;
		while (pCurrentChild)
		{
			pCurrentChild->OnTransformPlugsChanged();
			pCurrentChild = pCurrentChild->m_pNext;
		}
	}
}

void C_SceneNode::SetWorldMatrix(const Matrix44& worldMatrix, bool compensateChildren)
{
	//We need to be able to invert our matrices.
	if (UtMaths::GetDeterminant4x4(worldMatrix) == 0.f)
	{
		//Non invertable matrix!
		return;
	}

	Aftk::Matrix44 invExclusiveWorldMatrix;
	if (!m_pParent)
		UtMaths::SetIdentity(&invExclusiveWorldMatrix);
	else
	{
		Aftk::Matrix44 inv;
		UtMaths::Invert4x4(&inv, m_pParent->GetWorldMatrix());
		invExclusiveWorldMatrix = inv;
	}

	SetLocalMatrix(worldMatrix * invExclusiveWorldMatrix, compensateChildren);
}

const Matrix44& C_SceneNode::GetLocalMatrix() const
{
	const MathsStorage::Matrix44* p;
	m_staticPlugs[E_StaticPlug_LocalMatrix]->GetValue(p);
	return (const Matrix44&)*p;
}

const Matrix44& C_SceneNode::GetWorldMatrix() const
{
	const MathsStorage::Matrix44* p;
	m_staticPlugs[E_StaticPlug_WorldMatrix]->GetValue(p);
	return (const Matrix44&)*p;
}

void C_SceneNode::ImmediatePropagate(const C_ImmediateHintCollection& hc, bool dirtyDependents) 
{
	for (S32 i = 0; i < hc.hints.Size(); i++)
	{
		E_ImmediateHint type = hc.hints[i]->GetType();
		if (type == E_ImmediateHint_Transform)
		{
			bool xtStore = true;
			if (type == E_ImmediateHint_Transform)
			{
				C_ImmediateTransformHint* pHint = (C_ImmediateTransformHint*)hc.hints[i];
				xtStore = pHint->XtStore();
			}

			Matrix44 localMatrix;
			PlugHelpers::SetMatrix(
				localMatrix,
				GetScalePlug(),
				GetShearPlug(),
				GetRotatePlug(),
				GetTranslatePlug(),
				E_RotationOrder_XYZ,
				false);
			((C_StaticPlug<MathsStorage::Matrix44>*)m_staticPlugs[E_StaticPlug_LocalMatrix])->
				SetValue((MathsStorage::Matrix44&)(localMatrix));

			if (m_pParent)
			{
				//Apply parental transform - this will potentially trigger re-evaluation of the parent's
				//world matrix. Local space is transformed into world space by post-multiplication.
				GetWorldMatrixPlug().SetValue(
					(MathsStorage::Matrix44&)(localMatrix * m_pParent->GetWorldMatrix()));
				
			}
			else
				GetWorldMatrixPlug().SetValue((MathsStorage::Matrix44&)localMatrix);

			if (xtStore)
				OnStoreTransform();

			m_sceneNodeDirtyFlags.transformViaImmediateDirty = true;
		}
		
		if (type == E_ImmediateHint_PostInitialise || type == E_ImmediateHint_Reparent)
		{
			bool xtStore = true;
			if (type == E_ImmediateHint_Reparent)
			{
				C_ImmediateReparentHint* pHint = (C_ImmediateReparentHint*)hc.hints[i];
				xtStore = pHint->XtStore();
			}

			Matrix44 localMatrix = GetLocalMatrix();
			if (m_pParent)
			{
				//Apply parental transform - this will potentially trigger re-evaluation of the parent's
				//world matrix. Local space is transformed into world space by post-multiplication.
				GetWorldMatrixPlug().SetValue(
					(MathsStorage::Matrix44&)(GetLocalMatrix() * m_pParent->GetWorldMatrix()));
			}
			else
				GetWorldMatrixPlug().SetValue((MathsStorage::Matrix44&)localMatrix);

			if (xtStore)
			{
				OnStoreTransform();
				OnStoreParent();
			}
			m_sceneNodeDirtyFlags.transformViaImmediateDirty = true;
			m_sceneNodeDirtyFlags.parentViaImmediateDirty = true;
		}
	}

	C_SceneNode *pCurrentChild = m_pChild;
	while (pCurrentChild)
	{
		//False arg ensures we call dirtydependents only at the top level.
		//would like to find a nicer way of doing this.
		pCurrentChild->ImmediatePropagate(hc, false);
		pCurrentChild = pCurrentChild->m_pNext;
	}

	if (dirtyDependents)
		DirtyDependents();
}

void C_SceneNode::OnTransformPlugsChanged()
{
	C_ImmediateHintCollection hc(this);
	hc.hints.PushBack(new C_ImmediateTransformHint(true)); 
	ImmediatePropagate(hc);
}

C_SceneNode::SceneNodeFlags& C_SceneNode::GetSceneNodeFlags()
{
	return m_sceneNodeFlags;
}

const C_SceneNode::SceneNodeFlags& C_SceneNode::GetSceneNodeFlags() const
{
	return m_sceneNodeFlags;
}

void C_SceneNode::PostInitialiseImmediatePropagate()
{
	C_ImmediateHintCollection hc(this);
	hc.hints.PushBack(new C_ImmediatePostInitialiseHint());
	ImmediatePropagate(hc);
}

bool C_SceneNode::VetoParentChange(const C_SceneNode* pCandidateParent) const
{
	I_Level* pLevel1 = C_Core::Instance().GetDgManager().LevelOf(m_id);
	BIZ_ASSERT(pLevel1);
	I_Level* pLevel2 = C_Core::Instance().GetDgManager().LevelOf(pCandidateParent->m_id);
	BIZ_ASSERT(pLevel2);

	if (pCandidateParent && (
		!(m_sceneNodeFlags.canReparent && pCandidateParent->m_sceneNodeFlags.canAddChildren) || 
		 (pLevel1 != pLevel2) // Cannot reparent across levels.
		))
	{
		LOGDEBUG(C_Core::Instance().GetLogger(), "Veto parent change %s", m_name.GetValue().Chars());
		return true;
	}

	return false;
}

void C_SceneNode::OnSceneNodePlugChanged(I_Plug* pPlug)
{
	C_SceneNode* pThis = (C_SceneNode*)PlugHelpers::GetTopLevelParent(*pPlug);
	if (PlugHelpers::AreRelated((I_Plug*)pThis->m_staticPlugs[E_StaticPlug_Translate], pPlug) ||
		PlugHelpers::AreRelated((I_Plug*)pThis->m_staticPlugs[E_StaticPlug_Rotate], pPlug) ||
		PlugHelpers::AreRelated((I_Plug*)pThis->m_staticPlugs[E_StaticPlug_Scale], pPlug) ||
		PlugHelpers::AreRelated((I_Plug*)pThis->m_staticPlugs[E_StaticPlug_Shear], pPlug))
		pThis->OnTransformPlugsChanged();
}


}
