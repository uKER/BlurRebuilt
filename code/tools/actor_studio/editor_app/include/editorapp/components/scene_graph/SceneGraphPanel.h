#ifndef EDITOR_SCENE_GRAPH_VIEW
#define EDITOR_SCENE_GRAPH_VIEW

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>

#include <xtwin/TreeCtrl.h>

#include <EditorApp/Components/Scene_Graph/SceneGraphEnums.h>

class wxSplitterCtrl;

namespace Editor
{
	class C_SceneGraphFilter;
	class C_SceneGraphMenuBar;
	class C_SceneGraphPanel;
	class C_SceneTreeItemDgNode;

	typedef Btl::OpenHashMap<EditorLib::C_NodeId, C_SceneTreeItemDgNode*, EditorLib::HashKeyOpNodeId> SceneNodeDataMap;

	class C_SceneTreeItem : public TreeItem
	{
	public:
		virtual bool OnItemDeleting() { return false; }
		virtual bool OnStartRename(Sz16& startName) { return false; }
		virtual bool OnItemSelectionChanged(bool selected) { return !selected; }
		virtual bool OnReparent(TreeItem* parent) { return false; }

		virtual E_SceneGraphItemType GetType() { return E_SceneGraphItemType_Generic; }
	};

	class C_SceneTreeItemDgNode : public C_SceneTreeItem
	{
	public:
		C_SceneTreeItemDgNode(C_SceneGraphPanel* sceneGraphPanel, const EditorLib::I_DgNode* pDgNode, SceneNodeDataMap* dataMap, E_SceneGraphDataType dataType=E_SceneGraphDataType_Unknown);
		virtual ~C_SceneTreeItemDgNode();

		const EditorLib::I_DgNode* GetDgNode() const;	
		E_SceneGraphDataType GetDataType() const;

		long GetItemNumber();
		static void Reset();

		void SetLevelReadOnly(bool readOnly);

		virtual void OnContextMenu(const WinPoint& pt);
		virtual bool OnItemDeleting();
		virtual void OnItemDeleted();
		virtual bool OnItemSelectionChanged(bool selected);
		virtual bool OnReparent(TreeItem* parent);
		virtual bool OnReorder(int newIndex);
		virtual bool OnStartRename(Sz16& startName);
		virtual bool OnEndRename(Sz16& newName);
		virtual bool OnActivate();

		virtual E_SceneGraphItemType GetType() { return E_SceneGraphItemType_DgNode; }

	private:
		const EditorLib::I_DgNode*	m_pDgNode;
		E_SceneGraphDataType		m_dataType;
		long						m_itemNumber;			

		C_SceneGraphPanel*			m_pSceneGraphPanel;
		SceneNodeDataMap*			m_pDataMap;

		static long itemCount;
	};

	class C_SceneTreeCtrl : public TreeCtrl
	{
	public:
		C_SceneTreeCtrl(C_SceneGraphPanel* sceneGraphPanel)
		{
			m_pSceneGraphPanel = sceneGraphPanel;
			m_AlphaSortA.Compile("(([0-9]*[^0-9]+)*)([0-9]*)", wxRE_ICASE);
			m_AlphaSortB.Compile("(([0-9]*[^0-9]+)*)([0-9]*)", wxRE_ICASE);
		}

		void SetSceneFilter(C_SceneGraphFilter* sceneFilter)
		{
			m_pSceneFilter = sceneFilter;
		}

		virtual bool OnCompareItems(TreeItem* a, TreeItem* b);
		virtual bool OnRemoveItems(const XtUt::Array<TreeItem*>& items);
		virtual void OnSelectionChanged(const XtUt::Array<TreeItem*>& selection, TcSelectionType selectionType);

	private:
		C_SceneGraphPanel* m_pSceneGraphPanel;
		C_SceneGraphFilter* m_pSceneFilter;
		wxRegEx m_AlphaSortA;
		wxRegEx m_AlphaSortB;
	};

	class C_SceneGraphPanel : public wxPanel
	{
	public:
		struct C_Flags
		{
			bool filterRefDirty : 1;
			bool filterDgDirty : 1;
			bool filterDirty : 1;
			bool pendingSync : 1;
			bool filterChanged : 1;
			bool dragging : 1;
			bool autoSelect : 1;
			bool pendingDeletion : 1;
		};

		C_SceneGraphPanel(
			wxWindow *parent, 
			wxWindowID = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200,400),
			long style = wxTAB_TRAVERSAL|wxNO_BORDER);
		virtual ~C_SceneGraphPanel();

		virtual void Shutdown();

		void Freeze();
		void Thaw();

		void OnFilterMenuUpdate();
		void FrameSelection();

		//bool AddMapping(const EditorLib::I_DgNode* dgNode, C_SceneTreeItemDgNode* treeItem);
		//bool RemoveMapping(const EditorLib::I_DgNode* dgNode);

		void ShowMenu(C_SceneTreeItem* item, const wxPoint& pt);

		C_Flags& GetFlags();

	private:
		#define DEF(e, str) e,
		enum E_TreeViewNode
		{
			#include "TreeIconMapping.h"
			E_TreeViewNode_Count
		};
		#undef DEF

