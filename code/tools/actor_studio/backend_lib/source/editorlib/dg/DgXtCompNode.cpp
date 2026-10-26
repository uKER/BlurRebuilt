#include "stdafx.h"

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>


namespace EditorLib
{

void C_DgXtCompNode::XtBind() const
{
	for (DgPropArray::ConstIterator it = m_properties.Begin();
		it != m_properties.End(); ++it)
	{
		Sys::WeakPtr<C_DgXtPropNodeBase> pPropNode = *it;
		if (!pPropNode)
			continue;

		pPropNode->AsXtObject()->XtBind();	
	}

	C_XtObject::XtBind();
}

void C_DgXtCompNode::XtUnbind() const
{
	for (DgPropArray::ConstIterator it = m_properties.Begin();
		it != m_properties.End(); ++it)
	{
		Sys::WeakPtr<C_DgXtPropNodeBase> pPropNode = *it;
		if (!pPropNode)
			continue;

		pPropNode->AsXtObject()->XtUnbind();
	}

	C_XtObject::XtUnbind();
}

bool C_DgXtCompNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgXtCompNode::GetStaticType(), pHc);

	if (m_compNodeFlags.initialUpdatePending)
	{
		C_XtObject::C_Unbinder unbinder(this);
		RefreshPropertiesFromSchema();
		m_compNodeFlags.initialUpdatePending = false;
	}

	if (!__super::Update(&signaller.GetCollection())) return false;

	if (m_pParent)
	{
		m_compNodeDirtyFlags.hierarchyDirty = false;
	}
	else
	{
		BIZ_ASSERT(!"Parent must be set before initial update!");
	}
	return true;
}

bool C_DgXtCompNode::DirtyDependents(bool setThisDirty)
{
	if (!__super::DirtyDependents(setThisDirty))
		return false;

	for (DgPropArray::Iterator it = m_properties.Begin();
		it != m_properties.End(); ++it)
	{
		//Validate weak ptr.
		Sys::WeakPtr<C_DgXtPropNodeBase> pPropNode = *it;
		if (!pPropNode)
			continue;

		pPropNode->DirtyDependents();
	}

	return true;
}

bool C_DgXtCompNode::IsNonXtDirty() const
{
	return m_compNodeDirtyFlags.data != 0;
}

void C_DgXtCompNode::ResetNonXtDirty()
{
	__super::ResetNonXtDirty();
	m_compNodeDirtyFlags.data = 0;
}

const char* C_DgXtCompNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_DgXtCompNode::GetStaticTypeName()
{
	static char* p = "dgActorComponentNode";
	return p;
}

E_ObjType C_DgXtCompNode::GetStaticType()
{
	return E_ObjType_DgXtCompNode;
}

void* C_DgXtCompNode::Create()
{
	return new C_DgXtCompNode();
}

void C_DgXtCompNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
	C_Core::Instance().GetDgManager().AddForDeferredUpdate(m_id, typeChanged);
	DirtyDependents();
	{
		C_XtObject::C_Unbinder unbinder(this);
		RefreshPropertiesFromSchema();
	}
}


C_DgXtCompNode::C_DgXtCompNode() : C_DgNodeBase(), C_XtObject()
{
	m_compNodeDirtyFlags.data = 0xffffffff;
	m_compNodeFlags.data = 0;
	m_compNodeFlags.initialUpdatePending = true;
	m_pParent = 0;
}

C_DgXtCompNode::~C_DgXtCompNode()
{
	for (DgPropArray::Iterator it = m_properties.Begin();
		it != m_properties.End(); ++it)
	{
		//Validate weak ptr.
		Sys::WeakPtr<C_DgXtPropNodeBase> pPropNode = *it;
		if (!pPropNode)
			continue;

		//Check our bind status is consistent.
		assert(IsBound() == pPropNode->IsBound());
		C_Core::Instance().GetDgManager().DestroyNode(pPropNode.RawPtr(), true);
	}

	if (IsBound())
	{
		//We would delete any Xt data here.
	}
	
	XtNodeHelpers::ReleaseXtId(m_id, this);
}

