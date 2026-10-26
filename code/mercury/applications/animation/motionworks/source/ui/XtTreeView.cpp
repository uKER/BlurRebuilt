#include "stdafx.h"
#include "XtTreeView.h"
#include "XtWindow.h"
#include <btl/containers/map.h>
#include <btl/containers/vector.h>
#include <wx/wupdlock.h>
#include "../Db.h"

wxImageList* LoadIcons( const char** iconFilenames, U32 numIcons )
{
	wxImageList* icons = new wxImageList( 16, 16 );
	for ( U32 i = 0; i < numIcons; ++i )
	{
		wxImage img;
		bool loadedImage = false;
		if ( iconFilenames[i] != NULL )
		{
			wxString filename = "icons/";
			filename += iconFilenames[i];
			filename += ".png";
			loadedImage = img.LoadFile( filename );
		}
		icons->Add( img );
	}
	return icons;
}

C_XtTreeView::C_XtTreeView(
	C_XtDatabase& db,
	C_AnimationGraphEditor& animGraphEditor,
	C_XtWindow& xtWindow,
	wxWindow* parent,
	wxWindowID winId,
	const wxPoint& pos,
	const wxSize& size,
	U32 styleFlags
) : wxTreeCtrl(parent, winId, pos, size, styleFlags, wxDefaultValidator),
	m_db(db),
	m_animGraphEditor(animGraphEditor),
	m_xtWindow(xtWindow)
{
	Connect(winId, wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler(C_XtTreeView::OnContextMenu));
	Connect(winId, wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT, wxTreeEventHandler(C_XtTreeView::OnItemRenameStart));
	Connect(winId, wxEVT_COMMAND_TREE_END_LABEL_EDIT, wxTreeEventHandler(C_XtTreeView::OnItemRenameEnd));
	Connect(winId, wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(C_XtTreeView::OnItemActivated));
	Connect(winId, wxEVT_KEY_DOWN, wxKeyEventHandler(C_XtTreeView::OnKeyDown) );
	
	
	static const char* kClosedIconFilenames[E_IconType_Count] =
	{
		"xt_treeview_root",
		"xt_treeview_record",
		"xt_treeview_pack_closed",
		"xt_treeview_filter_closed"
	};

	static const char* kOpenIconFilenames[E_IconType_Count] =
	{
		"xt_treeview_root",
		"xt_treeview_record",
		"xt_treeview_pack_open",
		"xt_treeview_filter_open"
	};
	AssignImageList( LoadIcons( kClosedIconFilenames, E_IconType_Count ) );
	AssignStateImageList( LoadIcons( kOpenIconFilenames, E_IconType_Count ) );
}

C_XtTreeView::~C_XtTreeView()
{
	Disconnect(GetId(), wxEVT_COMMAND_TREE_ITEM_MENU);
	Disconnect(GetId(), wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT);
	Disconnect(GetId(), wxEVT_COMMAND_TREE_END_LABEL_EDIT);
	Disconnect(GetId(), wxEVT_COMMAND_TREE_ITEM_ACTIVATED );
}

