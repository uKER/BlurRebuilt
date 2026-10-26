#ifndef EDITOR_SCENE_GRAPH_FILTER_MENU_H
#define EDITOR_SCENE_GRAPH_FILTER_MENU_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorApp/Components/scene_graph/SceneGraphPanel.h>

namespace Editor
{
	class C_SceneGraphMenuBar;

	class C_SceneGraphFilter;

	class C_SceneGraphFilterMenu
	{
	public:
		C_SceneGraphFilterMenu(C_SceneGraphMenuBar *pMenuBar, C_SceneGraphFilter* pSceneFilter);
		~C_SceneGraphFilterMenu();
		bool InitialiseUI();

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_SceneGraphFilterMenu* pSceneGraphFilterMenu);
			void Connect();
			void Disconnect();

			void OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);
		private:
			C_SceneGraphFilterMenu* m_pSceneGraphFilterMenu;
		};

		void OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);

		void OnSetSortType(wxCommandEvent &e);
		void OnSetGlobalFilter(wxCommandEvent &e);
		void OnSetActorFilter(wxCommandEvent &e);
		void OnUpdateNull(wxUpdateUIEvent &e);
		void OnUpdateGlobalFilter(wxUpdateUIEvent &e);
		void OnUpdateActorFilter(wxUpdateUIEvent &e);
		void OnUpdateSortFilter(wxUpdateUIEvent &e);

		Ut::UtMap<S32,Btl::String> m_ActorFilterMenuIds;
		Ut::UtMap<Btl::String,bool> m_ActorFilterTypes;

		Ut::UtMap<S32,E_SceneGraphDataType> m_GlobalFilterMenuIds;
		Ut::UtMap<E_SceneGraphDataType,bool> m_GlobalFilterTypes;
		
		C_SceneGraphMenuBar*	m_pMenuBar;
		
		C_SceneGraphFilter*		m_pSceneFilter;

		S32						m_ClearGlobalId;
		S32						m_ClearActorsId;

		S32						m_SortPlacementId;
		S32						m_SortAlphaAscId;
		S32						m_SortAlphaDescId;
		S32						m_SortTypeAscId;
		S32						m_SortTypeDescId;

		C_Slots					m_Slots;
	};

}

#endif