#ifndef EDITOR_DYNAMIC_OBJECT_DIALOG_H
#define EDITOR_DYNAMIC_OBJECT_DIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>

class wxNotebook;

namespace Editor
{

class C_DynamicObjectPanel : public wxPanel
{
public:
	C_DynamicObjectPanel(wxWindow *parent, wxWindowID winid = wxID_ANY,
						const wxPoint& pos = wxDefaultPosition,
						const wxSize& size = wxSize(300, 500),
						long style = wxTAB_TRAVERSAL | wxNO_BORDER,
						const wxString& name = wxPanelNameStr);

	~C_DynamicObjectPanel();

	void Shutdown();

private:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		C_Slots(C_DynamicObjectPanel* pForm);
		void Connect();
		void Disconnect();

		void OnDatabaseEventSlot(Xt::DomDb&, EditorLib::E_DatabaseEvent);

	private:
		C_DynamicObjectPanel* m_pDynamicObjectPanel;
	};

	struct C_Flags
	{
		U32 dataDirty : 1;
		U32 connected : 1;
	};

	void OnRefresh(wxCommandEvent& e);
	void OnSelectObject(wxCommandEvent& e);
	void OnLevelComboChanged(wxCommandEvent& e);
	void OnIdle(wxIdleEvent& e);

	void OnCreateObject(EditorLib::I_DgNode* dgNode);

	void RefreshVisualisationLocations(Btl::String location, U32 depth=0);
	void PopulateVisualisations(Btl::String filePath, Btl::String visualisation);

	void Populate();
	void PopulateTabs(const Btl::String& level);
	wxWindow* GetTab(const Btl::String& tabName);

	Btl::OpenHashMap<int, XtId> m_WxXtMap;
	Btl::OpenHashMap<Btl::String, wxWindow*, Btl::HashKeyOpBtlString> m_Tabs;
	Btl::Vector<Btl::Pair<LtStruct*,Xt::DomRecord*> > m_DynamicObjects;
	Btl::OpenHashMap<Btl::String, Btl::String, Btl::HashKeyOpBtlString> m_Visualisations;

	wxComboBox*			m_pLevel;
	wxButton*			m_pRefresh;
	wxNotebook*			m_pTabs;

	XtId				m_ObjectId;
	Btl::String			m_ObjectName;

	C_Flags				m_Flags;
	C_Slots				m_Slots;
};

}

#endif