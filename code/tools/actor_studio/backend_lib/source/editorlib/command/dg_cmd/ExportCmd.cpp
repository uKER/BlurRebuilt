#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtLarvaHelper.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Dg_Cmd/ExportCmd.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneXtNode.h>
#include <EditorLib/Dg/DgXtGameDataNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeHelpers.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Level/LevelHelpers.h>

namespace EditorLib
{

C_ExportCmd::C_ExportCmd(U32 type) : I_Command(type)
{
}

C_ExportCmd::~C_ExportCmd()
{
}

C_Status C_ExportCmd::Do()
{
	//Undo checkpoint.
	C_CheckpointHelper c(&m_pChange, GetName());
	C_Status status;

	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	//Get target level
	const ArgumentDataArray& data = *m_args.GetArgData('l');
	Btl::String targetLevelStr;
	data[0].GetValue(targetLevelStr);
	XtId targetLevelId = Xt::ParseId(targetLevelStr.Chars());
	Xt::DomRecord* pTargetLevelRecord = C_Core::Instance().GetXtManager().FindRecord(targetLevelId);

	// Source/target XtIds housekeeping
	XtUt::Dictionary<XtId, XtId> remapTable;
	Ut::UtVector< Ut::DynamicVector<XtId> > exportedArray;

	//Acquire our storage arrays.
	Xt::DomArray* pTargetLevelActorArray = Xt::AsArray(
		AcquireBySemantic(
			Xt::TypeOf(pSchema, pTargetLevelRecord), 
			&pTargetLevelRecord->root.data,
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_ActorArray).dom);

	Xt::DomArray* pTargetLevelNodeArray = Xt::AsArray(
		AcquireBySemantic(
			Xt::TypeOf(pSchema, pTargetLevelRecord), 
			&pTargetLevelRecord->root.data,
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_NodeArray).dom);

	Xt::DomArray* pTargetLevelDataArray = Xt::AsArray(
		AcquireBySemantic(
			Xt::TypeOf(pSchema, pTargetLevelRecord), 
			&pTargetLevelRecord->root.data,
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_DataArray).dom);

	//Get current selection
	C_Selection selection;
	C_Core::Instance().GetSelection(selection);
	ExpandSelection(selection);
	const C_Selection::ItemArray& items = selection.GetItems();

	for (S32 i=0; i<items.Size(); ++i)
	{
		I_DgNode* pSourceNode = C_Core::Instance().GetDgManager().FindNodeById(items[i].id);
		C_SceneXtNodeBase* pSourceXtNode = pSourceNode->IsA(E_ObjType_SceneXtNodeBase) ? 
			(C_SceneXtNodeBase*)pSourceNode : 0;

		if (pSourceXtNode)
		{
			XtId targetActorId = 0;
			
			targetActorId = LevelHelpers::ExportNode(targetLevelId, pSourceNode);

			if (targetActorId)
			{
				exportedArray.PushBack(targetActorId);
				XtId* pNewId = new XtId(targetActorId);

				bool insert = XtUt::Insert(&remapTable, pSourceXtNode->GetId().GetXtId(), pNewId);
				assert(insert);

				// Export the layers for the current node
				LevelHelpers::ExportNodeLayers(targetLevelId, targetActorId, pSourceNode);
			}
		} else {
			if (!pSourceNode->IsA(E_ObjType_DgXtGameDataNode))
				continue;

			XtId targetActorId = LevelHelpers::ExportNode(targetLevelId, pSourceNode);
			
			if (!targetActorId)
				continue;

			C_DgXtGameDataNode* pDataNode = (C_DgXtGameDataNode*)pSourceNode;

			exportedArray.PushBack(targetActorId);

			XtId* pNewId = new XtId(targetActorId);

			bool insert = XtUt::Insert(&remapTable, pDataNode->GetId().GetXtId(), pNewId);
			assert(insert);
		}
	}

	if (exportedArray.Size() == 0)
		LOGWARN(C_Core::Instance().GetLogger(), "The current selection contains items other than actor and data nodes which cannot be exported individually");

	//remap weak ptrs
	Ut::UtVector< Ut::DynamicVector<XtId> >::ConstIterator it;
	for (it=exportedArray.Begin(); it!=exportedArray.End(); ++it)
	{
		Xt::DomComponent* pTargetActor = 0;
		//Look for actor
		for(Xt::DomArray::ElementIterator elIt(pTargetLevelActorArray); elIt; ++elIt)
		{
			Xt::DomComponent* pTempComp = Xt::AsComponent(elIt);
			if(pTempComp->id == *it)
			{
				pTargetActor = pTempComp;
				break;
			}
		}
		//Look for ActorStudio node.
		if(!pTargetActor)
		{
			for(Xt::DomArray::ElementIterator elIt(pTargetLevelNodeArray); elIt; ++elIt)
			{
				Xt::DomComponent* pTempComp = Xt::AsComponent(elIt);
				if(pTempComp->id == *it)
				{
					pTargetActor = pTempComp;
					break;
				}
			}
		}
		// Look for data
		if (!pTargetActor)
		{
			for(Xt::DomArray::ElementIterator elIt(pTargetLevelDataArray); elIt; ++elIt)
			{
				Xt::DomComponent* pTempComp = Xt::AsComponent(elIt);
				if(pTempComp->id == *it)
				{
					pTargetActor = pTempComp;
					break;
				}
			}
		}
		assert(pTargetActor);		
		LtStruct* pTargetActorType = XtLarvaHelpers::FindLarvaType(pSchema, pTargetActor->type.Text(), E_XtSchemaType_Other);
		::MapWeakPtrs(pSchema, pTargetActorType, pTargetActor, &remapTable);
		  
		//manually remap __parent ptrs as these aren't specified in the schema
		Xt::DomRef *pField = Xt::AsRef(Xt::At(&pTargetActor->data, "__parent"));
		XtId *pNewId = pField ? XtUt::At(&remapTable, Xt::AsId(pField)) : 0;
		if (pNewId)
			Xt::Set(pField, *pNewId);
	}

	C_Core::Instance().GetXtManager().EnsureUniqueUids(false);
	LOGINFO(C_Core::Instance().GetLogger(), "Export Successful.");
	return E_Status_Success;
}

bool C_ExportCmd::IsUndoable() const
{
	return false;
}

void C_ExportCmd::GetResults(ResultsArray& results) const
{
}

I_Command* C_ExportCmd::Create(U32 type)
{
	return new C_ExportCmd(type);
}

C_Syntax* C_ExportCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		/*
		{
			C_Argument arg(
				'#', 
				"",
				"Export current selection",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			pSyntax->AddArg(arg);
		}  */
		{
			C_Argument arg(
				'l', 
				"level",
				"Target Level",
				C_Argument::E_Required_True);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
	}

