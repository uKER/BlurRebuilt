#ifndef EDITOR_FRAME_DISPLAY_MENU_H
#define EDITOR_FRAME_DISPLAY_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_PropertyVisibilityData;

	class C_FrameDisplayMenu
	{
	public:
		C_FrameDisplayMenu(C_FrameMenuBar* pMenuBar);
		~C_FrameDisplayMenu();

		bool InitialiseUI();
		void OnMenuUpdate(wxUpdateUIEvent& e);

		bool AddProperty(const Btl::String& displayName, C_PropertyVisibilityData* pVisibilityData);

	private:
		C_FrameMenuBar* m_pMenuBar;

		static Ut::UtMap<S32,Btl::String> m_ActorFilterMenuIds;

		static void HideProperties();
		static void ShowPropertiesAll();
		static void ShowPropertiesSelected();

		static void UpdatePropertyAll(wxUpdateUIEvent& e);
		static void UpdatePropertySelected(wxUpdateUIEvent& e);
		static void UpdatePropertyNone(wxUpdateUIEvent& e);

		static void TogglePropertyAll(wxCommandEvent& e);
		static void TogglePropertySelected(wxCommandEvent& e);
		static void TogglePropertyNone(wxCommandEvent& e);
	};
}

#endif
