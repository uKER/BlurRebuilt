#include "stdafx.h"
#include <algorithm>

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>

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

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Misc/SelectionHelpers.h>
#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Core.h>

#include <UserCmd/Spline/InsertPointsCmd.h>
#include <UserNode/Props/Spline/DgSplinePropNode.h>

namespace EditorUserCmd
{

C_SplineInsertPointsCmd::C_SplineInsertPointsCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_SplineInsertPointsCmd::~C_SplineInsertPointsCmd()
{
}

EditorLib::C_Status C_SplineInsertPointsCmd::Do()
{
	//Undo checkpoint.
	EditorLib::C_CheckpointHelper c(&m_pChange, GetName());

	EditorLib::C_Status status;
	EditorLib::C_XtLevel& level = *EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel();

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

	std::sort(item.components.Begin(), item.components.End());
	//Insert points into spline.
	Ut::Vector<U32> addedComponents = pSplineNode->InsertPoints(item.components);
	
	//Clear selection
	selection.Clear();
	//Adjust selection.
	if (addedComponents.Size() > 0)
	{
		const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = pSplineNode->GetCurveData()->GetKnots();
		Ut::Vector<U32> selectionComponents;
		for (S32 i = 0; i < addedComponents.Size(); ++i)
		{
			U32 componentIndex = addedComponents[i];
			assert(componentIndex > 1 && componentIndex < pSplineNode->GetNumComponents()-3);
			
			selectionComponents.PushBack(componentIndex);
			//Select next knot if we're not the last in our knot array.
			if (pSplineNode->GetComponentKnotIndex(componentIndex+3) < (U32)knots.Size())
				selectionComponents.PushBack(componentIndex+3);
		}

		selection.Add(EditorLib::C_Selection::E_Mode_Component, pSplineNode->GetId(), selectionComponents);

		EditorLib::C_Core::Instance().SetSelection(selection);
		return EditorLib::E_Status_Success;
	} 
	else 
		return EditorLib::E_Status_Failure;
}

bool C_SplineInsertPointsCmd::IsUndoable() const
{
	return true;
}

void C_SplineInsertPointsCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_SplineInsertPointsCmd::Create(U32 type)
{
	return new C_SplineInsertPointsCmd(type);
}

EditorLib::C_Syntax* C_SplineInsertPointsCmd::CreateSyntax()
{
	EditorLib::C_Syntax* pSyntax = new EditorLib::C_Syntax();
	{
		EditorLib::C_Argument arg(
			'#', 
			"splineinsertpoints",
			"adds new points to an existing spline",
			EditorLib::C_Argument::E_Required_False);
		EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
		//dataTypes.PushBack(E_DataType_S32);
		pSyntax->AddArg(arg);
	}

	return pSyntax;
}

const char* C_SplineInsertPointsCmd::GetName()
{
	static char* p = "splineinsertpoints";
	return p;
}

}
