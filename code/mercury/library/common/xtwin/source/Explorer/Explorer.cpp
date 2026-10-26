#include "Core/XtWinPch.h"

#include <core/utility/Delegate.h>
#include <xtwin/Explorer.h>

Explorer::ExplorerData::ExplorerData(Explorer* explorer)
: _DomRecord(0), _DomPack(0), _Type(kExItemTypeRoot), _Filter(""), _Explorer(explorer)
{
	SetIcon(kExItemTypeRoot);
}

Explorer::ExplorerData::ExplorerData(Explorer* explorer, Xt::DomRecord* pRecord)
: _DomRecord(pRecord), _DomPack(0), _Type(kExItemTypeRecord), _Filter(""), _Explorer(explorer)
{
	SetIcon(kExItemTypeRecord);
	_Monitor.SetCallback(&ExplorerData::StaticMonitorEvent, this);
	Xt::Monitor(&_Monitor, pRecord);

	XtUt::Insert(&_Explorer->_DataMap, pRecord, new ExplorerData*(this));
}

Explorer::ExplorerData::ExplorerData(Explorer* explorer, Xt::DomPack* pPack)
: _DomRecord(0), _DomPack(pPack), _Type(kExItemTypePack), _Filter(""), _Explorer(explorer)
{
	SetIcon(kExItemTypePack);
	_Monitor.SetCallback(&ExplorerData::StaticMonitorEvent, this);
	Xt::Monitor(&_Monitor, pPack);

	XtUt::Insert(&_Explorer->_DataMap, pPack, new ExplorerData*(this));
}

Explorer::ExplorerData::ExplorerData(Explorer* explorer, const XtString& filterFolder, Xt::DomPack* pPack)
: _DomRecord(0), _DomPack(pPack), _Filter(filterFolder), _Explorer(explorer)
{
	if (!pPack)
	{
		_Type = kExItemTypeFilterPack;
		SetIcon(kExItemTypeFilterPack);
	}
	else
	{
		_Type = kExItemTypeFilterRecord;
		SetIcon(kExItemTypeFilterRecord);
	}
}

Explorer::ExplorerData::~ExplorerData()
{
	Xt::Monitor(&_Monitor, 0);

	if (_Type == kExItemTypePack)
		XtUt::Remove(&_Explorer->_DataMap, XtUt::FirstOf(&_Explorer->_DataMap, _DomPack), XtUt::LastOf(&_Explorer->_DataMap, _DomPack));
	else if (_Type == kExItemTypeRecord)
		XtUt::Remove(&_Explorer->_DataMap, XtUt::FirstOf(&_Explorer->_DataMap, _DomRecord), XtUt::LastOf(&_Explorer->_DataMap, _DomRecord));
}

ExItemType Explorer::ExplorerData::GetType() const
{
	return _Type;
}

Xt::DomRecord* Explorer::ExplorerData::GetDomRecord()
{
	return _DomRecord;
}

Xt::DomPack* Explorer::ExplorerData::GetDomPack()
{
	return _DomPack;
}

bool Explorer::ExplorerData::IsFilter() const
{
	if (_Type == kExItemTypeFilterRecord || _Type == kExItemTypeFilterPack)
		return true;

	return false;
}

const XtString& Explorer::ExplorerData::GetFilter() const
{
	return _Filter;
}

void Explorer::ExplorerData::OnContextMenu(const WinPoint& pt)
{
	_Explorer->OnContextMenu(this, pt);
}

bool Explorer::ExplorerData::OnItemDeleting()
{
	return false;
}

bool Explorer::ExplorerData::OnReparent(TreeItem* parent)
{
	return false;
}

