#ifndef DMAN_EXPORTWIZARD_H
#define DMAN_EXPORTWIZARD_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_ExpSelectItemTypePage;
class C_ExpSelectFilePage;
class C_ExcelWriter;

class C_ExportWizard : public wxWizard
{
public:
    C_ExportWizard(wxFrame* frame, bool useSizer = true);
	~C_ExportWizard();

    C_ExpSelectItemTypePage&	GetPage1()			{ return *m_pPage1; }
	C_ExpSelectFilePage&		GetPage2()			{ return *m_pPage2; }
	C_ExcelWriter&				GetExcelWriter()	{ return *m_pExcelWriter; }
	
private:
    C_ExpSelectItemTypePage*	m_pPage1;
	C_ExpSelectFilePage*		m_pPage2;
	C_ExcelWriter*				m_pExcelWriter;
};

class C_ExpSelectItemTypePage : public wxWizardPageSimple
{
public:
	C_ExpSelectItemTypePage(wxWizard* parent = NULL,
		wxWizardPage* prev = NULL,
		wxWizardPage* next = NULL);

	void OnWizardPageChanging(wxWizardEvent& event);
private:
	C_ExportWizard*		m_pWizard;
	wxChoice*			m_pTypeChoice;
	void				FillTypeChoice();
	
	DECLARE_EVENT_TABLE()
};

class C_ExpSelectFilePage : public wxWizardPageSimple
{
public:
	C_ExpSelectFilePage(wxWizard* parent = NULL,
		wxWizardPage* prev = NULL,
		wxWizardPage* next = NULL);

	void OnWizardPageChanging(wxWizardEvent& event);
private:
	C_ExportWizard*		m_pWizard;
	wxFilePickerCtrl*	m_pFileNamePicker;

	DECLARE_EVENT_TABLE()
};

} //namespace DMan

#endif //DMAN_EXPORTWIZARD_H
