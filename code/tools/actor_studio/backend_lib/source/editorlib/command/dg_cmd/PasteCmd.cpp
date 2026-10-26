#include "stdafx.h"

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/Dg_Cmd/PasteCmd.h>

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgNodeHelpers.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Level/LevelHelpers.h>
#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/Clipboard.h>
#include <EditorLib/Misc/StringUtil.h>
#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

C_PasteCmd::C_PasteCmd(U32 type) : I_Command(type)
{
}

C_PasteCmd::~C_PasteCmd()
{
}

C_Status C_PasteCmd::Do()
{
	//Undo checkpoint.
	C_CheckpointHelper c(&m_pChange, GetName());

	C_Status status;

	C_Clipboard clipboard;
	C_Core::Instance().GetClipboard(clipboard);
	const C_Clipboard::ItemArray& clipboardItems = clipboard.GetItems();
	if (clipboardItems.Size() == 0)
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Nothing to paste!");
		return E_Status_Success;
	}

	C_Selection selection;

	XtId recordId = C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId();
	Xt::DomRecord* pLevelRecord = C_Core::Instance().GetXtManager().FindRecord(recordId);
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomArray* pLevelActorArray = Xt::AsArray(
		AcquireBySemantic(Xt::TypeOf(pSchema, pLevelRecord), 
		&pLevelRecord->root.data, 
		kXtSemanticAttr,
		kXtSemanticParam,
		kXtSemanticLabel_ActorArray).dom);

	XtUt::Dictionary<XtId, XtId> remapTable;
	Btl::Vector<C_SceneXtNodeBase*> clonedActorArray;

	bool nonActors = false;
	for (C_Clipboard::ItemArray::ConstIterator it=clipboardItems.Begin(); it!=clipboardItems.End(); ++it)
	{
		I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(it->id);
		if (pNode && pNode->IsA(E_ObjType_SceneXtNodeBase))
		{
			C_SceneXtNodeBase* pActor = (C_SceneXtNodeBase*)pNode;
			I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(it->id);
			BIZ_ASSERT(pLevel);
			BIZ_ASSERT(pLevel->GetType() == E_LevelType_XtLevel);

			I_DgNode* pClone = LevelHelpers::CloneNode((C_XtLevel*)pLevel, pNode);
			if (pClone)
			{
				C_SceneXtNodeBase* pClonedActor = (C_SceneXtNodeBase*)pClone;
				selection.Add(C_Selection::E_Mode_Object, pClone->GetId());
				clonedActorArray.PushBack(pClonedActor);

				XtId* pNewId = new XtId(pClonedActor->GetId().GetXtId());
				
				bool insert = XtUt::Insert(&remapTable, pActor->GetId().GetXtId(), pNewId);
				BIZ_ASSERT(insert);
			}
		}
		else
			nonActors = true;
	}
	if (nonActors)
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Copy buffer contains items other than actor nodes, for example properties such as boxes and spheres. These cannot be copy&pasted individually, please select their parent actor.");

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
	C_Core::Instance().SetSelection(selection);
	return E_Status_Success;
}

bool C_PasteCmd::IsUndoable() const
{
	return true;
}

void C_PasteCmd::GetResults(ResultsArray& results) const
{
}

I_Command* C_PasteCmd::Create(U32 type)
{
	return new C_PasteCmd(type);
}

C_Syntax* C_PasteCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		{
			C_Argument arg(
				'#', 
				"",
				"Paste clipboard contents",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			pSyntax->AddArg(arg);
		}
	}

	return pSyntax;
}

const char* C_PasteCmd::GetName()
{
	static char* p = "paste";
	return p;
}




}
