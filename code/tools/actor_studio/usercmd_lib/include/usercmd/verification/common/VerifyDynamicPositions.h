#ifndef EDITOR_USERCMD_VERIFY_DYNAMIC_POSITIONS_CMD_H
#define EDITOR_USERCMD_VERIFY_DYNAMIC_POSITIONS_CMD_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Misc/Status.h>

namespace EditorUserCmd
{

class C_VerifyDynamicPositionsCmd : public EditorLib::I_Command
{
public:
	C_VerifyDynamicPositionsCmd(U32 type);
	virtual ~C_VerifyDynamicPositionsCmd();

	virtual EditorLib::C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(EditorLib::ResultsArray& results) const;

	static EditorLib::I_Command*		Create(U32 type);
	static EditorLib::C_Syntax*			CreateSyntax();
	static const char*					GetName();

private:
	EditorLib::C_Status Verify(Xt::DomElement* pElement);
	EditorLib::C_Status VerifyRecord(Xt::DomRecord* pRecord);
};

}

#endif