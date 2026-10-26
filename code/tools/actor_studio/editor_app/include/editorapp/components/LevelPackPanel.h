#ifndef EDITOR_LEVEL_PACK_PANEL_H
#define EDITOR_LEVEL_PACK_PANEL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>

#include <xtwin/Explorer.h>

namespace EditorLib
{
	enum E_PackEvent;
}

namespace Editor
{

class C_LevelPackPanel;

class C_LevelPackPanelExplorer : public Explorer
{
public:
	C_LevelPackPanelExplorer(C_LevelPackPanel* pPanel);

protected:
	virtual bool OnFilterItem(Explorer::ExplorerData* pData);
	virtual void OnContextMenu(Explorer::ExplorerData* pData, const WinPoint& pt);
	virtual void OnActivateItem(Explorer::ExplorerData* pData);

private:
	C_LevelPackPanel* m_pPanel;
};

class C_LevelPackPanel : public wxPanel
{
public:
	C_LevelPackPanel(
			wxWindow *parent, 
			wxWindowID = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200,400),
			long style = wxTAB_TRAVERSAL|wxNO_BORDER);

	~C_LevelPackPanel();

	void Shutdown();

	void ShowMenu(Explorer::ExplorerData* pData, const WinPoint& pt);

private:
	void OnDatabaseEvent(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);
	void OnPackEvent(const EditorLib::C_LevelPack& level, EditorLib::E_PackEvent evt);

private:
	void OnSize(wxSizeEvent& e);
	void OnClosePack(wxCommandEvent& e);
	void OnLoadLevel(wxCommandEvent& e);
	void OnUnloadLevel(wxCommandEvent& e);
	void OnSetActiveLevel(wxCommandEvent& e);

	wxPanel*  m_pTreeCtrlPanel;
	C_LevelPackPanelExplorer* m_pExplorer;

private:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		C_Slots(C_LevelPackPanel* pLevelPackPanel);
		void Connect();
		void Disconnect();
		
		void OnDatabaseEventSlot(Xt::DomDb& db, EditorLib::E_DatabaseEvent evt);
		void OnPackEventSlot(const EditorLib::C_LevelPack& level, EditorLib::E_PackEvent evt);

		FRIEND_CLASS C_LevelPackPanel;

	private:
		C_LevelPackPanel* m_pLevelPackPanel;
	};

	Explorer::ExplorerData* m_pRCRecord;

	C_Slots	m_slots;
};

}

#endif