#ifndef XTWIN_WX_EXPLORER_H
#define XTWIN_WX_EXPLORER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace XtWin
{
	#define DEF(e, str1, str2) e,
	enum E_XtTreeViewNode
	{
		#include "IconMapping.h"
		E_XtTreeViewNode_Count	
	};
	#undef DEF


	class C_Explorer : public wxTreeCtrl
	{
	public:
		class C_ExplorerData : public wxTreeItemData
		{
		public:
			enum E_ItemType
			{
				E_ItemType_Root,
				E_ItemType_Record,
				E_ItemType_Filter,
				E_ItemType_Pack
			};

			C_ExplorerData()
			: m_pDomRecord(0), m_pDomPack(0), m_Type(E_ItemType_Root), m_Filter("")
			{ }
			C_ExplorerData(Xt::DomRecord* pRecord)
			: m_pDomRecord(pRecord), m_pDomPack(0), m_Type(E_ItemType_Record), m_Filter("")
			{ }
			C_ExplorerData(Xt::DomPack* pPack)
			: m_pDomRecord(0), m_pDomPack(pPack), m_Type(E_ItemType_Pack), m_Filter("")
			{ }
			C_ExplorerData(Xt::DomPack* pPack, const Btl::String& filterFolder)
			: m_pDomRecord(0), m_pDomPack(pPack), m_Type(E_ItemType_Filter), m_Filter(filterFolder)
			{ }
			~C_ExplorerData()
			{ }

			int GetType() const;

			const Xt::DomRecord* GetDomRecord() const;
			
			Xt::DomRecord* GetDomRecord();

			const Xt::DomPack* GetDomPack() const;

			Xt::DomPack* GetDomPack();

			const Btl::String& GetFilter() const;

		private:
			Xt::DomPack* m_pDomPack;
			Xt::DomRecord* m_pDomRecord;
			const int m_Type;
			Btl::String m_Filter;
		};

	public:
		C_Explorer(
			wxWindow *parent, 
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 
				wxTR_EDIT_LABELS|
				wxTR_HAS_BUTTONS|
				wxTR_SINGLE|
				wxTR_EXTENDED|
				wxTR_FULL_ROW_HIGHLIGHT|
				wxTAB_TRAVERSAL|
				wxHSCROLL|
				wxVSCROLL);

		virtual ~C_Explorer();

		virtual void Populate(
			Xt::DomDb* pDb,
			Ut::Delegate<bool(const XtString&)> *pCallback = 0,
			bool filterEmpty = false);		

		void Clear();

	protected:

		//should re-Populate on derived class
		//@see \code\tools\dialogueman\source\dialogueman\components\explorer\Explorer.cpp::Reload()
		virtual void Reload(){}; 
		
		Xt::DomPack* FindPack(const XtString& name);

		wxTreeItemId MoveItem(wxTreeItemId source, wxTreeItemId destination);

		void ShowMenu(wxTreeItemId id, const wxPoint& pt);
		void OnRenameItem(wxCommandEvent& evt);
		void OnItemRenameStart(wxTreeEvent& evt);
		void OnItemRenameEnd(wxTreeEvent& evt);
		void OnNodeDrag(wxTreeEvent & e);
		void OnNodeSelect(wxTreeEvent & e);
		void OnContextMenu(wxTreeEvent& evt);
		
		virtual void OnFilterPacks(wxCommandEvent& evt);
		virtual void OnCreateNewPack(wxCommandEvent& evt);
		virtual void OnDeletePack(wxCommandEvent& evt);		
		virtual void OnCreateNewRecord(wxCommandEvent& evt){};
		virtual void OnDeleteRecord(wxCommandEvent& evt);

		static const char* m_pTreeIconStringTable[E_XtTreeViewNode_Count][2];

		Xt::DomDb* m_pDb;
		bool m_shouldFilterPacks;
	};
}

#endif