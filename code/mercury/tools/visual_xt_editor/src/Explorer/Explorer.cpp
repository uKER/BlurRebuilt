
// For compilers that support precompilation, includes "wx/wx.h".
#include "stdafx.h"

#include "Explorer.h"

#include "wx/image.h"

#include "../MenuEnums.h"
#include "../Record/Record.h"

#include "../icon1.xpm"
#include "../icon2.xpm"
#include "../icon3.xpm"
#include "../icon4.xpm"
#include "../icon5.xpm"

#include "system/misc/random.h"
#include "xt/xttool.h"

Util::C_Random	g_Random;

// C_Explorer

#if USE_GENERIC_TREECTRL
IMPLEMENT_DYNAMIC_CLASS(C_Explorer, wxGenericTreeCtrl)
#else
IMPLEMENT_DYNAMIC_CLASS(C_Explorer, wxTreeCtrl)
#endif

#if USE_GENERIC_TREECTRL
BEGIN_EVENT_TABLE(C_Explorer, wxGenericTreeCtrl)
#else
BEGIN_EVENT_TABLE(C_Explorer, wxTreeCtrl)
#endif
    EVT_TREE_BEGIN_DRAG(Explorer_Ctrl, C_Explorer::OnBeginDrag)
    EVT_TREE_END_DRAG(Explorer_Ctrl, C_Explorer::OnEndDrag)
    EVT_TREE_BEGIN_LABEL_EDIT(Explorer_Ctrl, C_Explorer::OnBeginLabelEdit)
    EVT_TREE_END_LABEL_EDIT(Explorer_Ctrl, C_Explorer::OnEndLabelEdit)

	EVT_TREE_SEL_CHANGED(Explorer_Ctrl, C_Explorer::OnTreeSelectionChanged)
	EVT_TREE_ITEM_ACTIVATED(Explorer_Ctrl, C_Explorer::OnTreeItemActivated)

	EVT_TREE_KEY_DOWN(Explorer_Ctrl, C_Explorer::OnTreeKeyDown)
    // so many differents ways to handle right mouse button clicks...
    EVT_CONTEXT_MENU(C_Explorer::OnContextMenu)
    // EVT_TREE_ITEM_MENU is the preferred event for creating context menus
    // on a tree pDefinition, because it includes the point of the click or item,
    // meaning that no additional placement calculations are required.

END_EVENT_TABLE()

C_Explorer::C_Explorer(wxWindow *parent, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style)
          : wxTreeCtrl(parent, id, pos, size, style)
{
    m_reverseSort = false;

    CreateImageList();

    // Add some items to the tree
    Initialise();
}

void C_Explorer::CreateImageList(int size)
{
    if ( size == -1 )
    {
        SetImageList(NULL);
        return;
    }
    if ( size == 0 )
        size = m_imageSize;
    else
        m_imageSize = size;

    // Make an image list containing small icons
    wxImageList *images = new wxImageList(size, size, true);

    // should correspond to TreeCtrlIcon_xxx enum
    wxBusyCursor wait;
    wxIcon icons[5];
    icons[0] = wxIcon(icon1_xpm);
    icons[1] = wxIcon(icon2_xpm);
    icons[2] = wxIcon(icon3_xpm);
    icons[3] = wxIcon(icon4_xpm);
    icons[4] = wxIcon(icon5_xpm);

    int sizeOrig = icons[0].GetWidth();
    for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
    {
        if ( size == sizeOrig )
        {
            images->Add(icons[i]);
        }
        else
        {
            images->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
        }
    }

    AssignImageList(images);
}

