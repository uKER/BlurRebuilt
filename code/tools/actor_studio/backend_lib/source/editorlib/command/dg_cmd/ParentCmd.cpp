#include "stdafx.h"
#include <Btl/Containers/MultiMap.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeHelpers.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Command/dg_cmd/ParentCmd.h>

namespace EditorLib
{

C_ParentCmd::C_ParentCmd(U32 type) : I_Command(type)
{
}

C_ParentCmd::~C_ParentCmd()
{
}

C_Status C_ParentCmd::Do()
{
	//Undo checkpoint.
	C_CheckpointHelper c(&m_pChange, GetName());
	C_ViewDisabler viewDisabler;

	const ArgumentDataArray* pData = m_args.GetArgData('#');
	if (!pData || pData->Size() == 0)
	{
		LOGERROR(C_Core::Instance().GetLogger(), "No nodes specified!");
		return E_Status_Failure;
	}

	NodeIdArray nodeIds;
	Btl::String nodeStr;
	(*pData)[0].GetValue(nodeStr);
	DgNodeHelpers::BuildNodeList(nodeIds, nodeStr, false, false);

	bool fail = false;
	//Unparenting
	if (m_args.IsSet('u'))
	{
		Btl::MultiMap<U32, C_SceneNode*> depthMap;
		for (S32 i = 0; i < nodeIds.Size(); ++i)
		{
			I_DgNode* pSceneNode = C_Core::Instance().GetDgManager().FindNodeById(nodeIds[i]);
			if (pSceneNode && pSceneNode->IsA(E_ObjType_SceneNode))
				depthMap.Insert(Btl::MultiMap<U32, C_SceneNode*>::DataType(
					C_SceneNodeHelpers::GetDepth((C_SceneNode*)pSceneNode),(C_SceneNode*)pSceneNode));
		}

		for (Btl::MultiMap<U32, C_SceneNode*>::Iterator it = depthMap.Begin(); 
			it != depthMap.End(); ++it)
		{
			C_SceneNode* pSceneNode = it->GetValue();
			if (!pSceneNode->GetParent() || !pSceneNode->GetParent()->GetParent() || pSceneNode->IsReadOnly())
				fail = true;
			else if (!C_SceneNodeHelpers::ReparentSceneNode(
				(C_SceneNode*)pSceneNode, (C_SceneNode*)pSceneNode->GetParent()->GetParent(), true))
				fail = true;
		}
	}
	//Parenting
	else if (m_args.IsSet('p'))
	{
		NodeIdArray parentId;
		Btl::String parentStr;
		const ArgumentDataArray* pData = m_args.GetArgData('p');
		if (pData && pData->Size() != 0)
			(*pData)[0].GetValue(parentStr);
		DgNodeHelpers::BuildNodeList(parentId, parentStr, false, false);

		if (parentId.Size() != 1)
			return E_Status_Failure;

		I_DgNode* pParentNode = C_Core::Instance().GetDgManager().FindNodeById(parentId[0]);
		if (!pParentNode || !pParentNode->IsA(E_ObjType_SceneNode))
			return E_Status_Failure;

		for (S32 i=0; i < nodeIds.Size(); ++i)
		{
			I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(nodeIds[i]);
			if (pNode == pParentNode)
			{
				fail = true;
				break;
			}
		}

		for (S32 i=0; !fail && i < nodeIds.Size(); ++i)
		{
			I_DgNode* pNode = C_Core::Instance().GetDgManager().FindNodeById(nodeIds[i]);
			if (!pNode || pNode == pParentNode)
				continue;

			if (pNode->IsReadOnly() || pNode->IsA(E_ObjType_SceneNode) && 
				!C_SceneNodeHelpers::ReparentSceneNode(
					(C_SceneNode*)pNode, (C_SceneNode*)pParentNode, true))
				fail = true;
		}
	}
	if (fail)
		LOGERROR(C_Core::Instance().GetLogger(), "Selection contains items which cannot be reparented");

	return E_Status_Success;

}

bool C_ParentCmd::IsUndoable() const
{
	return true;
}

void C_ParentCmd::GetResults(ResultsArray& results) const
{
}

I_Command* C_ParentCmd::Create(U32 type)
{
	return new C_ParentCmd(type);
}

C_Syntax* C_ParentCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		{
			C_Argument arg(
				'#',
				"",
				"[Nodes] Node List",
				C_Argument::E_Required_True);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'p', 
				"parent",
				"Parents nodes",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'u', 
				"unparent",
				"Unparents nodes",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			pSyntax->AddArg(arg);
		}
	}
	return pSyntax;
}

const char* C_ParentCmd::GetName()
{
	static char* p = "parent";
	return p;
}

}