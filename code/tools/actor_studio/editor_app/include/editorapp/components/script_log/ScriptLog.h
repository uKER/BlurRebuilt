#ifndef EDITOR_SCRIPT_LOG_H
#define EDITOR_SCRIPT_LOG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Debug
{
	class TextSubscriber;
}

namespace Editor
{
	class C_ScriptLogPanel : public wxPanel
	{
	public:
		C_ScriptLogPanel(
			wxWindow *parent, 
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxTAB_TRAVERSAL | wxNO_BORDER);

		virtual ~C_ScriptLogPanel();

		virtual void OnExecute(wxCommandEvent& e);

		void Detach();

	private:
		wxTextCtrl*		m_pLogCtrl;
		wxTextCtrl*		m_pPromptCtrl;
		Debug::TextSubscriber*		
						m_pLogTextSubscriber;
	};
}


#endif