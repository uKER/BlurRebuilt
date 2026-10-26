#ifndef EDITOR_EDITOR_PROPERTIES_H
#define EDITOR_EDITOR_PROPERTIES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_EditorPropertyGroup : public I_PropertyGroup
	{
	public:
		C_EditorPropertyGroup();
		~C_EditorPropertyGroup();

		void Initialise(wxPropertyGridManager* pPgMgr);
		
		void OnPropertyChanged(wxPropertyGridEvent& e);
		void Apply();
		void Revert();

	private:
		wxPropertyGridManager*	m_pPgMgr;
		wxPGProperty*			m_pAutosaveTimerProperty;

		wxPGProperty*			m_pGridSizeProperty;
		wxPGProperty*			m_pGridSpacingProperty;

		wxPGProperty*			m_pUIPromptProperty;
		wxPGProperty*			m_pDisableClearUndoProperty;
	};
}

#endif
