#ifndef EDITOR_PROGRESS_BAR_H
#define EDITOR_PROGRESS_BAR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ProgressStatusBar : public wxStatusBar, public Progress::I_ProgressBar
	{
	public:
		C_ProgressStatusBar(wxWindow *parent, wxWindowID id = wxID_ANY,
							long style = wxST_SIZEGRIP, const wxString& name = wxStatusBarNameStr);
		~C_ProgressStatusBar();

		void OnSize(wxSizeEvent& e);
		bool ProgressNotify(int iHierarchyDepth, const F32 percent, const Util::C_String &description);

		//overloaded to also reset background colour
		void SetStatusText(const wxString& text, int number=0);

	private:
		wxGauge		*m_pGauge;

		DECLARE_EVENT_TABLE()
	};
}

#endif