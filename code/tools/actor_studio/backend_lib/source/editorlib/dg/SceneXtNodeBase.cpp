#include "stdafx.h"

#include <EditorLib/Collision/Collision.h>
#include <EditorLib/Dg/SceneXtNodeBase.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/Hints/UpdateNameHint.h>
#include <EditorLib/Dg/Hints/UpdateTransformHint.h>
#include <EditorLib/Dg/Hints/ImmediateTransformHint.h>
#include <EditorLib/Dg/Hints/ImmediateReparentHint.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>


namespace EditorLib
{

//From I_DgObject
void C_SceneXtNodeBase::SetName(const Btl::String& name)
{
	Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());
	assert(pComponent);

	XtNodeHelpers::XtStoreName(name, *this, AsStruct(GetXtType()), &pComponent->data);
}

bool C_SceneXtNodeBase::IsA(E_ObjType type) const
{
	if (C_SceneNode::IsA(type))
		return true;

	if (type == E_ObjType_SceneXtNodeBase) 
		return true;

	return false;
}


//From I_DgNode
bool C_SceneXtNodeBase::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, E_ObjType_SceneXtNodeBase, pHc);

	Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());

	if (GetXtData())
	{
		//Name
		{
			Xt::DomField* pField  = Xt::AsField(FindBySemantic(
				AsStruct(GetXtType()),
				&pComponent->data,
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_Name).dom);
			if (GetXtDirty().IsDirty(pField))
			{
				//Load existing value.
				Btl::String tmp(m_name.GetValue(false));

				//Load new value.
				XtNodeHelpers::XtDeserialiseName(tmp, this, &pComponent->data);

				//Store.
				m_name.SetValue(tmp, false);
				//Add hint to indicate what's just happened.
				signaller.GetCollection().hints.PushBack(new C_UpdateNameHint());
				m_nodeDirtyFlags.nameDirty = false;
			}
		}

		//Transform
		{
			Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(&pComponent->data, "__transform"));
			if (GetXtDirty().IsDirty(pBlock))
			{
				XtNodeHelpers::XtDeserialiseTransform(
					*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Scale],
					*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Shear],
					*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Rotate],
					*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Translate],
					false,
					&pComponent->data);

				C_ImmediateHintCollection hc(this);
				hc.hints.PushBack(new C_ImmediateTransformHint(false)); 
				ImmediatePropagate(hc);
				signaller.GetCollection().hints.PushBack(new C_UpdateTransformHint());
			}
			
			//Update collision info.
			I_CollisionInstance* pInstance = C_Core::Instance().GetCollisionManager().FindCollisionInstance(this);
			if (pInstance)
			{
				pInstance->toWorldSpace = GetWorldMatrix();
				UtMaths::Invert4x4(
					&pInstance->toObjectSpace, 
					pInstance->toWorldSpace);
			}
		}

		//Parenting
		{
			bool updateIfParentChanged = false;
			XtId xtParentId = 0;
			//Non movement logic parent.
			{
				Xt::DomRef* pParentRef = Xt::AsRef(Xt::At(&pComponent->data, "__parent"));
				if (GetXtDirty().IsDirty(pParentRef))
				{
					xtParentId = pParentRef->value;
					updateIfParentChanged = true;
				}
			}

			//Implemnent parent change.
			if (updateIfParentChanged)
			{
				C_NodeId parentId(xtParentId, kNoChildId);
				EditorLib::I_DgNode *pParent = C_Core::Instance().GetDgManager().FindNodeById(parentId);
				//If we've no valid parent, use the root of the level.
				if (!pParent)
				{
//					LOGDEBUG(C_Core::Instance().GetLogger(), "Could not find parent %x for %s",
//						xtParentId, GetName().Chars());
					I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(m_id);
					BIZ_ASSERT(pLevel);
					pParent = pLevel->GetRootSceneNode();
				}

				BIZ_ASSERT(pParent);
				//If our parent has changed...
				if (pParent != m_pParent)
				{
					BIZ_ASSERT(pParent->IsA(E_ObjType_SceneNode));
					C_SceneNode* pParentSceneNode = ((C_SceneNode*)pParent);
					if (!m_pParent)
						pParentSceneNode->AddChild(this);
					else
						C_SceneNodeHelpers::ReparentSceneNode(this, pParentSceneNode, false);

					//Add hint. TODO: Not implemented
					//signaller.GetCollection().hints.PushBack(new C_UpdateHierarchyHint());
				}
			}
		}
	}

	//Update superclass.
	if (!__super::Update(&signaller.GetCollection())) return false;

	return true;
}

