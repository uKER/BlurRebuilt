#ifndef EDITOR_LIB_XTDBCMD_H
#define EDITOR_LIB_XTDBCMD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

class C_XtDbCmd : public I_Command
{
public:
	C_XtDbCmd(U32 type);
	virtual ~C_XtDbCmd();

	virtual C_Status Do();

	virtual bool IsUndoable() const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();
};

}

#endif