#include "stdafx.h"
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Misc_Cmd/UndoCmd.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

C_UndoCmd::C_UndoCmd(U32 type) : I_Command(type)
{
}

C_UndoCmd::~C_UndoCmd()
{
}

C_Status C_UndoCmd::Do()
{
	return C_Core::Instance().GetCommandProcessor().Undo();
}

bool C_UndoCmd::IsUndoable() const
{
	return false;
}

I_Command* C_UndoCmd::Create(U32 type)
{
	return new C_UndoCmd(type);
}

C_Syntax* C_UndoCmd::CreateSyntax()
{
	return 0;
}

const char* C_UndoCmd::GetName()
{
	const char* p = "undo";
	return p;
}

}
