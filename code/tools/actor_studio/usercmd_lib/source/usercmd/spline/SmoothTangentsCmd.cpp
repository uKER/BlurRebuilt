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
#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Spline/DgSplinePropNode.h>
#include <UserCmd/Spline/SmoothTangentsCmd.h>

namespace EditorUserCmd
{

C_SmoothTangentsCmd::C_SmoothTangentsCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_SmoothTangentsCmd::~C_SmoothTangentsCmd()
{
}

EditorLib::C_Status C_SmoothTangentsCmd::Do()
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

		Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = pSplineNode->GetCurveData()->GetKnots();

		//In object mode, operate on entire spline.
		Ut::Vector<U32> components = it->components;
		if (components.Size() == 0 && m_args.IsSet('o'))
		{
			components.Resize(pSplineNode->GetNumComponents());
			for (S32 i = 0; i < components.Size(); ++i)
				components[i] = i;
		}

		//Adjust selection.
		for (S32 i = 0; i < components.Size(); ++i)
		{
			U32 knotIndex = pSplineNode->GetComponentKnotIndex(components[i]);
			if (m_args.IsSet('a'))
			{
				if (knotIndex > 1 && knotIndex < (U32)knots.Size() - 1)
				{		
					Maths::HVec3 tanIn = knots[knotIndex].tangentIn;
					Maths::HVec3 tanOut = knots[knotIndex].tangentOut;
					knots[knotIndex].tangentIn = (tanIn + tanOut)/2.f;
					knots[knotIndex].tangentOut = (tanOut + tanIn)/2.f;
				} else if (pSplineNode->IsSplineClosed())
				{
					Maths::HVec3 tanIn = knots[knots.Size()-1].tangentIn;
					Maths::HVec3 tanOut = knots[0].tangentOut;
					knots[knots.Size()-1].tangentIn = (tanIn + tanOut)/2.f;
					knots[0].tangentOut = (tanOut + tanIn)/2.f;				
				}
			} else {
				pSplineNode->GetCurve()->CalculateKnotTangents(knotIndex, true, true, pSplineNode->IsSplineClosed());
				if (!m_args.IsSet('t'))
				{
					knots[knotIndex].tangentIn = knots[knotIndex].tangentIn * pSplineNode->GetDefaultTangentScale();
					knots[knotIndex].tangentOut = knots[knotIndex].tangentOut * pSplineNode->GetDefaultTangentScale();
				}
			}
		}

		status = pSplineNode->Apply();
		if (!status)
		{
			LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Failed to apply spline changes.");
			return status;
		}
		
		status = pSplineNode->XtSave();
		if (!status)
		{
			LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "Failed to store spline changes.");
			return status;
		}
	}

	return EditorLib::E_Status_Success;
}

bool C_SmoothTangentsCmd::IsUndoable() const
{
	return true;
}

void C_SmoothTangentsCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_SmoothTangentsCmd::Create(U32 type)
{
	return new C_SmoothTangentsCmd(type);
}

EditorLib::C_Syntax* C_SmoothTangentsCmd::CreateSyntax()
{
	EditorLib::C_Syntax* pSyntax = new EditorLib::C_Syntax();
	{
		EditorLib::C_Argument arg(
			'#', 
			"splinemodifytangents",
			"Modifies the tangents of a spline, default operation is smooth tangents",
			EditorLib::C_Argument::E_Required_False);
		EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
		//dataTypes.PushBack(E_DataType_S32);
		pSyntax->AddArg(arg);
	}

	{
		EditorLib::C_Argument arg(
			'o', 
			"objectmode",
			"Operate on entire spline",
			EditorLib::C_Argument::E_Required_False);
		EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
		pSyntax->AddArg(arg);
	}

	{
		EditorLib::C_Argument arg(
			'a', 
			"average tangents",
			"Average the tangents instead of smoothing them",
			EditorLib::C_Argument::E_Required_False);
		EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
		pSyntax->AddArg(arg);
	}

	{
		EditorLib::C_Argument arg(
			't', 
			"do not scale tangents",
			"Don't scale tangents by the default scaling factor. Results in a smoother curve.",
			EditorLib::C_Argument::E_Required_False);
		EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
		pSyntax->AddArg(arg);
	}

	return pSyntax;
}

const char* C_SmoothTangentsCmd::GetName()
{
	static char* p = "splinemodifytangents";
	return p;
}

}