void C_XtTreeView::Populate()
{
	wxWindowUpdateLocker updateLock( this );

	DeleteAllItems();

	Xt::DomDb* db = m_db.GetDomDb();
	
	if ( NULL == db ) return;
	
	wxTreeItemId rootId = AddRoot( db->name.Text(), E_IconType_Root, -1, new C_XtTreeItemData( true ) );
	
	for(Xt::DomDb::Iterator packIt(db); packIt; ++packIt)// XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&db->packs); packIt; ++packIt)
	{
		Xt::DomPack* pack = packIt;
		wxTreeItemId packId = AppendItem( rootId, pack->name.Text(), E_IconType_Pack, -1, new C_XtTreeItemData(pack) );

		Btl::Map<XtString, wxTreeItemId> filters;

		for(Xt::DomPack::Iterator it(pack); it; ++it)
		{
			Xt::DomRecord* record = it;
			
			S32 labelStart = LastIndexOf(record->name, ".") + 1;

			wxTreeItemId currentId = packId;
			const char* text = record->name;
			const char* prevText = text;

			// Generate intermediate folder nodes using record name
			for ( S32 split=0; split<labelStart; ++split )
			{
				if (*text++ == '.')
				{
					XtString str( record->name.Text(), split );
					Btl::Map<XtString, wxTreeItemId>::ConstIterator it = filters.Find(str);
					if (it == filters.End())
					{
						XtString label(prevText, text-prevText-1);
						currentId = AppendItem(currentId, label.Text(), E_IconType_Filter, -1, new C_XtTreeItemData );
						filters.Insert(Btl::Map<XtString, wxTreeItemId>::DataType(str, currentId));
					}
					else
					{
						currentId = it->GetValue();
					}

					prevText = text;
				}
			}

			wxString label = &(record->name.Text()[labelStart]);
			label << "." << record->root.type.Text();
			currentId = AppendItem(currentId, label, E_IconType_Record, -1, new C_XtTreeItemData(record));
		}
	}


	//Sort nodes
	{
		SortChildren(rootId);

		Btl::Vector<wxTreeItemId> stack;

		wxTreeItemIdValue cookie;
		wxTreeItemId child = GetFirstChild(rootId, cookie);
		if (child.IsOk())
			stack.PushBack(child);

		while (stack.Size() > 0)
		{
			child = stack.Back();
			if (GetChildrenCount(child))
				SortChildren(child);

			child = GetFirstChild(child, cookie);
			if (child.IsOk())
				stack.PushBack(child);
			else
			{
				while (stack.Size() && !child.IsOk())
				{
					child = stack.Back();
					stack.PopBack();
					child = GetNextSibling(child);

					if (child.IsOk())
						stack.PushBack(child);
				}
			}
		}
	}

	Expand(rootId);
}

void C_XtTreeView::SelectRecord( Xt::DomRecord* record )
{
	Btl::Vector<wxTreeItemId> stack;

	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild(GetRootItem(), cookie);
	if (child.IsOk())
		stack.PushBack(child);

	while (stack.Size() > 0)
	{
		child = stack.Back();
		
		// process.
		const C_XtTreeItemData* itemData =  (const C_XtTreeItemData*) GetItemData( child );
		
		if ( itemData )
		{
			if ( itemData->IsRecord() && itemData->domRecord == record )
			{
				SelectItem( child );
				EnsureVisible( child );
				return;
			}
		}

		child = GetFirstChild(child, cookie);
		if (child.IsOk())
		{
			stack.PushBack(child);
		}
		else
		{
			while (stack.Size() && !child.IsOk())
			{
				child = stack.Back();
				stack.PopBack();
				child = GetNextSibling(child);

				if (child.IsOk())
				{
					stack.PushBack(child);
				}
			}
		}
	}
}

void C_XtTreeView::OnContextMenu( wxTreeEvent& e )
{
    wxTreeItemId itemId = e.GetItem();
    wxPoint clientPt = e.GetPoint();
    e.Skip();
    
    if ( !itemId.IsOk() )
	{
		return;
	}
   
	enum
	{
		E_MenuId_NewPack	= 0,
		E_MenuId_NewGraph,
		E_MenuId_NewFolder,
		E_MenuId_Rename,
		E_MenuId_Delete,
		E_MenuId_Open
	};

	SelectItem( itemId );

	const C_XtTreeItemData* itemData = (const C_XtTreeItemData*) GetItemData( itemId );

	wxMenu* menu = new wxMenu; // ( GetItemText( itemId ) );
	
	if ( itemData )
	{
		switch ( itemData->type )
		{
			case C_XtTreeItemData::E_Type_Root:
				menu->Append( E_MenuId_NewPack, _T("New Pack") );
				break;
				
			case C_XtTreeItemData::E_Type_Folder:
			case C_XtTreeItemData::E_Type_Pack:
				menu->Append( E_MenuId_NewGraph, _T("New Graph") );
				//menu->Append( E_MenuId_NewFolder, _T("New Folder") );
				menu->AppendSeparator();
				menu->Append( E_MenuId_Rename, _T("Rename") );
				menu->Append( E_MenuId_Delete, _T("Delete") );
				break;
			
			case C_XtTreeItemData::E_Type_Record:
				// Open (in bold)
				// separator.
				menu->Append( E_MenuId_Open, _T("Open") );
				menu->AppendSeparator();
				menu->Append( E_MenuId_Rename, _T("Rename") );
				menu->Append( E_MenuId_Delete, _T("Delete") );
				break;
				
			default:
				break;
		}
	}

	Connect( E_MenuId_NewPack, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( C_XtTreeView::OnNewPack ) );
	Connect( E_MenuId_NewFolder, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( C_XtTreeView::OnNewFolder ) );
	Connect( E_MenuId_NewGraph, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( C_XtTreeView::OnNewGraph ) );
	Connect( E_MenuId_Rename, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( C_XtTreeView::OnRename ) );
	Connect( E_MenuId_Delete, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( C_XtTreeView::OnDelete ) );
	Connect( E_MenuId_Open, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( C_XtTreeView::OnOpen ) );

	PopupMenu( menu, clientPt );

	Disconnect( E_MenuId_NewPack, wxEVT_COMMAND_MENU_SELECTED );
	Disconnect( E_MenuId_NewFolder, wxEVT_COMMAND_MENU_SELECTED );
	Disconnect( E_MenuId_NewGraph, wxEVT_COMMAND_MENU_SELECTED );
	Disconnect( E_MenuId_Rename, wxEVT_COMMAND_MENU_SELECTED );
	Disconnect( E_MenuId_Delete, wxEVT_COMMAND_MENU_SELECTED );
	Disconnect( E_MenuId_Open, wxEVT_COMMAND_MENU_SELECTED );

	delete menu;
}

