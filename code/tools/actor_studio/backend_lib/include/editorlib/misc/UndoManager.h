#ifndef EDITOR_LIB_UNDO_MANAGER_H
#define EDITOR_LIB_UNDO_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <sigslot/sigslot.h>

namespace EditorLib
{

//Store all undoable non xt state info here.
struct C_NonXtState
{
	C_NonXtState();
	C_NonXtState(const C_NonXtState& rhs);
	~C_NonXtState();

	const C_NonXtState& operator=(const C_NonXtState& rhs);

	bool operator==(const C_NonXtState& rhs) const;

	bool operator!=(const C_NonXtState& rhs) const;

	void Clear();

	C_Clipboard*	pClipboard;
	C_Selection*	pSelection;
};

//We use this to take a snapshot of our current state.
struct C_Snapshot
{
	C_Snapshot();
	~C_Snapshot();

	C_NonXtState	nonXtState;
	Xt::DomComponent*	pXtState;

private:
	const C_Snapshot& operator=(const C_Snapshot& rhs);
};

//Delta values used to roll back changes.
struct C_Change
{
	C_Change(
		const C_NonXtState& nonXtStateUndo, 
		const C_NonXtState& nonXtStateRedo, 
		Xt::Change* pXtUndo, 
		Xt::Change* pXtRedo, 
		const char* name);
	~C_Change();

	C_Change* Clone() const;

	C_NonXtState	nonXtStateUndo;
	C_NonXtState	nonXtStateRedo;
	Xt::Change*		pXtUndo;
	Xt::Change*		pXtRedo;
	Btl::String		name;

private:
	C_Change();
	C_Change(const C_Change& rhs);
	const C_Change& operator=(const C_Change& rhs);
};

//Used to set checkpoint when popping off stack.
class C_CheckpointHelper
{
public:
	C_CheckpointHelper(C_Change** ppChange, const char* name, bool doXtDiff = true);
	~C_CheckpointHelper();

private:
	C_Change** m_ppChange;
	Btl::String m_name;
};

class C_CheckpointDisabler
{
public:
	C_CheckpointDisabler();
	~C_CheckpointDisabler();
};

class C_ChangeCommandDisabler
{
public:
	C_ChangeCommandDisabler();
	~C_ChangeCommandDisabler();
};

class C_UndoManager
{
public:
	C_UndoManager();
	~C_UndoManager();

	void OnLevelEvent(const I_Level* level, E_LevelEvent evt);

private:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		void Connect();
		void Disconnect();

		void OnLevelEventSlot(const I_Level* level, E_LevelEvent evt);

		FRIEND_CLASS C_UndoManager;
	};

	class C_CheckpointStack
	{
	public:
		C_CheckpointStack();
		~C_CheckpointStack();

		void Push(const char* contextName, bool doXtDiff);
		void Pop(const char* contextName);

		U32	GetStackDepth() const;

		bool DoXtDiff() const;

	private:
		struct C_Context
		{
			Btl::String name;
			bool doXtDiff;
		};
		Btl::Vector<C_Context>	m_stack;
	};

	static void StaticOnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent, 
			C_UndoManager* pThis);

public:
	C_Change* GetExternalChange();
	void PushUserCheckpoint(const char* contextName, bool doXtDiff);
	void PopUserCheckpoint(C_Change** ppChange, const char* contextName);
	void Patch(const C_Change& change, bool redo);
	bool IsCurrentSnapshotDirty() const;

	void SetClipboard(const C_Clipboard& clipboard);
	void SetSelection(const C_Selection& selection);
	void GetClipboard(C_Clipboard& clipboard) const;
	void GetSelection(C_Selection& selection) const;

private:
	void EnableChangeCommands(bool flag);
	void EnableCheckpoints(bool flag);

	void FullCheckpoint(Xt::DomRecord* pRecord);
	void IncrementalCheckpoint(Xt::DomRecord* pRecord, Xt::Change* pChange);
	void StoreChanges();

	bool					m_currentSnapshotDirty;
	bool					m_checkpointPending;
	S32						m_disableChangeCommands;

	C_NonXtState			m_nonXtState;

	C_CheckpointStack		m_userCheckpointStack;
	Xt::DomMonitor			m_mainRecordMonitor;
	C_Snapshot*				m_pCurrentSnapshot;
	C_Change*				m_pExternalChange;

	C_Slots m_slots;

	FRIEND_CLASS C_Core;
	FRIEND_CLASS C_CheckpointDisabler;
	FRIEND_CLASS C_ChangeCommandDisabler;
	FRIEND_CLASS C_CommandProcessor;
	FRIEND_CLASS C_LevelManager;
};

}

#endif