bool C_SceneXtNodeBase::DirtyDependents(bool setThisDirty)
{
	if (!__super::DirtyDependents(setThisDirty))
		return false;

	for (DgActorComponentArray::Iterator it = m_components.Begin(); 
		it != m_components.End(); ++it)
	{
		(*it)->DirtyDependents();
	}

	return true;
}

bool C_SceneXtNodeBase::IsNonXtDirty() const
{
	return __super::IsNonXtDirty() || m_sceneXtNodeDirtyFlags.data;
}

void C_SceneXtNodeBase::ResetNonXtDirty()
{
	__super::ResetNonXtDirty();
	m_sceneXtNodeDirtyFlags.data = 0;
}

C_XtObject* C_SceneXtNodeBase::AsXtObject()
{
	return (C_XtObject*)this;
}

const C_XtObject* C_SceneXtNodeBase::AsXtObject() const
{
	return (const C_XtObject*)this;
}


//From C_SceneNode

bool C_SceneXtNodeBase::VetoParentChange(const C_SceneNode* pCandidateParent) const
{
	if (C_SceneNode::VetoParentChange(pCandidateParent))
		return true;

	I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(m_id);
	BIZ_ASSERT(pLevel);

	if (!pCandidateParent || pCandidateParent == pLevel->GetRootSceneNode())
		//Don't veto
		return false;

	//May be parented to root or another group node.
	if (pCandidateParent->IsA(EditorLib::E_ObjType_SceneXtNode))
		//Don't veto
		return false;

	//Veto
	return true;
}

void C_SceneXtNodeBase::OnStoreTransform()
{
	C_XtObject::C_Unbinder unbinder(this);
	Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());
	if (!pComponent) return;

	//Store local transform (for tools use only).
	{
		Vector3 shear, scale, rotate, translate;
		PlugHelpers::GetCompoundValue(GetScalePlug(), scale, false);
		PlugHelpers::GetCompoundValue(GetShearPlug(), shear, false);
		PlugHelpers::GetCompoundValue(GetRotatePlug(), rotate, false);
		PlugHelpers::GetCompoundValue(GetTranslatePlug(), translate, false);
		
		Xt::DomBlock *pTransform = 
			Xt::AcquireBlock(&pComponent->data, "__transform");
		{
			Xt::DomBlock *pComponent = 0;
			pComponent = Xt::AsBlock(Xt::At(pTransform, "scale"));
			XtFieldHelper::SetVector(pComponent, scale);

			pComponent = Xt::AsBlock(Xt::At(pTransform, "shear"));
			XtFieldHelper::SetVector(pComponent, shear);

			pComponent = Xt::AsBlock(Xt::At(pTransform, "rotate"));
			XtFieldHelper::SetVector(pComponent, rotate);

			pComponent = Xt::AsBlock(Xt::At(pTransform, "translate"));
			XtFieldHelper::SetVector(pComponent, translate);
		}
	}
	//Store the world matrix (for game use only)
	{
		const Matrix44& worldMatrix = GetWorldMatrix();
		Xt::DomBlock* pMatrixBlock = Xt::AsBlock(AcquireBySemantic(
			AsStruct(GetXtType()), 
			&pComponent->data,
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_WorldMatrix).dom);
		if (pMatrixBlock)
		{			
			//Callbacks disabled - set entire matrix.
			XtFieldHelper::SetMatrix(pMatrixBlock, worldMatrix);
		}
	}
}

