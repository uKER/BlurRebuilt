#ifndef EDITOR_LIB_CREATE_GROUP_CMD_H
#define EDITOR_LIB_CREATE_GROUP_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

class C_CreateGroupCmd : public I_Command
{
public:
	C_CreateGroupCmd(U32 type);
	virtual ~C_CreateGroupCmd();

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