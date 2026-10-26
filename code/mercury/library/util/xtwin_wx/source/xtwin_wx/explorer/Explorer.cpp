#include "stdafx.h"

#include <core/utility/Delegate.h>
#include <Btl/Containers/openhashmap.h>
#include <Btl/Utility/string.h>
#include <Btl/Utility/Hashops.h>
#include <wx/treelistctrl/TreeListCtrl.h>
#include <xtwin_wx/explorer/Explorer.h>


namespace XtWin
{

int C_Explorer::C_ExplorerData::GetType() const
{
	return m_Type;
}

const Xt::DomRecord* C_Explorer::C_ExplorerData::GetDomRecord() const
{
	return m_pDomRecord;
}

Xt::DomRecord* C_Explorer::C_ExplorerData::GetDomRecord()
{
	return m_pDomRecord;
}

const Xt::DomPack* C_Explorer::C_ExplorerData::GetDomPack() const
{
	return m_pDomPack;
}

Xt::DomPack* C_Explorer::C_ExplorerData::GetDomPack()
{
	return m_pDomPack;
}

const Btl::String& C_Explorer::C_ExplorerData::GetFilter() const
{
	return m_Filter;
}


const char* C_Explorer::m_pTreeIconStringTable[E_XtTreeViewNode_Count][2] = 
{
#define DEF(e, str1, str2) {str1, str2},
#include <xtwin_wx/explorer/IconMapping.h>
#undef DEF
};

C_Explorer::C_Explorer(
	wxWindow *parent, 
	wxWindowID id,
	const wxPoint &pos,
	const wxSize &size, 
	long style)
: wxTreeCtrl(
	parent,
	id,
	pos,
	size,
	style, 
	wxDefaultValidator),
m_shouldFilterPacks(false)
{
	{
		wxImageList *iconList = new wxImageList(16,16);

		for (U32 i=0; i<E_XtTreeViewNode_Count; ++i)
		{
			wxImage image;
			if (strlen(m_pTreeIconStringTable[i][0]))
			{
				image.LoadFile(wxString(m_pTreeIconStringTable[i][0], *wxConvCurrent));
				iconList->Add(image);
			}
			else
				iconList->Add(wxNullBitmap);
		}
		AssignImageList(iconList);
	}

	Connect(id, wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler(C_Explorer::OnContextMenu));
	Connect(id, wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT, wxTreeEventHandler(C_Explorer::OnItemRenameStart));
	Connect(id, wxEVT_COMMAND_TREE_END_LABEL_EDIT, wxTreeEventHandler(C_Explorer::OnItemRenameEnd));
	Connect(id, wxEVT_COMMAND_TREE_BEGIN_DRAG, wxTreeEventHandler(C_Explorer::OnNodeDrag));
	Connect(id, wxEVT_COMMAND_TREE_END_DRAG, wxTreeEventHandler(C_Explorer::OnNodeDrag));
}

C_Explorer::~C_Explorer()
{
	Disconnect(GetId(), wxEVT_COMMAND_TREE_ITEM_MENU);
	Disconnect(GetId(), wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT);
	Disconnect(GetId(), wxEVT_COMMAND_TREE_END_LABEL_EDIT);
	Disconnect(GetId(), wxEVT_COMMAND_TREE_BEGIN_DRAG);
	Disconnect(GetId(), wxEVT_COMMAND_TREE_END_DRAG);
}

Btl::String CreateDottedPath(Btl::Vector<Btl::String>& elements, S32 size)
{
	if (elements.Size() == 0)
		return "";

	Btl::String path = elements[0];
	for (S32 i = 1; i < size+1; ++i)
	{
		path = path + "." + elements[i];
	}

	return path;
}

void C_Explorer::Populate(
	Xt::DomDb* pDb,
	Ut::Delegate<bool(const XtString&)> *pCallback,
	bool filterEmpty)
{
	if (!pDb)
	{
		BIZ_ASSERT(!"No DB!");
		return;
	}
	m_pDb = pDb;

	Freeze();

	DeleteAllItems();

	wxTreeItemId rootId = AddRoot(
		wxString(pDb->name.Text(), *wxConvCurrent), 
		E_XtTreeViewNode_Root, 
		-1, 
		new C_ExplorerData());
	BIZ_ASSERT(rootId.IsOk());

	//Map from path to tree mapping so we can collapse items together.
	typedef Btl::OpenHashMap<Btl::String, wxTreeItemId, Btl::HashKeyOpBtlString> PathTreeMap;
	PathTreeMap packPathToTreeItemMapping;
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(pDb->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;

		//check if records are of correct type, if not skip this pack
		bool skipPack = true;
		for(Xt::DomPack::Iterator it(pPack); it; ++it)
		{
			Xt::DomRecord* pRec = it;
			
			if (pCallback && ((*pCallback)(pRec->root.type)))
			{
				skipPack = false;
				break;
			}
		}
		if (filterEmpty && skipPack)
			continue;

		if(m_shouldFilterPacks && skipPack)
			continue;

		Btl::String packNameRaw(pPack->name.Text());
		Btl::Vector<Btl::String> packPath;
		packNameRaw.Tokenise(packPath, '.');
		if (packPath.Size() == 0)
			continue;

		wxTreeItemId currentId = rootId;
		S32 pathSize = packPath.Size();
		for (S32 i = 0; i < pathSize-1; ++i)
		{
			XtString label(packPath[i].Chars());

			Btl::String dottedPath = CreateDottedPath(packPath, i);
			PathTreeMap::Iterator it = packPathToTreeItemMapping.Find(dottedPath);
			if (it == packPathToTreeItemMapping.End())
			{
				currentId = AppendItem(
					currentId, 
					wxString(label.Text(), *wxConvCurrent), 
					E_XtTreeViewNode_PackFolder, -1, 
					new C_ExplorerData(pPack, label.Text()));
				packPathToTreeItemMapping.Insert(PathTreeMap::DataType(dottedPath, currentId));
			}
			else
			{
				currentId = it->GetValue();
			}
		}
		wxTreeItemId packId;
		{
			XtString label(packPath[pathSize-1].Chars());
			packId = AppendItem(
				currentId, 
				wxString(label.Text(), *wxConvCurrent), 
				E_XtTreeViewNode_Pack, 
				-1, 
				new C_ExplorerData(pPack));
		}
		
		//Add pack members.
		PathTreeMap recordPathToTreeItemMapping;
		for(Xt::DomPack::Iterator it(pPack); it; ++it)
		{
			Xt::DomRecord* pRec = it;
			if (pCallback && !((*pCallback)(pRec->root.type)))
				continue;

			Btl::String recordNameRaw(pRec->name.Text());
			Btl::Vector<Btl::String> recordPath;
			recordNameRaw.Tokenise(recordPath, '.');
			if (recordPath.Size() == 0)
				continue;

			currentId = packId;
			S32 pathSize = recordPath.Size();
			for (S32 i = 0; i < pathSize-1; ++i)
			{
				Btl::String dottedPath = CreateDottedPath(recordPath, i);
				PathTreeMap::Iterator it = recordPathToTreeItemMapping.Find(dottedPath);
				if (it == recordPathToTreeItemMapping.End())
				{
					XtString label(recordPath[i].Chars());
					currentId = AppendItem(
						currentId, 
						wxString(label.Text(), *wxConvCurrent), 
						E_XtTreeViewNode_RecordFolder, -1, 
						new C_ExplorerData(pPack, label.Text()));
					recordPathToTreeItemMapping.Insert(PathTreeMap::DataType(dottedPath, currentId));
				}
				else
				{
					currentId = it->GetValue();
				}
			}
			{
				wxString label = wxString(recordPath[pathSize-1].Chars(), *wxConvCurrent) + 
					wxT(".") + wxString(pRec->root.type.Text(), *wxConvCurrent);
				currentId = AppendItem(
					currentId, 
					label, 
					E_XtTreeViewNode_Record, 
					-1, 
					new C_ExplorerData(pRec));
			}
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
	Thaw();
}

void C_Explorer::Clear()
{
	DeleteAllItems();
	m_pDb = 0;
}

Xt::DomPack* C_Explorer::FindPack(const XtString& name)
{
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(m_pDb->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		if (name == (const char *)pPack->name)
			return pPack;
	}

	return 0;
}

wxTreeItemId C_Explorer::MoveItem(wxTreeItemId source, wxTreeItemId destination)
{
	C_ExplorerData* srcData = (C_ExplorerData*)GetItemData(source);
	C_ExplorerData* newData;
	if (srcData) {
		if (srcData->GetType() == C_ExplorerData::E_ItemType_Pack) {
			newData = new C_ExplorerData(srcData->GetDomPack());
		} else if (srcData->GetType() == C_ExplorerData::E_ItemType_Record) {
			newData = new C_ExplorerData(srcData->GetDomRecord());
		} else if (srcData->GetType() == C_ExplorerData::E_ItemType_Filter) {
			newData = new C_ExplorerData(srcData->GetDomPack(), srcData->GetFilter());
		} else {
			newData = new C_ExplorerData();
		}
	} else {
		newData = new C_ExplorerData();
	}

	wxTreeItemId newItem = AppendItem(
		destination, 
		GetItemText(source), 
		GetItemImage(source),
		GetItemImage(source, wxTreeItemIcon_Selected), 
		newData);
	Delete(source);
	return newItem;
}

void C_Explorer::OnNodeDrag(wxTreeEvent & e)
{
	wxTreeItemId dstId = e.GetItem();

	if (!dstId.IsOk())
		return;

	C_Explorer::C_ExplorerData *pDstData = (C_Explorer::C_ExplorerData*)GetItemData(dstId);

	if (!pDstData)
		return;

	if (e.GetEventType() == wxEVT_COMMAND_TREE_BEGIN_DRAG) {
		UnselectAll();
		SelectItem(dstId, true);

		if (pDstData->GetType() == C_ExplorerData::E_ItemType_Record) {
			e.Allow();
		}
	} else {
		wxTreeItemId srcId = GetSelection();
	
		C_Explorer::C_ExplorerData *pSrcData = (C_Explorer::C_ExplorerData*)GetItemData(srcId);

		if (!pSrcData)
			return;

		if (pDstData->GetType() == C_ExplorerData::E_ItemType_Pack || pDstData->GetType() == C_ExplorerData::E_ItemType_Filter) {
			Xt::MoveRecord(pSrcData->GetDomRecord(), pDstData->GetDomPack());
			Freeze();
			MoveItem(srcId, dstId);
			//Flush
			Thaw();
		}
	}
}

void C_Explorer::OnContextMenu(wxTreeEvent& evt)
{
    wxTreeItemId itemId = evt.GetItem();
    wxPoint clientpt = evt.GetPoint();
    ShowMenu(itemId, clientpt);
    evt.Skip();
}

void C_Explorer::ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
	enum
	{
		MENU_FILTER_PACKS,
		MENU_NEW_PACK,
		MENU_DELETE_PACK,
		MENU_NEW_RECORD,
		MENU_DELETE_RECORD,
		MENU_RENAME_ITEM
	};

	SelectItem(id);

	const C_Explorer::C_ExplorerData *pItemData = 
		id.IsOk()
			? (const C_Explorer::C_ExplorerData*) GetItemData(id)
			: 0;

	wxMenu* menu;

    wxString title;
    if ( id.IsOk() )
    {
        menu = new wxMenu(GetItemText(id));
    }
    else
    {
        menu = new wxMenu();
    }

	if (pItemData) {
		switch (pItemData->GetType())
		{
		case (C_ExplorerData::E_ItemType_Root): {
				menu->AppendCheckItem(MENU_FILTER_PACKS, wxT("Filter Packs"));
				menu->Check(MENU_FILTER_PACKS, m_shouldFilterPacks);
				menu->Append(MENU_NEW_PACK, wxT("New Pack"));
				break;
			}
		case (C_ExplorerData::E_ItemType_Pack): {
				menu->Append(MENU_NEW_RECORD, wxT("New Record"));
				menu->Append(MENU_RENAME_ITEM, wxT("Rename"));	
				break;
			}
		case (C_ExplorerData::E_ItemType_Record): {
				menu->Append(MENU_RENAME_ITEM, wxT("Rename"));
				menu->Append(MENU_DELETE_RECORD, wxT("Delete"));
				break;
			}
		}
	}

	Connect(MENU_FILTER_PACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Explorer::OnFilterPacks));
	Connect(MENU_NEW_PACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Explorer::OnCreateNewPack));
	Connect(MENU_NEW_RECORD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Explorer::OnCreateNewRecord));
	Connect(MENU_DELETE_PACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Explorer::OnDeletePack));
	Connect(MENU_DELETE_RECORD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Explorer::OnDeleteRecord));
	Connect(MENU_RENAME_ITEM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_Explorer::OnRenameItem));

	PopupMenu(menu, pt);

	Disconnect(MENU_FILTER_PACKS, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_NEW_PACK, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_NEW_RECORD, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_DELETE_PACK, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_DELETE_RECORD, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_RENAME_ITEM, wxEVT_COMMAND_MENU_SELECTED);

	delete menu;
}