C_DgXtPropNodeBase* C_DgXtCompNode::CreateProperty(
	S32 index, LarvaType* pType, Xt::DomElement* pElement, const char* name, C_Status& status)
{
	I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(m_id);
	BIZ_ASSERT(pLevel);
	BIZ_ASSERT(pLevel->GetType() == E_LevelType_XtLevel);

	C_DgXtPropNodeBase* pProperty = 
		(C_DgXtPropNodeBase*)C_Core::Instance().GetDgManager().CreateNode(
		pLevel, 
		name);
	if (!pProperty)
	{
		LOGWARN(C_Core::Instance().GetLogger(), "Failed to create property node of type %s.", pType->name.Text());
		return 0;
	}
	pProperty->GetNodeFlags().userDeletable = 0;
	pProperty->SetParent(this, index);
	status = pProperty->XtInitialise((C_XtLevel*)pLevel, pType, pElement);
	if (IsBound())
		pProperty->XtBind();
	pProperty->SetName(name);
	pProperty->GetNodeFlags().canRename = false;
	return pProperty;
}

void C_DgXtCompNode::DestroyProperty(C_DgXtPropNodeBase* pProperty)
{
	if (!pProperty) return;

	C_Core::Instance().GetDgManager().DestroyNode(
		pProperty, true);
}

C_Status C_DgXtCompNode::RefreshPropertiesFromSchema()
{
	Xt::DomElement* pData = GetXtData();
	//Return if we're a null component.
	if (!pData) return E_Status_Success;

	C_Status status;
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	Xt::DomBlock* pBlockData = 0;
	if (pData->kind == Xt::kDomComponent)
		pBlockData = &Xt::AsComponent(pData)->data;
	else if (pData->kind == Xt::kDomBlock)
		pBlockData = Xt::AsBlock(pData);
	else 
	{
		BIZ_ASSERT(!"Failed to create properties!");
		return E_Status_Failure;
	}
	BIZ_ASSERT(pBlockData);
	Btl::Vector<XtNodeHelpers::C_XtPropertyDesc> propertyDescArray;
	XtNodeHelpers::BuildXtActorPropertyList(propertyDescArray, GetXtType(), pBlockData, true);

	S32 nProperties = m_properties.Size();
	bool updateExisting = m_properties.Size() > 0;
	S32 propertyIndex = 0;
	for (S32 i = 0; i < propertyDescArray.Size(); ++i)
	{
		XtNodeHelpers::C_XtPropertyDesc& propDesc = propertyDescArray[i];
		LarvaType* pNewType = 0;
		Xt::DomElement* pNewData = 0;
		if (propDesc.pLarvaField && propDesc.pLarvaField->kind == kLfPtr)
		{
			pNewData = Xt::At(pBlockData, propDesc.pLarvaField);
			pNewType = Xt::TypeOf(pSchema, propDesc.pLarvaField, pNewData);
			Xt::DomComponent* pDomComponent = Xt::AsComponent(pNewData);
			pNewData = &pDomComponent->data;
		}
		else if (propDesc.pLarvaField && propDesc.pLarvaField->kind == kLfWeakPtr)
		{
			//If updating a weak ref to another node.
			if (updateExisting)
			{
				Xt::DomRef* pRef = Xt::AsRef(propDesc.pData);
				XtId id = pRef->value;
				C_NodeId nodeId(id, kNoChildId);
				EditorLib::I_DgNode* pSource = C_Core::Instance().GetDgManager().FindNodeById(nodeId);
				
				XtString propType;
				if (pSource && pSource->AsXtObject())
				{
					//Initialise to point at source.
					pNewType = pSource->AsXtObject()->GetXtType();
					if (pNewType)
					{
						propType = pNewType->attrs.GetString("SceneView", "property");
						pNewData = pSource->AsXtObject()->GetXtData();
					}
				}
				//Override with field itself.
				{
					XtString tmp = propDesc.pLarvaField->attrs.GetString("SceneView", "property");
					if (tmp != "")
					{
						pNewType = propDesc.pLarvaField->Type();
						pNewData = propDesc.pData;
						propType = tmp;
					}
				}
			}
			//Else we're performing our first initialisation. At this point the node
			//we want to refer to may well not yet have been created, so we defer
			//initialisation of this property until our xt callback.
			else
			{
				pNewData = 0;
				pNewType = 0;
			}
		}
		else
		{
			pNewType = propDesc.pLarvaField->Type();
			pNewData = Xt::Acquire(pData, propDesc.pLarvaField);
		}
	
		bool isNewPropertyNull = pNewType == 0 || pNewData == 0;

		//Determine property node type.
		XtString attrPropType = 
			pNewType ? propDesc.pLarvaField->attrs.GetString("SceneView", "property") : "";
		
		const char* pNewTypeName = "";
		if (pNewType)
		{
			pNewTypeName = attrPropType != "" ? 
				attrPropType.Text() : propDesc.pLarvaField->Type()->name.Text();
		}

		if (updateExisting)
		{
			if (propertyIndex >= nProperties)
			{
				BIZ_ASSERT_FAIL("Failed to refresh properties!");
				return E_Status_Failure;
			}

			const char* pExistingTypeName = 
				m_properties[propertyIndex] ? m_properties[propertyIndex]->GetTypeName() : "";
			if (strcmp(pExistingTypeName, pNewTypeName) || m_properties[propertyIndex]->GetXtData() != pNewData)
			{
				if (m_properties[propertyIndex].RawPtr())
					//Changing type/data. Destroy!
					DestroyProperty(m_properties[propertyIndex].RawPtr());
			}
			//Data and type the same.
			else
			{
				++propertyIndex;
				continue;
			}
		}

		C_DgXtPropNodeBase* pProperty;
		if (isNewPropertyNull)
		{
			pProperty = 0;
		}
		else
		{
			pProperty = CreateProperty(
				propertyIndex,
				pNewType, 
				pNewData, 
				pNewTypeName, 
				status);
		}
		if (!status) return status;

		if (updateExisting)
		{
			m_properties[propertyIndex] = pProperty;
		}
		else
		{
			m_properties.PushBack(); 
			m_properties.Back() = pProperty;
		}

		++propertyIndex;
	}

	m_properties.TrimCapacity();
	return E_Status_Success;
}

