#include "stdafx.h"

#include <EditorApp/misc/EventProxy.h>
#include <EditorApp/misc/MenuBuilder.h>

#include <wx/tokenzr.h>

#include <EditorApp/Components/PanelMenu.h>

namespace Editor {

C_MenuBar::C_MenuBar(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxWindow(parent, id, pos, size, style)
, m_IsActive(false)
{
	wxBoxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(pSizer);
	Layout();
}

C_MenuBar::~C_MenuBar()
{
	for (S32 i=0; i<m_Menus.Size(); ++i)
		delete m_Menus[i].pMenu;

	m_Menus.Clear();
}

bool C_MenuBar::Append(wxMenu *pMenu, const wxString& title)
{
	wxStaticText *pText = new wxStaticText(this, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);

	wxSizer *pSizer = GetSizer();
	if (!pSizer)
		return false;

	pSizer->Add(0, 0, 0, wxALL, 2);
	if (pSizer->Add(pText, 0, wxALL, 2))
	{
		MenuInfo info = {pText, pMenu};
		m_Menus.PushBack(info);
		Layout();

		pText->Connect(pText->GetId(), wxEVT_LEFT_DOWN, wxMouseEventHandler(C_MenuBar::OnMenuClick), 0, this);
		return true;
	}
	else
	{
		pText->Destroy();
	}
	return false;
}

wxMenu* C_MenuBar::Remove(size_t pos)
{
	if (pos<(U32)m_Menus.Size() && GetSizer()->Detach(m_Menus[pos].pTitle))
	{
		wxMenu *pMenu = m_Menus[pos].pMenu;
		Disconnect(m_Menus[pos].pTitle->GetId());
		m_Menus.EraseByIndex(pos);
		return pMenu;
	}
	return 0;
}

S32 C_MenuBar::FindMenu(const wxString& title)
{
	for (S32 i=0; i<m_Menus.Size(); ++i)
		if (m_Menus[i].pTitle->GetLabel() == title)
			return i;

	return wxNOT_FOUND;
}

wxMenu* C_MenuBar::GetMenu(size_t pos)
{
	if (pos<(U32)m_Menus.Size())
		return m_Menus[pos].pMenu;

	return 0;
}

void C_MenuBar::OnMenuClick(wxMouseEvent &e)
{
	for (S32 i=0; i<m_Menus.Size(); ++i)
	{
		if (m_Menus[i].pTitle == e.GetEventObject())
		{
			//m_Menus[i].pTitle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
			wxPoint pos = m_Menus[i].pTitle->GetPosition();
			pos.y += m_Menus[i].pTitle->GetSize().y;

			//use our own modified popup menu function
			//PopupMenu(m_Menus[i].pMenu, pos);
			ShowMenu(m_Menus[i].pMenu, pos);
		}
	}
}

//Modified wxWindow code from msw/window.cpp
bool C_MenuBar::ShowMenu(wxMenu *menu, wxPoint pos)
{
	menu->SetInvokingWindow(this);
	menu->UpdateUI();

	if (pos.x == wxDefaultCoord && pos.y==wxDefaultCoord )
	{
		wxPoint mouse = ScreenToClient(wxGetMousePosition());
		pos.x = mouse.x; pos.y = mouse.y;
	}

	HWND hWnd = GetHwnd();
	HMENU hMenu = GetHmenuOf(menu);
	POINT point;
	point.x = pos.x;
	point.y = pos.y;
	::ClientToScreen(hWnd, &point);

	//here's the important line... 
	//set the left mouse button flag so we don't need to unclick & reclick
	UINT flags = TPM_LEFTBUTTON;
	if ( wxGetWinVersion() >= wxWinVersion_5 )
		flags |= TPM_RECURSE;

	::TrackPopupMenu(hMenu, flags, point.x, point.y, 0, hWnd, NULL);
	{
		MSG msg;
		while ( ::PeekMessage(&msg, (HWND)0, WM_COMMAND, WM_COMMAND, PM_REMOVE) )
		{
			if ( msg.message == WM_QUIT )
			{
				// if we retrieved a WM_QUIT, insert back into the message queue.
				::PostQuitMessage(0);
				break;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
    }

	menu->SetInvokingWindow(NULL);
	return true;
}

C_PanelMenuBuilder::C_PanelMenuBuilder(C_MenuBar *pMenuBar)
: m_pMenuBar(pMenuBar)
{
}

C_PanelMenuBuilder::~C_PanelMenuBuilder()
{
}

S32 C_PanelMenuBuilder::OnAddMenuEntry(const char* pMenuEntry, E_MenuItemType type)
{
	wxStringTokenizer tokenizer(pMenuEntry, "|");
	if (tokenizer.CountTokens() < 2)
	{
		LOGERROR(g_Log, "Failed to add menu entry '%s'\n", pMenuEntry);
		return false;
	}
	
	wxString rootMenuTitle = tokenizer.GetNextToken();
	S32 rootIndex = m_pMenuBar->FindMenu(rootMenuTitle);
	
	wxMenu *pRootMenu = (rootIndex == wxNOT_FOUND) ? 
		new wxMenu() : m_pMenuBar->GetMenu(rootIndex);
	if (!pRootMenu)
	{
		LOGERROR(g_Log, "Failed to add menu entry '%s'\n", pMenuEntry);
		return -1;
	}

	if (rootIndex == wxNOT_FOUND)
		m_pMenuBar->Append(pRootMenu, rootMenuTitle);

	{
		wxMenu *pCurrentMenu = pRootMenu;
		while (tokenizer.CountTokens()>1)
		{
			wxString token = tokenizer.GetNextToken();
			wxMenu *pSubMenu = NULL;

			//check to see if there isn't already an item by the same name.
			wxMenuItemList& menuItems = pCurrentMenu->GetMenuItems();
			for (U32 i=0; i<menuItems.size(); ++i)
			{
				const wxString& name = ((wxMenuItemBase*)menuItems[i])->GetName();
				if (name==token && menuItems[i]->IsSubMenu())
				{
					pSubMenu = ((wxMenuItemBase*)menuItems[i])->GetSubMenu();
					break;
				}
			}

			if (!pSubMenu)
			{
				//wx allows menu items to be added with the same label
				pSubMenu = new wxMenu();
				pCurrentMenu->Append(wxID_ANY, token, pSubMenu);
			}

			pCurrentMenu = pSubMenu;
		}

		wxString token = tokenizer.GetNextToken();
		assert(!tokenizer.HasMoreTokens());

		wxMenuItem *pMenuItem = (type == E_MenuItem_Separator )
			? pCurrentMenu->Append(wxID_SEPARATOR, token, "")
			: pCurrentMenu->Append(wxID_ANY, token, "", (wxItemKind)type);

		return pMenuItem->GetRealId();
	}	
}

S32 C_PanelMenuBuilder::OnDeleteMenuEntry(const char* pMenuEntry)
{
	wxStringTokenizer tokenizer(pMenuEntry, "|");

	wxString token = tokenizer.GetNextToken();
	S32 index = m_pMenuBar->FindMenu(token);

	if (index == wxNOT_FOUND)
		return -1;

	wxMenu *pRootMenu = m_pMenuBar->GetMenu(index);
	wxMenu *pCurrentMenu = pRootMenu;

	//need this for later as there doesn't seem to be a way of traversing back up a menu hierarchy...
	Ut::UtVector< Ut::DynamicVector<wxMenuItem*> > menuItemStack;

	while (tokenizer.CountTokens()>1)
	{
		wxString token = tokenizer.GetNextToken();
		wxMenu *pSubMenu = NULL;

		wxMenuItemList& menuItems = pCurrentMenu->GetMenuItems();
		for (U32 i=0; i<menuItems.size(); ++i)
		{
			const wxString& name = ((wxMenuItemBase*)menuItems[i])->GetName();
			if (name==token && menuItems[i]->IsSubMenu())
			{
				menuItemStack.PushBack(menuItems[i]);
				pSubMenu = ((wxMenuItemBase*)menuItems[i])->GetSubMenu();
				break;
			}
		}

		if (!pSubMenu)
			return -1;

		pCurrentMenu = pSubMenu;
	}

	token = tokenizer.GetNextToken();
	assert(!tokenizer.HasMoreTokens());

	S32 eventId = -1;
	{
		wxMenuItemList& menuItems = pCurrentMenu->GetMenuItems();
		wxMenuItem *pMenuItem = NULL;

		for (U32 i=0; i<menuItems.size(); ++i)
		{
			const wxString& name = ((wxMenuItemBase*)menuItems[i])->GetName();
			if (name==token && !menuItems[i]->IsSubMenu())
			{
				pMenuItem = menuItems[i];
				eventId = pMenuItem->GetId();
				break;
			}
		}

		if (!pMenuItem)
			return -1;
		else
			pCurrentMenu->Delete(pMenuItem);

		//Menu clean up ... traverse back up menu hierarchy & remove any empty submenus
		for (S32 i=menuItemStack.Size(); i>1; --i)
			if (menuItemStack[i-1]->GetSubMenu()->GetMenuItemCount() == 0)
				menuItemStack[i-2]->GetSubMenu()->Delete(menuItemStack[i-1]);

		if (menuItemStack.Size() && menuItemStack[0]->GetSubMenu()->GetMenuItemCount()==0)
			pRootMenu->Destroy(menuItemStack[0]);

		if (pRootMenu->GetMenuItemCount()==0)
			delete (m_pMenuBar->Remove(index));
	}

	return eventId;
}

}