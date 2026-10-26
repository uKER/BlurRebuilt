#include "stdafx.h"
#include <Misc_Shared/Variant/KeyValue.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Misc/SelectionHelpers.h>
#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <UserNode/Props/Poly/DgPolyPropNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

#include <EditorLib/Core.h>

#include <UserCmd/Poly/PolyAddPointsCmd.h>

namespace EditorUserCmd
{

C_PolyAddPointsCmd::C_PolyAddPointsCmd(U32 type) : I_Command(type)
{
}

C_PolyAddPointsCmd::~C_PolyAddPointsCmd()
{
}

EditorLib::C_Status C_PolyAddPointsCmd::Do()
{
	//Undo checkpoint.
	EditorLib::C_CheckpointHelper c(&m_pChange, GetName());

	EditorLib::C_Status status;

	const EditorLib::ArgumentDataArray& data = *m_args.GetArgData('#');

	//Grab current selection.
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	EditorLib::C_Selection::ItemArray filteredItems;
	//Find all interesting properties.
	EditorLib::SelectionHelpers::FilterPropertySelectionIsA(
		filteredItems, selection.GetItems(), EditorLib::E_ObjType_DgPolyPropNode);
	
	//Can only select one spline at a time
	if (filteredItems.Size() != 1)
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Please select a single polygon.");
		return EditorLib::E_Status_Failure;
	}

	EditorLib::C_Selection::C_Item item = *filteredItems.Begin(); 
	EditorLib::C_DgPolyPropNode* pPolyNode = 
		(EditorLib::C_DgPolyPropNode*)EditorLib::C_Core::Instance().GetDgManager().FindNodeById(item.id);
	BIZ_ASSERT(pPolyNode);

	if (pPolyNode->IsReadOnly())
		return EditorLib::E_Status_Failure;

	EditorLib::C_Core::Instance().SetContext("createpoly");
	return EditorLib::E_Status_Success;
}

bool C_PolyAddPointsCmd::IsUndoable() const
{
	return true;
}

void C_PolyAddPointsCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_PolyAddPointsCmd::Create(U32 type)
{
	return new C_PolyAddPointsCmd(type);
}

EditorLib::C_Syntax* C_PolyAddPointsCmd::CreateSyntax()
{
	EditorLib::C_Syntax* pSyntax = new EditorLib::C_Syntax();
	{
		EditorLib::C_Argument arg(
			'#', 
			"polyaddpoints",
			"adds new points to an existing polygon",
			EditorLib::C_Argument::E_Required_False);
		EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
		pSyntax->AddArg(arg);
	}

	return pSyntax;
}

const char* C_PolyAddPointsCmd::GetName()
{
	static char* p = "polyaddpoints";
	return p;
}

}