#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
void C_Explorer::CreateButtonsImageList(int size)
{
    if ( size == -1 )
    {
        SetButtonsImageList(NULL);
        return;
    }

    // Make an image list containing small icons
    wxImageList *images = new wxImageList(size, size, true);

    // should correspond to TreeCtrlIcon_xxx enum
    wxBusyCursor wait;
    wxIcon icons[4];
    icons[0] = wxIcon(icon3_xpm);   // closed
    icons[1] = wxIcon(icon3_xpm);   // closed, selected
    icons[2] = wxIcon(icon5_xpm);   // open
    icons[3] = wxIcon(icon5_xpm);   // open, selected

    for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
    {
        int sizeOrig = icons[i].GetWidth();
        if ( size == sizeOrig )
        {
            images->Add(icons[i]);
        }
        else
        {
            images->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
        }
    }

    AssignButtonsImageList(images);
#else
void C_Explorer::CreateButtonsImageList(int WXUNUSED(size))
{
#endif
}

int C_Explorer::OnCompareItems(const wxTreeItemId& item1,
                               const wxTreeItemId& item2)
{
    if ( m_reverseSort )
    {
        // just exchange 1st and 2nd items
        return wxTreeCtrl::OnCompareItems(item2, item1);
    }
    else
    {
        return wxTreeCtrl::OnCompareItems(item1, item2);
    }
}

void C_Explorer::Initialise()
{
	// add the root item
	DeleteAllItems();
	m_rootId = AddRoot(wxT("Database"), C_Explorer::TreeCtrlIcon_Folder, C_Explorer::TreeCtrlIcon_Folder, new C_ExplorerItemData(wxT("Root item"), E_ExplorerItemType_Unknown, NULL));

	Expand(m_rootId);
}

void C_Explorer::ExpandRoot()
{
	Expand(m_rootId);
}

void C_Explorer::DoToggleIcon(const wxTreeItemId& item)
{
    int image = GetItemImage(item) == TreeCtrlIcon_Folder ? TreeCtrlIcon_File
                                                          : TreeCtrlIcon_Folder;

    SetItemImage(item, image);
}

void C_Explorer::OnTreeKeyDown(wxTreeEvent& event)
{
    wxKeyEvent key = event.GetKeyEvent();

	wxTreeItemId selection = GetSelection();

	if(selection.IsOk())
	{
		C_ExplorerItemData* data = (C_ExplorerItemData *)GetItemData(selection);

		switch(key.GetKeyCode())
		{
			case(WXK_F2):
				
				break;
			case(WXK_DELETE):
				
				break;
		}
	}

    event.Skip();
}

void C_Explorer::OnBeginDrag(wxTreeEvent& event)
{
	C_ExplorerItemData *data = (C_ExplorerItemData *)GetItemData(event.GetItem());
    
	// need to explicitly allow drag
    if (event.GetItem() == GetRootItem())
    {
		
    }
	else
	{
        m_draggedItem = event.GetItem();
        event.Allow();
    }
        
}

wxTreeItemId C_Explorer::GetItemFromIndex(wxTreeItemId parent, S32 i)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId id = GetFirstChild(parent, cookie);

	for(int j = 0; j < i; ++j)
		id = GetNextSibling(id);

    return id;
}

wxTreeItemId C_Explorer::GetItemFromName(wxTreeItemId parent, const wxString& name)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId id = GetFirstChild(parent, cookie);

	while(id.IsOk())
	{
		C_ExplorerItemData *data = (C_ExplorerItemData *)GetItemData(id);
		if(data->GetName() == name)
			return id;

		id = GetNextSibling(id);
	}

    return wxTreeItemId();
}

S32 C_Explorer::GetItemIndex(wxTreeItemId id) const
{
	wxTreeItemId parent = GetItemParent(id);
	S32 i = 0;
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild(parent, cookie);

	while(child.IsOk() && child != id)
	{
		++i;
		child = GetNextSibling(child);
	}

	if(child.IsOk())
		return i;
	else
		return -1;
}

void C_Explorer::OnEndDrag(wxTreeEvent& event)
{
    wxTreeItemId itemSrc = m_draggedItem,
                 itemDst = event.GetItem();
    m_draggedItem = (wxTreeItemId)0l;

	wxPoint pt = event.GetPoint();
	wxClientDC dc(this);

	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.DrawRectangle(event.GetPoint().x, event.GetPoint().y, 10, 10);

    if ( !itemDst.IsOk() )
    {
        return;
    }

    wxString text = GetItemText(itemSrc);

	// get the source data
	C_ExplorerItemData* srcData = (C_ExplorerItemData *)GetItemData(itemSrc);

	
}

void C_Explorer::OnBeginLabelEdit(wxTreeEvent& event)
{
    // for testing, prevent this item's label editing
    wxTreeItemId itemId = event.GetItem();

	C_ExplorerItemData *data = (C_ExplorerItemData *)GetItemData(itemId);

	SetItemText(itemId, data->GetName());
}

void C_Explorer::OnEndLabelEdit(wxTreeEvent& event)
{
	bool valid = true;
	wxString error;

	wxTreeItemId itemId = event.GetItem();
	C_ExplorerItemData *data = (C_ExplorerItemData *)GetItemData(itemId);
	wxString label = event.GetLabel();
	
	// don't allow anything except letters in the labels
    if ( !valid && !event.IsEditCancelled() )
    {
		wxMessageBox(wxT("Invalid label: ") + error);
		event.Veto();
    }
	else
	{
		data->SetName(label);
		SetItemText(itemId, data->GetName());
		event.Veto();
	}
}

