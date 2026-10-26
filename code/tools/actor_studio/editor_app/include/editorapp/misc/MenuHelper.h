#ifndef EDITOR_MENU_HELPER_H
#define EDITOR_MENU_HELPER_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Editor
{
	class MenuHelper
	{
	public:
		static void Sort(wxMenu* menu, U32 first = 0);
	
	private:
		static bool SortPredicate(wxMenuItem* a, wxMenuItem* b);
	};
}

#endif