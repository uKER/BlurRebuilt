#ifndef EDITOR_LIB_SELECT_CMD_H
#define EDITOR_LIB_SELECT_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

class C_SelectCmd : public I_Command
{
public:
	C_SelectCmd(U32 type);
	virtual ~C_SelectCmd();

	virtual C_Status Do();
	virtual bool IsUndoable() const;

	virtual void GetResults(ResultsArray& results) const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();

private:
	bool IsInteresting(I_DgNode* pNode);
};

}

#endif