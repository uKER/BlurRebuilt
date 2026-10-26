#ifndef DMAN_WX_SCARAB_FORM_H
#define DMAN_WX_SCARAB_FORM_H

#ifdef _MSC_VER
#pragma once
#endif

class ScarabForm;

namespace DMan
{
	class C_WxScarabForm : public wxWindow
	{
	public:
		C_WxScarabForm(
			wxFrame* pFrame, 
			wxWindow* pParent, 
			const wxWindowID id=wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxDEFAULT_FRAME_STYLE);

		~C_WxScarabForm();

		bool SetRecord(Xt::DomRecord* pRecord);
		bool SetDom(Xt::DomElement* pElement, LarvaType* pLarvaType);
		bool SetDom(Xt::DomComponent* pElement, LarvaType* pLarvaType);
		bool SetDom(Xt::DomBlock* pBlock, LarvaType* pLarvaType);

		void OnSelectionChanged();

		ScarabForm*		GetScarabForm();

		void OnSize(wxSizeEvent& e);

		void Shutdown();

	private:
		class C_Flags
		{
		public:
			int dirty : 1;
		};

		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_WxScarabForm* pForm);
			void Connect();
			void Disconnect();

			void OnSelectionChangedSlot();
			void OnDbCloseSlot();

			FRIEND_CLASS C_WxScarabForm;

		private:
			C_WxScarabForm* m_pForm;
		};

		void OnIdle(wxIdleEvent& e);
		void OnDbClose();

		C_Slots			m_slots;
		C_Flags			m_flags;
		wxFrame*		m_pFrame;
		ScarabForm*		m_pScarabForm;
	};
}

#endif //DMAN_WX_SCARAB_FORM_H