void C_Explorer::OnTreeSelectionChanged(wxTreeEvent& event)
{
	wxTreeItemId selection = GetSelection();

	if(selection.IsOk())
	{
		if(C_ExplorerItemData *data = (C_ExplorerItemData *)GetItemData(selection))
		{
			switch(data->GetType())
			{
				case E_ExplorerItemType_Filter:
					wxLogMessage(_T("Filter"));
					break;
				case E_ExplorerItemType_Record:
					data->GetRecord()->Select();
					break;
			}
		}
	}
}

void C_Explorer::OnTreeItemActivated(wxTreeEvent& event)
{
	wxTreeItemId selection = GetSelection();

	if(selection.IsOk())
	{
		if(C_ExplorerItemData *data = (C_ExplorerItemData *)GetItemData(selection))
		{
			switch(data->GetType())
			{
				case E_ExplorerItemType_Filter:
					wxLogMessage(_T("Filter Activated"));
					break;
				case E_ExplorerItemType_Record:
					data->GetRecord()->Open();
					break;
			}
		}
	}
}

void C_Explorer::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint pt = event.GetPosition();
    wxTreeItemId item;
    if ( !HasFlag(wxTR_MULTIPLE) )
        item = GetSelection();

    // check if event was generated by keyboard (MSW-specific?)
    if ( pt.x == -1 && pt.y == -1 ) //(this is how MSW indicates it)
    {
        // attempt to guess where to show the menu
        if ( item.IsOk() )
        {
            // if an item was clicked, show menu to the right of it
            wxRect rect;
            GetBoundingRect(item, rect, true /* only the label */);
            pt = wxPoint(rect.GetRight(), rect.GetTop());
        }
        else
        {
            pt = wxPoint(0, 0);
        }
    }
    else // event was generated by mouse, use supplied coords
    {
        pt = ScreenToClient(pt);
    }

    ShowMenu(item, pt);
}

void C_Explorer::ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
    wxString title;
    if ( !id.IsOk() )
		return;

	
#if wxUSE_MENUS
	wxTreeItemId parentId = GetItemParent(id);
	
	C_ExplorerItemData *item;

	item = (C_ExplorerItemData *)GetItemData(id);

	wxMenu menu;

		
#endif // wxUSE_MENUS
}

wxTreeItemId C_Explorer::AddItem(C_Record* pRecord)
{
#if 1
	return AddItem(wxTreeItemId(), pRecord->GetPath(), E_ExplorerItemType_Record, pRecord);
#else

	wxString type((const char*)pRecord->GetDomRecord()->type);

	// see if filter already exists
	wxTreeItemId parent = m_rootId;
	wxTreeItemId filter = GetItemFromName(parent, type);

	if(!filter.IsOk())
	{
		// it doesnt so create it
		C_ExplorerItemData *variable = new C_ExplorerItemData(type, E_ExplorerItemType_Filter, NULL);
		filter = AppendItem(parent, variable->GetName(), TreeCtrlIcon_Folder, TreeCtrlIcon_Folder, variable);
	}

	parent = filter;
	{
		wxString afterFirst = pRecord->GetPath().AfterLast('.');
		C_ExplorerItemData *variable = new C_ExplorerItemData(afterFirst, E_ExplorerItemType_Record, pRecord);
		return AppendItem(parent, variable->GetName(), TreeCtrlIcon_File, TreeCtrlIcon_File, variable);
	}
	

#endif
}

wxTreeItemId C_Explorer::AddItem(wxTreeItemId parent, const wxString& name, const E_ExplorerItemType type, C_Record* pRecord)
{
	if(!parent.IsOk())
		parent = m_rootId;

	C_ExplorerItemData *data = (C_ExplorerItemData *)GetItemData(parent);
	wxTreeItemId item;

	wxString beforeFirst = name.BeforeFirst('.');
	wxString afterFirst = name.AfterFirst('.');

	// see if we've removed all the directory names
	if(beforeFirst.Len() == name.Len())
	{
		C_ExplorerItemData *variable = new C_ExplorerItemData(name, type, pRecord);
		item = AppendItem(parent, variable->GetName(), TreeCtrlIcon_File, TreeCtrlIcon_File, variable);
	}
	else
	{
		// see if filter already exists
		wxTreeItemId filter = GetItemFromName(parent, beforeFirst);

		if(!filter.IsOk())
		{
			// it doesnt so create it
			C_ExplorerItemData *variable = new C_ExplorerItemData(beforeFirst, E_ExplorerItemType_Filter, NULL);
			filter = AppendItem(parent, variable->GetName(), TreeCtrlIcon_Folder, TreeCtrlIcon_Folder, variable);
		}

		// keep adding until we get to file
		item = AddItem(filter, afterFirst, type, pRecord);

	}
		
	return item;
}

	