#ifndef EDITOR_LIB_PARENT_CMD_H
#define EDITOR_LIB_PARENT_CMD_H

#ifdef _MSC_VER
#pragma once
#endif

namespace EditorLib
{

class C_ParentCmd : public I_Command
{
public:
	C_ParentCmd(U32 type);
	virtual ~C_ParentCmd();

	virtual C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(ResultsArray& results) const;

	static I_Command*	Create(U32 type);
	static C_Syntax*	CreateSyntax();
	static const char*	GetName();

private:

};

}

#endif