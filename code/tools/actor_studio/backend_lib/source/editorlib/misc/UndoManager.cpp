#include "stdafx.h"

#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Misc/Clipboard.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

namespace EditorLib
{

C_NonXtState::C_NonXtState()
{
	pClipboard = new C_Clipboard;
	pSelection = new C_Selection;
}

C_NonXtState::C_NonXtState(const C_NonXtState& rhs)
{
	pClipboard = new C_Clipboard;
	pSelection = new C_Selection;

	*this = rhs;
}

C_NonXtState::~C_NonXtState()
{
	delete pClipboard;
	delete pSelection;
}

const C_NonXtState& C_NonXtState::operator=(const C_NonXtState& rhs)
{
	if (&rhs == this)
		return *this;

	*pClipboard = *rhs.pClipboard;
	*pSelection = *rhs.pSelection;

	return *this;
}

bool C_NonXtState::operator==(const C_NonXtState& rhs) const
{
	assert(pClipboard && pSelection && rhs.pClipboard && rhs.pSelection);
	return *pClipboard == *rhs.pClipboard && *pSelection == *rhs.pSelection;
}

bool C_NonXtState::operator!=(const C_NonXtState& rhs) const
{
	return !(*this==rhs);
}

void C_NonXtState::Clear()
{
	pClipboard->Clear();
	pSelection->Clear();
}


C_Snapshot::C_Snapshot()
{
	pXtState = 0;
}

C_Snapshot::~C_Snapshot()
{
	delete pXtState;
}



C_Change::C_Change(
	const C_NonXtState& _nonXtStateUndo,
	const C_NonXtState& _nonXtStateRedo,
	Xt::Change* _pXtUndo, 
	Xt::Change* _pXtRedo, 
	const char* _name)
{
	nonXtStateUndo = _nonXtStateUndo;
	nonXtStateRedo = _nonXtStateRedo;
	pXtUndo = _pXtUndo;
	pXtRedo = _pXtRedo;
	name = _name;
}

C_Change::C_Change()
{
}

C_Change* C_Change::Clone() const
{
	C_Change* pNew = new C_Change;
	pNew->nonXtStateUndo = nonXtStateUndo;
	pNew->nonXtStateRedo = nonXtStateRedo;
	pNew->pXtUndo = pXtUndo->Clone();
	pNew->pXtRedo = pXtRedo->Clone();
	pNew->name = name;
	return pNew;
}

C_Change::~C_Change()
{
	delete pXtUndo;
	delete pXtRedo;
}

C_CheckpointHelper::C_CheckpointHelper(C_Change** ppChange, const char* name, bool doXtDiff)
{
	m_ppChange = ppChange;
	if (m_ppChange)
	{
		m_name = name;
		C_Core::Instance().GetUndoManager().PushUserCheckpoint(name, doXtDiff);
	}
}

C_CheckpointHelper::~C_CheckpointHelper()
{
	if (m_ppChange)
	{
		C_Core::Instance().GetUndoManager().PopUserCheckpoint(m_ppChange, m_name.Chars());
	}
}



C_ChangeCommandDisabler::C_ChangeCommandDisabler()
{
	C_Core::Instance().GetUndoManager().EnableChangeCommands(false);
}

C_ChangeCommandDisabler::~C_ChangeCommandDisabler()
{
	C_Core::Instance().GetUndoManager().EnableChangeCommands(true);
}



C_CheckpointDisabler::C_CheckpointDisabler()
{
	C_Core::Instance().GetUndoManager().EnableCheckpoints(false);
}

C_CheckpointDisabler::~C_CheckpointDisabler()
{
	C_Core::Instance().GetUndoManager().EnableCheckpoints(true);
}

C_UndoManager::C_UndoManager()
{
	m_slots.Connect();

	m_currentSnapshotDirty = false;
	m_disableChangeCommands = 0;

	m_mainRecordMonitor.SetCallback(&StaticOnXtDomMonitorEvent, this);
	m_pCurrentSnapshot = 0;
	m_pExternalChange = 0;
}

C_UndoManager::~C_UndoManager()
{
	m_slots.Disconnect();

	BIZ_ASSERT(!m_pCurrentSnapshot);
	m_currentSnapshotDirty = false;
}

void C_UndoManager::OnLevelEvent(const I_Level* level, E_LevelEvent evt)
{
	if (evt == E_LevelEvent_ActiveLevelChanging)
	{
		if (level)
		{
			Xt::Monitor(&m_mainRecordMonitor, 0);
			EditorLib::C_Core::Instance().GetCommandProcessor().ClearUndoHistory();
			m_nonXtState.Clear();
			SAFE_DELETE_PTR(m_pCurrentSnapshot);
			SAFE_DELETE_PTR(m_pExternalChange);
			m_currentSnapshotDirty = false;
		}
	} else if (evt == E_LevelEvent_ActiveLevelChanged)
	{
		if (level)
		{
			BIZ_ASSERT(level->GetType() == E_LevelType_XtLevel);
			const C_XtLevel* xtLevel = (const C_XtLevel*)level;

			//Enable undo.
			XtId recordId = xtLevel->GetXtRecordId();
			Xt::DomRecord* pRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(recordId);
			EditorLib::C_Core::Instance().GetUndoManager().FullCheckpoint(pRecord);
			Xt::Monitor(&m_mainRecordMonitor, &pRecord->root);
		}
	}
}

void C_UndoManager::EnableChangeCommands(bool flag)
{
	if (flag)
		m_disableChangeCommands--;
	else
		m_disableChangeCommands++;
	assert(m_disableChangeCommands >= 0);
}

void C_UndoManager::StaticOnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent, 
	C_UndoManager* pThis)
{
	if (pThis->m_disableChangeCommands == 0) pThis->m_currentSnapshotDirty = true;
}