void C_XtTreeView::OnItemRenameStart( wxTreeEvent& e )
{
	wxTreeItemId selectedItem = e.GetItem();
	
	const C_XtTreeItemData* itemData =  (const C_XtTreeItemData*) GetItemData( selectedItem );
	
	if ( itemData && (itemData->IsPack() || itemData->IsRecord()) )
	{
		if ( itemData->IsRecord() )
		{
			XtString fullName = itemData->domRecord->name;
			S32 lastDot = LastIndexOf( fullName, "." );
			XtString name = Substring( fullName, lastDot + 1 );
			SetItemText( selectedItem, name.Text() );
		}
	}
	else
	{
		e.Veto();
	}
}

void C_XtTreeView::OnItemRenameEnd( wxTreeEvent& e )
{
	if (e.IsEditCancelled())
	{
		return;
	}

	wxTreeItemId selectedItem = e.GetItem();
	
	const C_XtTreeItemData* itemData =  (const C_XtTreeItemData*) GetItemData( selectedItem );
	
	if ( itemData && (itemData->IsPack() || itemData->IsRecord()) )
	{
		XtString label = e.GetLabel().c_str();
		if ( itemData->IsPack() )
		{
			// Disallow renaming to the same name as an already existing pack.
			if ( m_db.FindPack( label ) )
			{
				e.Veto();
			}
			else
			{
				itemData->domPack->name = label;
				SetDirty( itemData->domPack, false );
				m_db.Commit();
			}
		}
		else if ( itemData->IsRecord() )
		{
			e.Veto();
			
			XtString newText = XtsStripBoth(label, ' ');
			
			if ( XtsIsName( newText ) )
			{
				XtString newLabel = newText + "." + itemData->domRecord->root.type;
				SetItemText( selectedItem, newLabel.Text() );
			
				S32 lastDotIndex = LastIndexOf( itemData->domRecord->name, "." );
				XtString path = Substring( itemData->domRecord->name, 0, lastDotIndex + 1 );
				
				itemData->domRecord->name = path + newText;
				SetDirty(itemData->domRecord, false );
				m_db.Commit();
				
				if (m_xtWindow.GetRecord() == itemData->domRecord)
				{
					m_xtWindow.UpdateCaption();
				}
			}
		}
	}
	else
	{
		e.Veto();
	}
}

void C_XtTreeView::OnItemActivated( wxTreeEvent& e )
{
	OpenItem( e.GetItem() );
}

void C_XtTreeView::OpenItem( wxTreeItemId id )
{
	if ( !id.IsOk() ) return;
	const C_XtTreeItemData* itemData = (const C_XtTreeItemData*) GetItemData( id );
	
	if ( itemData )
	{
		switch ( itemData->type )
		{
			case C_XtTreeItemData::E_Type_Root:
			case C_XtTreeItemData::E_Type_Folder:
			case C_XtTreeItemData::E_Type_Pack:
				break;
				
			case C_XtTreeItemData::E_Type_Record:
				m_animGraphEditor.SetCurrentGraphRecord( itemData->domRecord );
				m_xtWindow.SetRecord( itemData->domRecord );
				m_db.UpdateFromRecord( itemData->domRecord );
				break;
				
			default:
				break;
		}
	}
}