	return pSyntax;
}

const char* C_ExportCmd::GetName()
{
	static char* p = "export";
	return p;
}

void C_ExportCmd::ExpandSelection(C_Selection& selection)
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	const C_Selection::ItemArray& items = selection.GetItems();
	for (S32 i=0; i<items.Size(); ++i)
	{
		I_DgNode* pSourceNode = C_Core::Instance().GetDgManager().FindNodeById(items[i].id);
		if (!pSourceNode->IsA(EditorLib::E_ObjType_SceneXtNode))
			continue;
	
		I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(pSourceNode->GetId());
		BIZ_ASSERT(pLevel);
		BIZ_ASSERT(pLevel->GetType() == E_LevelType_XtLevel);
		
		XtId sourceLevelId = ((C_XtLevel*)pLevel)->GetXtRecordId();
		Xt::DomRecord* pSourceLevelRecord = C_Core::Instance().GetXtManager().FindRecord(sourceLevelId);

		Xt::DomArray* pSourceLevelActorArray = Xt::AsArray(
			FindBySemantic(
				Xt::TypeOf(pSchema, pSourceLevelRecord), 
				&pSourceLevelRecord->root.data,
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_ActorArray).dom);
		if (!pSourceLevelActorArray) return;

		Xt::DomArray* pSourceLevelNodeArray = Xt::AsArray(
			FindBySemantic(
				Xt::TypeOf(pSchema, pSourceLevelRecord), 
				&pSourceLevelRecord->root.data,
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_NodeArray).dom);
		if (!pSourceLevelNodeArray) return;
	
		AddChildren(selection, pSourceNode, pSourceLevelActorArray, pSourceLevelNodeArray);	
	}
}

void C_ExportCmd::AddChildren(
	C_Selection& selection, 
	I_DgNode* pNode,
	Xt::DomArray* pActorsArray, 
	Xt::DomArray* pSceneNodeArray)
{
	if(!pNode || !pNode->IsA(E_ObjType_SceneXtNode) || !pActorsArray)
		return;

	C_SceneXtNode* pSceneXtNode = (C_SceneXtNode*)pNode;

	//Search for member actors
	for(Xt::DomArray::ElementIterator elIt(pActorsArray); elIt; ++elIt)
	{
		Xt::DomComponent* pMemberXtComp = Xt::AsComponent(elIt);
		Xt::DomRef* pMemberParentRef = Xt::AsRef(Xt::At(&pMemberXtComp->data, "__parent"));

		if(pMemberParentRef && pMemberParentRef->value == pSceneXtNode->GetId().GetXtId())
		{
			C_NodeId memberId(pMemberXtComp->id, kNoChildId);
			selection.Add(C_Selection::E_Mode_Object, memberId);
		}
	}

	for(Xt::DomArray::ElementIterator elIt(pSceneNodeArray); elIt; ++elIt)
	{
		Xt::DomComponent* pMemberXtComp = Xt::AsComponent(elIt);
		Xt::DomRef* pMemberParentRef = Xt::AsRef(Xt::At(&pMemberXtComp->data, "__parent"));

		if(pMemberParentRef && pMemberParentRef->value == pSceneXtNode->GetId().GetXtId())
		{
			C_NodeId memberId(pMemberXtComp->id, kNoChildId);
			selection.Add(C_Selection::E_Mode_Object, memberId);
			//Actors can't have children, but we can...
			I_DgNode* pMemberNode = C_Core::Instance().GetDgManager().FindNodeById(memberId);
			AddChildren(selection, (C_SceneXtNode*)pMemberNode, pActorsArray, pSceneNodeArray);	
		}
	}
}

}
