#include "stdafx.h"

#include <wx/tokenzr.h>

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorApp/Components/SplitterControl.h>
#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Frame/Menus/FrameMenuBar.h>
#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>
#include <EditorApp/Frame/Menus/FrameViewMenu.h>
#include <EditorApp/App.h>

namespace Editor
{

C_FrameViewMenu::C_FrameViewMenu(C_FrameMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

bool C_FrameViewMenu::InitialiseUI()
{
	C_Frame* pFrame = m_pMenuBar->GetFrame();
	wxMenuBar* pWxMenuBar = pFrame->GetMenuBar();
	wxMenu *pViewMenu = new wxMenu();
	pWxMenuBar->Append(pViewMenu, _T("View"));

	wxMenu *pToolBarSubMenu = new wxMenu();
	pViewMenu->Append(wxID_ANY, _T("Tool Bars"), pToolBarSubMenu);

	wxMenu *pWindowsSubMenu = new wxMenu();
	pViewMenu->Append(wxID_ANY, _T("Windows"), pWindowsSubMenu);

	pViewMenu->AppendSeparator();

	wxMenu *pAttributesSubMenu = new wxMenu();
	pViewMenu->Append(wxID_ANY, _T("Attributes"), pAttributesSubMenu);

	pViewMenu->AppendSeparator();

	wxMenuItem *pMenuItem;
	wxMenu *pViewLayoutMenu = new wxMenu();
	//Clean this up at some point - we're using E_SplitterLayout as the eventID...
	//Ideally, we'd use wxEVT_ANY and maintain a lookup table somewhere to map between the actual eventID and E_SplitterLayout
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_1, _T("Single Pane"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));

	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_2Vertical, _T("2 Panes Side by Side"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_2Horizontal, _T("2 Panes Stacked"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));

	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_3Vertical, _T("3 Panes Side by Side"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_3Horizontal, _T("3 Panes Stacked"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_3Bottom, _T("3 Panes Split Top"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_3Top, _T("3 Panes Split Bottom"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_3Right, _T("3 Panes Split Left"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_3Left, _T("3 Panes Split Right"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));

	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_4, _T("4 Panes"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_4Vertical, _T("4 Panes Side by Side"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_4Horizontal, _T("4 Panes Stacked"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_4Bottom, _T("4 Panes Split Top"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_4Top, _T("4 Panes Split Bottom"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_4Right, _T("4 Panes Split Left"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));
	pMenuItem = pViewLayoutMenu->Append(E_SplitterLayout_4Left, _T("4 Panes Split Right"));
	pFrame->Connect(pMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Frame::OnSetViewLayout));

	pViewMenu->AppendSubMenu(pViewLayoutMenu, _T("View Layout"));

	return true;
}

}
