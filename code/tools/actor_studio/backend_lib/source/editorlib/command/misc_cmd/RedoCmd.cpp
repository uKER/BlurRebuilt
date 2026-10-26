#include "stdafx.h"
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Misc_Cmd/RedoCmd.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

C_RedoCmd::C_RedoCmd(U32 type) : I_Command(type)
{
}

C_RedoCmd::~C_RedoCmd()
{
}

C_Status C_RedoCmd::Do()
{
	return C_Core::Instance().GetCommandProcessor().Redo();
}

bool C_RedoCmd::IsUndoable() const
{
	return false;
}

I_Command* C_RedoCmd::Create(U32 type)
{
	return new C_RedoCmd(type);
}

C_Syntax* C_RedoCmd::CreateSyntax()
{
	return 0;
}

const char* C_RedoCmd::GetName()
{
	const char* p = "redo";
	return p;
}

}