bool Explorer::ExplorerData::OnStartRename(Sz16& startName)
{
	if (!_Explorer->_AllowRename ||
		_Type != kExItemTypeRecord && 
		_Type != kExItemTypePack/* &&
		_Type != kExItemTypeFilterPack && 
		_Type != kExItemTypeFilterRecord*/)
	{
		return false;
	}

	if (_Type == kExItemTypeRecord)
	{
		XtString recordName = WcToMb(GetLabel()).Text();
		S32 dotIndex = LastIndexOf(recordName, ".");
		startName = MbToWc(Substring(recordName, 0, dotIndex));
	}
 
	return true;
}

bool Explorer::ExplorerData::OnEndRename(Sz16& newName)
{
	if (_Type != kExItemTypePack && 
		_Type != kExItemTypeRecord)
	{
		return false;
	}

	if (IndexOf(newName, L".")>0)
		return false;
 
	if (_Type == kExItemTypePack)
	{
		XtString packName = WcToMb(newName);

		XtString filteredName = packName;
		_Explorer->GetFilterName(this, filteredName);

		if (_Explorer->FindPack(filteredName) != 0)
			return false;
		
		GetDomPack()->name = filteredName;
		SetDirty(GetDomPack(), true);
	}
	else if (_Type == kExItemTypeRecord)
	{
		XtString recordName = WcToMb(newName);

		XtString filteredName = XtsStripBoth(recordName, ' ');
		
		// Check to see if this name is valid
		if (!XtsIsName(filteredName))
			return false;

		_Explorer->GetFilterName(this, filteredName);

		GetDomRecord()->name = filteredName;
		SetDirty(GetDomRecord(), true);
	}

	return false;
}

bool Explorer::ExplorerData::OnActivate()
{
	if (_Type == kExItemTypeRecord)
	{
		_Explorer->OnActivateItem(this);
		return true;
	} else
		return false;
}

void Explorer::ExplorerData::UpdateName()
{
	if (_Type == kExItemTypePack)
	{
		ExplorerData* rootData = (ExplorerData*)_Explorer->GetRootNode()->GetFirstChild();

		ExplorerData* parent = _Explorer->GetFilter(rootData, _DomPack->name, 0);
		this->SetParent(parent);

		S32 packNameIndex = LastIndexOf(_DomPack->name, ".");
		if (packNameIndex)
			SetLabel(Substring(_DomPack->name, packNameIndex+1, _DomPack->name.Length()));
		else
			SetLabel(_DomPack->name);
	}
	else if (_Type == kExItemTypeRecord)
	{
		ExplorerData* packData = _Explorer->GetExplorerData(_DomRecord->parent);
		BIZ_ASSERT(packData);

		ExplorerData* parent = _Explorer->GetFilter(packData, _DomRecord->name, Xt::AsPack(_DomRecord->parent));
		this->SetParent(parent);

		S32 recordNameIndex = LastIndexOf(_DomRecord->name, ".");
		if (recordNameIndex)
			SetLabel(Substring(_DomRecord->name, recordNameIndex+1, _DomRecord->name.Length()) + "." + _DomRecord->root.type);
		else
			SetLabel(_DomRecord->name + "." + _DomRecord->root.type);
	}
}

void Explorer::ExplorerData::OnMonitorEvent(Xt::DomEvent* evt)
{
	if (evt->code == Xt::kDomRemoved)
		delete this;
	else if (_Type == kExItemTypePack && evt->code == Xt::kDomPackChanged && evt->from == _DomPack)
		UpdateName();
	else if (_Type == kExItemTypeRecord && evt->code == Xt::kDomRecordChanged && evt->from == _DomRecord)
		UpdateName();

}

void Explorer::ExplorerData::StaticMonitorEvent(Xt::DomMonitor* monitor, Xt::DomEvent* evt, ExplorerData* data)
{
	data->OnMonitorEvent(evt);
}

Explorer::Explorer()
: _ShouldFilterEmtpy(false),
_AllowRename(true)
{
	_Monitor.SetCallback(&Explorer::StaticMonitorEvent, this);

	SetDb(0);
	GetFlags().AllowMultipleSelection = false;
}

