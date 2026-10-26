#ifndef EDITOR_LAYER_PROPERTY_PANEL_H
#define EDITOR_LAYER_PROPERTY_PANEL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>

namespace Editor
{
	class C_LayerPropertyMenuBar;

	#define DEF(e, str) e,
	enum E_LayerPropertyIcon
	{
		#include "LayerPropertyIconMapping.h"
		E_LayerPropertyIcon_Count
	};
	#undef DEF

	class C_LayerPropertyCtrl : public wxControl
	{
	public:

		enum E_CtrlType
		{
			E_CtrlType_Visibility,
			E_CtrlType_Selectable,
			E_CtrlType_Colour,
			E_CtrlType_Count,
			E_CtrlType_Unknown = E_CtrlType_Count
		};

		struct C_Flags
		{
			bool connected		: 1;
			bool pendingSync	: 1;
		};

		struct C_HitTestResult
		{
			S32					index;
			EditorLib::I_Layer*	pLayer;
			E_CtrlType			ctrlType;
		};

		struct C_LayerData
		{
			EditorLib::I_Layer*	pLayer;
			bool				selected;
		};

		typedef Btl::Vector<C_LayerData> LayerDataArray;

		C_LayerPropertyCtrl(wxWindow *parent, wxWindowID id = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxDefaultSize, long style = wxNO_BORDER);
		
		~C_LayerPropertyCtrl();

		void OnMouseEvent(wxMouseEvent& e);		
		void OnPaint(wxPaintEvent& e);
		void OnIdle(wxIdleEvent& e);
		void OnTextLabelEraseBackground(wxEraseEvent& e);
		void OnTextLabelEditFocus(wxFocusEvent& e);
		void OnTextLabelEditEnd(wxCommandEvent& e);

		wxSize DoGetBestSize() const;
		
		void Shutdown();
		void EditLayerName();
		void EditLayerName(const EditorLib::I_Layer* pLayer);
		void SetSelectedState(const EditorLib::I_Layer* pLayer, bool state);
		bool GetSelectedState(const EditorLib::I_Layer* pLayer) const;
		const LayerDataArray& GetLayerData() const;

		C_HitTestResult HitTest(wxCoord x, wxCoord y) const;

	protected:

		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_LayerPropertyCtrl *pLayerPropCtrl);
			void Connect();
			void Disconnect();

			void OnLevelOpeningSlot(const EditorLib::I_Level& level);
			void OnLevelOpenedSlot(const EditorLib::I_Level& level);
			void OnLevelClosingSlot(const EditorLib::I_Level& level);
			void OnLevelClosedSlot(const EditorLib::I_Level& level);

			void OnLayerEventSlot(const EditorLib::I_Layer& layer, EditorLib::E_LayerEvent e);
			void OnLayerDeserialisedSlot(const EditorLib::I_Layer& layer);

		private:
			C_LayerPropertyCtrl		*m_pLayerPropCtrl;

			FRIEND_CLASS C_LayerPropertyCtrl;
		};

		void OnLevelOpening(const EditorLib::I_Level& level);
		void OnLevelOpened(const EditorLib::I_Level& level);
		void OnLevelClosing(const EditorLib::I_Level& level);
		void OnLevelClosed(const EditorLib::I_Level& level);

		void OnLevelLayerAdded(const EditorLib::I_Layer& layer);
		void OnLevelLayerRemoved(const EditorLib::I_Layer& layer);
		void OnLayerDeserialised(const EditorLib::I_Layer& layer);

		void OnShowContextMenu();
		void OnSelectLayerObjects();
		void OnAddToLayer();
		void OnRemoveFromLayer();
		void OnDeleteLayer();
		
		void OnEditLayerNameUpdate(wxUpdateUIEvent& e);
		void OnAddToLayerUpdate(wxUpdateUIEvent& e);
		void OnRemoveFromLayerUpdate(wxUpdateUIEvent& e);


		wxImageList			*m_pImageList;
		wxTextCtrl			*m_pTextCtrl;
		
		LayerDataArray		m_layerDataArray;
		EditorLib::I_Layer	*m_pCurrentLayer;
		C_Flags				m_flags;
		C_Slots				m_slots;
		S32					m_lastSelection;

		wxMenu				*m_pContextMenu;

		static const char* m_pIconStringTable[E_LayerPropertyIcon_Count];

		DECLARE_CLASS(C_LayerPropertyCtrl);
		DECLARE_EVENT_TABLE();
	};


	class C_LayerPropertyPanel : public wxPanel
	{
	public:
		C_LayerPropertyPanel(wxWindow *parent, wxWindowID winid = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxSize(300, 500),
							long style = wxTAB_TRAVERSAL | wxNO_BORDER,
							const wxString& name = wxPanelNameStr);

		~C_LayerPropertyPanel();

		C_LayerPropertyCtrl*		GetLayerPropertyCtrl();
		void Shutdown();

	private:

		C_LayerPropertyMenuBar		*m_pMenuBar;
		C_LayerPropertyCtrl			*m_pLayerPropCtrl;

		DECLARE_CLASS(C_LayerPropertyPanel)
	};
}

#endif