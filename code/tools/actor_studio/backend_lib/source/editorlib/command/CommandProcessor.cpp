#include "stdafx.h"

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/ArgumentParser.h>
#include <EditorLib/Command/CommandFactory.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>


namespace EditorLib
{

bool C_CommandProcessor::m_isExecuting = false;

C_CommandProcessor::C_History::C_History() 
{
}

C_CommandProcessor::C_History::C_History(const C_CommandProcessor::C_History& rhs)
{ 
	*this = rhs; 
}

C_CommandProcessor::C_History::~C_History()
{
	SAFE_DELETE_PTR(pCommand);
}

const C_CommandProcessor::C_History& C_CommandProcessor::C_History::operator=(const C_CommandProcessor::C_History& rhs)
{
	pCommand = rhs.pCommand;
	name = rhs.name;
	return *this;
}

void C_CommandProcessor::C_History::Destroy()
{
	delete this;	
}

C_CommandProcessor::C_CommandProcessor()
{
	m_undoHistorySize = 500;
	m_pPreviousCommand = 0;
}

C_CommandProcessor::~C_CommandProcessor()
{
	TrimUndoHistory(0);
	assert(m_undoHistory.Size() == 0);
}

C_Status C_CommandProcessor::Execute(const Btl::String& command, I_Command** ppCommand)
{
	Btl::String args;
	ResultsArray results;
	return Execute(command, args, results, ppCommand);
}

C_Status C_CommandProcessor::Execute(const Btl::String& command, ResultsArray& results, I_Command** ppCommand)
{
	Btl::String args;
	return Execute(command, args, results, ppCommand);
}

C_Status C_CommandProcessor::Execute(const Btl::String& command, const Btl::String& args, I_Command** ppCommand)
{
	ResultsArray results;
	return Execute(command, args, results, ppCommand);
}

C_Status C_CommandProcessor::Execute(const Btl::String& command, const Btl::String& args, ResultsArray& results, I_Command** ppCommand)
{
	C_ChangeCommandDisabler cd;
	C_ViewDisabler viewDisabler;

	bool isMacroCommand = ppCommand ? true : false;
	//Check mutex like flag to ensure that commands cannot simply call other commands from within one another.
	//This would have dire implications for undo/redo.
	if (m_isExecuting && !isMacroCommand)
	{
//		assert(!"Cannot call a command whilst another command is being executed unless using macro-command method!");
		return E_Status_Failure;
	}
	else if (!m_isExecuting && isMacroCommand)
	{
		assert(!"Cannot use macro-command version of execute if not invoking a command from within a command!");
		return E_Status_Failure;
	}

	//Right, off we go,
	m_isExecuting = true;

	C_CommandFactory& registry = C_Core::Instance().GetCommandFactory();
	I_Command* pCommand;
	C_Syntax* pSyntax;
	if (!registry.Create(pCommand, pSyntax, command))
	{
		m_isExecuting = false;
		return E_Status_Failure;
	}

	C_ArgumentDatabase argDb;

	C_Status status;
	C_ArgumentParser* pParser;
	if (pSyntax)
	{
		pParser = new C_ArgumentParser(pSyntax);
		status = pParser->Parse(argDb, args);
		if (!status)
		{
			LOGERROR(
					C_Core::Instance().GetLogger(), "Failed to execute %s with args %s", command.Chars(), args.Chars());
			m_isExecuting = false;
			return status;
		}
	}
	else
	{
		pParser = 0;
	}

	pCommand->SetArguments(argDb);

	status = pCommand->Do();
	pCommand->GetResults(results);

	//Now safe to apply pending updates.
	C_Core::Instance().GetDgManager().FlushDeferredUpdates();

	if (status)
	{
		if (args.Length() < 128)
			LOGINFO(C_Core::Instance().GetLogger(), "%s %s", command.Chars(), args.Chars());
		else
			LOGINFO(C_Core::Instance().GetLogger(), "%s ...", command.Chars());

		if (m_pPreviousCommand && pCommand->CanMerge() && pCommand->Merge(*m_pPreviousCommand->pCommand))
		{
			assert(pCommand->IsUndoable());
			CommandArray::Iterator deleteIt(m_pPreviousCommand);
			m_pPreviousCommand = m_pPreviousCommand->LE_Previous();
			m_undoHistory.Erase(deleteIt);
		}

		if (!pCommand->IsUndoable())
			//We've performed a non undoable action, delete the command immediately.
			delete pCommand;
		else 
		{
			if (!isMacroCommand)
				//We take ownership of undo/redo for this.
				AddToHistory(pCommand, command);
			else
				//Return our command to the macro-command that invoked us.
				*ppCommand = pCommand;
		}
	}
	else
	{
		delete pCommand;
	}
	delete pParser;
	//We're done.
	m_isExecuting = false;
	return status;
}

C_Status C_CommandProcessor::Undo()
{
	if (C_Core::Instance().GetUndoManager().IsCurrentSnapshotDirty())
		return E_Status_Failure;

	C_ChangeCommandDisabler cd;
	C_ViewDisabler viewDisabler;

	if (!CanUndo())
	{
		LOGINFO(C_Core::Instance().GetLogger(), "Cannot undo");
		return E_Status_Success;
	}

	LOGINFO(C_Core::Instance().GetLogger(), "Undoing %s", m_pPreviousCommand->name.Chars());
	C_Status status = m_pPreviousCommand->pCommand->Undo();
	assert(status);
	if (!status) 
	{
		return status;
	}

	//Update checkpoint.
	{
		Xt::DomRecord* pLevelRecord = 
			C_Core::Instance().GetXtManager().FindRecord(C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId());
		assert(pLevelRecord);

		C_Change* pChange = m_pPreviousCommand->pCommand->GetChange();
		if (pChange)
			C_Core::Instance().GetUndoManager().IncrementalCheckpoint(pLevelRecord, pChange->pXtUndo);
		else
			C_Core::Instance().GetUndoManager().IncrementalCheckpoint(pLevelRecord, 0);
	}

	///Move command history pointer back in time.
	m_pPreviousCommand = m_pPreviousCommand->LE_Previous();
	return status;
}

C_Status C_CommandProcessor::Redo()
{
	C_ChangeCommandDisabler cd;
	C_ViewDisabler viewDisabler;

	if (!CanRedo())
	{
		LOGINFO(C_Core::Instance().GetLogger(), "Cannot redo");
		return E_Status_Success;
	}

	///Move command history pointer forward in time.
	if (!m_pPreviousCommand)
	{
		CommandArray::Iterator it = m_undoHistory.Begin();
		assert(it != m_undoHistory.End());
		m_pPreviousCommand = &(*it);
	}
	else
	{
		m_pPreviousCommand = m_pPreviousCommand->LE_Next();
		assert(m_pPreviousCommand);
	}
	LOGINFO(C_Core::Instance().GetLogger(), "Redoing %s", m_pPreviousCommand->name.Chars());
	C_Status status = m_pPreviousCommand->pCommand->Redo();
	//Update checkpoint.
	{
		Xt::DomRecord* pLevelRecord = 
			C_Core::Instance().GetXtManager().FindRecord(C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId());
		assert(pLevelRecord);
		
		C_Change* pChange = m_pPreviousCommand->pCommand->GetChange();
		if (pChange)
			C_Core::Instance().GetUndoManager().IncrementalCheckpoint(pLevelRecord, pChange->pXtRedo);
		else
			C_Core::Instance().GetUndoManager().IncrementalCheckpoint(pLevelRecord, 0);
	}

#if defined(BUILD_CONFIG_DEBUG) && defined(USER_KMACLEOD)
	DebugPrint();
#endif
	return status;
}

bool C_CommandProcessor::CanUndo() const
{
	if (m_undoHistory.Size() != 0 && m_pPreviousCommand)
		return true;
	return false;
}

bool C_CommandProcessor::CanRedo() const
{
	//If we've undo history and we've a next command, or no previous command (are at front of list) then we can redo.
	if (m_undoHistory.Size() != 0 && 
		((m_pPreviousCommand && m_pPreviousCommand->LE_Next()) || !m_pPreviousCommand))
		return true;
	return false;
}

void C_CommandProcessor::ClearUndoHistory()
{
	m_pPreviousCommand = 0;
	m_undoHistory.Clear();
}

void C_CommandProcessor::AddToHistory(I_Command* pCommand, const Btl::String& name)
{
	//If we're at the end of our list.
	if (!CanRedo())
	{
		TrimUndoHistory(m_undoHistorySize-1);
		C_History* pH = new C_History;
		pH->pCommand = pCommand;
		pH->name = name;
		m_undoHistory.PushBack(*pH);
		m_pPreviousCommand = pH;

#if defined(BUILD_CONFIG_DEBUG) && defined(USER_KMACLEOD)
		DebugPrint();
#endif
		return;
	}
	else
	{
		//Work from our previous command to the end of the list, erasing commands that were launched
		//afterward.
		{
			if (!m_pPreviousCommand)
				//Remove entire history.
				m_undoHistory.Clear();
			else
			{
				CommandArray::Iterator it(m_pPreviousCommand); ++it;
				for (; it != m_undoHistory.End();)
					it = m_undoHistory.Erase(it);
				assert(m_undoHistory.Size()-1 <= (S32)m_undoHistorySize);
			}
		}

		C_History* pH = new C_History;
		pH->pCommand = pCommand;
		pH->name = name;
		m_undoHistory.PushBack(*pH);
		m_pPreviousCommand = pH;
		assert(m_undoHistory.Size() <= (S32)m_undoHistorySize);

#if defined(BUILD_CONFIG_DEBUG) && defined(USER_KMACLEOD)
		DebugPrint();
#endif
	}
}

void C_CommandProcessor::TrimUndoHistory(U32 nItems)
{
	if (m_undoHistory.Size() <= (S32)nItems)
		return;

	U32 nToTrim = m_undoHistory.Size() - nItems;
	CommandArray::Iterator it(m_undoHistory.Begin());
	for (U32 i = 0; i < nToTrim; ++i)
	{
		if (&(*it) == m_pPreviousCommand)
		{
			//If history pointer is off our clip range, clear history.
			m_undoHistory.Clear();
			m_pPreviousCommand = 0;
			return;
		}
		it = m_undoHistory.Erase(it);
	}
}

void C_CommandProcessor::DebugPrint()
{
	LOGINFO(C_Core::Instance().GetLogger(), "-={ begin undo list }=-}\n");
	for (CommandArray::Iterator it(m_undoHistory.Begin()); it != m_undoHistory.End(); ++it)
	{
		C_History* p = &(*it);
		if (p == m_pPreviousCommand)
		{
			LOGINFO(C_Core::Instance().GetLogger(), "prev->\t%x\t%s\n", p->pCommand, p->name.Chars());
		}
		else
		{
			LOGINFO(C_Core::Instance().GetLogger(), "\t%x\t%s\n", p->pCommand, p->name.Chars());
		}
	}
	LOGINFO(C_Core::Instance().GetLogger(), "-={ end undo list }=-}\n");
}

}
