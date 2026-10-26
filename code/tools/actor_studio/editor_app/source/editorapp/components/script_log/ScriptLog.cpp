#include "stdafx.h"
#include <Python.h>

#include <EditorApp/Components/Log.h>
#include <EditorApp/Components/Script_Log/ScriptLog.h>
#include <EditorApp/App.h>

namespace Editor
{

C_ScriptLogPanel::C_ScriptLogPanel(
	wxWindow* pParent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style)
: 
wxPanel(
	pParent,
	id,
	pos,
	size,
	style)
{
	//create our custom log subscriber. wxWidgets will handle the clean up
	m_pLogCtrl = new wxTextCtrl(
		this, 
		wxID_ANY,
		wxEmptyString, 
		wxDefaultPosition,
		wxSize(500, 100),
		wxTE_RICH2 | wxNO_BORDER | wxTE_MULTILINE);
	m_pLogTextSubscriber = new Debug::TextSubscriber(m_pLogCtrl);
	Debug::GetRootLogger().AddSubscriber(m_pLogTextSubscriber);
	wxLog::SetActiveTarget(m_pLogTextSubscriber);

	m_pPromptCtrl = new wxTextCtrl(
		this, 
		wxID_ANY,
		wxEmptyString,
		wxDefaultPosition,
		wxDefaultSize,
		wxTE_PROCESS_ENTER);

	wxBoxSizer* pSizer = new wxBoxSizer( wxVERTICAL );
	pSizer->Add( m_pLogCtrl, 1, wxEXPAND, 5 );
	pSizer->Add( m_pPromptCtrl, 0, wxEXPAND | wxTOP, 5 );

	SetSizer( pSizer );
	pSizer->SetSizeHints( this );

	C_App *pApp = &wxGetApp();
	m_pPromptCtrl->Connect(m_pPromptCtrl->GetId(), wxEVT_COMMAND_TEXT_ENTER, 
		wxCommandEventHandler(C_ScriptLogPanel::OnExecute), 0, this);
}

C_ScriptLogPanel::~C_ScriptLogPanel()
{
}

void C_ScriptLogPanel::OnExecute(wxCommandEvent& e)
{
	if (PyRun_SimpleString(m_pPromptCtrl->GetValue().c_str()) == 0)
		m_pPromptCtrl->Clear();
}

void C_ScriptLogPanel::Detach()
{
	wxLog::SetActiveTarget(0);
	Debug::GetRootLogger().RemoveSubscriber(m_pLogTextSubscriber);
	SAFE_DELETE_PTR(m_pLogTextSubscriber);
}

}
