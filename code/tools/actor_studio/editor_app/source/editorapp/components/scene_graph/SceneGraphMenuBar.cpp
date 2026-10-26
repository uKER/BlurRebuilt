#include "stdafx.h"

#include <wx/treectrl.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>

#include <xtwin/TreeCtrl.h>

#include <EditorApp/Components/Scene_Graph/SceneGraphMenuBar.h>

#include <EditorApp/Components/Scene_Graph/SceneGraphPanel.h>
#include <EditorApp/Components/Scene_Graph/SceneGraphFilterMenu.h>


#include <EditorApp/misc/EventProxy.h>
#include <EditorApp/misc/MenuBuilder.h>

#include <EditorApp/Components/PanelMenu.h>

namespace Editor
{
C_SceneGraphMenuBar::C_SceneGraphMenuBar(C_SceneGraphPanel *pSceneGraphPanel, C_SceneGraphFilter* pSceneFilter)
: m_pSceneGraphPanel(pSceneGraphPanel)
{
	m_pSceneFilter = pSceneFilter;
}

C_SceneGraphMenuBar::~C_SceneGraphMenuBar()
{
	SAFE_DELETE_PTR(m_pMenuBuilder);
	SAFE_DELETE_PTR(m_pFilterMenu);
}

bool C_SceneGraphMenuBar::InitialiseUI()
{
	m_pMenuBar = new C_MenuBar(m_pSceneGraphPanel, wxID_ANY);
	m_pMenuBuilder = new C_PanelMenuBuilder(m_pMenuBar);
	
	if (m_pSceneGraphPanel->GetSizer())
		m_pSceneGraphPanel->GetSizer()->Add(m_pMenuBar, 0, wxRIGHT|wxLEFT, 0);

	m_pFilterMenu = new C_SceneGraphFilterMenu(this,m_pSceneFilter);

	if (!m_pFilterMenu->InitialiseUI()) return false;

	return true;
}

C_MenuBar* C_SceneGraphMenuBar::GetMenuBar()
{
	return m_pMenuBar;
}

C_MenuBuilderBase* C_SceneGraphMenuBar::GetMenuBuilder()
{
	return m_pMenuBuilder;
}

C_SceneGraphPanel* C_SceneGraphMenuBar::GetSceneGraphPanel()
{
	return m_pSceneGraphPanel;
}

C_SceneGraphFilterMenu* C_SceneGraphMenuBar::GetFilterMenu()
{
	return m_pFilterMenu;
}

}