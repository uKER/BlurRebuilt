#ifndef EDITOR_LIB_COMMAND_PROCESSOR_H
#define EDITOR_LIB_COMMAND_PROCESSOR_H

#pragma once

#include <btl/utility/string.h>

#include <Misc_Shared/Variant/Variant.h>
#include <Misc_Shared/Variant/KeyValue.h>
#include <EditorLib/Misc/Status.h>

namespace EditorLib
{

class I_Command;

typedef Btl::Vector<Util::C_KeyValue> ResultsArray;

class C_CommandProcessor
{
public:
	C_CommandProcessor();
	~C_CommandProcessor();

	C_Status Execute(
		const Btl::String& command,
		I_Command** ppCommand = 0);
	C_Status Execute(
		const Btl::String& command, 
		ResultsArray& results, 
		I_Command** ppCommand = 0);
	C_Status Execute(
		const Btl::String& command, 
		const Btl::String& args, 
		I_Command** ppCommand = 0);
	C_Status Execute(
		const Btl::String& command,
		const Btl::String& args, 
		ResultsArray& results, 
		I_Command** ppCommand = 0);

	C_Status Undo();
	C_Status Redo();

	bool CanUndo() const;
	bool CanRedo() const;

	void ClearUndoHistory();

private:
	void AddToHistory(I_Command* pCommand, const Btl::String& name);
	void TrimUndoHistory(U32 nItems);
	void DebugPrint();

	enum E_IteratorState
	{
		E_IteratorState_Front,
		E_IteratorState_Back,
		E_IteratorState_Middle
	};

	class C_History : public Btl::ListElementDouble<C_History>
	{
	public:
		C_History();
		C_History(const C_History& rhs);
		~C_History();
		const C_History& operator=(const C_History& rhs);
		void Destroy();

		I_Command* pCommand;
		Btl::String name;
	};

	typedef Btl::List<C_History>	CommandArray;
	CommandArray					m_undoHistory;
	C_History*						m_pPreviousCommand;

	U32								m_undoHistorySize;
	static bool						m_isExecuting;
};

}

#endif
