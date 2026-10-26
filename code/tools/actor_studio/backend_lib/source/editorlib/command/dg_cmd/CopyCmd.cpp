#include "stdafx.h"

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/Dg_Cmd/CopyCmd.h>
#include <EditorLib/Command/CommandProcessor.h>

#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/SceneNode.h>

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/Clipboard.h>
#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

C_CopyCmd::C_CopyCmd(U32 type) : I_Command(type)
{
}

C_CopyCmd::~C_CopyCmd()
{
}

C_Status C_CopyCmd::Do()
{
	//Undo checkpoint.
	C_CheckpointHelper c(&m_pChange, GetName());

	C_Status status;

	NodeIdArray ids;
	if (m_args.IsSet('#'))
	{
		const ArgumentDataArray& data = *m_args.GetArgData('#');
		Btl::String nodes;
		data[0].GetValue(nodes);
		DgNodeHelpers::BuildNodeList(ids, nodes, m_args.IsSet('n'), false);

		if (ids.Size() == 0)
			return E_Status_Failure;
	}
	else if (ids.Size() == 0)
	{
		//no explicit copying of node...copy current selection instead
		C_Selection selection;
		C_Core::Instance().GetSelection(selection);

		//ensure we're in object selection mode
		if (selection.GetMode() == C_Selection::E_Mode_Object)
		{
			const C_Selection::ItemArray& items = selection.GetItems();
			for (C_Selection::ItemArray::ConstIterator it = items.Begin(); it != items.End(); ++it)
				ids.PushBack(it->id);
		}
	}

	if (m_args.IsSet('c'))
	{
		//Clear selection - do nothing.
	}


	C_Clipboard clipboard;
	///// ACTIONS //////
	// Add to existing clipboard.
	if (m_args.IsSet('a'))
	{
		//The line below means that in obj selection we can have multiple objects in the selection list.
		//However, for component selection we will only have one object at a time.
		C_Core::Instance().GetClipboard(clipboard);

		bool idsOkay = true;

		for (S32 i = 0; i < ids.Size(); i++)
		{
			if (C_Core::Instance().GetDgManager().LevelOf(ids[0]) == C_Core::Instance().GetLevelManager()->GetActiveLevel())
				clipboard.Add(ids[i]);
			else
				idsOkay = false;
		}
		
		if (!idsOkay)
			LOGWARN(C_Core::Instance().GetLogger(), "Some nodes in the selection are not in the active level, and have not been copied to the clipboard");
	}
	// Remove from existing clipboard.
	else if (m_args.IsSet('r'))
	{
		C_Core::Instance().GetClipboard(clipboard);
		for (S32 i = 0; i < ids.Size(); i++)
			clipboard.Remove(ids[i]);
	}
	// Copy. 
	else
	{
		bool idsOkay = true;

		//If we've no ids in our array then we'll end up clearing the clipboard
		for (S32 i = 0; i < ids.Size(); i++)
		{
			if (C_Core::Instance().GetDgManager().LevelOf(ids[0]) == C_Core::Instance().GetLevelManager()->GetActiveLevel())
				clipboard.Add(ids[i]);
			else
				idsOkay = false;
		}

		if (!idsOkay)
			LOGWARN(C_Core::Instance().GetLogger(), "Some nodes in the selection are not in the active level, and have not been copied to the clipboard");
	}

	//Apply selection.
	C_Core::Instance().SetClipboard(clipboard);

	return E_Status_Success;
}

bool C_CopyCmd::IsUndoable() const
{
	return false;
}

void C_CopyCmd::GetResults(ResultsArray& results) const
{
}

I_Command* C_CopyCmd::Create(U32 type)
{
	return new C_CopyCmd(type);
}

C_Syntax* C_CopyCmd::CreateSyntax()
{
	C_Syntax *pSyntax = new C_Syntax();
	{
		{
			C_Argument arg(
				'#',
				"",
				"copy selection",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			pSyntax->AddArg(arg);
		}
	}
	return pSyntax;
}

const char* C_CopyCmd::GetName()
{
	static char* p = "copy";
	return p;
}

}