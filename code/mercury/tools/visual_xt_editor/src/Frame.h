#ifndef XT_EDITOR_FRAME_H
#define XT_EDITOR_FRAME_H

// standard wxWidgets
class wxAuiManager;
class wxAuiNotebook;
class wxDisplayWindow;
class wxLog;
class wxTextCtrl;

// custom widgets
class C_PreviewWindow;
class C_Explorer;
class C_Record;
class C_GenericRecord;
class C_EffectRecord;

typedef Ut::UtVector<Ut::DynamicVector<C_Record*> > Records;

#include "Src/Xt/XtLoader.h"
#include <core\utility\Singleton.h>

class C_XtNetCon;

namespace Samples
{
	class C_Section;
}

namespace DynamicForm
{
	class C_Form;
}

namespace Xt
{
	struct DomRecord;
}

class wxAuiNotebookEvent;

// Define a new frame type: this is going to be our main frame
class C_Frame : public wxFrame, public Ut::Singleton<C_Frame, Ut::CreateExternal>
{
public:
    // ctor(s)
	C_Frame() {};
	C_Frame(const wxString& title);
	~C_Frame();

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	
	void OnOpen(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);

	void OnSave(wxCommandEvent& event);

	void OnRecordGraphNotebookPageClose(wxAuiNotebookEvent& event);
	void OnRecordGraphNotebookPageChanged(wxAuiNotebookEvent& event);

	void OnTimer(wxTimerEvent& event);
	void OnMyEvent(wxCommandEvent& event);
	

	C_GenericRecord*	AddRecord(Xt::DomRecord* pRecord, const wxString& name);
	void				RemoveRecords();

	wxAuiNotebook*		GetRecordGraphNotebook() { return m_pRecordGraphNotebook; }
	C_Explorer*			GetExplorer() { return m_pExplorer; }
	C_PreviewWindow*	GetPreviewWindow() { return m_pPreviewWindow; }
	C_XtNetCon*			GetNetwork() { return m_pXtNetCon; }

	void				NetworkAddRecord(Xt::DomRecord* pRecord);

private:
	void				CloseDb();

	wxAuiManager*		m_pAuiManager;
	wxAuiNotebook*		m_pDatabaseNotebook;
	wxAuiNotebook*		m_pRecordGraphNotebook;
	wxLog*				m_pLog;
	wxTextCtrl*			m_pText;

	C_Explorer*			m_pExplorer;
	C_PreviewWindow*	m_pPreviewWindow;

	DynamicForm::C_Form*	m_pDynamicForm;

	Samples::C_Section*	m_pSection;

	C_XtLoader			m_XtLoader;
	Records				m_Records;

	// network stuff
	void						NetworkSendRecords();

	C_XtNetCon*					m_pXtNetCon;
	Ut::Vector<Xt::DomRecord*>	m_NetworkRecords;
	wxTimer						m_NetworkPumpTimer;
	wxTimer						m_NetworkIdleTimer;
	

    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxEVT_MY_EVENT, -1)


#endif
