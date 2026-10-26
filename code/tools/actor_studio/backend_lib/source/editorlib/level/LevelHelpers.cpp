#include "stdafx.h"

#include <errno.h>

#include <EditorLib/Misc/StringUtil.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgXtGameDataNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>

#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Level/LevelHelpers.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Core.h>

namespace EditorLib
{
namespace LevelHelpers
{

Xt::DomArray* AcquireXtArray(XtId targetLevelId, I_DgNode* pNode)
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomRecord* pLevelRecord = C_Core::Instance().GetXtManager().FindRecord(targetLevelId);

	if (!pLevelRecord)
		return 0;

	if (pNode->IsA(E_ObjType_SceneXtActorNode))
	{
		return Xt::AsArray(
			AcquireBySemantic(
				Xt::TypeOf(pSchema, pLevelRecord), 
				&pLevelRecord->root.data,
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_ActorArray).dom);
	}
	else if (pNode->IsA(E_ObjType_SceneXtNode))
	{
		return Xt::AsArray(
			AcquireBySemantic(
				Xt::TypeOf(pSchema, pLevelRecord), 
				&pLevelRecord->root.data,
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_NodeArray).dom);
	}
	else if (pNode->IsA(E_ObjType_DgXtGameDataNode))
	{
		return Xt::AsArray(
			AcquireBySemantic(
				Xt::TypeOf(pSchema, pLevelRecord), 
				&pLevelRecord->root.data,
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_DataArray).dom);
	}

