#ifndef EDITOR_VISIBILITY_CMD_H
#define EDITOR_VISIBILITY_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{

class C_VisibilityCmd : public EditorLib::I_Command
{
public:
	C_VisibilityCmd(U32 type);
	virtual ~C_VisibilityCmd();

	virtual EditorLib::C_Status Do();
	virtual bool IsUndoable() const;

	virtual void GetResults(EditorLib::ResultsArray& results) const;

	static EditorLib::I_Command*		Create(U32 type);
	static EditorLib::C_Syntax*		CreateSyntax();
	static const char*		GetName();

private:
	bool IsInteresting(EditorLib::I_DgNode* pNode);
	EditorLib::ResultsArray			m_results;
};

}

#endif