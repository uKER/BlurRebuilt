#include "stdafx.h"

#include <wx_shared/EventProxy.h>
#include <wx_shared/MenuBuilder.h>

#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/Frame/Menus/FrameMenuBar.h>
#include <DialogueMan/Frame/Menus/FrameFileMenu.h>
#include <DialogueMan/Frame/Menus/FrameEditMenu.h>
#include <DialogueMan/Frame/Menus/FrameVersionControlMenu.h>
#include <DialogueMan/Frame/Menus/FrameToolsMenu.h>
#include <DialogueMan/Frame/Menus/FrameHelpMenu.h>

namespace DMan
{

C_FrameMenuBar::C_FrameMenuBar(C_Frame* pFrame)
{
	m_pFrame = pFrame;
	m_pMenuBuilder = 0;

	m_pFileMenu = 0;
	m_pEditMenu = 0;
	m_pVersionControlMenu = 0;
	m_pToolsMenu = 0;
	m_pHelpMenu = 0;
}

C_FrameMenuBar::~C_FrameMenuBar()
{
	delete m_pMenuBuilder;

	delete m_pFileMenu;
	delete m_pEditMenu;
	delete m_pVersionControlMenu;
	delete m_pToolsMenu;
	delete m_pHelpMenu;
}

bool C_FrameMenuBar::InitialiseUI(const Util::C_String& projectName)
{
	m_pMenuBar = new wxMenuBar(wxMB_DOCKABLE);
	m_pMenuBuilder = new Tools::C_MenuBuilder(m_pMenuBar);
	m_pFrame->SetMenuBar(m_pMenuBar);

	m_pFileMenu				= new C_FrameFileMenu(this);
	m_pEditMenu				= new C_FrameEditMenu(this);
	m_pVersionControlMenu	= new C_FrameVersionControlMenu(this);
	m_pToolsMenu			= new C_FrameToolsMenu(this);
	m_pHelpMenu				= new C_FrameHelpMenu(this);

	if (!m_pFileMenu->InitialiseUI()) return false;
	if (!m_pEditMenu->InitialiseUI()) return false;	
	if (!m_pVersionControlMenu->InitialiseUI()) return false;
	if (!m_pToolsMenu->InitialiseUI()) return false;
	if (!m_pHelpMenu->InitialiseUI())	return false;

	return true;
}

Tools::C_MenuBuilder* C_FrameMenuBar::GetMenuBuilder()
{
	return m_pMenuBuilder;
}

C_Frame* C_FrameMenuBar::GetFrame()
{
	return m_pFrame;
}

}
