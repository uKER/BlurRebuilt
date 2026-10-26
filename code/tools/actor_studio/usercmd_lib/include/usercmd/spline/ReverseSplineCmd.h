#ifndef EDITOR_USERCMD_REVERSE_SPLINE_CMD_H
#define EDITOR_USERCMD_REVERSE_SPLINE_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorUserCmd
{

class C_ReverseSplineCmd : public EditorLib::I_Command
{
public:
	C_ReverseSplineCmd(U32 type);
	virtual ~C_ReverseSplineCmd();

	virtual EditorLib::C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(EditorLib::ResultsArray& results) const;

	static EditorLib::I_Command*		Create(U32 type);
	static EditorLib::C_Syntax*			CreateSyntax();
	static const char*					GetName();
};

}

#endif