E_ObjType C_DgXtCompNode::GetType() const
{
	return E_ObjType_DgXtCompNode;
}

bool C_DgXtCompNode::IsA(E_ObjType type) const
{
	if (C_DgNodeBase::IsA(type))
		return true;

	if (type == C_DgXtCompNode::GetType()) 
		return true;

	return false;
}


C_Status C_DgXtCompNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;
	C_Core::Instance().GetDgManager().AddForDeferredUpdate(m_id, false);

	status = RefreshPropertiesFromSchema();
	if (!status) return status;

	return E_Status_Success;
}

const DgPropArray& C_DgXtCompNode::GetProperties()
{
	return m_properties;
}

const ConstDgPropArray& C_DgXtCompNode::GetProperties() const
{
	return (ConstDgPropArray&)m_properties;
}

I_DgNode* C_DgXtCompNode::GetParent()
{
	return m_pParent;
}

const I_DgNode* C_DgXtCompNode::GetParent() const
{
	return m_pParent;
}

void C_DgXtCompNode::SetParent(I_DgNode* pParent, U32 childId)
{
	//Cannot reparent component.
	BIZ_ASSERT(!m_pParent);
	BIZ_ASSERT(childId < 0xffff);
	m_pParent = pParent;

	I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(m_pParent->GetId());
	BIZ_ASSERT(pLevel);
	BIZ_ASSERT(pLevel->GetType() == E_LevelType_XtLevel);

	bool result =
		XtNodeHelpers::AssumeXtId((C_XtLevel*)pLevel, m_id, this, pParent->GetId().GetXtId(), (childId << 16) | 0xffff);
	BIZ_ASSERT(result);

	DirtyDependents();
}

C_XtObject* C_DgXtCompNode::AsXtObject()
{
	return (C_XtObject*)this;
}

const C_XtObject* C_DgXtCompNode::AsXtObject() const
{
	return (const C_XtObject*)this;
}

}
