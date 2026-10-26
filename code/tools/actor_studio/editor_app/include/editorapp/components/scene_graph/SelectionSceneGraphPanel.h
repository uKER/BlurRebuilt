#ifndef EDITOR_SELECTION_SCENE_GRAPH_VIEW
#define EDITOR_SELECTION_SCENE_GRAPH_VIEW

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Misc/Selection.h>
#include <EditorApp/Components/Scene_Graph/SceneGraphPanel.h>

namespace Editor
{
	class C_SelectionSceneGraphPanel : public C_SceneGraphPanel
	{
	public:
		struct C_Flags
		{
			bool selectionInvalid : 1;
		};

		C_SelectionSceneGraphPanel(
			wxWindow *parent, 
			wxWindowID = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200,400),
			long style = wxTAB_TRAVERSAL|wxNO_BORDER);
		~C_SelectionSceneGraphPanel();

		void Shutdown();

		virtual void OnGraphClearSelection();

		virtual bool OnSelectionChanging(TreeItem* item, bool selected);

		const EditorLib::C_Selection& GetSelection();
		
		void OnGraphIdle();

		C_Flags m_Flags;

		virtual bool BuildObjIdStrFromSelection(Btl::String& objIdStr);

		mutable EditorLib::C_Selection m_selectionCache;
	};
}

#endif
