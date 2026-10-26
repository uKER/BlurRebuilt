#ifndef EDITOR_LIB_XT_UNDO_REDO_H
#define EDITOR_LIB_XT_UNDO_REDO_H

#ifdef _MSC_VER_
#pragma once
#endif
/*
namespace EditorLib
{
	namespace State
	{
		struct C_Checkpoint
		{
			C_Checkpoint() { pXtUndoDiff = 0; }
			~C_Checkpoint() { delete pXtUndoDiff; }

			Xt::Change* pXtUndoDiff;
			C_Selection* pUndoSelection;
			C_Clipboard* pUndoClipboard;

			Btl::String name;
		private:
			C_Checkpoint(const C_Checkpoint& rhs);
			const C_Checkpoint& operator=(const C_Checkpoint& rhs);
		};

		void ContextBegin(const char* name);
		void ContextEnd(C_Checkpoint** ppCheckpoint, const char* name);
		void Undo(const C_Checkpoint& checkpoint);
	}
}
*/
#endif
