#ifndef PANEL_MENU_H
#define PANEL_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorApp/Misc/MenuBuilder.h>

namespace Editor
{
	//wxwidgets doesn't allow menubars on anything other than the top level frame...
	//mimic it using static text boxes and popup menus...
	class C_MenuBar : public wxWindow
	{
	public:
		C_MenuBar(wxWindow *parent, wxWindowID id, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=0);
		~C_MenuBar();

		bool Append(wxMenu *pMenu, const wxString& title);
		wxMenu* Remove(size_t pos);
		S32 FindMenu(const wxString& title);
		wxMenu* GetMenu(size_t pos);
		void OnMenuClick(wxMouseEvent &e);

		//Modified wxWindow code from msw/window.cpp
		bool ShowMenu(wxMenu *menu, wxPoint pos);

	protected:
		struct MenuInfo
		{
			wxStaticText	*pTitle;
			wxMenu			*pMenu;
		};

		Ut::Vector<MenuInfo>	m_Menus;
		bool					m_IsActive;
	};


	class C_PanelMenuBuilder : public C_MenuBuilderBase
	{
	public:
		C_PanelMenuBuilder(C_MenuBar *pMenuBar);
		~C_PanelMenuBuilder();

		S32 OnAddMenuEntry(const char* pMenuEntry, E_MenuItemType type=E_MenuItem_Normal);
		S32 OnDeleteMenuEntry(const char* pMenuEntry);

	private:
		C_MenuBar *m_pMenuBar;
	};
}

#endif