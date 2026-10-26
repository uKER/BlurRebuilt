#include "stdafx.h"

#include <System/Shell/ShellExec.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/Xt_Cmd/BuildLevelPackCmd.h>

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Xt/Export/XtBuildLevelPack.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>

#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

C_BuildLevelPackCmd::C_BuildLevelPackCmd(U32 type) : I_Command(type)
{
}

C_BuildLevelPackCmd::~C_BuildLevelPackCmd()
{
}

C_Status C_BuildLevelPackCmd::Do()
{
	EditorLib::C_XtLevel* pLevel = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel();
	if (!pLevel)
		return E_Status_Failure;

	EditorLib::C_Status status = EditorLib::C_Core::Instance().ExportLevel();
	if (!status)
	{
		LOGERROR(C_Core::Instance().GetLogger(), "Xt data export failed");
		return E_Status_Failure;
	}

	return E_Status_Failure;
}

bool C_BuildLevelPackCmd::IsUndoable() const
{
	return false;
}

I_Command* C_BuildLevelPackCmd::Create(U32 type)
{
	return new C_BuildLevelPackCmd(type);
}

C_Syntax* C_BuildLevelPackCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		C_Argument arg(
			't', 
			"target",
			"copy to target",
			C_Argument::E_Required_False);
		
		pSyntax->AddArg(arg);
	}
	return pSyntax;
}

const char* C_BuildLevelPackCmd::GetName()
{
	static char* p = "buildLevelPack";
	return p;
}

}
