#ifndef EDITOR_LIB_DUPLICATE_CMD_H
#define EDITOR_LIB_DUPLICATE_CMD_H

#ifdef _MSC_VER
#pragma once
#endif

namespace EditorLib
{

class C_DuplicateCmd : public I_Command
{
public:
	C_DuplicateCmd (U32 type);
	virtual ~C_DuplicateCmd();

	virtual C_Status Do();
	virtual bool IsUndoable() const;

	virtual void GetResults(ResultsArray& results) const;

	static I_Command*	Create(U32 type);
	static C_Syntax*	CreateSyntax();
	static const char*	GetName();
};

}

#endif