#include "stdafx.h"

#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Components/Scene_Graph/EditorSceneGraphPanel.h>
#include <EditorApp/Components/LevelInfoPanel.h>
#include <EditorApp/Components/LevelPackPanel.h>
#include <EditorApp/Misc/Registry.h>
#include <EditorApp/Tasks/UITasks.h>

#include <wx/notebook.h>

namespace Editor
{

class PackWrapper : public wxPanel
{
public:
	PackWrapper(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxPanelNameStr)
	: wxPanel(parent, winid, pos, size, style, name)
	{
	}

	bool MSWShouldPreProcessMessage(WXMSG* msg)
	{
		return __super::MSWShouldPreProcessMessage(msg);
	}

	bool MSWTranslateMessage(WXMSG* msg)
	{
		if (msg->message == WM_KEYDOWN)
		{
			if (wxWindow::MSWTranslateMessage(msg))
				return true;
			else
			{
				//send a new event to parent panel.
				//This is because ::IsDialog() will dispatch messages to the underlying ScarabForm
				//so we'll never see it again in wxWidgets land
				::SendMessage((HWND)GetHWND(), msg->message, msg->wParam, msg->lParam);
				return false;
			}
		}
		else
			return __super::MSWTranslateMessage(msg);
	}
};

C_LevelInfoPanel::C_LevelInfoPanel(
	wxWindow *parent, 
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxPanel(parent, id, pos, size, style)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer( wxVERTICAL );
	SetSizer(pSizer0);

	m_pTabs = new wxNotebook(this, wxID_ANY, pos, size);

	m_pSceneGraphPanel = new C_EditorSceneGraphPanel(m_pTabs, wxID_ANY, pos, size);

	m_pLevelPackPanel = new C_LevelPackPanel(m_pTabs, wxID_ANY, pos, size);

	m_pTabs->AddPage(m_pSceneGraphPanel, "Scene Graph", true);
	m_pTabs->AddPage(m_pLevelPackPanel, "Pack Browser");

	GetSizer()->Add(m_pTabs, 1, wxALL|wxEXPAND, 0);

	Layout();
}

C_LevelInfoPanel::~C_LevelInfoPanel() 
{
	delete m_pLevelPackPanel;
	delete m_pSceneGraphPanel;
}

void C_LevelInfoPanel::Shutdown()
{
	m_pSceneGraphPanel->Shutdown();
	m_pLevelPackPanel->Shutdown();
}

}