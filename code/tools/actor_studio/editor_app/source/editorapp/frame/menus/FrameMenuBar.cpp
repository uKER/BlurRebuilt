#include "stdafx.h"

#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameCreateMenu.h>
#include <EditorApp/Frame/Menus/FrameDisplayMenu.h>
#include <EditorApp/Frame/Menus/FrameEditMenu.h>
#include <EditorApp/Frame/Menus/FrameVersionControlMenu.h>
#include <EditorApp/Frame/Menus/FrameToolsMenu.h>
#include <EditorApp/Frame/Menus/FrameViewMenu.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Frame/Menus/FrameFileMenu.h>
#include <EditorApp/Frame/Menus/FrameHelpMenu.h>

namespace Editor
{

C_FrameMenuBar::C_FrameMenuBar(C_Frame* pFrame)
{
	m_pFrame = pFrame;
	m_pMenuBuilder = 0;

	m_pCreateMenu = 0;
	m_pEditMenu = 0;
	m_pFileMenu = 0;
	m_pVersionControlMenu = 0;
	m_pToolsMenu = 0;
	m_pViewMenu = 0;
	m_pDisplayMenu = 0;
	m_pHelpMenu = 0;
}

C_FrameMenuBar::~C_FrameMenuBar()
{
	delete m_pMenuBuilder;
	delete m_pCreateMenu;
	delete m_pEditMenu;
	delete m_pFileMenu;
	delete m_pVersionControlMenu;
	delete m_pToolsMenu;
	delete m_pViewMenu;
	delete m_pDisplayMenu;
	delete m_pHelpMenu;
}

bool C_FrameMenuBar::InitialiseUI(const Util::C_String& projectName)
{
	m_pMenuBar = new wxMenuBar(wxMB_DOCKABLE);
	m_pMenuBuilder = new C_MenuBuilder(m_pMenuBar);
	m_pFrame->SetMenuBar(m_pMenuBar);

	m_pFileMenu				= new C_FrameFileMenu(this);
	m_pVersionControlMenu	= new C_FrameVersionControlMenu(this);
	m_pEditMenu				= new C_FrameEditMenu(this);
	m_pToolsMenu			= new C_FrameToolsMenu(this);
	m_pViewMenu				= new C_FrameViewMenu(this);
	m_pCreateMenu			= new C_FrameCreateMenu(this, m_pMenuBar);
	m_pDisplayMenu			= new C_FrameDisplayMenu(this);
	m_pHelpMenu				= new C_FrameHelpMenu(this);

	if (!m_pFileMenu->InitialiseUI()) return false;
	if (!m_pEditMenu->InitialiseUI()) return false;	
	{
		wxMenu *pMenu = new wxMenu();
		m_pMenuBar->Append(pMenu, _T("Modify"));
	}
	if (!m_pToolsMenu->InitialiseUI()) return false;
	if (!m_pVersionControlMenu->InitialiseUI()) return false;
	if (!m_pViewMenu->InitialiseUI()) return false;
	if (!m_pCreateMenu->InitialiseUI())	return false;
	if (!m_pDisplayMenu->InitialiseUI())	return false;
	{
		wxMenu *pMenu = new wxMenu();
		m_pMenuBar->Append(pMenu, _T(projectName.Chars()));
	}
	if (!m_pHelpMenu->InitialiseUI())	return false;

	return true;
}

C_MenuBuilder* C_FrameMenuBar::GetMenuBuilder()
{
	return m_pMenuBuilder;
}

C_Frame* C_FrameMenuBar::GetFrame()
{
	return m_pFrame;
}

}
