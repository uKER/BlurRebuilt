#include "stdafx.h"

#include <EditorApp/Misc/MenuHelper.h>
#include <algorithm>

namespace Editor
{

void MenuHelper::Sort(wxMenu* menu, U32 first)
{
	Btl::Vector<wxMenuItem*> menuItems;
	while (menu->GetMenuItemCount() > first)
	{
		menuItems.PushBack(menu->Remove(menu->FindItemByPosition(first)));
	}

	std::sort(menuItems.Begin(), menuItems.End(), SortPredicate);

	for (Btl::Vector<wxMenuItem*>::Iterator it = menuItems.Begin(); it != menuItems.End(); ++it)
	{
		menu->Append(*it);
	}
}

bool MenuHelper::SortPredicate(wxMenuItem* a, wxMenuItem* b)
{
	return (a->GetLabel() < b->GetLabel());
}

}