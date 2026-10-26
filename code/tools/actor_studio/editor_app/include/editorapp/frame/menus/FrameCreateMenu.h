#ifndef EDITOR_FRAME_CREATE_MENU_H
#define EDITOR_FRAME_CREATE_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>
#include <EditorLib/Xt/XtDefinitions.h>

namespace Editor
{
	class C_FrameCreateMenu
	{
	public:
		C_FrameCreateMenu(C_FrameMenuBar* pMenuBar, wxMenuBar* pWxMenuBar);
		~C_FrameCreateMenu();

		bool InitialiseUI();

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_FrameCreateMenu* pToolPanel);
			void Connect();
			void Disconnect();

			void OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);
		private:
			C_FrameCreateMenu* m_pFrameCreateMenu;
		};

		void OnCreateActor(wxCommandEvent& e);
		void OnCreateAnimation(wxCommandEvent& e);
		void OnCreateGroup(wxCommandEvent& e);

		void OnMenuUpdate(wxUpdateUIEvent& e);

		void OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);

		bool CreateDynamicMenu(const char* menuName, EditorLib::E_XtType xtBaseType);

		Ut::UtMap<S32, Btl::String> m_ActorCreationMenuIds;

		C_FrameMenuBar* m_pMenuBar;

		wxMenuBar* m_pWxMenuBar;

		C_Slots		m_Slots;
	};
}

#endif
