#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Dg_Cmd/DuplicateCmd.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeHelpers.h>

#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelHelpers.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

namespace EditorLib
{

C_DuplicateCmd::C_DuplicateCmd(U32 type) : I_Command(type)
{
}

C_DuplicateCmd::~C_DuplicateCmd()
{
}

C_Status C_DuplicateCmd::Do()
{
	//Undo checkpoint.
	C_CheckpointHelper c(&m_pChange, GetName());

	C_Status status;

	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	XtUt::Dictionary<XtId, XtId> remapTable;
	Btl::Vector<C_SceneXtNodeBase*> clonedActorArray;

	bool nonActors = false;
	C_Selection oldSelection, newSelection;
	C_Core::Instance().GetSelection(oldSelection);
	const C_Selection::ItemArray& items = oldSelection.GetItems();


	for (S32 i=0; i<items.Size(); ++i)
	{
		if (C_Core::Instance().GetDgManager().LevelOf(items[i].id) != C_Core::Instance().GetLevelManager()->GetActiveLevel())
		{
			LOGWARN(C_Core::Instance().GetLogger(), "Unable to duplicate nodes - the selection contains nodes not in the active level");
			return E_Status_Failure;
		}
	}

	for (S32 i=0; i<items.Size(); ++i)
	{
		I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(items[i].id);
		C_SceneXtNodeBase* pSceneXtNode = pNode->IsA(E_ObjType_SceneXtNodeBase) ?
			(C_SceneXtNodeBase*)pNode : 0;

		if (pSceneXtNode)
		{
			I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(pNode->GetId());
			BIZ_ASSERT(pLevel);
			BIZ_ASSERT(pLevel->GetType() == E_LevelType_XtLevel);

			I_DgNode* pClone = LevelHelpers::CloneNode((C_XtLevel*)pLevel, pNode);
			if (pClone)
			{
				C_SceneXtNodeBase* pCloneSceneXtNode = (C_SceneXtNodeBase*)pClone;
				BIZ_ASSERT(pCloneSceneXtNode);
				newSelection.Add(C_Selection::E_Mode_Object, pCloneSceneXtNode->GetId());
				clonedActorArray.PushBack(pCloneSceneXtNode);

				XtId* pNewId = new XtId(pCloneSceneXtNode->GetId().GetXtId());
				
				bool insert = XtUt::Insert(&remapTable, pSceneXtNode->GetId().GetXtId(), pNewId);
				BIZ_ASSERT(insert);
			}
		}
		else
			nonActors = true;
	}
	if (nonActors)
		LOGWARN(EditorLib::C_Core::Instance().GetLogger(), "The current selection contains items other than actor nodes which cannot be duplicated individually");

	LevelHelpers::RenameSelection(newSelection);

	//remap weak ptrs
	{
		Btl::Vector<C_SceneXtNodeBase*>::ConstIterator it;
		for (it=clonedActorArray.Begin(); it!=clonedActorArray.End(); ++it)
		{
			::MapWeakPtrs(pSchema, AsStruct((*it)->GetXtType()), Xt::AsComponent((*it)->GetXtData()), &remapTable);

			//manually remap __parent ptrs as these aren't specified in the schema
			Xt::DomRef *pField = Xt::AsRef(Xt::At(&Xt::AsComponent((*it)->GetXtData())->data, "__parent"));
			XtId *pNewId = pField ? XtUt::At(&remapTable, Xt::AsId(pField)) : 0;
			if (pNewId)
				Xt::Set(pField, *pNewId);
		}
	}

	C_Core::Instance().GetXtManager().EnsureUniqueUids(true);

	//Apply selection.
	C_Core::Instance().SetSelection(newSelection);
	return E_Status_Success;
}

bool C_DuplicateCmd::IsUndoable() const
{
	return true;
}

void C_DuplicateCmd::GetResults(ResultsArray& results) const
{
}

I_Command* C_DuplicateCmd::Create(U32 type)
{
	return new C_DuplicateCmd(type);
}

C_Syntax* C_DuplicateCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		{
			C_Argument arg(
				'#', 
				"",
				"Duplicate current selection",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			pSyntax->AddArg(arg);
		}
	}

	return pSyntax;
}

const char* C_DuplicateCmd::GetName()
{
	static char* p = "duplicate";
	return p;
}




}
