#pragma once

#ifndef EDITOR_LIB_QUIT_CMD_H
#define EDITOR_LIB_QUIT_CMD_H

namespace EditorLib
{

class C_QuitCmd : public I_Command
{
public:
	C_QuitCmd(U32 type);
	virtual ~C_QuitCmd();

	virtual C_Status Do();

	virtual bool IsUndoable() const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();
};

}

#endif