void C_Explorer::OnRenameItem(wxCommandEvent& evt)
{
	EditLabel(GetSelection());
}

void C_Explorer::OnItemRenameStart(wxTreeEvent& evt)
{
	wxTreeItemId selectedItem = evt.GetItem();

	const C_Explorer::C_ExplorerData *pItemData = 
		selectedItem.IsOk()
			? (const C_Explorer::C_ExplorerData*) GetItemData(selectedItem)
			: 0;

	if (!pItemData || 
		pItemData->GetType() != C_ExplorerData::E_ItemType_Pack && 
		pItemData->GetType() != C_ExplorerData::E_ItemType_Record &&
		pItemData->GetType() != C_ExplorerData::E_ItemType_Filter)
	{
		evt.Veto();
		return;
	}
 
	if (pItemData->GetType() == C_ExplorerData::E_ItemType_Record)
	{
		const char* dot = pItemData->GetDomRecord()->name;
		for(const char* ptr = dot; *ptr != '\0'; ptr++)
			if (*ptr == '.')
				dot = ptr + 1;

		SetItemText(selectedItem, wxString(dot, *wxConvCurrent));
	}
}


void C_Explorer::OnItemRenameEnd(wxTreeEvent& evt)
{
	if (evt.IsEditCancelled()) {
		return;
	}

	wxTreeItemId selectedItem = evt.GetItem();

	C_Explorer::C_ExplorerData *pItemData = 
		selectedItem.IsOk()
			? (C_Explorer::C_ExplorerData*) GetItemData(selectedItem)
			: 0;

	if (!pItemData || 
		pItemData->GetType() != C_ExplorerData::E_ItemType_Pack && 
		pItemData->GetType() != C_ExplorerData::E_ItemType_Record)
	{
		evt.Veto();
		return;
	}
 
	if (pItemData->GetType() == C_ExplorerData::E_ItemType_Pack )
	{
		XtString xtStr = (const char*)evt.GetLabel().mb_str(*wxConvCurrent);
		if (FindPack(xtStr) == 0) 
		{
			pItemData->GetDomPack()->name = xtStr;
			SetDirty(pItemData->GetDomPack(), false);
			//Flush
		} 
		else 
		{
			evt.Veto();
		}
	}
	else if (pItemData->GetType() == C_ExplorerData::E_ItemType_Record) 
	{
		evt.Veto();

		XtString newText = XtsStripBoth((const char*)evt.GetLabel().mb_str(*wxConvCurrent), ' ');
		
		if (!XtsIsName(newText)) {
			evt.Veto();
			return;
		}

		wxString newLabel = wxString((const char*)newText, *wxConvCurrent) << wxT(".") + wxString((const char*)pItemData->GetDomRecord()->root.type, *wxConvCurrent);
		SetItemText(selectedItem, newLabel);

		const char* start	= pItemData->GetDomRecord()->name;
		const char* dot		= start;

		for(const char* ptr = dot; *ptr != '\0'; ptr++)
			if (*ptr == '.')
				dot = ptr + 1;

		pItemData->GetDomRecord()->name = XtString(start, (int)(dot-start)) + newText;
		SetDirty(pItemData->GetDomRecord(), true);
		//Flush
	}
}

