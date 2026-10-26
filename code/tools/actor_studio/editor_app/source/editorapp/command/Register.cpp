#include "stdafx.h"
#include <Misc_Shared/Variant/KeyValue.h>

#include <EditorLib/Misc/Status.h>

#include <EditorLib/Core.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/CommandFactory.h>

#include <EditorApp/Command/Register.h>
#include <EditorApp/Command/Visibility/VisibilityCmd.h>

namespace Editor
{

C_Register::C_Register()
{
}

C_Register::~C_Register()
{
}

EditorLib::C_Status C_Register::Initialise(const Btl::String& projectName)
{
	EditorLib::C_Status status;

	status = EditorLib::C_Core::Instance().GetCommandFactory().Register<C_VisibilityCmd>();
	
	if (!status) return status;

	return EditorLib::E_Status_Success;
}

void C_Register::Shutdown()
{
}

}
