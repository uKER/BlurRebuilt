#ifndef EDITOR_LIB_GETLATESTREFERENCES_CMD_H
#define EDITOR_LIB_GETLATESTREFERENCES_CMD_H

#ifdef _MSC_VER
#pragma once
#endif


namespace EditorLib
{

class C_GetLatestReferencesCmd : public I_Command
{
public:
	C_GetLatestReferencesCmd(U32 type);
	virtual ~C_GetLatestReferencesCmd();

	virtual C_Status Do();

	virtual bool IsUndoable() const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();
};

}

#endif