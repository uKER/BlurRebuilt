#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Dg_Cmd/RenameCmd.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

C_RenameCmd::C_RenameCmd(U32 type) : I_Command(type)
{
}

C_RenameCmd::~C_RenameCmd()
{
}

C_Status C_RenameCmd::Do()
{
	I_DgNode* pNode;
	Btl::String newNodeName;
	/*if (m_args.IsSet('n'))
	{
		const ArgumentDataArray& data = *m_args.GetArgData('#');
		Btl::String nodeName;
		data[0].GetValue(nodeName);
		data[1].GetValue(newNodeName);

		pNode = C_Core::Instance().GetDgManager().FindNodeByName(nodeName);
	}
	else*/
	{
		const ArgumentDataArray& data = *m_args.GetArgData('#');
		C_NodeId id;
		Btl::String nodeId;
		data[0].GetValue(nodeId);
		data[1].GetValue(newNodeName);

		id.FromString(nodeId);
		pNode = C_Core::Instance().GetDgManager().FindNodeById(id);
	}

	if (!pNode || pNode->IsReadOnly())
		return E_Status_Failure;

	if (pNode->GetName() == newNodeName)
		//Nothing to do...
		return E_Status_Success;
	{
		C_CheckpointHelper c(&m_pChange, GetName());
		//Perform operation.
		pNode->SetName(newNodeName);
	}

	return E_Status_Success;
}

bool C_RenameCmd::IsUndoable() const
{
	return true;
}

I_Command* C_RenameCmd::Create(U32 type)
{
	return new C_RenameCmd(type);
}

C_Syntax* C_RenameCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		{
			C_Argument arg(
				'#', 
				"",
				"node and new name",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
		/*{
			C_Argument arg(
				'n', 
				"name",
				"Use name for selection instead of id",
				C_Argument::E_Required_False);
			VarTypeArray& dataTypes = arg.GetDataTypes();
			pSyntax->AddArg(arg);
		}*/
	}

	return pSyntax;
}

const char* C_RenameCmd::GetName()
{
	static char* p = "rename";
	return p;
}

}
