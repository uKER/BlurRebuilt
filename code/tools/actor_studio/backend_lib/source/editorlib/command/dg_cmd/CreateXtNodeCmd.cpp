#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <Aftk/AftkEuler.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/Dg_Cmd/CreateXtNodeCmd.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Level/LevelHelpers.h>

namespace EditorLib
{

bool C_CreateXtNodeCmdContext::RegisterPostCreate(Btl::String postCreate, Ut::Delegate<void(EditorLib::I_DgNode*)> postCreateDelegate)
{
	if (GetPostCreate(postCreate))
		return false;

	m_PostCreate.PushBack(Btl::Pair<Btl::String, Ut::Delegate<void(EditorLib::I_DgNode*)> >(postCreate, postCreateDelegate));

	return true;
}

Ut::Delegate<void(EditorLib::I_DgNode*)>* C_CreateXtNodeCmdContext::GetPostCreate(Btl::String postCreate)
{
	for (Btl::List<Btl::Pair<Btl::String, Ut::Delegate<void(EditorLib::I_DgNode*)> > >::Iterator it = m_PostCreate.Begin(); it != m_PostCreate.End(); ++it)
	{
		if (it->first == postCreate)
			return &it->second;
	}

	return 0;
}

C_CreateXtNodeCmd::C_CreateXtNodeCmd(U32 type) : I_Command(type)
{
}

C_CreateXtNodeCmd::~C_CreateXtNodeCmd()
{
}

C_Status C_CreateXtNodeCmd::Do()
{
	C_Status status;
	C_XtLevel& level = *C_Core::Instance().GetLevelManager()->GetActiveLevel();

	const ArgumentDataArray& data = *m_args.GetArgData('#');
	Btl::String nodeType;
	data[0].GetValue(nodeType);

	Vector3 locationWorld(0,0,0);
	Vector3 rotation(0,0,0);
	if (m_args.IsSet('t'))
	{
		const ArgumentDataArray* pData = m_args.GetArgData('t');
		(*pData)[0].GetValue(locationWorld[0]);
		(*pData)[1].GetValue(locationWorld[1]);
		(*pData)[2].GetValue(locationWorld[2]);
	}
	if (m_args.IsSet('r'))
	{
		const ArgumentDataArray* pData = m_args.GetArgData('r');
		(*pData)[0].GetValue(rotation[0]);
		(*pData)[1].GetValue(rotation[1]);
		(*pData)[2].GetValue(rotation[2]);
	}
	{
		C_CheckpointHelper c(&m_pChange, GetName());

		I_DgNode* pNode = 0;
		if (m_args.IsSet('n'))
		{
			Btl::String nodeName;
			(*m_args.GetArgData('n'))[0].GetValue(nodeName);
			pNode = LevelHelpers::CreateNode(
				C_Core::Instance().GetLevelManager()->GetActiveLevel(), nodeType.Chars(), nodeName.Chars());
		} else
		{
			pNode = LevelHelpers::CreateNode(
				C_Core::Instance().GetLevelManager()->GetActiveLevel(), nodeType.Chars());
		}

		if (!pNode)
		{
			return E_Status_Failure;
		}

		if (m_args.IsSet('p'))
		{
			Btl::String postCreate;
			(*m_args.GetArgData('p'))[0].GetValue(postCreate);
			Ut::Delegate<void(EditorLib::I_DgNode*)>* postCreateDelegate = GetContext()->GetPostCreate(postCreate);
			if (postCreateDelegate)
				(*postCreateDelegate)(pNode);
		}

		m_results.nodeId = pNode->GetId();

		if (pNode->IsA(EditorLib::E_ObjType_SceneNode))
		{
			C_SceneNode* pSceneNode = (C_SceneNode*)pNode;
			EditorLib::Matrix44 m = pSceneNode->GetWorldMatrix();
			EditorLib::Matrix44 rot;
			Aftk::EulerToMatrixXYZ(rot, rotation);
			m = m * rot;
			(EditorLib::Vector3&)m[3] = locationWorld;
			pSceneNode->SetWorldMatrix(m);
		}

		//select newly created object
		C_Selection s;
		s.Add(C_Selection::E_Mode_Object, pNode->GetId());
		C_Core::Instance().SetSelection(s);
	}

	return E_Status_Success;
}

bool C_CreateXtNodeCmd::IsUndoable() const
{
	return true;
}

void C_CreateXtNodeCmd::GetResults(ResultsArray& results) const
{
	results.PushBack();
	results[0].SetValue(m_results.nodeId.AsString());
}

I_Command* C_CreateXtNodeCmd::Create(U32 type)
{
	return new C_CreateXtNodeCmd(type);
}

C_Syntax* C_CreateXtNodeCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		{
			C_Argument arg(
				'#', 
				"",
				"node type name",
				C_Argument::E_Required_True);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				't', 
				"translate",
				"x y z translate values",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'r', 
				"rotate",
				"x y z rotate values",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			dataTypes.PushBack(Util::E_VarType_F32);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'p', 
				"postcreate",
				"name of the post creation handler to activate",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
		{
			C_Argument arg(
				'n', 
				"name",
				"name of the new node",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
	}

	return pSyntax;
}

const char* C_CreateXtNodeCmd::GetName()
{
	static char* p = "createXtNode";
	return p;
}

C_CreateXtNodeCmdContext* C_CreateXtNodeCmd::GetContext()
{
	static C_CreateXtNodeCmdContext* context = new C_CreateXtNodeCmdContext();
	return context;
}

}
