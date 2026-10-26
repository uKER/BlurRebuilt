#ifndef DMAN_LOC_FORM_H
#define DMAN_LOC_FORM_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_Row;

class C_LocForm : public wxScrolledWindow
{
public:

	enum {
		ID_NAME = 1,
		ID_LANGUAGE,
		ID_LINE,
		ID_COMMENT
	};

	C_LocForm(wxWindow *parent,
			wxWindowID = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200,400),
			long style = wxTAB_TRAVERSAL|wxNO_BORDER);

	void Shutdown();

	void ClearAll();
	void DisableAll();
	void EnableAll();
	void ShowAll();
	void HideAll();

private:

	class C_Flags
	{
	public:
		int dirty : 1;
	};

	class C_Slots : public sigslot::has_slots<>
	{
	public:
		C_Slots(C_LocForm* pLocForm);
		void Connect();
		void Disconnect();

		void OnSelectionChangedSlot();
		void OnDbCloseSlot();

		FRIEND_CLASS C_LocForm;

	private:
		C_LocForm* m_pLocForm;
	};

	void OnSelectionChanged();
	void OnDbClose();
	void OnIdle(wxIdleEvent& e);

	void OnNameChanged(wxCommandEvent& event);
	void OnLanguageSelected(wxCommandEvent& event);
	void OnLineChanged(wxCommandEvent& event);
	void OnCommentChanged(wxCommandEvent& event);

	void PopulateLanChoice();
	void ReadValues();


	bool SetCurrentRow(C_RowSet* pRowSet, int rowIndex);
	
	DECLARE_CLASS(C_LocForm);
	DECLARE_EVENT_TABLE()

	C_Slots			m_slots;
	C_Flags			m_flags;

	wxTextCtrl*		m_pNameTxtCtrl;
	wxChoice*		m_pLanChoice;
	wxTextCtrl*		m_pLineTxtCtrl;
	wxTextCtrl*		m_pCommentTxtCtrl;
	
	wxStaticText*	m_pNameLabel;
	wxStaticText*	m_pLanLabel;
	wxStaticText*	m_pLineLabel;
	wxStaticText*	m_pCommentLabel;

	int				m_englishIndex;
	C_Row*			m_pCurrRow;
	Xt::DomBlock*	m_pCurrStringBlock;	
};

}

#endif //DMAN_LOC_FORM_H