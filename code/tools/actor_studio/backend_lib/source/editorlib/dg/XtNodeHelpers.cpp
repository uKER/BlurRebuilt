#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>

#include <EditorLib/Misc/GuidManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{
namespace XtNodeHelpers
{

C_Status InitialiseXtId(Xt::DomComponent* pData)
{
	return GenerateIdForComponent(pData, false) ? E_Status_Success : E_Status_Failure;
}

bool AssumeXtId(C_XtLevel* pLevel, C_NodeId& nodeId, I_DgNode* pNode, XtId xtId, U32 childId, Xt::DomComponent* pComponent)
{
	if (pLevel)
		pLevel->OnDetachDgNode(pNode);

	//If not direct xt backed.
	if (childId != kNoChildId)
	{
		C_NodeId oldNodeId = nodeId; 
		nodeId = C_NodeId(xtId, childId);
		bool result = C_Core::Instance().GetDgManager().RegisterId(pNode, nodeId);
		if (result)
		{
			if (nodeId != oldNodeId)
			{
				C_Core::Instance().GetDgManager().UnregisterId(oldNodeId);
			}
			if (pLevel)
				pLevel->OnAttachDgNode(pNode);
		}
		return result;
	}

	if (!pNode->AsXtObject())
		return false;

	//Store original id values to maybe free up later.
	U32 oldGuid = nodeId.GetXtId();
	C_NodeId oldNodeId = nodeId; 
	//Attempt to register new id.
	nodeId = C_NodeId(xtId, childId);
	U32 guid = xtId;
	while (
		!C_Core::Instance().GetGuidManager().RegisterGuid(guid) ||
		!C_Core::Instance().GetDgManager().RegisterId(pNode, nodeId))
	{
		//Xt component id conflict. Regenerate.
		if (!pComponent)
		{
			BIZ_ASSERT_FAIL("No component supplied!");
			return false;
		}
		
		if (!GenerateIdForComponent(pComponent, true))
			return false;

		//Write back new node id.
		nodeId = C_NodeId(pComponent->id, childId);
		guid = pComponent->id;
	}

	//Unregister old ids.
	if (nodeId != oldNodeId)
		C_Core::Instance().GetDgManager().UnregisterId(oldNodeId);
	if (oldGuid != guid)
		C_Core::Instance().GetGuidManager().DestroyGuid(oldGuid);

	if (pLevel)
		pLevel->OnAttachDgNode(pNode);

	return true;
}

void ReleaseXtId(C_NodeId& nodeId, I_DgNode* pNode)
{
	//Unregister top level node guid.
	if (nodeId.GetChildId() == kNoChildId)
		C_Core::Instance().GetGuidManager().DestroyGuid(nodeId.GetXtId());

	C_Core::Instance().GetDgManager().UnregisterId(nodeId);
	//Clear id so I_DGNode doesn't attempt to clear it.
	nodeId = C_NodeId();
}

I_DgNode* FindNodeByXtId(XtId xtId)
{
	for (C_DgNodeIterator dgNodeIt = C_Core::Instance().GetDgManager().GetDgIterator(); (I_DgNode*)dgNodeIt; ++dgNodeIt)
	{
		I_DgNode* pDgNode = (I_DgNode*)dgNodeIt;
		C_XtObject* pXtObject = pDgNode->AsXtObject();
		if (pDgNode && pDgNode->GetId().GetXtId() == xtId && pDgNode->GetId().GetChildId() == kNoChildId)
		{
			return pDgNode;
		}
	}
	return 0;
}

void XtStoreName(
	const Btl::String& proposedName, 
	I_DgNode& dgNode,
	LtStruct* pType,
	Xt::DomBlock* pData)
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomField* pField  = Xt::AsField(AcquireBySemantic(
		pType, 
		pData, 
		kXtSemanticAttr,
		kXtSemanticParam,
		kXtSemanticLabel_Name).dom);

	if (strcmp(proposedName.Chars(), (const char*)pField->value))
		Xt::Set(pField, proposedName.Chars());
}

void XtInitialiseName(
	Btl::String& existingName, 
	I_DgNode* pDgNode, 
	LtStruct* pType,
	Xt::DomBlock* pData)
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomField* pField  = Xt::AsField(AcquireBySemantic(
		pType,
		pData,
		kXtSemanticAttr,
		kXtSemanticParam,
		kXtSemanticLabel_Name).dom);

	Btl::String tmp = (const char*)pField->value;

	//Ensure unique.
	C_Core::Instance().GetDgManager().CreateName(pDgNode, tmp);
	//Store data in Xt if not done so already.
	if (strcmp(tmp.Chars(), (const char*)pField->value))
		Xt::Set(pField, tmp.Chars());

	existingName = (const char*)pField->value;
}

