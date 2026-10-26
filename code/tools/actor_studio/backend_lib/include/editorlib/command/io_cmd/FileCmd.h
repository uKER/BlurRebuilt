#ifndef EDITOR_LIB_LEVEL_FILE_H
#define EDITOR_LIB_LEVEL_FILE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

class C_FileCmd : public I_Command
{
public:
	C_FileCmd(U32 type);
	virtual ~C_FileCmd();

	virtual C_Status Do();

	virtual bool IsUndoable() const;

	virtual void GetResults(ResultsArray& results) const;

	static I_Command*		Create(U32 type);
	static C_Syntax*		CreateSyntax();
	static const char*		GetName();

private:
	enum E_LastCmd
	{
		E_LastCmd_New,
		E_LastCmd_Open,
		E_LastCmd_Save,
		E_LastCmd_Export,
		E_LastCmd_Close
	};

	struct C_NewResults
	{
		XtId recordId;
	};

	struct Results
	{
		C_NewResults m_new;
	};
	E_LastCmd m_lastCommand;
	Results m_results;
};

}

#endif