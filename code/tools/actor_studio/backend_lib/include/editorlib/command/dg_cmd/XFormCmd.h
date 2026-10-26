#ifndef EDITOR_LIB_XFORM_CMD_H
#define EDITOR_LIB_XFORM_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

class C_XFormCmd : public I_Command
{
public:
	C_XFormCmd(U32 type);
	virtual ~C_XFormCmd();

	virtual C_Status Do();
	virtual bool IsUndoable() const;

	virtual void GetResults(ResultsArray& results) const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();

private:
	enum E_Action
	{
		E_Action_Unknown,
		E_Action_Scale,
		E_Action_Rotate,
		E_Action_Shear,
		E_Action_Translate,
		E_Action_Matrix,
	};

	bool IsInteresting(I_DgNode* pNode);
};

}

#endif