Explorer::~Explorer()
{
	SetDb(0);
}

void Explorer::SetDb(Xt::DomDb* db)
{
	_Db = db;

	SetRootNode(0);

	if (!db)
	{
		Xt::Monitor(&_Monitor, 0);
		return;
	}
	
	Xt::Monitor(&_Monitor, _Db);

	Populate();
}

Xt::DomDb* Explorer::GetDb()
{
	return _Db;
}

void Explorer::SetFilterEmpty(bool filter)
{
	_ShouldFilterEmtpy = filter;
}

void Explorer::SetAllowRename(bool allowRename)
{
	_AllowRename = allowRename;
}

void Explorer::UpdateFilter()
{
	TreeItem* root = GetRootNode();
	ExplorerData* dataRoot = (ExplorerData*)root->GetFirstChild();

	FilterItem(dataRoot);
	if (_ShouldFilterEmtpy)
		FilterEmpty(dataRoot);
}

void Explorer::GetFilterName(Explorer::ExplorerData* pData, XtString& filter)
{
	if (pData->GetType() == kExItemTypeFilterPack || pData->GetType() == kExItemTypeFilterRecord)
		_GetFilterName(pData, filter);
	else if (pData->GetType() == kExItemTypePack || pData->GetType() == kExItemTypeRecord)
		_GetFilterName((Explorer::ExplorerData*)pData->GetParent(), filter);
}

void Explorer::StaticMonitorEvent(Xt::DomMonitor* monitor, Xt::DomEvent* evt, Explorer* data)
{
	data->OnMonitorEvent(evt);
}

void Explorer::OnMonitorEvent(Xt::DomEvent* evt)
{
	if (evt->code == Xt::kDomPackParentChanged || evt->code == Xt::kDomRecordParentChanged)
		ParentChanged(evt->from);
	else if (evt->code == Xt::kDomPackChanged || evt->code == Xt::kDomDbChanged)
		UpdateFilter();
	else if (evt->code == Xt::kDomDbClosing && evt->from == _Db)
		SetDb(0);
}

void Explorer::ParentChanged(Xt::DomElement* element)
{
	ExplorerData* explorerData = GetExplorerData(element);
	if (!explorerData)
	{
		if (element->kind == Xt::kDomPack)
			AddPack((Explorer::ExplorerData*)GetRootNode()->GetFirstChild(), Xt::AsPack(element));
		else if (element->kind == Xt::kDomRecord)
		{
			ExplorerData* packData = GetExplorerData(element->parent);
			BIZ_ASSERT (packData);

			AddRecord(packData, Xt::AsPack(element->parent), Xt::AsRecord(element));
		}
	} else {
		ExplorerData* newParent = GetExplorerData(element->parent);
		BIZ_ASSERT(newParent);

		explorerData->SetParent(newParent);
	}
}

void Explorer::Populate()
{
	Freeze();

	TreeItem* root = new TreeItem();
	SetRootNode(root);

	ExplorerData* packRoot = new ExplorerData(this);
	packRoot->SetLabel(_Db->name.Text());
	root->AddChild(packRoot);

	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(_Db->packs)); packIt; ++packIt)
	{
		AddPack(packRoot, packIt);
	}

	//Sort nodes
	packRoot->SortChildren(true);

	packRoot->SetExpanded(true);

	UpdateFilter();

	Thaw();
}

void Explorer::AddPack(ExplorerData* pRootData, Xt::DomPack* pPack)
{
	ExplorerData* pack = new ExplorerData(this, pPack);
	pack->UpdateName();

	XtUt::Dictionary<XtString, TreeItem*> filters;
	
	for(Xt::DomPack::Iterator it(pPack); it; ++it)
	{
		AddRecord(pack, pPack, it);
	}

	pack->SortChildren(true);
}

void Explorer::AddRecord(ExplorerData* pPackData, Xt::DomPack* pPack, Xt::DomRecord* pRecord)
{
	ExplorerData* record = new ExplorerData(this, pRecord);
	record->UpdateName();
}

