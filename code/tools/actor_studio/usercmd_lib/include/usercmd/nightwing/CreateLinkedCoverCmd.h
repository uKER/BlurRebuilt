#ifndef EDITOR_USERCMD_CREATE_LINKED_COVER_CMD_H
#define EDITOR_USERCMD_CREATE_LINKED_COVER_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Misc/Status.h>

namespace EditorUserCmd
{

class C_CreateLinkedCoverCmd : public EditorLib::I_Command
{
public:
	C_CreateLinkedCoverCmd(U32 type);
	virtual ~C_CreateLinkedCoverCmd();

	virtual EditorLib::C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(EditorLib::ResultsArray& results) const;

	static EditorLib::I_Command*		Create(U32 type);
	static EditorLib::C_Syntax*		CreateSyntax();
	static const char*		GetName();
};

}

#endif