bool XtDeserialiseName(
	Btl::String& existingName, 
	I_DgNode* pDgNode, 
	Xt::DomElement* pFrom)
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	C_XtObject* pObj = pDgNode->AsXtObject();
	LtStruct* pLarvaType = AsStruct(pObj->GetXtType());
	assert (pLarvaType);
	
	Xt::DomComponent* pComponent = Xt::AsComponent(pObj->GetXtData());
	assert(pComponent);

	Xt::DomBlock* pRoot = &pComponent->data;
	assert(pRoot);

	Xt::DomField* pField  = Xt::AsField(FindBySemantic(
		pLarvaType,
		pRoot,
		kXtSemanticAttr,
		kXtSemanticParam,
		kXtSemanticLabel_Name).dom);
	if (pField && pFrom ? XtHelpers::AreRelated(pField, pFrom) : true)
	{
		Btl::String proposedName = (const char*)pField->value;

		//Don't do anything if we don't have to.
		if (proposedName == existingName)
			return false;

		//Remove old name from registry. Not needed when initialising
		C_Core::Instance().GetDgManager().DestroyName(
			pDgNode, existingName);

		//Ensure unique.
		C_Core::Instance().GetDgManager().CreateName(0, proposedName);
		
		//If we cannot use the name we're deserialising, use a newly proposed name we've just created.
		//This is an instance where we're knowingly violating the Xt rule that you don't modify the
		//db within a monitor callback. We need to veto clashing names however and we'll only recurse
		//once.
		if (strcmp(proposedName.Chars(), (const char*)pField->value))
		{
			C_XtObject::C_Unbinder unbinder(pDgNode->AsXtObject());
			Xt::Set(pField, proposedName.Chars());
			//Recurse.
			if (!XtDeserialiseName(existingName, pDgNode, pFrom))
				return false;
		}
		else
		{
			C_Core::Instance().GetDgManager().CreateName(pDgNode, proposedName);
		}

		existingName = (const char*)pField->value;
		return true;
	}
	
	return false;
}

void XtInitialiseTransform(
	C_NullPlug& scale,
	C_NullPlug& shear,
	C_NullPlug& rotate,
	C_NullPlug& translate,
	bool triggerTopLevelParentOnChanged,
	Xt::DomBlock* pFrom)
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomBlock* pDomElement = Xt::AsBlock(Xt::At(pFrom, "__transform"));
	if (pDomElement)
	{
		//transform block exists. set plug values
		Xt::DomBlock *pComponentElement = 0;
		Vector3 componentVector;

		pComponentElement = Xt::AsBlock(Xt::At(pDomElement, "scale"));
		XtFieldHelper::GetVector(componentVector, pComponentElement);
		PlugHelpers::SetCompoundValue(scale, componentVector, triggerTopLevelParentOnChanged);

		pComponentElement = Xt::AsBlock(Xt::At(pDomElement, "shear"));
		XtFieldHelper::GetVector(componentVector, pComponentElement);
		PlugHelpers::SetCompoundValue(shear, componentVector, triggerTopLevelParentOnChanged);

		pComponentElement = Xt::AsBlock(Xt::At(pDomElement, "rotate"));
		XtFieldHelper::GetVector(componentVector, pComponentElement);
		PlugHelpers::SetCompoundValue(rotate, componentVector, triggerTopLevelParentOnChanged);

		pComponentElement = Xt::AsBlock(Xt::At(pDomElement, "translate"));
		XtFieldHelper::GetVector(componentVector, pComponentElement);
		PlugHelpers::SetCompoundValue(translate, componentVector, triggerTopLevelParentOnChanged);
	}
	else
	{
		//transform block does not exist
		pDomElement = Xt::AcquireBlock(pFrom, "__transform");

		Xt::DomBlock *pComponentElement = 0;
		Vector3 componentVector;
		pComponentElement = Xt::AcquireBlock(pDomElement, "scale");
		assert(pComponentElement);
		PlugHelpers::GetCompoundValue(scale, componentVector);
		XtFieldHelper::SetVector(pComponentElement, componentVector);

		pComponentElement = Xt::AcquireBlock(pDomElement, "shear");
		assert(pComponentElement);
		PlugHelpers::GetCompoundValue(shear, componentVector);
		XtFieldHelper::SetVector(pComponentElement, componentVector);

		pComponentElement = Xt::AcquireBlock(pDomElement, "rotate");
		assert(pComponentElement);
		PlugHelpers::GetCompoundValue(rotate, componentVector);
		XtFieldHelper::SetVector(pComponentElement, componentVector);

		pComponentElement = Xt::AcquireBlock(pDomElement, "translate");
		assert(pComponentElement);
		PlugHelpers::GetCompoundValue(translate, componentVector);
		XtFieldHelper::SetVector(pComponentElement, componentVector);
	}
}

