#include "stdafx.h"

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Level/LevelHelpers.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Collision/Collision.h>

#include <UserCmd/Drop/DropCmd.h>

namespace EditorUserCmd
{

C_DropCmd::C_DropCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_DropCmd::~C_DropCmd()
{
}

EditorLib::C_Status C_DropCmd::Do()
{
	//Undo checkpoint.
	EditorLib::C_CheckpointHelper c(&m_pChange, GetName());

	EditorLib::C_Status status;

	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);
	
	if (selection.GetItems().Size() == 0)
	{
		LOGERROR(EditorLib::C_Core::Instance().GetLogger(), "No object selected.");
		return EditorLib::E_Status_Failure;
	}
	
	F32 offsetY = 0;
	if (m_args.IsSet('y'))
	{
		const EditorLib::ArgumentDataArray* pData;

		pData = m_args.GetArgData('y');
		if (pData && pData->Size() != 0)
			(*pData)[0].GetValue(offsetY);
	}

	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();


	for (EditorLib::C_Selection::ItemArray::ConstIterator it = items.Begin(); 
			it != items.End(); 
			++it)
	{
		EditorLib::C_NodeId id = it->id;
		EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(id);
		if (!IsInteresting(pNode))
			continue;
		
		EditorLib::C_SceneNode* pSceneNode = (EditorLib::C_SceneNode*)pNode;
		EditorLib::Matrix44 wm = pSceneNode->GetWorldMatrix();
		EditorLib::C_Ray ray;
		ray.origin = wm[3];
		ray.end = ray.origin + EditorLib::Vector3(0, -1000.f, 0);
				 
		EditorLib::C_CollisionManager& colMan = EditorLib::C_Core::Instance().GetCollisionManager();
		EditorLib::C_RayResult rayRes;

		bool intRes = colMan.Intersect(
			rayRes,
			ray,
			EditorLib::E_CollisionQueryType_Complex); 

		if(intRes)
		{
			wm[3][1] = rayRes.intersectionPoint.y+offsetY; 
			pSceneNode->SetWorldMatrix(wm);
		}
		else
		{
			LOGINFO(EditorLib::C_Core::Instance().GetLogger(), "Can't drop because there is no object below this one");
		}
	}
		
	return EditorLib::E_Status_Success;
}

bool C_DropCmd::IsUndoable() const
{
	return true;
}

void C_DropCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_DropCmd::Create(U32 type)
{
	return new C_DropCmd(type);
}

EditorLib::C_Syntax* C_DropCmd::CreateSyntax()
{
	EditorLib::C_Syntax* pSyntax = new EditorLib::C_Syntax();
	{
		{
			EditorLib::C_Argument arg(
				'y',
				"offsetY",
				"Y offset distance. Optional.",
				EditorLib::C_Argument::E_Required_False);
			EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_F32);
			pSyntax->AddArg(arg);
		}
	}
	return pSyntax;
}

const char* C_DropCmd::GetName()
{
	static char* p = "drop";
	return p;
}

bool C_DropCmd::IsInteresting(EditorLib::I_DgNode* pNode)
{
	if (pNode && !pNode->IsReadOnly() && pNode->IsA(EditorLib::E_ObjType_SceneNode))
		return true;

	return false;
}

}
