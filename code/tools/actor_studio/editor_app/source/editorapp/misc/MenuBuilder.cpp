#include "stdafx.h"

#include <wx/tokenzr.h>

#include <EditorApp/Misc/EventProxy.h>
#include <EditorApp/Misc/MenuBuilder.h>

namespace Editor
{

C_MenuBuilder::C_MenuBuilder(wxMenuBar* pMenuBar)
{
	m_pMenuBar = pMenuBar;
}

C_MenuBuilder::~C_MenuBuilder()
{
}

S32 C_MenuBuilder::OnAddMenuEntry(const char *pMenuEntry, E_MenuItemType type)
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

S32 C_MenuBuilder::OnDeleteMenuEntry(const char *pMenuEntry)
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