void C_UndoManager::FullCheckpoint(Xt::DomRecord* pRecord)
{
	if (C_Core::Instance().GetLevelManager()->GetActiveLevel() && C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetFlags().pendingSync)
		return;

	//Create new xt snapshot.
	if (!m_pCurrentSnapshot)
		m_pCurrentSnapshot = new C_Snapshot;

	if (!m_pCurrentSnapshot->pXtState)
		m_pCurrentSnapshot->pXtState = new Xt::DomComponent;
	bool status = Xt::Clone(m_pCurrentSnapshot->pXtState, &pRecord->root);
	if (!status)
		LOGERROR(C_Core::Instance().GetLogger(), "Failed to create undo checkpoint!");
#ifdef BUILD_CONFIG_DEBUG
	else
		LOGDEBUG(C_Core::Instance().GetLogger(), "FullCheckpoint");
#endif
	m_pCurrentSnapshot->nonXtState = m_nonXtState;
	m_currentSnapshotDirty = false;
}

void C_UndoManager::IncrementalCheckpoint(Xt::DomRecord* pRecord, Xt::Change* pChange)
{
	if (pChange)
	{
		//Incrementally update snapshot via diff.
		bool status = Xt::Patch(m_pCurrentSnapshot->pXtState, pChange);
		if (!status)
		{
			LOGDEBUG(C_Core::Instance().GetLogger(), "Incremental checkpoint update failed.");
			FullCheckpoint(pRecord);

		}
//#ifdef BUILD_CONFIG_DEBUG
//		else
//			LOGDEBUG(GetLogger(), "Checkpoint via incremental update.");
//#endif
	}
	else
	{
//#ifdef BUILD_CONFIG_DEBUG
//		LOGDEBUG(GetLogger(), "No change list for incremental checkpoint.");
//#endif
	}

	m_pCurrentSnapshot->nonXtState = m_nonXtState;
	m_currentSnapshotDirty = false;
}

