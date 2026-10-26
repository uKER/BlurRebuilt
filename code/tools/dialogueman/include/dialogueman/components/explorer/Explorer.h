#ifndef DMAN_EXPLORER_H
#define DMAN_EXPLORER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_Explorer : public XtWin::C_Explorer
	{
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

		void Shutdown();

	protected:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_Explorer* pExplorer);
			void Connect();
			void Disconnect();

			void OnAppDbOpenedSlot();
			void OnAppDbCloseSlot();
			void OnAppSelectionChangedSlot();
			
			FRIEND_CLASS C_Explorer;

		private:
			C_Explorer* m_pExplorer;
		};

		virtual void Reload();

		void OnAppDbOpened();
		void OnAppDbClose();
		void OnAppSelectionChanged();

		void OnNodeSelect(wxTreeEvent & e);

		virtual void OnCreateNewRecord(wxCommandEvent& evt);
		virtual void OnDeleteRecord(wxCommandEvent& evt);
		
		Xt::DomRecord* FindRecord(Xt::DomPack* pPack, const XtString& name);

		C_Slots		m_slots;

	private:
		void ObtainLeafRecords(Btl::Vector<Xt::DomRecord*>& leafs, const wxTreeItemId& parent);
	};
}

#endif //DMAN_EXPLORER_H