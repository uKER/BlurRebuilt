#include "stdafx.h"

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Xt_Cmd/XtDbCmd.h>

#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

C_XtDbCmd::C_XtDbCmd(U32 type) : I_Command(type)
{
}

C_XtDbCmd::~C_XtDbCmd()
{
}

C_Status C_XtDbCmd::Do()
{
	bool ensureUnique = false;
	if (m_args.IsSet('u'))
	{
		const ArgumentDataArray& data = *m_args.GetArgData('u');
		data[0].GetValue(ensureUnique);
	}

	if (m_args.IsSet('o'))
	{
		const ArgumentDataArray& data = *m_args.GetArgData('o');
		Btl::String xtDb;
		data[0].GetValue(xtDb);
		return C_Core::Instance().GetXtManager().Open(xtDb, ensureUnique);
	}
	else if (m_args.IsSet('s'))
	{
		return C_Core::Instance().GetXtManager().Save();
	}
	else if (m_args.IsSet('c'))
	{
		return C_Core::Instance().GetXtManager().Close();
	}
	
	return E_Status_Failure;
}

bool C_XtDbCmd::IsUndoable() const
{
	return false;
}

I_Command* C_XtDbCmd::Create(U32 type)
{
	return new C_XtDbCmd(type);
}

C_Syntax* C_XtDbCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();

	{
		C_Argument arg(
			'u', 
			"unique",
			"enforce unique serialisable ids",
			C_Argument::E_Required_False);
		
		VarTypeArray& dataTypes = arg.GetDataTypes();
		dataTypes.PushBack(Util::E_VarType_Boolean);

		pSyntax->AddArg(arg);
	}

	{
		C_Argument arg(
			'o', 
			"open",
			"opens xt database",
			C_Argument::E_Required_False);
		
		VarTypeArray& dataTypes = arg.GetDataTypes();
		dataTypes.PushBack(Util::E_VarType_String);

		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			's', 
			"save",
			"saves xt database",
			C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			'c', 
			"close",
			"closes xt database",
			C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		C_Argument arg(
			'e', 
			"export",
			"exports xt database",
			C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}

	return pSyntax;
}

const char* C_XtDbCmd::GetName()
{
	static char* p = "xtdb";
	return p;
}

}