void C_UndoManager::StoreChanges()
{
	//If we're up to date, or if there's an active user checkpoint, abort.
	if ((C_Core::Instance().GetLevelManager()->GetActiveLevel() && C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetFlags().pendingSync) || m_disableChangeCommands > 0 || !m_currentSnapshotDirty || m_userCheckpointStack.GetStackDepth() > 0)
		return;

	//Should already be created.
	assert(m_pCurrentSnapshot);
	
	//Grab current state.
	Xt::DomRecord* pRecord = C_Core::Instance().GetXtManager().FindRecord(C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId()); 
	BIZ_ASSERT(pRecord); 

	//Need to reset snapshot when initialising level and after popping user undo stack.
	if (m_pCurrentSnapshot->pXtState)
	{
		Xt::Change* pDoChange = Xt::Diff(
			const_cast<Xt::DomComponent*>(m_pCurrentSnapshot->pXtState),
			const_cast<Xt::DomComponent*>(&pRecord->root));
		Xt::Change* pUndoChange = Xt::Diff(
			const_cast<Xt::DomComponent*>(&pRecord->root),
			const_cast<Xt::DomComponent*>(m_pCurrentSnapshot->pXtState));

		if (pUndoChange)
		{
			assert(pDoChange);
			const char* changeName = "Various changes";
			if (!m_pExternalChange)
				m_pExternalChange = new C_Change(
					m_pCurrentSnapshot->nonXtState, //undo
					m_nonXtState, //redo
					pUndoChange, //undo
					pDoChange, //redo
					changeName);
			else
			{
				m_pExternalChange->nonXtStateUndo = m_pCurrentSnapshot->nonXtState;
				m_pExternalChange->pXtUndo = pUndoChange;
				m_pExternalChange->pXtRedo = pDoChange;
				m_pExternalChange->name = changeName;
			}

			//Store existing snapshot.
			C_Status status = C_Core::Instance().GetCommandProcessor().Execute("storeChanges");
			assert(status);

			//Update snapshot to new state.
			IncrementalCheckpoint(pRecord, pDoChange);

			SAFE_DELETE_PTR(m_pExternalChange);
		}
		else
		{
			//Update snapshot to new state.
			IncrementalCheckpoint(pRecord, 0);
		}
	}
}

void C_UndoManager::PushUserCheckpoint(const char* contextName, bool doXtDiff)
{
	if (m_pCurrentSnapshot)
		m_userCheckpointStack.Push(contextName, doXtDiff);
}

void C_UndoManager::PopUserCheckpoint(C_Change** ppChange, const char* contextName)
{
	//If we're about to clear our change context, build a change list.
	if (m_userCheckpointStack.GetStackDepth() == 1 && C_Core::Instance().GetLevelManager()->GetActiveLevel())
	{
		//Grab current state.
		Xt::DomRecord* pRecord = C_Core::Instance().GetXtManager().FindRecord(C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId()); 
		assert(pRecord); 

		Xt::Change* pUndo = 0;
		Xt::Change* pRedo = 0;

		if (m_userCheckpointStack.DoXtDiff())
		{
			pUndo = Xt::Diff( //undo
				const_cast<Xt::DomComponent*>(&pRecord->root), 
				const_cast<Xt::DomComponent*>(m_pCurrentSnapshot->pXtState)),
			pRedo = Xt::Diff( //redo
				const_cast<Xt::DomComponent*>(m_pCurrentSnapshot->pXtState),
				const_cast<Xt::DomComponent*>(&pRecord->root));
		}

		//If we're not redoing
		if (!(*ppChange))
		{
			BIZ_ASSERT(m_pCurrentSnapshot);
			*ppChange = new C_Change(
				m_pCurrentSnapshot->nonXtState, //undo
				m_nonXtState, //redo
				pUndo,
				pRedo,
				contextName);
		}

		//Dirty our default snapshot so that it gets refreshed now that our user context has been 
		//popped off
		IncrementalCheckpoint(pRecord, (*ppChange)->pXtRedo);

	}
	m_userCheckpointStack.Pop(contextName);
}

