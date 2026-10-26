#ifndef DMAN_COLUMN_EDITOR_H
#define DMAN_COLUMN_EDITOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_ColumnEditor : public wxDialog
	{
	public:
		C_ColumnEditor(wxWindow *parent);

		void OnClose(wxCloseEvent& event);
		void OnEditorShown(wxGridEvent& event);
		void OnCellValueChanged(wxGridEvent& event);

	private:
		std::map< int, int > m_allAndHideMap;
		wxGrid* m_pGrid;

		DECLARE_EVENT_TABLE()
	};
}

#endif
