#include "stdafx.h"
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Io_Cmd/FileCmd.h>

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

C_FileCmd::C_FileCmd(U32 type) : I_Command(type)
{
}

C_FileCmd::~C_FileCmd()
{
}

C_Status C_FileCmd::Do()
{
	//Extract windows message information.
	//Post a message to the hosting application.

	if (m_args.IsSet('n'))
	{
		m_lastCommand = E_LastCmd_New;
		const ArgumentDataArray& data = *m_args.GetArgData('n');
		XtId packId;
		data[0].GetValue(packId);

		EditorLib::C_XtLevel* level = 0;

		if (EditorLib::C_Core::Instance().GetLevelManager()->OpenPack(packId))
			level = EditorLib::C_Core::Instance().GetLevelManager()->GetPack(packId)->NewLevel();

		if (!level) return E_Status_Failure;
		
		m_results.m_new.recordId = level->GetXtRecordId();
		return E_Status_Success;
	}
	else if (m_args.IsSet('o'))
	{
		m_lastCommand = E_LastCmd_Open;
		const ArgumentDataArray& data = *m_args.GetArgData('o');

		XtId id;
		data[0].GetValue(id);

		return C_Core::Instance().GetLevelManager()->Open(id);
	}
	else if (m_args.IsSet('s'))
	{
		m_lastCommand = E_LastCmd_Save;
		const ArgumentDataArray& data = *m_args.GetArgData('s');

		return C_Core::Instance().GetXtManager().Save();
	}
	else if (m_args.IsSet('c'))
	{
		m_lastCommand = E_LastCmd_Close;
		const ArgumentDataArray& data = *m_args.GetArgData('c');

		XtId id;
		data[0].GetValue(id);

		if (C_Core::Instance().GetXtManager().FindPack(id))
		{
			C_Core::Instance().GetLevelManager()->ClosePack(id);
			return E_Status_Success;
		} else if (C_Core::Instance().GetXtManager().FindRecord(id)) {
			C_Core::Instance().GetLevelManager()->GetLevel(id)->Unload();
			return E_Status_Success;
		}

		return E_Status_Failure;
	}

	return E_Status_Failure;
}

bool C_FileCmd::IsUndoable() const
{
	return false;
}

void C_FileCmd::GetResults(ResultsArray& results) const
{
	switch (m_lastCommand)
	{
	case E_LastCmd_New:
		{
			results.PushBack();
			Util::C_KeyValue& attr = results.Back();
			attr.SetValue(m_results.m_new.recordId);
		}
		break;

	default:
		break;
	};
}

I_Command* C_FileCmd::Create(U32 type)
{
	return new C_FileCmd(type);
}

C_Syntax* C_FileCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		C_Argument arg(
			'n', 
			"new",
			"creates a new level",
			C_Argument::E_Required_False);
		VarTypeArray& dataTypes = arg.GetDataTypes();
		dataTypes.PushBack(Util::E_VarType_U32);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			'o', 
			"open",
			"opens a level",
			C_Argument::E_Required_False);
		
		VarTypeArray& dataTypes = arg.GetDataTypes();
		dataTypes.PushBack(Util::E_VarType_U32);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			's', 
			"save",
			"saves the current level",
			C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			'e', 
			"export",
			"exports the current level",
			C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			'c', 
			"close",
			"closes the specified level",
			C_Argument::E_Required_False);

		VarTypeArray& dataTypes = arg.GetDataTypes();
		dataTypes.PushBack(Util::E_VarType_U32);
		pSyntax->AddArg(arg);
	}
	return pSyntax;
}

const char* C_FileCmd::GetName()
{
	static char* p = "file";
	return p;
}

}
