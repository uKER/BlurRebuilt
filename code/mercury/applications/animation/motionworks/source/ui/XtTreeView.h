#ifndef XTTREEVIEW_H
#define XTTREEVIEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include <wx/treectrl.h>
#include <xt/xttool.h>


class C_XtDatabase;
class C_AnimationGraphEditor;
class C_XtWindow;

class C_XtTreeItemData: public wxTreeItemData
{
public:
	enum E_Type
	{
		E_Type_Root	= 0,
		E_Type_Record,
		E_Type_Pack,
		E_Type_Folder
	};
	
	explicit C_XtTreeItemData( bool isRoot = false ):
		domPack(NULL), domRecord(NULL), type(isRoot ? E_Type_Root : E_Type_Folder)
	{}
	
	C_XtTreeItemData( Xt::DomRecord* record ):
		domPack(NULL),
		domRecord(record),
		type(E_Type_Record)
	{
	}
	
	C_XtTreeItemData( Xt::DomPack* pack ):
		domPack(pack),
		domRecord(NULL),
		type( E_Type_Pack )
	{
	}
	
	bool IsRoot() const	{ return type == E_Type_Root; }
	bool IsFolder() const	{ return type == E_Type_Folder; }
	bool IsRecord() const	{ return type == E_Type_Record; }
	bool IsPack() const	{ return type == E_Type_Pack; }

public:	
	Xt::DomPack*	domPack;
	Xt::DomRecord*	domRecord;
	E_Type			type;
};


// Stolen from ActorStudio :)
class C_XtTreeView: public wxTreeCtrl
{
public:

	enum E_IconType
	{
		E_IconType_Root	 = 0,
		E_IconType_Record,
		E_IconType_Pack,
		E_IconType_Filter,
		E_IconType_Count
	};
	
	C_XtTreeView(
		C_XtDatabase& db,
		C_AnimationGraphEditor& animGraphEditor,
		C_XtWindow& xtWindow,
		wxWindow* parent,
		wxWindowID winId = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		U32 styleFlags = wxTR_EDIT_LABELS|wxTR_HAS_BUTTONS|wxTR_SINGLE|wxTR_EXTENDED|wxTR_FULL_ROW_HIGHLIGHT|wxTAB_TRAVERSAL|wxHSCROLL|wxVSCROLL
	);
	
	virtual ~C_XtTreeView();

	void Populate();
	void SelectRecord( Xt::DomRecord* record );

	void OnContextMenu( wxTreeEvent& e );

	void OnItemRenameStart( wxTreeEvent& e );
	void OnItemRenameEnd( wxTreeEvent& e );
	void OnItemActivated( wxTreeEvent& e );
	void OpenItem( wxTreeItemId id );
	
	void OnNewPack( wxCommandEvent& e );
	void OnNewGraph( wxCommandEvent& e );
	void OnNewFolder( wxCommandEvent& e );
	void OnRename( wxCommandEvent& e );
	void OnDelete( wxCommandEvent& e );
	void OnOpen( wxCommandEvent& e );
	void OnKeyDown( wxKeyEvent& e );
	
	bool IsRecordSelected() const;
	
	const C_XtTreeItemData* GetXtTreeItemData( wxTreeItemId id ) const;
	
private:
	C_XtDatabase&			m_db;
	C_AnimationGraphEditor&	m_animGraphEditor;
	C_XtWindow&				m_xtWindow;

};




#endif
