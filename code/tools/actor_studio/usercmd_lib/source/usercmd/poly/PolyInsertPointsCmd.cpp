#include "stdafx.h"
#include <algorithm>
#include <Misc_Shared/Variant/KeyValue.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Misc/SelectionHelpers.h>
#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Core.h>

#include <UserNode/Props/Poly/DgPolyPropNode.h>

#include <UserCmd/Poly/PolyInsertPointsCmd.h>



namespace EditorUserCmd
{

C_PolyInsertPointsCmd::C_PolyInsertPointsCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_PolyInsertPointsCmd::~C_PolyInsertPointsCmd()
{
}

EditorLib::C_Status C_PolyInsertPointsCmd::Do()
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

	std::sort(item.components.Begin(), item.components.End());
	//Insert points into spline.
	Ut::Vector<U32> addedComponents = pPolyNode->InsertPoints(item.components);
	
	//Clear selection
	selection.Clear();
	//Adjust selection.
	if (addedComponents.Size() > 0)
	{
		selection.Add(EditorLib::C_Selection::E_Mode_Component, pPolyNode->GetId(), addedComponents);

		EditorLib::C_Core::Instance().SetSelection(selection);
		return EditorLib::E_Status_Success;
	} 
	else 
		return EditorLib::E_Status_Failure;
}

bool C_PolyInsertPointsCmd::IsUndoable() const
{
	return true;
}

void C_PolyInsertPointsCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_PolyInsertPointsCmd::Create(U32 type)
{
	return new C_PolyInsertPointsCmd(type);
}

EditorLib::C_Syntax* C_PolyInsertPointsCmd::CreateSyntax()
{
	EditorLib::C_Syntax* pSyntax = new EditorLib::C_Syntax();
	{
		EditorLib::C_Argument arg(
			'#', 
			"polyinsertpoints",
			"adds new points to an existing polygon",
			EditorLib::C_Argument::E_Required_False);
		EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
		//dataTypes.PushBack(E_DataType_S32);
		pSyntax->AddArg(arg);
	}

	return pSyntax;
}

const char* C_PolyInsertPointsCmd::GetName()
{
	static char* p = "polyinsertpoints";
	return p;
}

}