Explorer::ExplorerData* Explorer::GetExplorerData(Xt::DomElement* element)
{
	if (XtUt::Exists(&_DataMap, element))
		return *_DataMap[element];
	else
		return 0;
}

Explorer::ExplorerData* Explorer::GetFilter(Explorer::ExplorerData* parent, const XtString& path, Xt::DomPack* pPack)
{
	S32 seperator = IndexOf(path, ".");

	if (seperator == -1)
		return parent;

	XtString split(path, seperator);
	
	ExplorerData* child = (ExplorerData*)parent->GetFirstChild();
	while (child)
	{
		if (child->GetLabel() == MbToWc(split))
			break;
		child = (ExplorerData*)child->GetNextSibling();
	}

	if (!child)
	{
		child = new ExplorerData(this, split, pPack);
		child->SetLabel(split);
		parent->AddChild(child);
	}

	return GetFilter(child, Substring(path, seperator+1, path.Length()), pPack);
}

void Explorer::Clear()
{
	SetRootNode(0);
	_Db = 0;
}

void Explorer::FilterItem(ExplorerData* item)
{
	bool filterItem = OnFilterItem(item);
	item->SetHidden(!filterItem);

	if (filterItem)
	{
		ExplorerData* child = (ExplorerData*)item->GetFirstChild();
		while (child)
		{
			FilterItem(child);
			child = (ExplorerData*)child->GetNextSibling();
		}
	}
}

void Explorer::FilterEmpty(ExplorerData* item)
{
	if (item->GetType() == kExItemTypeFilterPack || item->GetType() == kExItemTypeFilterRecord || item->GetType() == kExItemTypePack)
	{
		if (IsEmpty(item))
			item->SetHidden(true);
	}

	if (!item->IsHidden())
	{
		ExplorerData* child = (ExplorerData*)item->GetFirstChild();
		while (child)
		{
			FilterEmpty(child);
			child = (ExplorerData*)child->GetNextSibling();
		}
	}
}

bool Explorer::IsEmpty(ExplorerData* item)
{
	if (item->GetType() == kExItemTypeRecord && !item->IsHidden())
		return false;

	ExplorerData* child = (ExplorerData*)item->GetFirstChild();
	while (child)
	{
		if (!IsEmpty(child))
			return false;
		child = (ExplorerData*)child->GetNextSibling();
	}
	
	return true;
}

bool Explorer::OnCompareItems(TreeItem* a, TreeItem* b)
{
	ExplorerData* ea = (ExplorerData*)a;
	ExplorerData* eb = (ExplorerData*)b;

	if (ea->GetType() == eb->GetType())
	{
		return TreeCtrl::OnCompareItems(a, b);
	}
	else if (ea->IsFilter() || eb->IsFilter())
	{
		if (ea->IsFilter())
			return true;
		else
			return false;
	} else {
		return TreeCtrl::OnCompareItems(a, b);
	}
}

Xt::DomPack* Explorer::FindPack(const XtString& name)
{
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(_Db->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		if (name == pPack->name)
			return pPack;
	}

	return 0;
}