bool XtDeserialiseTransform(
	C_NullPlug& scale,
	C_NullPlug& shear,
	C_NullPlug& rotate,
	C_NullPlug& translate,
	bool triggerTopLevelParentOnChanged,
	Xt::DomBlock* pFrom)
{
	bool updated = false;

	//Is this a transform block?
	Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pFrom, "__transform"));
	if (pBlock)
	{
		Xt::DomBlock *pComponentElement = 0;
		Vector3 vScale, vShear, vRotate, vTranslate;

		pComponentElement = Xt::AsBlock(Xt::At(pBlock, "scale"));
		XtFieldHelper::GetVector(vScale, pComponentElement);
		PlugHelpers::SetCompoundValue(scale, vScale, triggerTopLevelParentOnChanged);

		pComponentElement = Xt::AsBlock(Xt::At(pBlock, "shear"));
		XtFieldHelper::GetVector(vShear, pComponentElement);
		PlugHelpers::SetCompoundValue(shear, vShear, triggerTopLevelParentOnChanged);

		pComponentElement = Xt::AsBlock(Xt::At(pBlock, "rotate"));
		XtFieldHelper::GetVector(vRotate, pComponentElement);
		PlugHelpers::SetCompoundValue(rotate, vRotate, triggerTopLevelParentOnChanged);

		pComponentElement = Xt::AsBlock(Xt::At(pBlock, "translate"));
		XtFieldHelper::GetVector(vTranslate, pComponentElement);
		PlugHelpers::SetCompoundValue(translate, vTranslate, triggerTopLevelParentOnChanged);

		updated = true;
	}

	return updated;
}

void BuildXtActorPropertyList(
	Btl::Vector<C_XtPropertyDesc>& properties,
	LarvaType* pLarvaType,
	Xt::DomBlock* pRoot,
	bool acquire)
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	BIZ_ASSERT(pLarvaType->kind == kLtStruct);
	LtStruct* pLarvaStruct = (LtStruct*)pLarvaType;
	if (pLarvaStruct->base)
	{
		BuildXtActorPropertyList(properties, pLarvaStruct->base->type, pRoot, acquire);
	}

	C_XtTypeDatabase& dgTypeDatabase = C_Core::Instance().GetXtTypeDatabase();

	//Create properties as specified in the schema.
	for(U32 i = 0; i < pLarvaStruct->fields.count; i++)
	{
		LarvaField* pFieldSchema = pLarvaStruct->fields[i];
		LarvaType* pFieldType = pFieldSchema->Type();

		const char* pTypeName = pFieldType ? pFieldType->name.Text() : 0;
		if (!pTypeName)
			continue;

		I_XtType* pXtFieldType = dgTypeDatabase.FindType(pTypeName);
		if (pXtFieldType && pXtFieldType->IsA(E_XtType_DgXtPropNode))
		{
			// Skip fields with annotation as these are handled manually.
			if (pFieldSchema->attrs.GetString("SceneView", "semantic") != "")
				continue;
		}
		else
		{
			XtString propType = pFieldSchema->attrs.GetString("SceneView", "property");
			XtString semantic = pFieldSchema->attrs.GetString("SceneView", "semantic");

			if (!(*propType) && 
				(C_Core::Instance().GetXtTypeDatabase().GetType((const char*)semantic) != 
				E_XtType_DgXtPropNode))
				continue;
		}		

		Xt::DomElement* pFieldData = acquire ? 
			Xt::Acquire(pRoot, pFieldSchema) : Xt::At(pRoot, pFieldSchema);
		BIZ_ASSERT(pFieldData && "Could not get data block!");

		properties.PushBack();
		properties.Back().pData = pFieldData;
		properties.Back().pLarvaField = pFieldSchema;
	}
}

S32 GetPropertyIndex(const C_DgXtCompNode* pParent, C_DgXtPropNodeBase* pProp)
{
	const ConstDgPropArray& props = pParent->GetProperties();
	for (S32 i = 0; i < props.Size(); ++i)
		if (props[i] == pProp)
			return i;

	return -1;
}

bool IsExternalActor(Xt::DomBlock* pRoot)
{
	Xt::DomField* pExtActorField = Xt::AsField(Xt::At(pRoot, "__extActor"));
	Xt::DomField* pExtActorFieldNew = Xt::AsField(Xt::At(pRoot, "IsExtActor"));
	if (pExtActorFieldNew && pExtActorFieldNew->value != "")
		return pExtActorFieldNew->value == "1";
	else if (pExtActorField && pExtActorField->value != "")
		return pExtActorField->value == "true";

	return false;
}

void SetIsExternalActor(Xt::DomBlock* pRoot, bool isExternalActor)
{
	Xt::DomField* pExtActorField = Xt::AcquireField(pRoot, "__extActor");
	Xt::DomField* pExtActorFieldNew = Xt::AcquireField(pRoot, "IsExtActor");
	Xt::Set(pExtActorField, isExternalActor ? "true" : "false");
	Xt::Set(pExtActorFieldNew, isExternalActor ? "1" : "0");
}

}
}