void C_XtTreeView::OnNewPack( wxCommandEvent& e )
{
	Xt::DomPack* pack = m_db.NewPack( "New Pack" );
	
	m_db.Commit();
	
	// Add to tree view and allow user to edit name.
	wxTreeItemId packItemId = AppendItem( GetRootItem(), pack->name.Text(), E_IconType_Pack, -1, new C_XtTreeItemData( pack ) );
	//SelectItem( packItemId );
	EnsureVisible( packItemId );
	EditLabel( packItemId );
}

void C_XtTreeView::OnNewGraph( wxCommandEvent& e )
{
	wxTreeItemId id = GetSelection();
	if ( !id.IsOk() ) return;
	
	const C_XtTreeItemData* itemData = (const C_XtTreeItemData*) GetItemData( id );
	
	if ( !itemData ) return;
	if ( !itemData->IsPack() ) return;
	
	Xt::DomRecord* record = m_animGraphEditor.NewGraphRecord( itemData->domPack, "New Graph" );
	
	m_db.Commit();
	
	wxTreeItemId graphId = AppendItem( id, record->name.Text(), E_IconType_Record, -1, new C_XtTreeItemData( record ) );
	//SelectItem( graphId );
	EnsureVisible( graphId );
	EditLabel( graphId );
}

void C_XtTreeView::OnNewFolder( wxCommandEvent& e )
{
}

void C_XtTreeView::OnRename( wxCommandEvent& e )
{
	EditLabel(GetSelection());
}

void C_XtTreeView::OnDelete( wxCommandEvent& e )
{
	wxTreeItemId id = GetSelection();
	if ( !id.IsOk() ) return;
	
	const C_XtTreeItemData* itemData = (const C_XtTreeItemData*) GetItemData( id );
	
	if ( !itemData ) return;
	
	if ( itemData->IsPack() )
	{
		int result = ::wxMessageBox(
			wxString::Format(_T("Are you sure you want to delete pack '%s' and all its contents?"), GetItemText(id).c_str() ),
			_T("Delete Pack"),
			wxYES_NO | wxICON_EXCLAMATION,
			this
		);
		
		if ( result == wxYES )
		{
		
			if ( m_animGraphEditor.GetCurrentPack() == itemData->domPack )
			{
				m_animGraphEditor.SetCurrentGraphRecord( NULL );
				m_xtWindow.SetRecord( NULL );
			}

			m_db.DeletePack( itemData->domPack );
			m_db.Commit();
			
			Delete( id );
		}
		
	}
	else if ( itemData->IsRecord() )
	{
		int result = ::wxMessageBox(
		
			wxString::Format(_T("Are you sure you want to delete graph '%s'?"), GetItemText(id).c_str() ),
			_T("Delete Graph"),
			wxYES_NO | wxICON_EXCLAMATION,
			this
		);
		
		if ( result == wxYES )
		{
			m_db.DeleteRecord( itemData->domRecord );
			m_db.Commit();
		
			if ( m_animGraphEditor.GetCurrentGraphRecord() == itemData->domRecord )
			{
				m_animGraphEditor.SetCurrentGraphRecord( NULL );
				m_xtWindow.SetRecord( NULL );
			}
			
			Delete( id );
		}		
	}
}

void C_XtTreeView::OnOpen( wxCommandEvent& e )
{
	OpenItem( GetSelection() );
}

void C_XtTreeView::OnKeyDown( wxKeyEvent& e )
{
	if ( e.GetKeyCode() == WXK_F2 )
	{
		EditLabel( GetSelection() );
	}
	else
	{
		e.Skip();
	}
}

bool C_XtTreeView::IsRecordSelected() const
{
	wxTreeItemId id = GetSelection();
	if ( !id.IsOk() ) return false;

	const C_XtTreeItemData* itemData = GetXtTreeItemData( id );
	
	if ( itemData == NULL ) return false;

	return itemData->IsRecord();
}

const C_XtTreeItemData* C_XtTreeView::GetXtTreeItemData( wxTreeItemId id ) const
{
	const C_XtTreeItemData* itemData = NULL;
	if ( id.IsOk() )
	{
		itemData = (const C_XtTreeItemData*) GetItemData( id );
	}
	return itemData;
}
