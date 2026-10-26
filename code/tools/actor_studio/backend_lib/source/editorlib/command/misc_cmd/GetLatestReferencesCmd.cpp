#include "stdafx.h"
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Misc_Cmd/GetLatestReferencesCmd.h>

#include <EditorLib/Aftk/AftkRefHelpers.h>

namespace EditorLib
{

C_GetLatestReferencesCmd::C_GetLatestReferencesCmd(U32 type) : I_Command(type)
{
}

C_GetLatestReferencesCmd::~C_GetLatestReferencesCmd()
{
}

C_Status C_GetLatestReferencesCmd::Do()
{
	if (!AftkRefHelpers::GetLatestReferences(0))
		return E_Status_Failure;

	//TODO: Move ref refreshing out.
	if (!AftkRefHelpers::ReloadReferences(true, true, 0))
		return E_Status_Failure;

	return E_Status_Success;
}

bool C_GetLatestReferencesCmd::IsUndoable() const
{
	return false;
}

I_Command* C_GetLatestReferencesCmd::Create(U32 type)
{
	return new C_GetLatestReferencesCmd(type);
}

C_Syntax* C_GetLatestReferencesCmd::CreateSyntax()
{
	return new C_Syntax();
}

const char* C_GetLatestReferencesCmd::GetName()
{
	const char* p = "getLatestReferences";
	return p;
}

}
