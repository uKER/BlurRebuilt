#ifndef EDITOR_VIEWPORT_SHADING_MENU_H
#define EDITOR_VIEWPORT_SHADING_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ViewportMenuBar;

	class C_ViewportShadingMenu
	{
	public:
		C_ViewportShadingMenu(C_ViewportMenuBar *pMenuBar);
		C_ViewportShadingMenu::~C_ViewportShadingMenu();
		bool InitialiseUI();

	private:

		void OnSetRenderMode(wxCommandEvent &e);
		void OnUpdateRenderMode(wxUpdateUIEvent &e);

		C_ViewportMenuBar *m_pMenuBar;

		struct MenuItem
		{
			enum E_ItemId
			{
				E_ItemId_Fillmode,
				E_ItemId_Cullmode,
				E_ItemId_WireframeOverlay,
				E_ItemId_XRay,
				E_ItemId_Texturing,
				E_ItemId_Lighting
			};

			S32				wxId;
			E_ItemId		id;
			U32				value;
		};

		Ut::Vector<MenuItem>	m_menuItems;
	};

}

#endif
