#ifndef EDITOR_LIB_RENAME_CMD_H
#define EDITOR_LIB_RENAME_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

class C_RenameCmd : public I_Command
{
public:
	C_RenameCmd(U32 type);
	virtual ~C_RenameCmd();

	virtual C_Status Do();

	virtual bool IsUndoable() const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();
};

}

#endif