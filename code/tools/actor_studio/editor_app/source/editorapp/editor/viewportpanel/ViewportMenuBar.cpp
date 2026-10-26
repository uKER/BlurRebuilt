#include "stdafx.h"

#include <EditorApp/Editor/viewportpanel/ViewportMenuBar.h>

#include <EditorApp/Editor/viewportpanel/ViewportPanel.h>
#include <EditorApp/Editor/viewportpanel/ViewportShadingMenu.h>
#include <EditorApp/Editor/viewportpanel/ViewportRendererMenu.h>
#include <EditorApp/Editor/viewportpanel/ViewportCameraMenu.h>

#include <EditorApp/misc/EventProxy.h>
#include <EditorApp/misc/MenuBuilder.h>

#include <wx/tokenzr.h>

#include <EditorApp/Components/PanelMenu.h>

namespace Editor
{
C_ViewportMenuBar::C_ViewportMenuBar(C_ViewportPanel *pViewportPanel)
: m_pViewportPanel(pViewportPanel)
{
}

C_ViewportMenuBar::~C_ViewportMenuBar()
{
	SAFE_DELETE_PTR(m_pMenuBuilder);
	SAFE_DELETE_PTR(m_pShadingMenu);
	SAFE_DELETE_PTR(m_pRendererMenu);
	SAFE_DELETE_PTR(m_pCameraMenu);
}

bool C_ViewportMenuBar::InitialiseUI()
{
	C_MenuBar *pMenuBar = new C_MenuBar(m_pViewportPanel, wxID_ANY);
	m_pMenuBuilder = new C_PanelMenuBuilder(pMenuBar);
	
	if (m_pViewportPanel->GetSizer())
		m_pViewportPanel->GetSizer()->Add(pMenuBar, 0, wxRIGHT|wxLEFT, 0);

	m_pShadingMenu = new C_ViewportShadingMenu(this);
	m_pRendererMenu = new C_ViewportRendererMenu(this);
	m_pCameraMenu = new C_ViewportCameraMenu(this);

	if (!m_pShadingMenu->InitialiseUI()) return false;
	if (!m_pRendererMenu->InitialiseUI()) return false;
	if (!m_pCameraMenu->InitialiseUI()) return false;

	return true;
}

C_MenuBuilderBase* C_ViewportMenuBar::GetMenuBuilder()
{
	return m_pMenuBuilder;
}

C_ViewportPanel* C_ViewportMenuBar::GetViewportPanel()
{
	return m_pViewportPanel;
}

}