void C_UndoManager::Patch(const C_Change& change, bool redo)
{
	Xt::DomRecord* pRecord = C_Core::Instance().GetXtManager().FindRecord(C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId()); 
	assert(pRecord); 
	if (redo)
	{
		Xt::Patch(&pRecord->root, change.pXtRedo);
		C_Core::Instance().SetClipboard(*change.nonXtStateRedo.pClipboard);
		C_Core::Instance().SetSelection(*change.nonXtStateRedo.pSelection);
	}
	else
	{
		Xt::Patch(&pRecord->root, change.pXtUndo);
		C_Core::Instance().SetClipboard(*change.nonXtStateUndo.pClipboard);
		C_Core::Instance().SetSelection(*change.nonXtStateUndo.pSelection);
	}

	C_Core::Instance().GetGlobalSignals().coreDirtyViews(E_DirtyEventType_Patch);
}

C_Change* C_UndoManager::GetExternalChange()
{
	return m_pExternalChange;
}

void C_UndoManager::EnableCheckpoints(bool flag)
{
	if (!C_Core::Instance().GetLevelManager()->GetActiveLevel() || C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetFlags().pendingSync)
		return;

	//About to begin period of disabled checkpoints.
	if (!flag && m_disableChangeCommands == 0)
	{
		//delete existing checkpoing
		SAFE_DELETE_PTR(m_pCurrentSnapshot);
	}
	//About to re-enable checkpoints after a period of them being disabled.
	else if (flag && m_disableChangeCommands == 1)
	{
		Xt::DomRecord* pLevelRecord = C_Core::Instance().GetXtManager().FindRecord(C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId());
		assert(pLevelRecord);
		FullCheckpoint(pLevelRecord);
	}

	EnableChangeCommands(flag);
}

bool C_UndoManager::IsCurrentSnapshotDirty() const
{
	return m_currentSnapshotDirty;
}

void C_UndoManager::SetClipboard(const C_Clipboard& clipboard)
{
	if (m_disableChangeCommands == 0) m_checkpointPending = true;
	*m_nonXtState.pClipboard = clipboard;
}

void C_UndoManager::SetSelection(const C_Selection& selection)
{
	*m_nonXtState.pSelection = selection;
	if (m_disableChangeCommands == 0) m_checkpointPending = true;
}

void C_UndoManager::GetClipboard(C_Clipboard& clipboard) const
{
	clipboard = *m_nonXtState.pClipboard;
}

void C_UndoManager::GetSelection(C_Selection& selection) const
{
	selection = *m_nonXtState.pSelection;
}

C_UndoManager::C_CheckpointStack::C_CheckpointStack()
{
}

C_UndoManager::C_CheckpointStack::~C_CheckpointStack()
{
}

void C_UndoManager::C_CheckpointStack::Push(
	const char* contextName,
	bool doXtDiff)
{
	m_stack.PushBack();
	m_stack.Back().name = contextName;
	m_stack.Back().doXtDiff = doXtDiff;

	//If at any point we find we need to do an xt diff, set this flag on the front item in our stack.
	if (m_stack.Size() > 1 && doXtDiff)
		m_stack[0].doXtDiff = true;
}

void C_UndoManager::C_CheckpointStack::Pop(const char* contextName)
{
	//Must have valid stack and matching push/pop pairs.
	assert(m_stack.Size() != 0 && !strcmp(m_stack.Back().name.Chars(), contextName));
	m_stack.PopBack();
}

U32	C_UndoManager::C_CheckpointStack::GetStackDepth() const
{
	return m_stack.Size();
}

bool C_UndoManager::C_CheckpointStack::DoXtDiff() const
{
	return m_stack.Size() > 0 && m_stack[0].doXtDiff;
}

void C_UndoManager::C_Slots::Connect()
{
	C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelEvent.connect(this, &C_UndoManager::C_Slots::OnLevelEventSlot);
}

void C_UndoManager::C_Slots::Disconnect()
{
	C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelEvent.disconnect(this);
}

void C_UndoManager::C_Slots::OnLevelEventSlot(const I_Level* level, E_LevelEvent evt)
{
	C_Core::Instance().GetUndoManager().OnLevelEvent(level, evt);
}

}