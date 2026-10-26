#ifndef EDITOR_LAYER_CONTROL_H
#define EDITOR_LAYER_CONTROL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "sigslot/SigSlot.h"

#include <xtwin/TreeCtrl.h>

class wxSplitterCtrl;

namespace EditorLib
{
	class I_Layer;
	class I_UpdateHint;
}

namespace Editor
{
	class C_LayerEditPanel;
	class C_LayerEditMenuBar;
	class C_LayerSceneGraphPanel;

	class C_LayerEditTreeItem : public TreeItem
	{
	protected:
		virtual bool OnStartRename(Sz16& startName) { return false; }
		virtual bool OnItemSelectionChanged(bool selected) { return false; }
	};

	class C_LayerEditTreeLevelItem : public C_LayerEditTreeItem
	{
	public:
		C_LayerEditTreeLevelItem(const EditorLib::I_Level* level);
		
		const EditorLib::I_Level* GetLevel() const;
		void UpdateName();

	private:
		const EditorLib::I_Level* m_Level;
	};

	class C_LayerEditTreeLayerItem : public C_LayerEditTreeItem
	{
	public:
		C_LayerEditTreeLayerItem(EditorLib::I_Layer* layer);

		EditorLib::I_Layer* GetLayer();
		void UpdateName();

	protected:
		virtual bool OnItemSelectionChanged(bool selected) { return true; }

	private:
		EditorLib::I_Layer* m_Layer;
	};

	class C_LayerEditTreeDgNodeItem : public C_LayerEditTreeItem
	{
	public:
		C_LayerEditTreeDgNodeItem(const EditorLib::I_DgNode* dgNode);

		const EditorLib::I_DgNode* GetDgNode() const;
		void UpdateName();

	private:
		const EditorLib::I_DgNode* m_DgNode;
	};

	class C_LayerEditTree : public TreeCtrl
	{
	public:
		C_LayerEditTree(C_LayerEditPanel* panel);

		C_LayerEditTreeLevelItem* GetLevelItem(const EditorLib::I_Level*);
		C_LayerEditTreeLayerItem* GetLayerItem(const EditorLib::I_Layer*);

		EditorLib::I_Layer* GetSelectedLayer();

	protected:
		virtual void OnSelectionChanged(const XtUt::Array<TreeItem*>& selection, TcSelectionType selectionType);

	private:
		C_LayerEditPanel* m_Panel;

		EditorLib::I_Layer* m_SelectedLayer;
	};

	class C_LayerEditPanel : public wxPanel
	{
	public:
		struct C_Flags
		{
			bool Connected		: 1;
			bool RefreshColours	: 1;
		};

		C_LayerEditPanel(	wxWindow *parent, wxWindowID winid = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxSize(300, 500),
							long style = wxTAB_TRAVERSAL | wxNO_BORDER,
							const wxString& name = wxPanelNameStr);

		~C_LayerEditPanel();

		void Shutdown();

		EditorLib::I_Layer* GetSelectedLayer();

	private:
		#define DEF(e, str) e,
		enum E_LayerViewNode
		{
			#include "LayerIconMapping.h"
			E_LayerViewNode_Count
		};
		#undef DEF

		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_LayerEditPanel *pLayerControl);
			void Connect();
			void Disconnect();

			void OnLevelNameChangedSlot(const EditorLib::I_Level& level);
			void OnLevelOpeningSlot(const EditorLib::I_Level& level);
			void OnLevelOpenedSlot(const EditorLib::I_Level& level);
			void OnLevelClosingSlot(const EditorLib::I_Level& level);
			void OnLevelClosedSlot(const EditorLib::I_Level& level);

			void OnLayerEventSlot(const EditorLib::I_Layer& layer, EditorLib::E_LayerEvent e);
			
			void OnLayerDeserialisedSlot(const EditorLib::I_Layer& layer);
			void OnLayerNodeAddedSlot(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode);
			void OnLayerNodeRemovedSlot(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode);

			void OnDgNodeInitialisedSlot(const EditorLib::I_DgNode& dgNode);

		private:
			C_LayerEditPanel	*m_pLayerEditPanel;

			FRIEND_CLASS C_LayerEditPanel;
		};

		typedef Btl::OpenHashMap<Btl::String, S32, Btl::HashKeyOpBtlString> NodeIconMap;

		void OnLevelNameChanged(const EditorLib::I_Level& level);
		void OnLevelOpening(const EditorLib::I_Level& level);
		void OnLevelOpened(const EditorLib::I_Level& level);
		void OnLevelClosing(const EditorLib::I_Level& level);
		void OnLevelClosed(const EditorLib::I_Level& level);

		void OnLayerAdded(const EditorLib::I_Layer& layer);
		void OnLayerRemoved(const EditorLib::I_Layer& layer);
		void OnLayerDeserialised(const EditorLib::I_Layer& layer);

		void OnLayerNodeAdded(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode);
		void OnLayerNodeRemoved(const EditorLib::I_Layer& layer, const EditorLib::I_DgNode& dgNode);
		
		void OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode);
		void OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode);

		void OnLayerTreeSelectionChanged(wxTreeEvent& e);
		void OnNodeTreeSelectionChanging(wxTreeEvent& e);
		void OnSize(wxSizeEvent& e);
		void OnFilterTextUpdate(wxCommandEvent& e);

	private:
		void HighlightLayerNodes(const EditorLib::I_Layer *pLayer);

		C_Slots				m_Slots;
		C_LayerEditMenuBar	*m_pMenuBar;
		wxSplitterCtrl		*m_pSplitterCtrl;

		wxPanel* m_pLayerTreePanel;

		C_LayerEditTree* m_pLayerTree;
		C_LayerSceneGraphPanel* m_pNodeTree;

		NodeIconMap			m_NodeIconMap;

		Ut::Vector<const EditorLib::I_DgNode*>	m_currentItems;
		C_Flags				m_Flags;

		static const char* m_pLayerIconStringTable[E_LayerViewNode_Count];

		FRIEND_CLASS C_LayerEditTree;
	};
}


#endif