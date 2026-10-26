#include "stdafx.h"
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Misc_Cmd/QuitCmd.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

C_QuitCmd::C_QuitCmd(U32 type) : I_Command(type)
{
}

C_QuitCmd::~C_QuitCmd()
{
}

C_Status C_QuitCmd::Do()
{
	//Extract windows message information.
	//Post a message to the hosting application.
	C_Core::Instance().Quit();
	return E_Status_Success;
}

bool C_QuitCmd::IsUndoable() const
{
	return false;
}

I_Command* C_QuitCmd::Create(U32 type)
{
	return new C_QuitCmd(type);
}

C_Syntax* C_QuitCmd::CreateSyntax()
{
	return new C_Syntax();
}

const char* C_QuitCmd::GetName()
{
	const char* p = "quit";
	return p;
}

}
