#ifndef EDITOR_LIB_CREATE_XT_NODE_CMD_H
#define EDITOR_LIB_CREATE_XT_NODE_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Command/ICommand.h>

namespace EditorLib
{

class C_CreateXtNodeCmdContext
{
public:
	bool RegisterPostCreate(Btl::String postCreate, Ut::Delegate<void(EditorLib::I_DgNode*)> postCreateDelegate);
	Ut::Delegate<void(EditorLib::I_DgNode*)>* GetPostCreate(Btl::String postCreate);

private:
	Btl::List<Btl::Pair<Btl::String, Ut::Delegate<void(EditorLib::I_DgNode*)> > > m_PostCreate;
};

class C_CreateXtNodeCmd : public I_Command
{
public:
	C_CreateXtNodeCmd(U32 type);
	virtual ~C_CreateXtNodeCmd();

	virtual C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(ResultsArray& results) const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();

	static C_CreateXtNodeCmdContext* GetContext();

private:
	struct C_Results
	{
		C_NodeId nodeId;
	};

	C_Results m_results;
};

}

#endif