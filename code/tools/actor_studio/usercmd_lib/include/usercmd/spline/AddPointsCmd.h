#ifndef EDITOR_USERCMD_SPLINE_ADD_POINTS_CMD_H
#define EDITOR_USERCMD_SPLINE_ADD_POINTS_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Misc/Status.h>

namespace EditorUserCmd
{

class C_SplineAddPointsCmd : public EditorLib::I_Command
{
public:
	C_SplineAddPointsCmd(U32 type);
	virtual ~C_SplineAddPointsCmd();

	virtual EditorLib::C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(EditorLib::ResultsArray& results) const;

	static EditorLib::I_Command*		Create(U32 type);
	static EditorLib::C_Syntax*			CreateSyntax();
	static const char*					GetName();
};

}

#endif