	return 0;
}

I_DgNode* CreateNodeInPlace(	C_XtLevel* pTargetLevel, 
								E_ObjType nodeType,
								Xt::DomComponent* pDomComponent)
{
	if (!pDomComponent || nodeType == E_ObjType_Unknown)
		return 0;

	C_XtObject::C_Unbinder levelUnbind(pTargetLevel);

	//We'll need a valid xtid too.
	C_Status status = XtNodeHelpers::InitialiseXtId(pDomComponent);
	if (!status) 
		return 0;

	I_DgNode* pNode = C_Core::Instance().GetDgManager().CreateNode(
		pTargetLevel, nodeType);
	C_XtObject* pXtObject = pNode->AsXtObject();
	if (pXtObject)
	{
		if (pNode->IsA(E_ObjType_SceneNode))
			pTargetLevel->GetRootSceneNode()->AddChild((C_SceneNode*)pNode);

		//Attach new Xt object
		LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
		LarvaType* pLarvaType = FindTypeByName(pSchema, pDomComponent->type);

		pXtObject->XtInitialise(pTargetLevel, pLarvaType, pDomComponent);
		pXtObject->XtBind();

		//Flag node as being dirty to trigger initial update.
		pXtObject->GetXtDirty().SetDirty(pXtObject->GetXtData());
	}
	else
	{
		BIZ_ASSERT_FAIL("Failed to create in place!");
		C_Core::Instance().GetDgManager().DestroyNode(pNode, true);
		return 0;
	}

	return pNode;
}

I_DgNode* CreateNode(C_XtLevel* pTargetLevel,
					 const char* typeName,
					 const char* name)
{
	C_XtObject::C_Unbinder unbinder(pTargetLevel);
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	I_XtType* pXtType = C_Core::Instance().GetXtTypeDatabase().FindType(typeName);
	if (!pXtType)
		return 0;

	E_ObjType concreteBaseType = E_ObjType_Unknown;
	if (pXtType->IsA(E_XtType_SceneXtActorNode))
		concreteBaseType = E_ObjType_SceneXtActorNode;
	else if (pXtType->IsA(E_XtType_SceneXtNode))
		concreteBaseType = E_ObjType_SceneXtNode;
	else if (pXtType->IsA(E_XtType_DgXtGameDataNode))
		concreteBaseType = E_ObjType_DgXtGameDataNode;
	else
		return 0;

	Xt::DomComponent* pData = new Xt::DomComponent;
	Xt::SetType(pData, pXtType->GetXtType()->name);
	SetDefaults(pSchema, AsStruct(pXtType->GetXtType()), pData);
	
	I_DgNode* pNode = CreateNodeInPlace(pTargetLevel, concreteBaseType, pData);
	if (!pNode)
		return 0;

	//Set name.
	if (name)
		pNode->SetName(name);
	else
		pNode->SetName(Btl::String("New")+typeName);

	//Append to containing array.
	Xt::DomArray* pArray = AcquireXtArray(pTargetLevel->GetXtRecordId(), pNode);
	Xt::Append(pArray, pData);

	return pNode;
}


I_DgNode* ChangeXtType(C_XtLevel* pTargetLevel, I_DgNode* pNode)
{
	if (!pNode || !pNode->IsA(E_ObjType_SceneXtNodeBase))
		return 0;

	C_XtTypeDatabase& dgActorDatabase = C_Core::Instance().GetXtTypeDatabase();

	C_SceneXtNodeBase* pActor = (C_SceneXtNodeBase*)pNode;
	Xt::DomComponent* pDomComponent = Xt::AsComponent(pActor->GetXtData());

	const XtString& newType = pDomComponent->type;
	const XtString& oldType = XtString(pActor->GetXtTypeName());

	I_XtType* pNewXtType = dgActorDatabase.FindType((const char*)newType);
	if (!pNewXtType)
		//Todo - handle unknown actor type.
		return 0;

	if (newType == oldType)
		//No need to change type.
		return 0;

	//Unbind to ensure that the node xt data isn't deleted.
	pActor->XtUnbind();
	//Remove existing node.
	DestroyNode(pNode);

	E_ObjType newNodeType = E_ObjType_Unknown;
	if (pNewXtType->IsA(E_XtType_SceneXtActorNode))
		newNodeType = E_ObjType_SceneXtActorNode;
	else if (pNewXtType->IsA(E_XtType_SceneXtNode))
		newNodeType = E_ObjType_SceneXtNode;	
	else if (pNewXtType->IsA(E_XtType_DgXtGameDataNode))
		newNodeType = E_ObjType_DgXtGameDataNode;	

	I_DgNode* pNewNode = CreateNodeInPlace(pTargetLevel, newNodeType, pDomComponent);
	if (pNewNode)
	{
		//Sync orhpaned children. The reason that we have to do this is that when redoing the creation of a
		//parent, a null placeholder node is created first. This is then later replaced with the correct node
		//via this type change function.
		//Unfortunately at this point or poor children are orhphaned (although have the correct xt parent data).
		//So, at this point we need to go hunting for our children.
		for (C_DgNodeIterator dgNodeIt = C_Core::Instance().GetDgManager().GetDgIterator(); 
			(I_DgNode*)dgNodeIt; ++dgNodeIt)
		{
			I_DgNode* pPotentialChild = (I_DgNode*)dgNodeIt;
			if (!pPotentialChild || !pPotentialChild->IsA(E_ObjType_SceneXtNodeBase)) 
				continue;
			
			C_SceneXtNodeBase* pPotentialXtChild = (C_SceneXtNodeBase*)pPotentialChild;
			Xt::DomComponent* pComponent = Xt::AsComponent(pPotentialXtChild->GetXtData());
			Xt::DomRef* pParentRef = Xt::AsRef(Xt::At(&pComponent->data, "__parent"));
			if (pParentRef)
			{
				XtId xtParentId = pParentRef->value;
				if (xtParentId == pNewNode->GetId().GetXtId())
					C_SceneNodeHelpers::ReparentSceneNode((C_SceneNode*)pPotentialChild, (C_SceneNode*)pNewNode, false);
			}
		}

	}
	return pNewNode;
}

I_DgNode* CloneNode(C_XtLevel* pTargetLevel, I_DgNode* pSrcNode)
{
	I_Level* pSrcNodeLevel = C_Core::Instance().GetDgManager().LevelOf(pSrcNode->GetId());
	BIZ_ASSERT(pSrcNodeLevel->GetType() == E_LevelType_XtLevel);

	C_XtLevel* pXtSrcNodeLevel = (C_XtLevel*)pSrcNodeLevel;

	C_XtObject::C_Unbinder(pXtSrcNodeLevel->AsXtObject());
	if (!pTargetLevel)
		pTargetLevel = pXtSrcNodeLevel;

	if (!pSrcNode || !pSrcNode->IsA(E_ObjType_SceneXtNodeBase))
		return 0;
	C_SceneXtNodeBase* pSrcSceneNode = (C_SceneXtNodeBase*)pSrcNode;

	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomComponent* pClonedData = (Xt::DomComponent*)Duplicate(
		pSchema,
		AsStruct(pSrcSceneNode->GetXtType()),
		Xt::AsComponent(pSrcSceneNode->GetXtData()));

	//Remove external properties.
	{
		if (XtNodeHelpers::IsExternalActor(&pClonedData->data))
			XtNodeHelpers::SetIsExternalActor(&pClonedData->data, false);
	}

	//set name of cloned actor
	Xt::Set(&pClonedData->data, kXtSemanticLabel_Name, pSrcNode->GetName().Chars());
	
	Xt::DomArray* pArray = AcquireXtArray(pTargetLevel->GetXtRecordId(), pSrcNode);
	S32 idx = (S32)Xt::Count(pArray);
	Xt::Append(pArray, pClonedData);

	I_DgNode* pClone = 0;
	//Find newly created node.
	for (C_DgNodeIterator dgNodeIt = C_Core::Instance().GetDgManager().GetDgIterator(); 
		(I_DgNode*)dgNodeIt; ++dgNodeIt)
	{
		I_DgNode* pTmp = (I_DgNode*)dgNodeIt;
		if (!pTmp) 
			continue;

		C_XtObject* pXtObject = pTmp->AsXtObject();
		if (pXtObject && pXtObject->GetXtData() == pClonedData)
		{
			pClone = pTmp;
			break;
		}
	}

	return pClone;
}

bool DestroyNode(
	I_DgNode* pNode, 
	E_DeleteSceneNode childBehaviour, 
	bool deleteOverride)
{
	if (!pNode) 
		return false;

	//Handle parented scene nodes.
	if (pNode->IsA(E_ObjType_SceneNode) && ((C_SceneNode*)pNode)->GetParent())
	{
		C_SceneNode* pSceneNode  = ((C_SceneNode*)pNode);
		//If we can't reparent our children, get rid of them!
		if (!pSceneNode->GetParent())
			childBehaviour = E_DeleteSceneNode_DeleteChildren;

		switch(childBehaviour)
		{
			case E_DeleteSceneNode_ReparentChildrenAbsolute:
				return C_SceneNodeHelpers::DeleteSceneNodeAndReparentChildren(
					pSceneNode, true, deleteOverride);
			case E_DeleteSceneNode_ReparentChildren:
				return C_SceneNodeHelpers::DeleteSceneNodeAndReparentChildren(
					pSceneNode, false, deleteOverride);
			default:
				return C_SceneNodeHelpers::DeleteSceneNodeAndChildren(
					pSceneNode, deleteOverride);
		};
	}
	else
		return C_Core::Instance().GetDgManager().DestroyNode(pNode, deleteOverride);
}

void RenameSelection(const C_Selection& selection)
{
	int highestNumber = 0;

	// Loop through and find the highest numbered item
	for (C_Selection::ItemArray::ConstIterator it=selection.GetItems().Begin(); it<selection.GetItems().End(); ++it)
	{
		I_DgNode* node = C_Core::Instance().GetDgManager().FindNodeById(it->id);
		Btl::String name = node->GetName();

		int revSearch = name.RevSearch('_', 0);
		if (revSearch >= 0)
		{
			Btl::String number = name(revSearch+1, name.Length());
			int numberInt = strtol(number.Chars(), 0, 10);
			if ((numberInt != 0 || errno != EINVAL) && numberInt > highestNumber)
			{
				highestNumber = numberInt;
			}
		}
	}

	// Rename all objects in the selection, numbering them from the highest number upwards
	for (C_Selection::ItemArray::ConstIterator it=selection.GetItems().Begin(); it<selection.GetItems().End(); ++it)
	{
		I_DgNode* node = C_Core::Instance().GetDgManager().FindNodeById(it->id);
		Btl::String name = node->GetName();

		int revSearch = name.RevSearch('_', 0);
		if (revSearch >= 0)
		{
			Btl::String number = name(revSearch+1, name.Length());
			int numberInt = strtol(number.Chars(), 0, 10);
			if (numberInt != 0 || number == "0")
			{
				name = name(0, revSearch);
				name = Btl::String::Format("%s_%d", name.Chars(), ++highestNumber);
			} else {
				name = Btl::String::Format("%s_%d", name.Chars(), ++highestNumber);
			}
		} else {
			name = Btl::String::Format("%s_%d", name.Chars(), ++highestNumber);
		}

		node->SetName(name);
	}
}

Btl::String GetNextFreeName(const Btl::String& name)
{
	Btl::Vector<const I_DgNode*> nodes;
	C_Core::Instance().GetLevelManager()->GetActiveLevel()->BuildActorNodeArray(nodes);

	bool isNameFree = false;
	
	Btl::String newName = name;
	while (true)
	{
		Btl::Vector<const I_DgNode*>::Iterator it;
		for (it = nodes.Begin(); it != nodes.End(); ++it)
			if (newName == (*it)->GetName())
				break;
		
		if (it == nodes.End())
			return newName;

		int revSearch = newName.RevSearch('_', 0);
		if (revSearch >= 0)
		{
			Btl::String number = newName(revSearch+1, newName.Length());
			int numberInt = strtol(number.Chars(), 0, 10);
			if (numberInt != 0 || number == "0")
			{
				newName = newName(0, revSearch);
				newName = Btl::String::Format("%s_%d", newName.Chars(), numberInt+1);
			} else {
				newName = Btl::String::Format("%s_%d", newName.Chars(), 1);
			}
		} else {
			newName = Btl::String::Format("%s_%d", newName.Chars(), 1);
		}
	}
}

void OnChangeXtType(C_XtLevel* pTargetLevel, C_NodeId nodeId)
{
	BIZ_ASSERT(pTargetLevel);

	//Find the node whoes type is changing.
	I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(nodeId);
	if (!pNode)	return;

	//If the type change was successful then we will now be attached to a new node.
	LevelHelpers::ChangeXtType(pTargetLevel, pNode);
}


//Export content to another level.

XtId ExportNode(XtId targetLevelId, I_DgNode* pSourceNode)
{
	I_Level* pTargetLevel = C_Core::Instance().GetDgManager().LevelOf(pSourceNode->GetId());
	BIZ_ASSERT(pTargetLevel);
	BIZ_ASSERT(pTargetLevel->GetType() == E_LevelType_XtLevel);

	C_XtObject::C_Unbinder((C_XtLevel*)pTargetLevel);

	if (!pSourceNode)
		return 0;

	C_XtObject* pXtObject = pSourceNode->AsXtObject();
	if (!pXtObject)
		return 0;

	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomComponent* pData = (Xt::DomComponent*)Duplicate(
		pSchema,
		AsStruct(pXtObject->GetXtType()),
		Xt::AsComponent(pXtObject->GetXtData()));

	//Remove external properties.
	XtNodeHelpers::SetIsExternalActor(&pData->data, false);

	//set name of cloned actor
	Xt::Set(&pData->data, kXtSemanticLabel_Name, pSourceNode->GetName().Chars());

	Xt::DomArray* pArray = AcquireXtArray(targetLevelId, pSourceNode);
	S32 idx = (S32)Xt::Count(pArray);
	Xt::Append(pArray, pData);
	return pData->id;
}

void ExportNodeLayers(XtId targetLevelId, XtId targetNodeId, I_DgNode* pSourceNode)
{
	I_Level* pTargetLevel = C_Core::Instance().GetDgManager().LevelOf(pSourceNode->GetId());

	I_LayerSet* pLayerSet = C_Core::Instance().GetLayerManager().GetLayerSet(pTargetLevel);

	if (!pLayerSet)
		return;

	for (LayerList::ConstIterator layer = pLayerSet->GetLayerIterator(); layer.Pointer(); ++layer)
	{
		for(EditorLib::I_Layer::DgNodeSet::ConstIterator layerNodes = (*layer)->GetNodes().Begin(); layerNodes != (*layer)->GetNodes().End(); ++layerNodes)
		{
			if (pSourceNode->GetId() == layerNodes->GetValue()->GetId())
			{
				// Find layer
				Xt::DomComponent* pLayer = 0;

				LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
				Xt::DomRecord* pLevelRecord = C_Core::Instance().GetXtManager().FindRecord(targetLevelId);

				Xt::FieldDom fd = AcquireBySemantic(
						Xt::TypeOf(pSchema, pLevelRecord), 
						&pLevelRecord->root.data,
						kXtSemanticAttr,
						kXtSemanticParam,
						kXtSemanticLabel_Layers);
				Xt::DomArray* pLayers = Xt::AsArray(fd.dom);
				assert(pLayers);

				assert(fd.field->kind == kLfArray);
				LfArray* fieldArr = (LfArray*)fd.field;

				for(Xt::DomArray::IndexIterator it(pLayers); it; ++it)
				{
					Xt::DomComponent* pComponent = Xt::AsComponent(it);
					if (!pComponent)
						continue;

					Xt::DomBlock* pBlockData = &pComponent->data;

					Xt::DomField *pNameField = Xt::AsField(Xt::At(pBlockData, "Name"));
					if (!pNameField)
						continue;

					if ((*layer)->GetName() == pNameField->value.Text() && pComponent->type == "ActorLayer")
						pLayer = pComponent;
				}
				
				// If a layer with the same name doesn't exist, create one
				if (!pLayer)
				{
					// Add new layer
					pLayer = new Xt::DomComponent();
					if (!GenerateIdForComponent(pLayer, true))
					{
						delete pLayer;
						return;
					}
					
					Xt::SetType(pLayer, "ActorLayer");
					
					Xt::DomField *pNameField = Xt::AcquireField(&pLayer->data, "Name");
					assert(pNameField);
					
					pNameField->value = (*layer)->GetName().Chars();

					Xt::Append(pLayers, pLayer);
				}

				Xt::DomArray *pLayerActors = Xt::AcquireArray(&pLayer->data, "ActorArray");
				assert(pLayerActors);

				// Check for duplicates
				for (Xt::DomArray::IndexIterator it(pLayerActors); it; ++it)
				{
					Xt::DomRef* pRef = Xt::AsRef(it);
					if (!pRef)
						continue;

					if (pRef->value == targetNodeId)
						return;
				}

				// Append new actor to layer
				Xt::DomRef* pActor = new Xt::DomRef();
				pActor->value = targetNodeId;
				Xt::Append(pLayerActors, pActor);
				
				return;
			}
		}
	}
}

}
}
