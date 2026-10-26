#pragma once

#ifndef EDITOR_LIB_REDO_CMD_H
#define EDITOR_LIB_REDO_CMD_H

namespace EditorLib
{

class C_RedoCmd : public I_Command
{
public:
	C_RedoCmd(U32 type);
	virtual ~C_RedoCmd();

	virtual C_Status Do();
	virtual bool IsUndoable() const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();
};

}

#endif