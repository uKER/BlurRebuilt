#ifndef CONSTRAINTSPANEL_H
#define CONSTRAINTSPANEL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AnimationTools
{
	class C_ExportConstraints;
}

namespace Animation
{
	class C_Skeleton;
}

class wxListCtrl;

/*
Use a list view.
Constrained joint name, Constraint type, evaluation order, description
Lots of constraint specific info
	- display as string in the final column?
	- double click to open up constraint specific dialog?
	- Use some kind of custom control that opens up
	  a small list of stuff?
	

*/

class wxListEvent;

class C_ConstraintsPanel: public wxPanel
{
public:
	C_ConstraintsPanel(
		wxWindow* parent,
		wxWindowID winId = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL | wxNO_BORDER
	);

	void SetConstraints(
		const AnimationTools::C_ExportConstraints* constraints,
		const Animation::C_Skeleton* skeleton );

	void OnColumnClicked(wxListEvent& e);
	void OnItemSelected(wxListEvent& e);
	void OnItemActivated(wxListEvent& e);

	

private:


	enum E_Column
	{
		E_Column_Order			= 0,
		E_Column_Name,
		E_Column_Type,
		E_Column_Details,
		E_Column_Count
		
	};
	

	void Populate();
	void Sort(E_Column sortColumn, bool sortAscending);

	wxListCtrl*						m_listCtrl;
	wxStaticText*					m_infoText;
	const AnimationTools::C_ExportConstraints*	m_constraints;
	const Animation::C_Skeleton*			m_skeleton;
	E_Column m_sortColumn;
	bool m_sortAscending;

};

#endif
