#ifndef EDITOR_HYPERGRAPH_DIALOG_H
#define EDITOR_HYPERGRAPH_DIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include "sigslot/SigSlot.h"

class wxTreeListCtrl;
class wxTreeItemId;

namespace EditorLib
{
	class C_Actor;
}

namespace Editor
{
	class C_ReferenceData;

	class C_HypergraphPanel : public wxPanel
	{
	public:
		C_HypergraphPanel(wxWindow *parent,  wxWindowID id = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxSize(500, 300),
							long style = wxTAB_TRAVERSAL|wxNO_BORDER);

		~C_HypergraphPanel();
		void Shutdown();

		void OnNodeSelect(wxTreeEvent& e);

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public: 
			C_Slots(C_HypergraphPanel *pDialog);
			void Connect();
			void Disconnect();

			void OnCoreEnableViewsSlot(bool enable);
			void OnCoreSelectionChangedSlot();
			void OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc);
			void OnDgNodeDestroyedSlot(const EditorLib::I_DgNode& dgNode);
			void OnLevelOpeningSlot(const EditorLib::I_Level& level);
			void OnLevelOpenedSlot(const EditorLib::I_Level& level);
			void OnLevelClosingSlot(const EditorLib::I_Level& level);
			void OnLevelClosedSlot(const EditorLib::I_Level& level);

		private:
			C_HypergraphPanel* m_pDialog;
		};

		class HypergraphVisitor : public LvObjects
		{
		public:
			HypergraphVisitor(Btl::Vector<Btl::Pair<LtStruct*,Xt::DomBlock*> >& dom, const XtString& value);
			virtual ~HypergraphVisitor();

			virtual bool Visit(struct LtStruct* type, Xt::DomBlock* dom);

		private:
			Btl::Vector<Btl::Pair<LtStruct*,Xt::DomBlock*> >& m_Dom;
			XtString m_Value;
		};

		class C_TreeItemData : public wxTreeItemData
		{
		public:
			C_TreeItemData()
			{
				m_DgNode = 0;
			}

			C_TreeItemData(EditorLib::I_DgNode* dgNode)
			{
				m_DgNode = dgNode;
			}

			EditorLib::I_DgNode* GetData()
			{
				return m_DgNode;
			}

		private:
			EditorLib::I_DgNode* m_DgNode;
		};

		struct C_Flags
		{
			U32 dirty : 1;
			U32 pendingSync : 1;
		} m_Flags;

		wxTreeItemId FindRefNode(const char* pPath) const;
		wxTreeItemId FindRefNode(const EditorLib::C_AftkRef *pRef) const;

		void OnShow(wxShowEvent& e);
		void OnIdle(wxIdleEvent& e);
		void OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode);
		void OnDgNodeDestroyed(const EditorLib::I_DgNode& dgNode);
		void OnCoreEnableViews(bool enable);
		void OnCoreSelectionChanged();
		void OnLevelOpening(const EditorLib::I_Level& level);
		void OnLevelOpened(const EditorLib::I_Level& level);
		void OnLevelClosing(const EditorLib::I_Level& level);
		void OnLevelClosed(const EditorLib::I_Level& level);
		void SelectItem(const EditorLib::C_NodeId& nodeId);

		void ParseDatabase(EditorLib::C_XtObject* selectedItem);
		void ParseMessage(EditorLib::C_XtObject* selectedItem, LtStruct* type, Xt::DomBlock* dom);
		void ParseHasParent(EditorLib::C_XtObject* selectedItem, LtStruct* type, Xt::DomBlock* dom);

		bool GetField(LtStruct* type, Xt::DomBlock* dom, const Btl::String& value, const Btl::String& fieldType, Btl::Pair<Btl::Pair<LarvaField*, LarvaType*>, Xt::DomComponent*>& component);

		static Xt::DomComponent* FindParentActor(Xt::DomBlock* dom);
		static XtString GetAttr(struct LtStruct* type, XtString attr, XtString param);
		static Xt::DomElement* GetElementFromId(XtId id);
		static XtString GetName(Xt::DomComponent* block);
		static XtString GetSummaryText(Xt::DomComponent* block);
		wxTreeItemId GetTreeItem(wxTreeItemId root, wxString text);

		U32 m_viewEnabled;
		EditorLib::C_NodeId m_SelectedItem;
		//EditorLib::C_XtObject* m_pSelectedItem;

		C_Slots		m_Slots;

		wxTreeCtrl	*m_pTreeCtrl;
		bool		m_connected;

		wxTreeItemId m_pRootItem;
		wxTreeItemId m_pRootSourceItem;
		wxTreeItemId m_pRootDestinationItem;
		wxTreeItemId m_pRootParentOfItem;
		wxTreeItemId m_pRootChildOfItem;

		DECLARE_CLASS(C_HypergraphPanel)
	};

}
#endif