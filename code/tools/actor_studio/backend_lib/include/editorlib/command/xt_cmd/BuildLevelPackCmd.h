#ifndef EDITOR_LIB_BUILD_LEVEL_PACK_CMD_H
#define EDITOR_LIB_BUILD_LEVEL_PACK_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Command/ICommand.h>

namespace EditorLib
{

class C_BuildLevelPackCmd : public I_Command
{
public:
	C_BuildLevelPackCmd(U32 type);
	virtual ~C_BuildLevelPackCmd();

	virtual C_Status Do();

	virtual bool IsUndoable() const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();
};

}

#endif