/*
wxTreeItemId Explorer::MoveItem(wxTreeItemId source, wxTreeItemId destination)
{
	ExplorerData* srcData = (ExplorerData*)GetItemData(source);
	ExplorerData* newData;
	if (srcData) {
		if (srcData->GetType() == ExplorerData::E_ItemType_Pack) {
			newData = new ExplorerData(srcData->GetDomPack());
		} else if (srcData->GetType() == ExplorerData::E_ItemType_Record) {
			newData = new ExplorerData(srcData->GetDomRecord());
		} else if (srcData->GetType() == ExplorerData::E_ItemType_Filter) {
			newData = new ExplorerData(srcData->GetDomPack(), srcData->GetFilter());
		} else {
			newData = new ExplorerData();
		}
	} else {
		newData = new ExplorerData();
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

void Explorer::OnNodeDrag(wxTreeEvent & e)
{
	wxTreeItemId dstId = e.GetItem();

	if (!dstId.IsOk())
		return;

	Explorer::ExplorerData *pDstData = (Explorer::ExplorerData*)GetItemData(dstId);

	if (!pDstData)
		return;

	if (e.GetEventType() == wxEVT_COMMAND_TREE_BEGIN_DRAG) {
		UnselectAll();
		SelectItem(dstId, true);

		if (pDstData->GetType() == ExplorerData::E_ItemType_Record) {
			e.Allow();
		}
	} else {
		wxTreeItemId srcId = GetSelection();
	
		Explorer::ExplorerData *pSrcData = (Explorer::ExplorerData*)GetItemData(srcId);

		if (!pSrcData)
			return;

		if (pDstData->GetType() == ExplorerData::E_ItemType_Pack || pDstData->GetType() == ExplorerData::E_ItemType_Filter) {
			Xt::MoveRecord(pSrcData->GetDomRecord(), pDstData->GetDomPack());
			Freeze();
			MoveItem(srcId, dstId);
			//Flush
			Thaw();
		}
	}
}

void Explorer::OnContextMenu(wxTreeEvent& evt)
{
    wxTreeItemId itemId = evt.GetItem();
    wxPoint clientpt = evt.GetPoint();
    ShowMenu(itemId, clientpt);
    evt.Skip();
}

void Explorer::ShowMenu(wxTreeItemId id, const wxPoint& pt)
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

	const Explorer::ExplorerData *pItemData = 
		id.IsOk()
			? (const Explorer::ExplorerData*) GetItemData(id)
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
		case (ExplorerData::E_ItemType_Root): {
				menu->AppendCheckItem(MENU_FILTER_PACKS, wxT("Filter Packs"));
				menu->Check(MENU_FILTER_PACKS, _ShouldFilterPacks);
				menu->Append(MENU_NEW_PACK, wxT("New Pack"));
				break;
			}
		case (ExplorerData::E_ItemType_Pack): {
				menu->Append(MENU_NEW_RECORD, wxT("New Record"));
				menu->Append(MENU_RENAME_ITEM, wxT("Rename"));	
				break;
			}
		case (ExplorerData::E_ItemType_Record): {
				menu->Append(MENU_RENAME_ITEM, wxT("Rename"));
				menu->Append(MENU_DELETE_RECORD, wxT("Delete"));
				break;
			}
		}
	}

	Connect(MENU_FILTER_PACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Explorer::OnFilterPacks));
	Connect(MENU_NEW_PACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Explorer::OnCreateNewPack));
	Connect(MENU_NEW_RECORD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Explorer::OnCreateNewRecord));
	Connect(MENU_DELETE_PACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Explorer::OnDeletePack));
	Connect(MENU_DELETE_RECORD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Explorer::OnDeleteRecord));
	Connect(MENU_RENAME_ITEM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(Explorer::OnRenameItem));

	PopupMenu(menu, pt);

	Disconnect(MENU_FILTER_PACKS, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_NEW_PACK, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_NEW_RECORD, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_DELETE_PACK, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_DELETE_RECORD, wxEVT_COMMAND_MENU_SELECTED);
	Disconnect(MENU_RENAME_ITEM, wxEVT_COMMAND_MENU_SELECTED);

	delete menu;
}

void Explorer::OnRenameItem(wxCommandEvent& evt)
{
	EditLabel(GetSelection());
}

void Explorer::OnItemRenameStart(wxTreeEvent& evt)
{
	wxTreeItemId selectedItem = evt.GetItem();

	const Explorer::ExplorerData *pItemData = 
		selectedItem.IsOk()
			? (const Explorer::ExplorerData*) GetItemData(selectedItem)
			: 0;

	if (!pItemData ||
		pItemData->GetType() != ExplorerData::E_ItemType_Pack && 
		pItemData->GetType() != ExplorerData::E_ItemType_Record &&
		pItemData->GetType() != ExplorerData::E_ItemType_Filter)
	{
		evt.Veto();
		return;
	}
 
	if (pItemData->GetType() == ExplorerData::E_ItemType_Record)
	{
		const char* dot = pItemData->GetDomRecord()->name;
		for(const char* ptr = dot; *ptr != '\0'; ptr++)
			if (*ptr == '.')
				dot = ptr + 1;

		SetItemText(selectedItem, wxString(dot, *wxConvCurrent));
	}
}
*/

