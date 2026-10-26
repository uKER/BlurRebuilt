#include "stdafx.h"

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Dg_Cmd/DeleteCmd.h>

#include <EditorLib/Context/IDeleteContext.h>
#include <EditorLib/Context/DeleteContextFactory.h>
#include <EditorLib/Context/DeleteContextMapping.h>

#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeHelpers.h>
#include <EditorLib/Dg/SceneNodeIterator.h>

#include <EditorLib/Level/LevelHelpers.h>
#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

C_DeleteCmd::C_DeleteCmd(U32 type) : I_Command(type)
{
}

C_DeleteCmd::~C_DeleteCmd()
{
}

C_Status C_DeleteCmd::Do()
{
	C_Selection selection;
	C_Core::Instance().GetSelection(selection);
	if (selection.GetItems().Size() == 0)
		return E_Status_Success;
	{
		//Undo checkpoint.
		C_CheckpointHelper c(&m_pChange, GetName());
		C_ViewDisabler viewDisabler;

		C_Status status;
		C_XtLevel& level = *C_Core::Instance().GetLevelManager()->GetActiveLevel();

		const ArgumentDataArray* pData = m_args.GetArgData('#');

		C_Selection::E_Mode selectionMode = C_Selection::E_Mode_Object;

		NodeIdArray ids;
		if (!pData)
		{
			ids.Reserve(selection.GetItems().Size());

			for (C_Selection::ItemArray::ConstIterator it = selection.GetItems().Begin(); 
				it != selection.GetItems().End();
				++it)
			{
				C_NodeId id = it->id;
				ids.PushBack(id);
			}

			selectionMode = selection.GetMode();
		}
		else
		{
			Btl::String nodes;
			(*pData)[0].GetValue(nodes);
			DgNodeHelpers::BuildNodeList(ids, nodes, m_args.IsSet('n'), false);

			selectionMode = C_Selection::E_Mode_Object;
		}

		XtUt::Dictionary<XtId, XtId> remapTable;

		for (S32 i = 0; i < ids.Size(); i++)
		{
			I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(ids[i]);
			if (!pNode)
				continue;

			bool deleteNode = true;

			// If we're in component mode, call selection handler for current node
			if (selectionMode == C_Selection::E_Mode_Component)
			{
				E_DeleteContextType contextType = C_DeleteContextMapping::Instance().FindContext(pNode->GetType());
				I_DeleteContext* m_pContext = C_DeleteContextFactory::Instance().Create(contextType);
				if (m_pContext)
				{
					C_Selection selection;
					C_Core::Instance().GetSelection(selection);
					deleteNode = m_pContext->Delete(pNode, selection);
					delete m_pContext;
					C_Core::Instance().SetSelection(selection);
				}
			}

			// Check if the node is deletable
			if (pNode->IsReadOnly() || (!pNode->IsDeletable() && !pNode->IsA(E_ObjType_DgXtPropNodeBase)))
				continue;

			if (deleteNode) 
			{
				LevelHelpers::E_DeleteSceneNode deleteBehaviour = 
					(LevelHelpers::E_DeleteSceneNode)0;

				//Allow deletion of component selected.
				if (pNode->IsA(E_ObjType_DgXtCompNode))
					pNode = ((C_DgXtCompNode*)pNode)->GetParent();

				//Allow deletion if property selected.
				if (pNode->IsA(E_ObjType_DgXtPropNodeBase))
					pNode = ((C_DgXtPropNodeBase*)pNode)->GetParent()->GetParent();

				XtUt::Insert(&remapTable, pNode->GetId().GetXtId(), new XtId(0));
				if (pNode->IsA(E_ObjType_SceneNode))
				{
					C_SceneNode *pSceneNode = (C_SceneNode*)pNode;
					if (pSceneNode->GetChild())
					{
						C_SceneNodeConstIterator it(pSceneNode->GetChild());
						while ((C_SceneNode*) it)
						{
							XtUt::Insert(&remapTable, it->GetId().GetXtId(), new XtId(0));
							++it;
						}
					}

					deleteBehaviour = LevelHelpers::E_DeleteSceneNode_DeleteChildren;
				} 
				if (!LevelHelpers::DestroyNode(pNode, deleteBehaviour, false))
					return E_Status_Failure;
			}
		}


		//remap weak ptrs.
		::MapWeakPtrs(C_Core::Instance().GetXtManager().GetDb()->schema, AsStruct(level.GetXtType()), Xt::AsComponent(level.GetXtData()), &remapTable); // TODO: Components

		C_Core::Instance().DirtyViews();
		return E_Status_Success;
	}
}

bool C_DeleteCmd::IsUndoable() const
{
	return true;
}

void C_DeleteCmd::GetResults(ResultsArray& results) const
{
}

I_Command* C_DeleteCmd::Create(U32 type)
{
	return new C_DeleteCmd(type);
}

C_Syntax* C_DeleteCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		C_Argument arg(
			'#', 
			"",
			"Node to delete",
			C_Argument::E_Required_False);
		VarTypeArray& dataTypes = arg.GetDataTypes();
		dataTypes.PushBack(Util::E_VarType_String);
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

	return pSyntax;
}

const char* C_DeleteCmd::GetName()
{
	static char* p = "delete";
	return p;
}

}
