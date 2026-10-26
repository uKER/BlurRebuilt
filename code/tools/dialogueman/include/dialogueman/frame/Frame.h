#ifndef DMAN_FRAME_H
#define DMAN_FRAME_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Tools
{
	class C_MenuBuilder;
}

namespace DMan
{

class C_FrameMenuBar;
class C_DataGridPanel;
class C_LocForm;

enum
{
	ID_EXIT = 1,
	ID_ABOUT,
	ID_EXPLORER,
	ID_DIALOGUE_LINES,
	ID_IMPORT,
	ID_EXPORT,
	ID_SAVE,
	ID_REPORT,
	ID_CLEAR,
};

enum E_UIPanelType
{
	E_UIPanelType_Unknown = 0,
	E_UIPanelType_ToolBar,
	E_UIPanelType_Window,
};


class C_Frame: public wxFrame
{
public:
	C_Frame(wxWindow* parent,
			wxWindowID id,
			const wxString& title, 
			const wxPoint& pos = wxDefaultPosition, 
			const wxSize& size = wxDefaultSize,
			long style = wxDEFAULT_FRAME_STYLE);

	virtual ~C_Frame();

	bool Initialise();
	bool Uninitialise();

	//wrap window in a dockable panel & add toggle options to view menu
	bool AddUIPane(wxWindow *pWindow, wxAuiPaneInfo info, E_UIPanelType type=E_UIPanelType_Window);
	void RemoveUIPane(wxWindow *pWindow);
		
	wxAuiManager& GetAuiManager()
	{
		return *m_pAuiManager;
	}

	C_WxScarabForm& GetScarabForm()
	{
		return *m_pScarabForm;
	}

	C_LocForm& GetLocForm()
	{
		return *m_pLocForm;
	}

	DECLARE_EVENT_TABLE()

private:
	void	OnAbout(wxCommandEvent& e);
	void	OnIdle(wxIdleEvent& idle);

	C_Explorer*				CreateExplorer();
	C_DataGridPanel*		CreateGridPanel();
	C_WxScarabForm*			CreateForm();
	C_LocForm*				CreateLocForm();
	wxAuiNotebook*			CreateNotebook();

	wxAuiManager*			m_pAuiManager;
	C_DataGridPanel*		m_pGridPanel;
	C_Explorer*				m_pExplorer;
	wxAuiNotebook*			m_pNotebook;
	C_WxScarabForm*			m_pScarabForm;
	C_LocForm*				m_pLocForm;

	C_FrameMenuBar*			m_pMenuBar;
};

} 

#endif //DMAN_FRAME_H
