#ifndef EDITOR_LAYER_SCENE_GRAPH_VIEW
#define EDITOR_LAYER_SCENE_GRAPH_VIEW

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Misc/Selection.h>
#include <EditorApp/Components/Scene_Graph/SelectionSceneGraphPanel.h>

namespace EditorLib
{
	class I_Layer;
}

namespace Editor
{
	class C_LayerSceneGraphPanel : public C_SelectionSceneGraphPanel
	{
	public:
		C_LayerSceneGraphPanel(
			wxWindow *parent, 
			wxWindowID = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200,400),
			long style = wxTAB_TRAVERSAL|wxNO_BORDER);
		~C_LayerSceneGraphPanel();

		void SetLayer(EditorLib::I_Layer* layer);
		
	protected:
		virtual bool OnSelectionChanging(TreeItem* item, bool selected);

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_LayerSceneGraphPanel* pLayerSceneGraph);
			void Connect();
 			void Disconnect();

			void OnLayerEventSlot(const EditorLib::I_Layer& layer, EditorLib::E_LayerEvent evt);
			void OnLayerNodeAddedSlot(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode);
			void OnLayerNodeRemovedSlot(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode);

		private:
			C_LayerSceneGraphPanel* m_pLayerSceneGraph;

			FRIEND_CLASS C_LayerSceneGraphPanel;
		};

		void OnLayerEvent(const EditorLib::I_Layer& layer, EditorLib::E_LayerEvent evt);
		void OnLayerNodeAdded(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode);
		void OnLayerNodeRemoved(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode);

		bool m_IgnoreEvents;
		EditorLib::I_Layer* m_Layer;
		C_Slots m_Slots;
	};
}

#endif