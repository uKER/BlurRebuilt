#include "stdafx.h"
#include <algorithm>

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>
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
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Core.h>

#include <UserCmd/Spline/ReverseSplineCmd.h>
#include <UserNode/Props/Spline/DgSplinePropNode.h>

namespace EditorUserCmd
{

C_ReverseSplineCmd::C_ReverseSplineCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_ReverseSplineCmd::~C_ReverseSplineCmd()
{
}

EditorLib::C_Status C_ReverseSplineCmd::Do()
{
	//Undo checkpoint.
	EditorLib::C_CheckpointHelper c(&m_pChange, GetName());

	//Grab current selection.
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	EditorLib::C_Selection::ItemArray filteredItems;
	//Find all interesting properties.
	EditorLib::SelectionHelpers::FilterPropertySelectionIsA(
		filteredItems, selection.GetItems(), EditorLib::E_ObjType_DgSplinePropNode);
	
	//Bail if nothing interesting selected.
	if (filteredItems.Size() == 0)
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "No splines selected.");
		return EditorLib::E_Status_Failure;
	}

	for (EditorLib::C_Selection::ItemArray::ConstIterator it = filteredItems.Begin(); 
		it != filteredItems.End(); ++it)
	{
		EditorLib::C_DgSplinePropNode* pSplineNode = 
			(EditorLib::C_DgSplinePropNode*)EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
		BIZ_ASSERT(pSplineNode);

		if (pSplineNode->IsReadOnly())
			continue;

		//Reverse our spline.
		pSplineNode->Reverse();
	}
	
	return EditorLib::E_Status_Success;
}

bool C_ReverseSplineCmd::IsUndoable() const
{
	return true;
}

void C_ReverseSplineCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_ReverseSplineCmd::Create(U32 type)
{
	return new C_ReverseSplineCmd(type);
}

EditorLib::C_Syntax* C_ReverseSplineCmd::CreateSyntax()
{
	return new EditorLib::C_Syntax();
}

const char* C_ReverseSplineCmd::GetName()
{
	static char* p = "splinereverse";
	return p;
}

}