/*
void Explorer::OnItemRenameEnd(wxTreeEvent& evt)
{
	if (evt.IsEditCancelled()) {
		return;
	}

	wxTreeItemId selectedItem = evt.GetItem();

	Explorer::ExplorerData *pItemData = 
		selectedItem.IsOk()
			? (Explorer::ExplorerData*) GetItemData(selectedItem)
			: 0;

	if (!pItemData || 
		pItemData->GetType() != ExplorerData::E_ItemType_Pack && 
		pItemData->GetType() != ExplorerData::E_ItemType_Record)
	{
		evt.Veto();
		return;
	}
 
	if (pItemData->GetType() == ExplorerData::E_ItemType_Pack )
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
	else if (pItemData->GetType() == ExplorerData::E_ItemType_Record) 
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

void Explorer::OnFilterPacks(wxCommandEvent& evt)
{
	_ShouldFilterPacks = evt.IsChecked();
	Reload();
}
*/

void Explorer::_GetFilterName(Explorer::ExplorerData* data, XtString& filter)
{
	if (data->GetType() != kExItemTypeFilterPack && data->GetType() != kExItemTypeFilterRecord)
		return;
	else
	{
		filter = data->GetFilter() + "." + filter;
		_GetFilterName((Explorer::ExplorerData*)data->GetParent(), filter);
	}
}

void Explorer::Delete(ExplorerData* data)
{
	if (data->GetType() == kExItemTypePack)
		DeletePack(data);
	else if (data->GetType() == kExItemTypeFilterPack)
		DeletePackFolder(data);
	else if (data->GetType() == kExItemTypeRecord)
		DeleteRecord(data);
	else if (data->GetType() == kExItemTypeFilterRecord)
		DeleteRecordFolder(data);
}

void Explorer::DeletePack(ExplorerData* data)
{
	if (data->GetType() != kExItemTypePack)
		return;

	XtString msg = "Are you sure you wish to delete the entire pack: \n";
	msg += WcToMb(data->GetLabel()) + " and all it's contents?";
	
	if (MessageBox(_hwnd, msg, "Are you sure?", MB_YESNO|MB_ICONQUESTION) == IDYES) 
	{
		Xt::RemovePack(GetDb(), data->GetDomPack()->id);
	}
}

void Explorer::DeletePackFolder(Explorer::ExplorerData* data)
{
}

void Explorer::DeleteRecord(ExplorerData* data)
{
	if (data->GetType() != kExItemTypeRecord)
		return;

	XtString msg = "Are you sure you wish to delete the record: \n";
	msg += WcToMb(data->GetLabel()) + "?";

	if (MessageBox(_hwnd, msg, "Are you sure?", MB_YESNO|MB_ICONQUESTION) == IDYES) 
	{
		Xt::DomRecord* record = const_cast<Xt::DomRecord*>(data->GetDomRecord());
		if (record)
		{
			Xt::DomPack* pack = Xt::PackOf(record);
			assert(pack);

			Xt::RemoveRecord(pack, record->id);
		}
	}
}

void Explorer::DeleteRecordFolder(Explorer::ExplorerData* data)
{
}

bool Explorer::OnFilterItem(ExplorerData* data)
{
	return true;
}