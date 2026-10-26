#include "stdafx.h"
#include <Misc_Shared/Variant/KeyValue.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Level/LevelHelpers.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <EditorLib/Core.h>

#include <UserCmd/Nightwing/CreateLinkedCoverCmd.h>

namespace EditorUserCmd
{

C_CreateLinkedCoverCmd::C_CreateLinkedCoverCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_CreateLinkedCoverCmd::~C_CreateLinkedCoverCmd()
{
}

EditorLib::C_Status C_CreateLinkedCoverCmd::Do()
{
	//Undo checkpoint.
	EditorLib::C_CheckpointHelper c(&m_pChange, GetName());

	EditorLib::C_Status status;

	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	XtId recordId = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId();
	Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(recordId);
	LarvaSchema* pSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomArray* pLevelActorArray = Xt::AsArray(
		AcquireBySemantic(
			Xt::TypeOf(pSchema, pLevelRecord), 
			&pLevelRecord->root.data, 
			EditorLib::kXtSemanticAttr,
			EditorLib::kXtSemanticParam,
			EditorLib::kXtSemanticLabel_ActorArray).dom);

	if (selection.GetItems().Size() == 0)
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "No object selected.");
		return EditorLib::E_Status_Failure;
	}
	if (selection.GetItems().Size() > 1)
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Please ensure only one object is selected.");
		return EditorLib::E_Status_Failure;
	}

	EditorLib::C_NodeId actorId = selection.GetItems().Begin()->id;
	selection.Clear();

	EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(actorId);

	if (pNode->IsReadOnly())
		return EditorLib::E_Status_Failure;

	EditorLib::C_SceneXtActorNode* pActor = pNode->IsA(EditorLib::E_ObjType_SceneXtActorNode) ? 
		(EditorLib::C_SceneXtActorNode*)pNode : 0;

	//Is this an actor?
	if (!pActor)
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Incorrect node type selected. Please select a \"CoverActor\".");
		return EditorLib::E_Status_Failure;
	}

	//Is this actor of the correct type?
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(pActor->GetXtData());
		if (!pComponent) return EditorLib::E_Status_Failure;

		if (strcmp(pComponent->type, "CoverActor"))
		{
			LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Incorrect actor type selected. Please select a \"CoverActor\".");
			return EditorLib::E_Status_Failure;
		}
	}

	//Clone selected actor
	{
		EditorLib::I_Level* pLevel = EditorLib::C_Core::Instance().GetDgManager().LevelOf(pNode->GetId());
		BIZ_ASSERT(pLevel);
		BIZ_ASSERT(pLevel->GetType() == EditorLib::E_LevelType_XtLevel);

		EditorLib::C_XtObject::C_Unbinder((EditorLib::C_XtLevel*)pLevel);

		EditorLib::C_SceneXtActorNode* pOriginalActor = pNode->IsA(EditorLib::E_ObjType_SceneXtActorNode) ? 
			(EditorLib::C_SceneXtActorNode*)pNode : 0;
		if (!pOriginalActor) return EditorLib::E_Status_Failure;

		EditorLib::I_DgNode* pClone = EditorLib::LevelHelpers::CloneNode((EditorLib::C_XtLevel*)pLevel, pNode);
		if (pClone)
		{
			EditorLib::C_SceneXtActorNode* pNewActor = pNode->IsA(EditorLib::E_ObjType_SceneXtActorNode) ? 
				(EditorLib::C_SceneXtActorNode*)pClone : 0;
			BIZ_ASSERT(pNewActor);

			//Set link
			Xt::DomComponent *pXtData = Xt::AsComponent(pActor->GetXtData());
			if (pXtData)
			{
				Xt::DomRef* pLinkField = Xt::AcquireRef(
					&pXtData->data, "LinkedCover");
				if (!pLinkField) return EditorLib::E_Status_Failure;

				Xt::Set(pLinkField, pNewActor->GetId().GetXtId());

				selection.Add(EditorLib::C_Selection::E_Mode_Object, pClone->GetId());
			}
		}
		else
		{
			LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Failed to clone cover actor.");
			return EditorLib::E_Status_Failure;
		}
	}

//	EditorLib::C_Core::Instance().GetXtManager().EnsureUniqueUids(false);

	//Apply selection.
	EditorLib::C_Core::Instance().SetSelection(selection);
	return EditorLib::E_Status_Success;
}

bool C_CreateLinkedCoverCmd::IsUndoable() const
{
	return true;
}

void C_CreateLinkedCoverCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_CreateLinkedCoverCmd::Create(U32 type)
{
	return new C_CreateLinkedCoverCmd(type);
}

EditorLib::C_Syntax* C_CreateLinkedCoverCmd::CreateSyntax()
{
	EditorLib::C_Syntax* pSyntax = new EditorLib::C_Syntax();
	return pSyntax;
}

const char* C_CreateLinkedCoverCmd::GetName()
{
	static char* p = "createlinkedcover";
	return p;
}

}
