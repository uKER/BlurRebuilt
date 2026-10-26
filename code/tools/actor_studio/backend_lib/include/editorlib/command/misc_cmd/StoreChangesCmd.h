#pragma once

#ifndef EDITOR_LIB_STORE_CHANGES_CMD_H
#define EDITOR_LIB_STORE_CHANGES_CMD_H

namespace EditorLib
{

class C_StoreChangesCmd : public I_Command
{
public:
	C_StoreChangesCmd(U32 type);
	virtual ~C_StoreChangesCmd();

	virtual C_Status Do();
	virtual bool IsUndoable() const;
	virtual bool CanMerge() const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();
};

}

#endif