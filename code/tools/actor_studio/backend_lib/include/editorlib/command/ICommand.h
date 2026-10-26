#pragma once

#ifndef EDITOR_LIB_I_COMMAND_H
#define EDITOR_LIB_I_COMMAND_H

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/CommandProcessor.h>

namespace EditorLib
{

class I_Command
{
public:
	I_Command(U32 type);
	virtual ~I_Command();

	void SetArguments(const C_ArgumentDatabase& args);

	U32 GetType() const;

	bool Merge(const I_Command& lhs) const;

	C_Status Undo();

	C_Status Redo();

	virtual C_Status Do() = 0;
	
	virtual bool IsUndoable() const = 0;

	virtual void GetResults(ResultsArray& results) const {}

	virtual bool CanMerge() const { return false; }

	C_Change* GetChange() { return m_pChange; }

protected:
	void SetType(U32 type) { m_type = type; }
	C_ArgumentDatabase		m_args;
	U32						m_type;
	C_Change*				m_pChange;
};

}

#endif
