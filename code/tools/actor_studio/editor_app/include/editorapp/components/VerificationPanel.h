#ifndef EDITOR_VERIFICATION_PANEL_H
#define EDITOR_VERIFICATION_PANEL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xtwin/TreeCtrl.h>

namespace Editor
{

class C_VerificationPanel;

class C_VerificationTreeItem : public TreeItem
{
public:
	virtual bool OnItemDeleting();
	virtual bool OnStartRename(Sz16& newName);
};

class C_VerificationTreeItemLevel : public C_VerificationTreeItem
{
public:
	C_VerificationTreeItemLevel(C_VerificationPanel* pPanel, XtId xtId);
	virtual void OnContextMenu(const WinPoint& pt);
private:
	C_VerificationPanel* m_pVerificationPanel;
	XtId m_XtId;
};

class C_VerificationTreeItemActor : public C_VerificationTreeItem
{
public:
	C_VerificationTreeItemActor(XtId xtId);
	virtual bool OnActivate();
	virtual bool OnItemSelectionChanged(bool selected);
private:
	XtId m_XtId;
};

class C_VerificationTreeItemXt : public C_VerificationTreeItem
{
public:
	C_VerificationTreeItemXt(XtId xtId);
	virtual bool OnActivate();
private:
	XtId m_XtId;
};

class C_VerificationTree : public TreeCtrl
{
public:
	C_VerificationTree();
};

class C_VerificationPanel : public wxPanel
{
public:
	C_VerificationPanel(wxWindow *parent, wxWindowID winid = wxID_ANY,
					 const wxPoint& pos = wxDefaultPosition,
					 const wxSize& size = wxSize(300, 500),
					 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
					 const wxString& name = wxPanelNameStr);

	~C_VerificationPanel();

	void Shutdown();

private:
	void OnSize(wxSizeEvent& e);
	void OnButton(wxCommandEvent& e);
	void Populate();

	wxPanel* m_ResultsPanel;
	C_VerificationTree* m_Results;
	wxButton* m_DbVerificationButton;
	wxButton* m_AllLevelVerificationButton;
	wxButton* m_LevelVerificationButton;
};

}

#endif