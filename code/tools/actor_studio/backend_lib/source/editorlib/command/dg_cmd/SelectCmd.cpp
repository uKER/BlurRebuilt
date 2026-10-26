#include "stdafx.h"
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Dg_Cmd/SelectCmd.h>

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeHelpers.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

C_SelectCmd::C_SelectCmd(U32 type) : I_Command(type)
{
}

C_SelectCmd::~C_SelectCmd()
{
}

C_Status C_SelectCmd::Do()
{
	C_Status status;

	C_Selection::E_Mode selectionMode;

	NodeIdArray ids;
	if (m_args.IsSet('#'))
	{
		const ArgumentDataArray& data = *m_args.GetArgData('#');
		Btl::String nodes;
		data[0].GetValue(nodes);
		if(nodes.Search('-') == -1) //prevent looking for Id's when only flag arguments are passed
		{	
			DgNodeHelpers::BuildNodeList(ids, nodes.Chars(), m_args.IsSet('n'), m_args.IsSet('i'));
			if (nodes.Length() > 0 && ids.Size() == 0 && !m_args.IsSet('u'))
			{
				LOGERROR(C_Core::Instance().GetLogger(), "Nothing found to select!");
				return E_Status_Failure;
			}
		}
	}

	Ut::Vector<U32> components;
	if (m_args.IsSet('c'))
	{
		//Clear selection - do nothing.
	}
	//Component selection mode?
	else if (m_args.IsSet('i'))
	{
		if (ids.Size() > 1)
		{
			LOGERROR(C_Core::Instance().GetLogger(), "Multiple selection and component mode are mutually exclusive options!");
			return E_Status_Failure;
		}

		const ArgumentDataArray& data = *m_args.GetArgData('i');
		selectionMode = C_Selection::E_Mode_Component;

		//Acquire component string.
		Btl::String componentsString;
		data[0].GetValue(componentsString);
		//Tokenise component string.
		Btl::Vector<Btl::String> strings;
		componentsString.Tokenise(strings, ',');
		//Reserver storage.
		components.Resize(strings.Size());
		//Extract indices.
		for (S32 i = 0; i < strings.Size(); i++)
		{
			//Convert string to integer.
			components[i] = strtoul(strings[i].Chars(), 0, 10);
		}
	}
	else
		selectionMode = C_Selection::E_Mode_Object;

	C_Selection selection;
	///// ACTIONS //////
	// Add to existing selection.
	if (m_args.IsSet('a'))
	{
		C_Core::Instance().GetSelection(selection);
		for (S32 i = 0; i < ids.Size(); i++)
			selection.Add(selectionMode, ids[i], components);
	}
	// Remove from existing selection.
	else if (m_args.IsSet('r'))
	{
		C_Core::Instance().GetSelection(selection);
		for (S32 i = 0; i < ids.Size(); i++)
			selection.Remove(selectionMode, ids[i], components);
	}
	// Toggle selection.
	else if (m_args.IsSet('t'))
	{
		C_Core::Instance().GetSelection(selection);
		for (S32 i = 0; i < ids.Size(); i++)
			selection.Toggle(selectionMode, ids[i], components);
	}
	// Select all.
	else if (m_args.IsSet('l'))
	{
		C_Core::Instance().GetSelection(selection);
		C_DgNodeIterator it = C_Core::Instance().GetDgManager().GetDgIterator();
		while ((I_DgNode*)it)
		{
			if (!IsInteresting(it)) 
			{
				it++;
				continue;
			}
			
			ids.PushBack(it->GetId());
			it++;
		}
		for (S32 i = 0; i < ids.Size(); i++)
			selection.Add(selectionMode, ids[i]);
	}
	// Select. 
	else
	{
		//If we've no ids in our array then we'll end up clearing the selection when SetSelection is called.
		for (S32 i = 0; i < ids.Size(); i++)
			selection.Add(selectionMode, ids[i], components);
	}

	//Apply selection.
	C_Selection current;
	C_Core::Instance().GetSelection(current);
	if (current == selection)
		return E_Status_Cancelled;
	{
		//Undo checkpoint.
		C_CheckpointHelper c(&m_pChange, GetName(), false);
		//Do it...
		C_Core::Instance().SetSelection(selection);
	}
	return E_Status_Success;
}

bool C_SelectCmd::IsUndoable() const
{
	if(m_args.IsSet('U'))
		return false;

	return true;
}

void C_SelectCmd::GetResults(ResultsArray& results) const
{
}

I_Command* C_SelectCmd::Create(U32 type)
{
	return new C_SelectCmd(type);
}

C_Syntax* C_SelectCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		{
			C_Argument arg(
				'#', 
				"",
				"Node to select",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'i', 
				"component",
				"Component Indices",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'a', 
				"add",
				"add to selection",
				C_Argument::E_Required_False);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				't', 
				"toggle",
				"toggle selection",
				C_Argument::E_Required_False);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'r', 
				"remove",
				"remove from selection",
				C_Argument::E_Required_False);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'c', 
				"clear",
				"clear selection",
				C_Argument::E_Required_False);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'n', 
				"name",
				"Use name for selection instead of id",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'l', 
				"l",
				"select all",
				C_Argument::E_Required_False);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'U', 
				"U",
				"set as non-undoable",
				C_Argument::E_Required_False);
			pSyntax->AddArg(arg);
		}
	}

	return pSyntax;
}

const char* C_SelectCmd::GetName()
{
	static char* p = "select";
	return p;
}

bool C_SelectCmd::IsInteresting(I_DgNode* pNode)
{
	if (pNode && pNode->IsA(E_ObjType_SceneNode))
		return true;

	return false;
}


}
