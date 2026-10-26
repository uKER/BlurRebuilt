#pragma once

#ifndef EDITOR_LIB_UNDO_CMD_H
#define EDITOR_LIB_UNDO_CMD_H

namespace EditorLib
{

class C_UndoCmd : public I_Command
{
public:
	C_UndoCmd(U32 type);
	virtual ~C_UndoCmd();

	virtual C_Status Do();
	virtual bool IsUndoable() const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();
};

}

#endif