void C_SceneXtNodeBase::OnStoreParent()
{
	if (IsBound())
	{
		C_XtObject::C_Unbinder unbinder(this);

		Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());
		if (!pComponent) return;
		Xt::DomRef* pParentField = Xt::AcquireRef(&pComponent->data, "__parent");
		I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(m_id);
		BIZ_ASSERT(pLevel);
		if (!m_pParent || m_pParent == pLevel->GetRootSceneNode())
			Xt::Set(pParentField, XtId(0));
		else
			Xt::Set(pParentField, m_pParent->GetId().GetXtId());
	}
}

void C_SceneXtNodeBase::ImmediatePropagate(const C_ImmediateHintCollection& hc, bool dirtyDependents)
{
	__super::ImmediatePropagate(hc, dirtyDependents);
}


//From C_SceneXtNodeBase
C_SceneXtNodeBase::C_SceneXtNodeBase() : 
C_SceneNode(), C_XtObject()
{
	m_sceneXtNodeDirtyFlags.data = 0xffffffff;
	m_sceneXtNodeFlags.data = 0;
}

C_SceneXtNodeBase::~C_SceneXtNodeBase()
{
	//Remove collision information.
	C_CollisionScene& collisionScene = 
		C_Core::Instance().GetCollisionManager().GetGlobalCollisionScene();
	collisionScene.RemoveCollisionInstance(this);

	for (DgActorComponentArray::Iterator it = m_components.Begin();
		it != m_components.End(); ++it)
	{
		//Validate weak ptr.
		if (!(*it))
			continue;

		//Check our bind status is consistent.
		assert(IsBound() == (*it)->IsBound());
		C_Core::Instance().GetDgManager().DestroyNode((*it).RawPtr(), true);
	}

	XtNodeHelpers::ReleaseXtId(m_id, this);
}

C_DgXtCompNode* C_SceneXtNodeBase::CreateComponent(
	LarvaType* pType, Xt::DomElement* pElement, const char* name, C_Status& status)
{
	I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(m_id);
	BIZ_ASSERT(pLevel);
	BIZ_ASSERT(pLevel->GetType() == E_LevelType_XtLevel);

	//Create actor component.
	C_DgXtCompNode* pActorComponent = 
		(C_DgXtCompNode*)C_Core::Instance().GetDgManager().CreateNode(
			(C_XtLevel*)pLevel,
			E_ObjType_DgXtCompNode);
	if (!pActorComponent)
	{
		status = E_Status_Failure;
		return 0;
	}

	//set deletable flag as this will be propagated to any properties created
	pActorComponent->GetNodeFlags().userDeletable = 0;
	status = pActorComponent->XtInitialise((C_XtLevel*)pLevel, pType, pElement);
	pActorComponent->SetName(name);
	pActorComponent->GetNodeFlags().canRename = false;

	return pActorComponent;
}

void C_SceneXtNodeBase::DestroyComponent(C_DgXtCompNode* pComponent)
{
	if (!pComponent) return;

	C_Core::Instance().GetDgManager().DestroyNode(
		pComponent, true);
}

