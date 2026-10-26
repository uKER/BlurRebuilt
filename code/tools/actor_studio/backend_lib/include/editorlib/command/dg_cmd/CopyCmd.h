#ifndef EDITOR_LIB_COPY_CMD_H
#define EDITOR_LIB_COPY_CMD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Command/ICommand.h>

namespace EditorLib
{

class C_CopyCmd : public I_Command
{
public:
	C_CopyCmd(U32 type);
	virtual ~C_CopyCmd();

	virtual C_Status Do();
	virtual bool IsUndoable() const;

	virtual void GetResults(ResultsArray& results) const;

	static I_Command*	Create(U32 type);
	static C_Syntax*	CreateSyntax();
	static const char*	GetName();
};

}

#endif