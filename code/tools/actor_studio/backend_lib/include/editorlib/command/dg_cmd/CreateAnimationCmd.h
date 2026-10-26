#ifndef EDITOR_LIB_CREATE_ANIMATION_CMD_H
#define EDITOR_LIB_CREATE_ANIMATION_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

class C_CreateAnimationCmd : public I_Command
{
public:
	C_CreateAnimationCmd(U32 type);
	virtual ~C_CreateAnimationCmd();

	virtual C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(ResultsArray& results) const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();

private:
	struct C_Results
	{
		C_NodeId nodeId;
	};

	C_Results				m_results;
};

}

#endif