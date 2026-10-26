#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <xtwin_wx/explorer/Explorer.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>

#include <DialogueMan/Actions/DeleteRows.h>

#include <DialogueMan/Components/Explorer/Explorer.h>

#include <DialogueMan/Xt/XtTypeHelpers.h>
#include <DialogueMan/Xt/Types/IXtType.h>
#include <DialogueMan/Xt/XtTypeDatabase.h>

#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/RowSet.h>

#include <DialogueMan/App.h>
#include <DialogueMan/GlobalSignals.h>

namespace DMan
{

C_Explorer::C_Explorer(
	wxWindow* parent, 
	wxWindowID id,
	const wxPoint &pos,
	const wxSize &size, 
	long style)
: XtWin::C_Explorer(
	parent,
	id,
	pos, 
	size, 
	style),
	m_slots(this)
{
	Reload();
	Connect(id, wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler(C_Explorer::OnNodeSelect));
	m_slots.Connect();
}

C_Explorer::~C_Explorer()
{
	Disconnect(GetId(), wxEVT_COMMAND_TREE_SEL_CHANGED);
}

void C_Explorer::Shutdown()
{
	m_slots.Disconnect();
}

void C_Explorer::Reload()
{
	Ut::Delegate<bool(const XtString&)> callback(IsSxlRecord);
	if (wxGetApp().GetDb())
		Populate(wxGetApp().GetDb(), &callback);
}

void C_Explorer::OnNodeSelect(wxTreeEvent& e)
{
	if (!e.GetItem().IsOk())
		return;

	Btl::Vector<Xt::DomRecord*> leafs;
	ObtainLeafRecords(leafs, e.GetItem());
	wxGetApp().SetRecordsInCurrentFolder(leafs);

	C_ExplorerData* pData = (C_ExplorerData*)GetItemData(e.GetItem());
	if (!pData) return;

	Xt::DomRecord* pRecord = pData->GetDomRecord();
	Xt::DomPack* pPack;
	if (pRecord)
		pPack = Xt::PackOf(pData->GetDomRecord());
	else
		pPack = pData->GetDomPack();

	XtId recordId = pRecord ? pRecord->id : kInvalidXtId;
	XtId packId = pPack ? pPack->id : kInvalidXtId;

	C_DomId domId(packId, recordId, 0);
	if (!domId.IsValid())
		return;
	
	C_Selection::E_Mode mode;
	C_Selection selection;
	if (!pRecord)
		mode = C_Selection::E_Mode_Pack;
	else
		mode = C_Selection::E_Mode_Record;

	selection.Add(mode, domId);
	wxGetApp().SetSelection(selection);
}

void C_Explorer::OnCreateNewRecord(wxCommandEvent& evt)
{
	C_ExplorerData* pSelData = (C_ExplorerData*)GetItemData(GetSelection());
	Xt::DomPack* pPack = pSelData->GetDomPack();

	Btl::String recordName;

	for (int i=0;; i++) {
		recordName.Printf("New Record %i", i);
		
		if (FindRecord(pPack, recordName.Chars()) == 0) {
			break;
		}
	}

	LarvaType* pType = wxGetApp().GetXtTypeDatabase().GetType(E_XtType_SxlRecord);
	assert(pType);
	Xt::DomRecord* pRecord = Xt::NewRecord(pPack, recordName.Chars(), pType->name);
	
	wxTreeItemId recordId = AppendItem(
		GetSelection(), 
		wxString(recordName.Chars(), *wxConvCurrent),
		XtWin::E_XtTreeViewNode_Record, 
		-1, 
		new C_ExplorerData(pRecord));
	
	C_Record* pRec = wxGetApp().GetDbManager().AcquireRecord(pType, pRecord->id);
	assert(pRec);
	pRec->XtDeserialise();

	EditLabel(recordId);
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
				C_Record* pRec = wxGetApp().GetDbManager().GetRecord(pRecord->id);
				assert(pRec);

				for(U32 i = 0; i < pRec->GetNumRowSets(); i++)
				{
					C_RowSet* pRowSet = pRec->GetRowSet(i);
					Xt::DomArray* pArray = Xt::AsArray(pRowSet->GetXtData());
					while(int left = Xt::Count(pArray))
					{
						DeleteRows::Delete(pArray, left - 1);
					}
				}	

				bool deleted = wxGetApp().GetDbManager().RemoveRecord(pRecord->id);

				Xt::DomPack* pPack = Xt::PackOf(pRecord);
				assert(pPack);

				Xt::RemoveRecord(pPack, pRecord->id);
				//Flush
				Delete(selectedItem);
			}
		}
	}
}

Xt::DomRecord* C_Explorer::FindRecord(Xt::DomPack* pPack, const XtString& name)
{
	for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator recordIt(&(pPack->records)); recordIt; ++recordIt)
	{
		Xt::DomRecord* pRec = recordIt;
		if (name == (const char *)pRec->name)
			return pRec;
	}

	return 0;
}

void C_Explorer::OnAppDbOpened()
{
	Ut::Delegate<bool(const XtString&)> callback(IsSxlRecord);
	Populate(wxGetApp().GetDb(), &callback);
}

void C_Explorer::OnAppDbClose()
{
	Clear();
}

void C_Explorer::OnAppSelectionChanged()
{
}

void C_Explorer::ObtainLeafRecords(Btl::Vector<Xt::DomRecord*>& leafs, const wxTreeItemId& parent)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child = GetFirstChild(parent, cookie);
	while(child.IsOk())
	{
		ObtainLeafRecords(leafs, child);
		child = GetNextChild(parent, cookie);	
	}

	if(!child.IsOk()) //we are on a leaf
	{
		C_ExplorerData* pData = (C_ExplorerData*)GetItemData(parent);
		if (!pData) return;

		Xt::DomRecord* pRecord = pData->GetDomRecord();

		if(pRecord)
		{
			leafs.PushBack(pRecord);
		}
	}
}

C_Explorer::C_Slots::C_Slots(C_Explorer* pExplorer) :
m_pExplorer(pExplorer)
{
}

void C_Explorer::C_Slots::Connect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appDbOpened.connect(this, &C_Explorer::C_Slots::OnAppDbOpenedSlot);
	s.appDbClose.connect(this, &C_Explorer::C_Slots::OnAppDbCloseSlot);
	s.appSelectionChanged.connect(this, &C_Explorer::C_Slots::OnAppSelectionChangedSlot);
}

void C_Explorer::C_Slots::Disconnect()
{
	C_GlobalSignals& s = wxGetApp().GetGlobalSignals();

	s.appDbOpened.disconnect(this);
	s.appDbClose.disconnect(this);
	s.appSelectionChanged.disconnect(this);
}

void C_Explorer::C_Slots::OnAppDbOpenedSlot()
{
	m_pExplorer->OnAppDbOpened();
}

void C_Explorer::C_Slots::OnAppDbCloseSlot()
{
	m_pExplorer->OnAppDbClose();
}

void C_Explorer::C_Slots::OnAppSelectionChangedSlot()
{
	m_pExplorer->OnAppSelectionChanged();
}

} //namespace DMan