void C_Explorer::OnFilterPacks(wxCommandEvent& evt)
{
	m_shouldFilterPacks = evt.IsChecked();
	Reload();
}

void C_Explorer::OnCreateNewPack(wxCommandEvent& evt)
{
	Btl::String packName;

	for (int i=0;; i++) {
		packName.Printf("New Pack %i", i);
		
		if (FindPack(packName.Chars()) == 0) {
			break;
		}
	}

	Xt::DomPack* pPack = Xt::NewPack(m_pDb, packName.Chars());
	//Flush

	wxTreeItemId packId = AppendItem(
		GetRootItem(), 
		wxString(packName.Chars(), *wxConvCurrent),
		E_XtTreeViewNode_Pack, 
		-1, 
		new C_ExplorerData(pPack));
	
	EditLabel(packId);
}

void C_Explorer::OnDeletePack(wxCommandEvent& evt)
{
	wxTreeItemId selectedItem = GetSelection();

	const C_Explorer::C_ExplorerData *pItemData = 
		selectedItem.IsOk()
			? (const C_Explorer::C_ExplorerData*) GetItemData(selectedItem)
			: 0;

	if (pItemData->GetType() == C_ExplorerData::E_ItemType_Pack) {
		wxString msg;
		msg << wxT("Are you sure you wish to delete the entire pack: \n")
			<< GetItemText(selectedItem) << wxT(" and all it's contents?");
		wxMessageDialog dialog(this, msg, wxT("Are you sure?"), wxYES_NO|wxICON_QUESTION);
		if (dialog.ShowModal() == wxID_YES) 
		{
			Xt::RemovePack(m_pDb, pItemData->GetDomPack()->id);
			//Flush
			Delete(selectedItem);
		}
	}
}

void C_Explorer::OnDeleteRecord(wxCommandEvent& evt)
{
	wxTreeItemId selectedItem = GetSelection();

	const C_Explorer::C_ExplorerData *pItemData = 
		selectedItem.IsOk()
			? (const C_Explorer::C_ExplorerData*) GetItemData(selectedItem)
			: 0;

	if (pItemData->GetType() == C_ExplorerData::E_ItemType_Record) {
		wxString msg;
		msg << wxT("Are you sure you wish to delete the record: \n")
			<< GetItemText(selectedItem) << wxT("?");
		wxMessageDialog dialog(this, msg, wxT("Are you sure?"), wxYES_NO|wxICON_QUESTION);
		if (dialog.ShowModal() == wxID_YES) 
		{
			Xt::DomRecord* pRecord = const_cast<Xt::DomRecord*>(pItemData->GetDomRecord());
			if (pRecord)
			{
				Xt::DomPack* pPack = Xt::PackOf(pRecord);
				BIZ_ASSERT(pPack);

				Xt::RemoveRecord(pPack, pRecord->id);
				//Flush
				Delete(selectedItem);
			}
		}
	} else {
		int i = 0;
	}
}

}
