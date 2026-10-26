#ifndef EDITOR_EDITOR_SCENE_GRAPH_VIEW
#define EDITOR_EDITOR_SCENE_GRAPH_VIEW

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Misc/Selection.h>

#include <EditorApp/Components/Scene_Graph/SceneGraphPanel.h>

namespace Editor
{
	class C_EditorSceneGraphPanel : public C_SceneGraphPanel
	{
	public:
		struct C_Flags
		{
			bool selectionInvalid : 1;
		};

		C_EditorSceneGraphPanel(
			wxWindow *parent, 
			wxWindowID = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200,400),
			long style = wxTAB_TRAVERSAL|wxNO_BORDER);
		virtual ~C_EditorSceneGraphPanel();

		virtual void Shutdown();

		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_EditorSceneGraphPanel* pSceneGraphPanel);
			void Connect();
			void Disconnect();
			
			void OnCoreSelectionChangedSlot();

			FRIEND_CLASS C_EditorSceneGraphPanel;

		private:
			C_EditorSceneGraphPanel* m_pSceneGraphPanel;
		};

		C_Slots m_slots;

		void OnCoreSelectionChanged();

		virtual void OnGraphSyncSelection();
		virtual void OnGraphClearSelection();
		
		void OnGraphIdle();

		C_Flags m_Flags;

		void SyncSelectionToTree();
		void SyncToExistingSelection(const EditorLib::C_Selection& selection);

	protected:
		virtual bool BuildObjIdStrFromSelection(Btl::String& objIdStr);
		virtual void OnSelectionChanged(TreeCtrl* from);
		virtual bool OnSelectionChanging(TreeItem* item, bool selected);

		mutable EditorLib::C_Selection m_selectionCache;
	};
}

#endif
