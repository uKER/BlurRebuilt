#include "stdafx.h"

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Xt/XtMergeHelpers.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

I_Command::I_Command(U32 type)
{ 
	m_type = type;
	m_pChange = 0;
}
	
I_Command::~I_Command()
{
	delete m_pChange;
}

void I_Command::SetArguments(const C_ArgumentDatabase& args) 
{	
	m_args = args;
}

U32 I_Command::GetType() const
{
	return m_type;
}

bool I_Command::Merge(const I_Command& lhs) const
{
	if (GetType() != lhs.GetType())
		return false;

	///Cannot merge if differing non xt-state as merging of these has not yet been implemented.
	if (lhs.m_pChange->nonXtStateRedo != m_pChange->nonXtStateRedo ||
		lhs.m_pChange->nonXtStateUndo != m_pChange->nonXtStateUndo)
		return false;

	//Can merge?
	if (XtMergeHelpers::CanMerge(lhs.m_pChange->pXtRedo, m_pChange->pXtRedo))
	{
		//Merge, accepting rhs for conflicts.
		Xt::Change* pMerged = Xt::Merge(lhs.m_pChange->pXtRedo, m_pChange->pXtRedo);
		Xt::Change* pResult;
		bool resolved = XtMergeHelpers::ResolveConflicts(&pResult, pMerged);
		delete pMerged;
		if (!resolved)
		{
			LOGERROR(C_Core::Instance().GetLogger(), "Failed to resolve conflicts during command merge!");
			return false;
		}

		delete m_pChange->pXtRedo;
		m_pChange->pXtRedo = pResult;

		delete m_pChange->pXtUndo;
		m_pChange->pXtUndo = lhs.m_pChange->pXtUndo->Clone();
		return true;
	}

	return false;
}

C_Status I_Command::Undo()
{
	C_ViewDisabler viewDisabler;
	if (!IsUndoable())
		return E_Status_NotImplemented;

	if (!m_pChange)
		return E_Status_Success;

	C_Core::Instance().GetUndoManager().Patch(*m_pChange, false);
	return E_Status_Success;
}

C_Status I_Command::Redo()
{
	C_ViewDisabler viewDisabler;
	if (!IsUndoable())
		return E_Status_NotImplemented;

	if (!m_pChange)
		return E_Status_Success;

	C_Core::Instance().GetUndoManager().Patch(*m_pChange, true);
	return E_Status_Success;
}

}
