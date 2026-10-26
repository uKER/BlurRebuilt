#ifndef EDITOR_FRAME_H
#define EDITOR_FRAME_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef USE_WXFLATNOTEBOOK
class wxNotebook;
#endif

class wxLog;
class wxSplitterCtrl;
class wxAuiManager;
class wxAuiPaneInfo;


namespace Debug
{
	class TextSubscriber;
}
namespace Progress
{
	class I_ProgressBar;
}

namespace Editor
{
	//View menu toggle locations...
	enum E_UIPanelType
	{
		E_UIPanelType_Unknown = 0,
		E_UIPanelType_ToolBar,
		E_UIPanelType_Window,
	};

	class C_FrameMenuBar;
	class C_ProgressStatusBar;

	class C_Frame : public wxFrame
	{
	public:
		C_Frame(const wxString& title,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = wxDEFAULT_FRAME_STYLE);

		~C_Frame();

		bool Initialise(const Util::C_String& projectName);

		void OnUpdateUI(wxUpdateUIEvent &e);
		void OnSetVisibleUIPane(wxCommandEvent &e);
		void OnSetViewLayout(wxCommandEvent &e);

		//wrap window in a dockable panel & add toggle options to view menu
		bool AddUIPane(wxWindow *pWindow, wxAuiPaneInfo& info, E_UIPanelType type=E_UIPanelType_Window);
		void RemoveUIPane(wxWindow *pWindow);

		//add window to the tabbed attribute panel
		bool AddAttributePanel(wxWindow *pWindow, const char* pName);
		void RemoveAttributePanel(wxWindow *pWindow);

		Progress::I_ProgressBar* GetProgressBar();
		const Progress::I_ProgressBar* GetProgressBar() const;

		wxAuiManager*			GetAuiManager();
		const wxAuiManager*		GetAuiManager() const;

		C_FrameMenuBar*			GetFrameMenuBar();

	protected:
		void					OnIdle(wxIdleEvent& idle);

		wxAuiManager*			m_pAuiManager;
		wxSplitterCtrl*			m_pSplitterCtrl;
		C_FrameMenuBar*			m_pMenuBar;

#ifdef USE_WXFLATNOTEBOOK
		wxNotebook*				m_pNotebook;
#else
		wxAuiNotebook*			m_pNotebook
#endif
		C_ProgressStatusBar*	m_pStatusBar;

		
		Ut::UtMap<S32, wxWindow*>				
								m_MenuItemIdWindowMap;
		Ut::UtMap<wxWindow*, Util::C_String>	
								m_AttributeTabLabels;

		DECLARE_EVENT_TABLE()
	};



}

#endif