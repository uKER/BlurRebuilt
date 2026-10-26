#ifndef EDITOR_ACTORTYPE_TOOLPANEL_H
#define EDITOR_ACTORTYPE_TOOLPANEL_H


#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>

namespace Editor
{
	class C_ActorToolPanel : public wxPanel
	{
	public:
		C_ActorToolPanel(wxWindow *parent, wxWindowID id = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxDefaultSize,
							long style = wxTAB_TRAVERSAL|wxNO_BORDER);

		~C_ActorToolPanel();

		void OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);
		void OnSelectionChanged();
		void OnActorTypeChanged(wxCommandEvent& e);

	protected:
		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_ActorToolPanel *pToolPanel);
			void Connect();
			void Disconnect();

			void OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);
			void OnCoreSelectionChangedSlot();
		private:
			C_ActorToolPanel	*m_pToolPanel;
		};

		C_Slots		m_Slots;
		wxComboBox	*m_pActorTypeComboBox;

	};

}

#endif