		typedef Btl::OpenHashMap<Btl::String, C_SceneTreeItem*, Btl::HashKeyOpBtlString> ReferenceRootMap;
		typedef Btl::OpenHashMap<Btl::String, S32, Btl::HashKeyOpBtlString> NodeIconMap;

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_SceneGraphPanel* pSceneGraphPanel);
			void Connect();
			void Disconnect();
			
			void OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);
			void OnLevelEventSlot(const EditorLib::I_Level* level, EditorLib::E_LevelEvent evt);
			void OnLevelNameChangedSlot(const EditorLib::I_Level& level);
			void OnLevelOpeningSlot(const EditorLib::I_Level& level);
			void OnLevelOpenedSlot(const EditorLib::I_Level& level);
			void OnLevelClosingSlot(const EditorLib::I_Level& level);
			void OnLevelClosedSlot(const EditorLib::I_Level& level);

			void OnLevelAftkRefAddedSlot(const Btl::String& originalFilePath);
			void OnLevelAftkRefRemovedSlot(const Btl::String& originalFilePath);
			
			void OnAftkRefLoadedSlot(
				const Btl::String& originalFilePath, 
				const Aftk::C_Scene& aftkScene, 
				const EditorLib::C_SceneNode& sceneNode,
				const EditorLib::AftkDgMap& aftkDgMap);

			void OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& node, const EditorLib::C_UpdateHintCollection* pHc);
			void OnDgNodeDestroySlot(const EditorLib::I_DgNode& node);

			void OnCoreEnableViews(bool enable);

			void OnCoreSelectionChangedSlot();

			FRIEND_CLASS C_SceneGraphPanel;

		private:
			C_SceneGraphPanel* m_pSceneGraphPanel;
		};

		void InitialiseActorRoot();
		void InitialisePropertyRoot();

		void OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);
		void OnLevelNameChanged(const EditorLib::I_Level& level);
		void OnLevelOpening(const EditorLib::I_Level& level);
		void OnLevelOpened(const EditorLib::I_Level& level);
		void OnLevelClosing(const EditorLib::I_Level& level);
		void OnLevelClosed(const EditorLib::I_Level& level);

		void OnLevelAftkRefAdded(const Btl::String& orginalFilePath);
		void OnLevelAftkRefRemoved(const Btl::String& orginalFilePath);
		void OnAftkRefLoaded(
			const Btl::String& originalFilePath, 
			const Aftk::C_Scene& aftkScene, 
			const EditorLib::C_SceneNode& sceneNode,
			const EditorLib::AftkDgMap& aftkDgMap);

		void AddAftkRef(
			C_SceneTreeItem* parent,
			const EditorLib::C_SceneNode& sceneNode,
			const EditorLib::AftkDgMap& aftkDgMap);

		
		void OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc);
		void OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode);

		void UpdateDgNodeName(const EditorLib::I_DgNode& dgNode);
		void UpdateDgNodeIcon(const EditorLib::I_DgNode& dgNode);

		void OnSize(wxSizeEvent& e);

		void OnIdle(wxIdleEvent& e);

		void OnFilterTextUpdate(wxCommandEvent& e);

		void OnCoreEnableViews(bool enable);

		void OnCoreSelectionChanged();

		void OnSetActiveLevel(wxCommandEvent& evt);
		void OnUnloadLevel(wxCommandEvent& evt);
		void OnCopyNodeName(wxCommandEvent& evt);
		void OnRenameNode(wxCommandEvent& evt);
		void OnExportToLevel(wxCommandEvent& evt);
		void OnUnparentNode(wxCommandEvent& evt);
		void OnSetHidden(wxCommandEvent& evt);

		void SetPropertyNode(const EditorLib::I_DgNode& dgNode);
		void SetPropertyActor(const EditorLib::I_DgNode& actor);

		void AddActor(TreeItem* pTreeItem, const EditorLib::I_DgNode& actor);
		void AddComponent(TreeItem* pTreeItem, const EditorLib::C_DgXtCompNode& component);
		void AddProperty(TreeItem* pTreeItem, const EditorLib::C_DgXtPropNodeBase& property);

		void PopulateFromExistingData();
		void PopulateFromExistingData(EditorLib::C_SceneNode* pSceneNode);

		void UpdateData();

		bool FilterItem(C_SceneTreeItemDgNode* sceneGraphData);

		void UpdateFilter();

		void QueueReparent(TreeItem* item, TreeItem* parent);
		void PurgeReparentQueue();

		C_Slots				m_slots;

		C_SceneGraphFilter* m_pSceneFilter;

		wxTextCtrl*			m_pTextCtrl;
		C_SceneGraphMenuBar* m_pMenuBar;

		TreeItem*			m_rightClickItem;

		ReferenceRootMap	m_referenceRootMap;
		NodeIconMap			m_nodeIconMap;

		static const char* m_pTreeIconStringTable[E_TreeViewNode_Count];

	protected:
		virtual bool BuildObjIdStrFromSelection(Btl::String& objIdStr);
		virtual void OnGraphIdle() {}
		virtual void OnGraphClearSelection() {}
		virtual void OnGraphSyncSelection() {}
		virtual void OnSelectionChanged(TreeCtrl* from) {}
		virtual bool OnSelectionChanging(TreeItem* item, bool selected);
		
		C_Flags				m_Flags;

		wxSplitterCtrl*		m_pSplitter;
		
		wxPanel*			m_pNodeTreeCtrlPanel;
		C_SceneTreeCtrl*	m_pNodeTreeCtrl;
		
		wxPanel*			m_pPropertyTreeCtrlPanel;
		C_SceneTreeCtrl*	m_pPropertyTreeCtrl;

		SceneNodeDataMap	m_NodeDataMap;
		SceneNodeDataMap	m_PropertyDataMap;

		Btl::OpenHashMap<TreeItem*, Btl::Vector<TreeItem*> > m_ReparentQueue;

	private:
		DECLARE_CLASS(C_SceneGraphPanel);
		DECLARE_EVENT_TABLE()

	friend C_SceneTreeCtrl;
	friend C_SceneTreeItemDgNode;
	};
}

#endif
