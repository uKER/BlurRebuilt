#include "stdafx.h"

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Misc/SelectionHelpers.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <UserNode/Props/Spline/DgSplinePropNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Core.h>

#include <UserCmd/Spline/AddPointsCmd.h>

namespace EditorUserCmd
{

C_SplineAddPointsCmd::C_SplineAddPointsCmd(U32 type) : I_Command(type)
{
}

C_SplineAddPointsCmd::~C_SplineAddPointsCmd()
{
}

EditorLib::C_Status C_SplineAddPointsCmd::Do()
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
		filteredItems, selection.GetItems(), EditorLib::E_ObjType_DgSplinePropNode);
	
	//Can only select one spline at a time
	if (filteredItems.Size() != 1)
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Please select a single spline.");
		return EditorLib::E_Status_Failure;
	}

	EditorLib::C_Selection::C_Item item = *filteredItems.Begin(); 
	EditorLib::C_DgSplinePropNode* pSplineNode = 
		(EditorLib::C_DgSplinePropNode*)EditorLib::C_Core::Instance().GetDgManager().FindNodeById(item.id);
	BIZ_ASSERT(pSplineNode);

	if (pSplineNode->IsReadOnly())
		return EditorLib::E_Status_Failure;

	if (pSplineNode->IsSplineClosed())
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Cannot append points to closed spline.");
		return EditorLib::E_Status_Failure;
	}

	EditorLib::C_Core::Instance().SetContext("createspline");
	return EditorLib::E_Status_Success;
}

bool C_SplineAddPointsCmd::IsUndoable() const
{
	return true;
}

void C_SplineAddPointsCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_SplineAddPointsCmd::Create(U32 type)
{
	return new C_SplineAddPointsCmd(type);
}

EditorLib::C_Syntax* C_SplineAddPointsCmd::CreateSyntax()
{
	EditorLib::C_Syntax* pSyntax = new EditorLib::C_Syntax();
	{
		EditorLib::C_Argument arg(
			'#', 
			"splineaddpoints",
			"adds new points to an existing spline",
			EditorLib::C_Argument::E_Required_False);
		EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
		pSyntax->AddArg(arg);
	}

	return pSyntax;
}

const char* C_SplineAddPointsCmd::GetName()
{
	static char* p = "splineaddpoints";
	return p;
}

}
