#ifndef DMAN_IMPORTWIZARD_H
#define DMAN_IMPORTWIZARD_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_SelectFilePage;
class C_SelectItemTypePage;
class C_SelectLocTablePage;
class C_ExcelImportOptionsPage;
class C_FinalDraftImportOptionsPage;
class C_ExcelReader;
class C_FinalDraftReader;

/////////////////////////// C_ImportWizard \\\\\\\\\\\\\\\\\\\\\\\\\\\

class C_ImportWizard : public wxWizard
{
public:
    C_ImportWizard(wxFrame* frame, bool useSizer = true);
	virtual ~C_ImportWizard();
	
	C_SelectFilePage&		GetPage1()				{ return *m_pPage1; }
	C_Reader&				GetReader()				{ return *m_pReader; }

	void					SetReader(C_Reader::E_ReaderType type);

private:
	C_SelectFilePage*		m_pPage1;
	C_SelectItemTypePage*	m_pPage2;
	C_SelectLocTablePage*	m_pPage3;
		
	C_Reader*				m_pReader;
};

/////////////////////////// C_SelectFilePage \\\\\\\\\\\\\\\\\\\\\\\\\\\

class C_SelectFilePage : public wxWizardPageSimple
{
public:
	C_SelectFilePage(wxWizard* parent = NULL,
					 wxWizardPage* prev = NULL,
					 wxWizardPage* next = NULL);
	
	void OnWizardPageChanging(wxWizardEvent& event);

private:
	C_ImportWizard*					m_pWizard;
	wxFilePickerCtrl*				m_pFileNamePicker;
	
	DECLARE_EVENT_TABLE()
};

/////////////////////////// C_SelectItemTypePage \\\\\\\\\\\\\\\\\\\\\\\\\\\

class C_SelectItemTypePage : public wxWizardPageSimple
{
public:
	C_SelectItemTypePage(wxWizard* parent = NULL,
					 wxWizardPage* prev = NULL,
					 wxWizardPage* next = NULL);

	void OnWizardPageChanging(wxWizardEvent& event);
private:
	C_ImportWizard*		m_pWizard;
	wxChoice*			m_pTypeChoice;
	void				FillTypeChoice();

	DECLARE_EVENT_TABLE()
};

/////////////////////////// C_SelectLocTablePage \\\\\\\\\\\\\\\\\\\\\\\\\\\

class C_SelectLocTablePage : public wxWizardPageSimple
{
public:
	C_SelectLocTablePage(wxWizard* parent = NULL,
					 wxWizardPage* prev = NULL,
					 wxWizardPage* next = NULL);

	void OnWizardPageChanging(wxWizardEvent& event);

	virtual wxWizardPage* GetNext() const;
private:
	void PopulateLocTableChoice();

	C_ImportWizard*					m_pWizard;
	wxChoice*						m_pLocTableChoice;

	C_ExcelImportOptionsPage*		m_pXlsOptions;
	C_FinalDraftImportOptionsPage*	m_pFcfOptions;

	DECLARE_EVENT_TABLE()
};

/////////////////////////// C_ExcelImportOptionsPage \\\\\\\\\\\\\\\\\\\\\\\\\\\

class C_ExcelImportOptionsPage : public wxWizardPageSimple
{
public:

	static const wxString kIgnoreStr;
	static const int kPreviewRows;
	static const int kPreviewCols;

	enum
	{
		ID_SHEETS_CHOICE = 1,
		ID_RANGE_CTRL,
		ID_REFRESH_BTN,
		ID_EXPECTED_COLS_LIST,
		ID_MOVE_UP_BTN,
		ID_MOVE_DOWN_BTN,
		ID_REMOVE_BTN,
		ID_ADD_IGNORE_COL_BTN,
		ID_RESET_BTN
	};

	C_ExcelImportOptionsPage(wxWizard* parent = NULL,
					 wxWizardPage* prev = NULL,
					 wxWizardPage* next = NULL);

	void OnRefresh(wxCommandEvent& event);
	void OnMoveUp(wxCommandEvent& event);
	void OnMoveDown(wxCommandEvent& event);
	void OnRemove(wxCommandEvent& event);
	void OnAddIgnoreColumn(wxCommandEvent& event);
	void OnReset(wxCommandEvent& event);
	void OnSheetsChoiceSelected(wxCommandEvent& event);
	void OnWizardPageChanging(wxWizardEvent& event);
	
	void ResetExpectedColumns();
	void RefreshPreview();
	
	wxChoice& GetSheetsChoice()
	{
		return *m_pSheetsChoice;
	}

	wxTextCtrl& GetRangeCtrl()
	{
		return *m_pRangeCtrl;
	}

	wxListBox& GetExpectedColsList()
	{
		return *m_pExpectedColsList;
	}

private:
	C_ImportWizard* m_pWizard;
	wxChoice*		m_pSheetsChoice;
	wxTextCtrl*		m_pRangeCtrl;
	wxGrid*			m_pExcelPreview;
	wxListBox*		m_pExpectedColsList;

	DECLARE_EVENT_TABLE()
};

/////////////////////////// C_FinalDraftImportOptionsPage \\\\\\\\\\\\\\\\\\\\\\\\\\\

class C_FinalDraftImportOptionsPage : public wxWizardPageSimple
{
public:
	C_FinalDraftImportOptionsPage(wxWizard* parent = NULL,
					 wxWizardPage* prev = NULL,
					 wxWizardPage* next = NULL);

private:
	C_ImportWizard* m_pWizard;

	DECLARE_EVENT_TABLE()
};

} //namespace DMan

#endif //DMAN_IMPORTWIZARD_H