#ifndef EDITOR_SCENENODE_PROPERTY_PANEL_H
#define EDITOR_SCENENODE_PROPERTY_PANEL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>

namespace EditorLib
{
	class I_UpdateHint;
}

namespace Editor
{
	class C_SceneNodeProperties;

	class C_SceneNodePropertyPanel : public wxPanel
	{
	public:
		C_SceneNodePropertyPanel(wxWindow *parent, wxWindowID id=wxID_ANY,
								const wxPoint& pos = wxDefaultPosition,
								const wxSize& size = wxDefaultSize,
								long style = wxTAB_TRAVERSAL | wxNO_BORDER);

		~C_SceneNodePropertyPanel();

		void OnSelectionChanged();
		void OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode);
		void OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode);

		void OnPropertyChange(wxPropertyGridEvent& e);

		C_SceneNodeProperties* GetSceneNodePropertiesPtr();
		const C_SceneNodeProperties* GetSceneNodePropertiesPtr() const;

	private:

		class C_Slots : public sigslot::has_slots<>
		{
		public: 
			C_Slots(C_SceneNodePropertyPanel* pPanel);
			void Connect();
			void Disconnect();

			void OnCoreSelectionChangedSlot();
			void OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc);
			void OnDgNodeDestroySlot(const EditorLib::I_DgNode& dgNode);

		private:
			C_SceneNodePropertyPanel *m_pSceneNodePropertyPanel;

			FRIEND_CLASS C_SceneNodePropertyPanel;
		};

		wxPropertyGridManager	*m_pPropertyGridManager;
		C_SceneNodeProperties	*m_pProperties;
		wxStaticText			*m_pLabel;

		C_Slots					m_Slots;

		DECLARE_EVENT_TABLE()
	};
}


#endif