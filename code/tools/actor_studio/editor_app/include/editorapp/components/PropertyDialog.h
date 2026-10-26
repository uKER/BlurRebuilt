#ifndef EDITOR_WX_XT_PROPERTY_DIALOG_H
#define EDITOR_WX_XT_PROPERTY_DIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

class wxPropertyGridManager;
class wxPropertyGridEvent;
class wxTreeCtrl;
class wxTreeEvent;

namespace Editor
{
	class C_PropertyDialog;

	class I_PropertyGroup
	{
	public:
		virtual ~I_PropertyGroup() { }

		virtual void Initialise(wxPropertyGridManager* pPgMgr) = 0;
		virtual void OnPropertyChanged(wxPropertyGridEvent& e) = 0;
		virtual void Apply() = 0;
		virtual void Revert() = 0;
	};

	class C_PropertyDialog : public wxDialog
	{
	public:
		C_PropertyDialog(
			wxWindow* pParent, 
			wxWindowID id = wxID_ANY,
			const wxString& title = wxString("Properties"),
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(600,400),
			long style = wxDEFAULT_DIALOG_STYLE);

		~C_PropertyDialog();

		static void OnShowDialog();

		void AppendPropertyGroup(I_PropertyGroup *pPropGroup);

	private:
		void PopulateTree();

		void OnShow(wxShowEvent& e);

		void OnNodeSelect(wxTreeEvent& e);
		void OnPropertyChanged(wxPropertyGridEvent& e);
		void OnPropertySelected(wxPropertyGridEvent& e);

		void OnUpdateUI(wxUpdateUIEvent& e);
		void OnButtonClick(wxCommandEvent& e);

		Ut::Vector<I_PropertyGroup*> m_pPropertyGroups;
		wxTreeCtrl*		m_pTreeCtrl;
		wxStaticText*	m_pHelpText;
		wxButton*		m_pButtonCancel;
		wxButton*		m_pButtonOK;

		wxPropertyGridManager*		m_pPropGridManager;
	};

}

#endif