C_Status C_SceneXtNodeBase::RefreshComponentsFromSchema()
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	C_XtTypeDatabase& dgTypeDatabase = C_Core::Instance().GetXtTypeDatabase();

	//Create components as specified in the schema.
	LtStruct* pActorLarvaStruct = AsStruct(GetXtType());
	if (!pActorLarvaStruct)
		return E_Status_Success;

	C_Status status;
	Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());
	Xt::DomBlock* pBlockData = &pComponent->data;

	S32 iComponentIndex = m_sceneXtNodeFlags.hasDefaultComponent ? 1 : 0;
	for(U32 i = 0; i < pActorLarvaStruct->fields.count; i++)
	{
		LarvaField* pFieldSchema = pActorLarvaStruct->fields[i];
		LarvaType* pCandidateComponentType = 0;
		Xt::DomElement* pCandidateData = 0;

		if (pFieldSchema && pFieldSchema->kind == kLfPtr)
		{
			pCandidateData = Xt::At(pBlockData, pFieldSchema);
			pCandidateComponentType = Xt::TypeOf(pSchema, pFieldSchema, pCandidateData);
			Xt::DomComponent* pDomComponent = Xt::AsComponent(pCandidateData);
			pCandidateData = &pDomComponent->data;
		}
		else if (pFieldSchema && pFieldSchema->kind == kLfStruct)
		{
			pCandidateData = Xt::AcquireBlock(
				pBlockData, pActorLarvaStruct->fields[i]->name);
			pCandidateComponentType = pFieldSchema->Type();
		}
		else
			continue;

		//Find any existing component at this index.
		enum E_Action
		{
			E_Action_Replace,
			E_Action_Append,
		};
		E_Action action;

		C_DgXtCompNode* pExistingComponent(0);
		if (iComponentIndex < m_components.Size())
		{
			pExistingComponent = 
				(C_DgXtCompNode*)m_components[iComponentIndex]->RawPtr();
			action = E_Action_Replace;
		}
		else
			action = E_Action_Append;

		bool isNewComponentNull = pCandidateComponentType == 0;
		{
			if (pFieldSchema->kind == kLfPtr)
			{
				LfPtr* pPtr = (LfPtr*)pFieldSchema;
				const char* pBaseTypeName = pPtr->baseType->name.Text();
				I_XtType* pComponentType = dgTypeDatabase.FindType(pBaseTypeName);
				if (!pComponentType || !pComponentType->IsA(E_XtType_DgXtCompNode))
					continue;
			}
			else
			{
				const char* pDerivedTypeName = pCandidateComponentType->name.Text();
				I_XtType* pComponentType = dgTypeDatabase.FindType(pDerivedTypeName);
				if (!pComponentType || !pComponentType->IsA(E_XtType_DgXtCompNode))
					continue;
				else
					isNewComponentNull = false;
			}
		}

		if (pExistingComponent)
		{
			const char* pExistingTypeName = pExistingComponent->GetXtTypeName();
			const char* pNewTypeName = pCandidateComponentType ? 
				pCandidateComponentType->name.Text() : "";

			if (!strcmp(pExistingTypeName, pNewTypeName))
			{
				//Not changing type.
				++iComponentIndex;
				continue;
			}
		}

		C_DgXtCompNode* pNewActorComponent;
		if (isNewComponentNull)
		{
			pNewActorComponent = CreateComponent(
				0, 
				0, 
				pFieldSchema->name.Text(), 
				status);
		}
		else
		{
			pNewActorComponent = CreateComponent(
				pCandidateComponentType, 
				pCandidateData,
				pCandidateComponentType->name.Text(),
				status);
		}
		if (!status) return status;
		
		//Replace new.
		if (action == E_Action_Replace)
		{
			DestroyComponent(pExistingComponent);
			m_components[iComponentIndex] = pNewActorComponent;
		}
		//Append new.
		else 
		{
			m_components.PushBack(); 
			m_components.Back() = pNewActorComponent;
			BIZ_ASSERT(iComponentIndex == m_components.Size()-1);
		}

		//Set component parent and binding.
		BIZ_ASSERT(pNewActorComponent);
		pNewActorComponent->SetParent(this, iComponentIndex);
		if (IsBound())
			pNewActorComponent->XtBind();
		++iComponentIndex;
	}

	m_components.TrimCapacity();
	return E_Status_Success;
}

void C_SceneXtNodeBase::XtBind() const
{
	__super::XtBind();

	for (DgActorComponentArray::ConstIterator it = m_components.Begin();
		it != m_components.End(); ++it)
		if (*it)
			(*it)->XtBind();
}

void C_SceneXtNodeBase::XtUnbind() const
{
	for (DgActorComponentArray::ConstIterator it = m_components.Begin();
		it != m_components.End(); ++it)
		if (*it)
			(*it)->XtUnbind();

	__super::XtUnbind();
}

const DgActorComponentArray& C_SceneXtNodeBase::GetComponents()
{
	return m_components;
}

const ConstDgActorComponentArray& C_SceneXtNodeBase::GetComponents() const
{
	return (ConstDgActorComponentArray&)m_components;
}

