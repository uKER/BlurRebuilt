#include "stdafx.h"
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Misc_Cmd/StoreChangesCmd.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

C_StoreChangesCmd::C_StoreChangesCmd(U32 type) : I_Command(type)
{
}

C_StoreChangesCmd::~C_StoreChangesCmd()
{
}

C_Status C_StoreChangesCmd::Do()
{
	C_Change* pChange = C_Core::Instance().GetUndoManager().GetExternalChange();
	assert(pChange);
	m_pChange = pChange->Clone();
	if (!m_pChange)
		return E_Status_Failure;
	return E_Status_Success;
}

bool C_StoreChangesCmd::IsUndoable() const
{
	return true;
}

bool C_StoreChangesCmd::CanMerge() const
{
	return true;
}

I_Command* C_StoreChangesCmd::Create(U32 type)
{
	return new C_StoreChangesCmd(type);
}

C_Syntax* C_StoreChangesCmd::CreateSyntax()
{
	return 0;
}

const char* C_StoreChangesCmd::GetName()
{
	const char* p = "storeChanges";
	return p;
}

}
