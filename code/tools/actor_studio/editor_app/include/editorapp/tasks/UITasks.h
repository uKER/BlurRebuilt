#ifndef EDITOR_UITASKS_H
#define EDITOR_UITASKS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	namespace Tasks
	{
		void Undo();
		void Redo();
		void CopyToClipboard();
		void PasteFromClipboard();
		void DuplicateSelection();
		void SelectAll();
		void ShowNodePropertiesWindow();
		void ShowLayerEditWindow();
		void ShowDopeSheetDialog();
		void ShowReferencesDialog();
		void Quit();

		U32 GetAutosaveInterval();
		void SetAutosaveInterval(U32 interval);

		F32 GetGridSize();
		F32 GetGridSpacing();
		void SetGridSize(F32 size);
		void SetGridSpacing(F32 spacing);

		bool GetShowUIPromptOption();
		void SetShowUIPromptOption(bool show);

		bool GetShowHUDSelectedActors();
		void SetShowHUDSelectedActors(bool show);

		bool GetDisableClearUndoWarning();
		void SetDisableClearUndoWarning(bool disable);

		bool InstallWndProcHook();
		bool UninstallWndProcHook();
	}
}

#endif