C_Status C_SceneXtNodeBase::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	//Initialise xtid
	Xt::DomComponent* pComponent = Xt::AsComponent(pData);
	BIZ_ASSERT(pComponent);
	XtNodeHelpers::AssumeXtId(pLevel, m_id, this, pComponent->id, kNoChildId, pComponent);

	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;
	C_Core::Instance().GetDgManager().AddForDeferredUpdate(m_id, false);

	Xt::AcquireRef(&pComponent->data, "__parent");

	XtNodeHelpers::XtInitialiseTransform(
		*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Scale],
		*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Shear],
		*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Rotate],
		*(C_NullPlug*)m_staticPlugs[E_StaticPlug_Translate],
		false,
		&pComponent->data);

	Matrix44 identity;
	UtMaths::SetIdentity(&identity);

	GetWorldMatrixPlug().SetValue(
		(MathsStorage::Matrix44&)identity);

	// If we're creating a null actor, skip type specific initialisation.
	if (!pLarvaType)
		return E_Status_Success;

	{
		Btl::String name;
		XtNodeHelpers::XtInitialiseName(name, this, AsStruct(pLarvaType), &pComponent->data);
		m_name.SetValue(name);
	}

	//Initialise ->game exported world matrix.
	{
		LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
		Xt::DomBlock* pDomElement = Xt::AsBlock(AcquireBySemantic(
			AsStruct(GetXtType()), 
			&pComponent->data, 
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_WorldMatrix).dom);
		if (pDomElement) 
		{
			if (XtFieldHelper::IsNullMatrix(pDomElement))
				XtFieldHelper::SetMatrix(pDomElement, identity);
		}
	}

	//Create default component.
	BIZ_ASSERT(m_components.Size() == 0);
	{
		C_DgXtCompNode* pActorComponent = CreateComponent(
			GetXtType(), &pComponent->data, "Default", status);
		if (!status) return status;
		if (pActorComponent->GetProperties().Size() == 0)
		{
			DestroyComponent(pActorComponent);
			m_sceneXtNodeFlags.hasDefaultComponent = false;
		}
		else
		{
			m_sceneXtNodeFlags.hasDefaultComponent = true;
			m_components.PushBack(); m_components.Back() = pActorComponent;
			pActorComponent->SetParent(this, m_components.Size()-1);
		}
	}

	status = RefreshComponentsFromSchema();
	if (!status) return status;

	C_InternalCollisionInstance* pInstance = new C_InternalCollisionInstance;
	{
		//Initialise instance. 
		UtMaths::SetIdentity(&pInstance->toWorldSpace);
		UtMaths::SetIdentity(&pInstance->toObjectSpace);
		pInstance->pSource = &C_Core::Instance().GetCollisionManager().GetDefaultCollisionSource();
	}
	C_CollisionScene& collisionScene = 
		C_Core::Instance().GetCollisionManager().GetGlobalCollisionScene();
	collisionScene.AddCollisionInstance(this, pInstance);

	return E_Status_Success;
}

void C_SceneXtNodeBase::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
	C_Core::Instance().GetDgManager().AddForDeferredUpdate(m_id, typeChanged);
	DirtyDependents();

	Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());

	//Name dirty?
	{
		Xt::DomField* pField  = Xt::AsField(FindBySemantic(
			AsStruct(GetXtType()),
			&pComponent->data,
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_Name).dom);
		if (pField && pEvent->from ? XtHelpers::AreRelated(pField, pEvent->from) : true)
			GetXtDirty().SetDirty(pField);
	}

	//Transform dirty?
	{
		Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(&pComponent->data, "__transform"));
		if (pBlock && pEvent->from ? XtHelpers::AreRelated(pBlock, pEvent->from) : true)
			GetXtDirty().SetDirty(pBlock);
	}

	//Hierarchy dirty?
	{
		{
			Xt::DomRef* pField;
			XT_HELPERS_AT_REF_CHECKED(pField, &pComponent->data, "__parent");
			if (pField && pEvent->from ? XtHelpers::AreRelated(pField, pEvent->from) : true)
				GetXtDirty().SetDirty(pField);
		}
	}

	{
		C_XtObject::C_Unbinder unbinder(this);
		RefreshComponentsFromSchema();
	}
}

}
