#ifndef EDITOR_SORCERY_DIALOG_H
#define EDITOR_SORCERY_DIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

class wxSplitterCtrl;

class Explorer;
class ScarabForm;

#include <sigslot/sigslot.h>

#include <EditorApp/Components/BaseExplorer.h>

namespace Editor
{
	class C_SorceryPanel;

	class C_SorceryExplorer : public C_BaseExplorer
	{
	public:
		C_SorceryExplorer(C_SorceryPanel* pPanel);

	protected:
		virtual void OnActivateItem(ExplorerData* pData);
		virtual void OnContextMenu(ExplorerData* item, const WinPoint& pt);

	private:
		enum ExMenuItem
		{
			kExMenuItemNewPack = 1,
			kExMenuItemNewFolder,
			kExMenuItemRename,
			kExMenuItemDelete,
			kExMenuItemDuplicate,
			kExMenuItemOpen,
			kExMenuItemId,
			kExMenuItemNewRecord = 500,
		};

		HMENU AppendNewMenu(HMENU rootMenu, ExplorerData* item, Btl::OpenHashMap<UINT, Btl::String>& typeHash);

		C_SorceryPanel* m_pSorceryPanel;
	};

	class C_SorceryRecordWindow : public wxMDIChildFrame
	{
	public:
		C_SorceryRecordWindow(C_SorceryPanel* pPanel, wxMDIParentFrame* pParent, Xt::DomRecord* pRecord, long style=0);
		~C_SorceryRecordWindow();

		void Shutdown();

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_SorceryRecordWindow* pForm);
			void Connect();
			void Disconnect();

			void OnDatabaseEventSlot(Xt::DomDb&, EditorLib::E_DatabaseEvent);

		private:
			C_SorceryRecordWindow* m_pSorceryWindow;
		};

		void OnSize(wxSizeEvent& e);

		wxPanel*			m_pScarabFormPanel;
		ScarabForm*			m_pScarabForm;

		C_Slots				m_Slots;

		bool				m_Connected;
	};

	class C_SorceryPanel : public wxPanel
	{
	public:
		C_SorceryPanel(wxWindow *parent, wxWindowID winid = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxSize(300, 500),
							long style = wxTAB_TRAVERSAL | wxNO_BORDER,
							const wxString& name = wxPanelNameStr);

		~C_SorceryPanel();

		void Shutdown();

		void OpenRecord(Xt::DomRecord* pRecord);

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_SorceryPanel* pForm);
			void Connect();
			void Disconnect();

			void OnDatabaseEventSlot(Xt::DomDb&, EditorLib::E_DatabaseEvent);

		private:
			C_SorceryPanel* m_pSorceryPanel;
		};

		struct C_Flags
		{
			bool Connected	 : 1;
		};

		void OnSize(wxSizeEvent& e);

		wxPanel*			m_pExplorerPanel;
		wxMDIParentFrame*	m_pMDIFrame;

		Explorer*			m_pExplorer;

		wxSplitterCtrl*		m_pSplitter;

		C_Slots				m_Slots;
		C_Flags				m_Flags;

		FRIEND_CLASS C_Slots;
		FRIEND_CLASS C_SorceryExplorer;
	};
}

#endif