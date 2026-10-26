#include "stdafx.h"

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Dg_Cmd/CreateGroupCmd.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Level/LevelHelpers.h>

namespace EditorLib
{

C_CreateGroupCmd::C_CreateGroupCmd(U32 type) : I_Command(type)
{
}

C_CreateGroupCmd::~C_CreateGroupCmd()
{
}

C_Status C_CreateGroupCmd::Do()
{
	C_Status status;
	C_XtLevel *pLevel = C_Core::Instance().GetLevelManager()->GetActiveLevel();

	Btl::String groupName("");
	const ArgumentDataArray* pArgData = m_args.GetArgData('#');
	if (pArgData && pArgData->Size())
		(*pArgData)[0].GetValue(groupName);

	Vector3 locationWorld(0,0,0);

	C_SceneNode *pGroupParent = C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetRootSceneNode();
	Ut::UtVector< Ut::DynamicVector<C_SceneNode*> > groupMembers;
	
	if (!m_args.IsSet('e'))
	{
		//find common parent of selection
		Ut::OpenHashMap<C_SceneNode*, U32, Ut::HashKeyOpPtr> ancestorCount;

		C_Selection selection;
		C_Core::Instance().GetSelection(selection);
		const C_Selection::ItemArray& items = selection.GetItems();

		for (S32 i=0; i < items.Size(); ++i)
		{
			I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(items[i].id);
			if (!pNode->IsA(E_ObjType_SceneNode))
				continue;
			if (pNode && ((C_SceneNode*)pNode)->GetSceneNodeFlags().canReparent)
			{
				C_SceneNode *pSceneNode = (C_SceneNode*)pNode;
				groupMembers.PushBack(pSceneNode);

				while (pSceneNode->GetParent())
				{
					Ut::OpenHashMap<C_SceneNode*, U32, Ut::HashKeyOpPtr>::Iterator it = ancestorCount.Find(pSceneNode);
					if (it == ancestorCount.End())
						ancestorCount[pSceneNode] = 1;
					else
						it->GetValue()++;

					pSceneNode = pSceneNode->GetParent();
				}
			}
		}

		if (groupMembers.Size() > 1)
		{
			C_SceneNode *pCommonAncestor = 0;
			U32 commonAncestorDepth = 0;
			for (Ut::OpenHashMap<C_SceneNode*, U32, Ut::HashKeyOpPtr>::Iterator it = ancestorCount.Begin(); it != ancestorCount.End(); ++it)
			{
				if (it->GetValue() == groupMembers.Size())
				{
					U32 depth = C_SceneNodeHelpers::GetDepth(it->GetKey());
					if (!pCommonAncestor || depth > commonAncestorDepth)
					{
						commonAncestorDepth = depth;
						pCommonAncestor = it->GetKey();
					}
				}
			}

			if (pCommonAncestor)
				pGroupParent = pCommonAncestor;
		}
		else if (groupMembers.Size() == 1)
			pGroupParent = groupMembers[0]->GetParent();
	}

	{
		C_CheckpointHelper c(&m_pChange, GetName());

		{
			C_XtObject::C_Unbinder unbinder(pLevel);
			I_DgNode* pNode = LevelHelpers::CreateNode(pLevel, "AsSceneXtGroupNode");
			if (groupName.Length())
				pNode->SetName(groupName);
			if (!pNode) 
			{
				LOGERROR(EditorLib::C_Core::Instance().GetLogger(), 
					"Failed to create group node.");
				return E_Status_Failure;
			}
			if (!pNode->IsA(EditorLib::E_ObjType_SceneNode))
			{
				LOGERROR(C_Core::Instance().GetLogger(), 
					"Group node is of an invalid type.");
				LevelHelpers::DestroyNode(pNode);
				return E_Status_Failure;
			}

			C_SceneNode *pSceneNode = (C_SceneNode*)pNode;
			C_SceneNodeHelpers::ReparentSceneNode(pSceneNode, pGroupParent, false);

			//add members (preserve world position)
			for (S32 i=0; i<groupMembers.Size(); ++i)
				C_SceneNodeHelpers::ReparentSceneNode(groupMembers[i], pSceneNode, true);

			//select newly created object
			C_Selection s;
			s.Add(C_Selection::E_Mode_Object, pNode->GetId());
			C_Core::Instance().SetSelection(s);

			m_results.nodeId = pNode->GetId();
		}
	}

	return E_Status_Success;
}

bool C_CreateGroupCmd::IsUndoable() const
{
	return true;
}

void C_CreateGroupCmd::GetResults(ResultsArray& results) const
{
	results.PushBack();
	results[0].SetValue(m_results.nodeId.AsString());
}

I_Command* C_CreateGroupCmd::Create(U32 type)
{
	return new C_CreateGroupCmd(type);
}

C_Syntax* C_CreateGroupCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		{
			C_Argument arg(
				'#', 
				"",
				"group name",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'e', 
				"empty",
				"create empty group",
				C_Argument::E_Required_False);
			pSyntax->AddArg(arg);
		}
	}

	return pSyntax;
}

const char* C_CreateGroupCmd::GetName()
{
	static char* p = "createGroup";
	return p;
}

}
