#include "stdafx.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/manager.h>

#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/App.h>
#include <EditorApp/Components/DbPropertyDialog.h>

#include <wx/Dir.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/listctrl.h>

static time_t ConvertFileTime(FILETIME *input)
{
	time_t ret;
	ret = input->dwHighDateTime;
	ret = ret << 32;
	ret = ret | input->dwLowDateTime;
	return (ret - 0x19DB1DED53E8000)/10000000;
}

enum E_Sort
{
	E_Sort_Asc,
	E_Sort_Desc,
};

typedef int (wxCALLBACK *listCtrlCompare)(long item1, long item2, long sortData);

int wxCALLBACK listCompareName(long l, long r, long sort)
{
	Xt::DomDb *pXtDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();

	Xt::DomPack* pPackL = FindPackById(pXtDb, (XtId) l);
	Xt::DomPack* pPackR = FindPackById(pXtDb, (XtId) r);

	if (pPackL && !pPackR)
		return -1;
	else if (!pPackL && pPackR)
		return 1;
	else if (!pPackL && !pPackR)
		return 0;

	switch (sort)
	{
	case E_Sort_Asc:
		return strcmp(pPackL->name.Text(), pPackR->name.Text());
	case E_Sort_Desc:
		return strcmp(pPackR->name.Text(), pPackL->name.Text());
	default:
		return 0;
	}
}

int wxCALLBACK listCompareTime(long l, long r, long sort)
{
	Xt::DomDb *pXtDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();

	Xt::DomPack* pPackL = FindPackById(pXtDb, (XtId) l);
	Xt::DomPack* pPackR = FindPackById(pXtDb, (XtId) r);

	if (pPackL && !pPackR)
		return -1;
	else if (!pPackL && pPackR)
		return 1;
	else if (!pPackL && !pPackR)
		return 0;

	time_t timeL = ConvertFileTime(&pPackL->filetime.ft);
	time_t timeR = ConvertFileTime(&pPackR->filetime.ft);
	if (timeL== timeR)
		return 0;

	switch (sort)
	{
	case E_Sort_Asc:
		return timeL < timeR ? -1 : 1;
	case E_Sort_Desc:
		return timeL > timeR ? -1 : 1;
	default:
		return 0;
	}
}

namespace Editor
{
void C_DbPropertyDialog::OnShowDialog()
{
	C_DbPropertyDialog dialog(
		wxGetApp().GetFrame(), 
		wxID_ANY,
		wxString("Database Properties"));

	dialog.ShowModal();
}


C_DbPropertyDialog::C_DbPropertyDialog(wxWindow* pParent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxDialog(pParent, id, title, pos, size, style)
, m_currentPackId(0)
{
	wxBoxSizer* pSizer0;
	pSizer0 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* pSizer1;
	pSizer1 = new wxBoxSizer(wxHORIZONTAL);

	m_pListCtrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxSTATIC_BORDER);
	m_pListCtrl->InsertColumn(0, _T("Pack"));
	m_pListCtrl->InsertColumn(1, _T("Date Modified"));

	pSizer1->Add(m_pListCtrl, 1, wxALL|wxEXPAND, 1);

	m_pPropGrid = new wxPropertyGrid(
		this, 
		wxID_ANY,
		wxDefaultPosition, 
		wxDefaultSize, 
		wxNO_BORDER|wxPG_SPLITTER_AUTO_CENTER);
	const wxColour lightgray(212,208,200);
	const wxColour darkgray(113,111,100);

	m_pPropGrid->SetWindowStyle(wxNO_BORDER);
	m_pPropGrid->SetMarginColour(*wxWHITE);
	m_pPropGrid->SetCaptionBackgroundColour(*wxWHITE);
	m_pPropGrid->SetCellBackgroundColour(*wxWHITE);
	m_pPropGrid->SetCellTextColour(*wxBLACK);
	m_pPropGrid->SetCellDisabledTextColour(lightgray);
	m_pPropGrid->SetLineColour(lightgray);

	wxBoxSizer *pSizer2;
	pSizer2 = new wxBoxSizer(wxVERTICAL);

	pSizer2->Add(m_pPropGrid, 1, wxALL|wxEXPAND, 0);
	
	m_pHelpText = new wxStaticText(this, wxID_ANY, "");
	pSizer2->Add(m_pHelpText, 0, wxALL|wxEXPAND, 5);

	wxBoxSizer* pSizer3;
	pSizer3 = new wxBoxSizer(wxHORIZONTAL);

	m_pInitButton = new wxButton( this, wxID_ANY, wxT("Initialise Properties"), wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
	pSizer3->Add(m_pInitButton , 0, wxALL, 2);

	pSizer3->Add(0, 0, 1, wxEXPAND, 0);

	m_pOkButton = new wxButton( this, wxID_ANY, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	pSizer3->Add(m_pOkButton, 0, wxALL, 2);
	
	m_pCancelButton = new wxButton( this, wxID_ANY, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	pSizer3->Add(m_pCancelButton, 0, wxALL, 2);

	pSizer1->Add(pSizer2, 1, wxALL|wxEXPAND, 1);
	pSizer0->Add(pSizer1, 1, wxALL|wxEXPAND, 1);
	pSizer0->Add(pSizer3, 0, wxALIGN_RIGHT|wxALL|wxEXPAND, 1 );

	SetSizer(pSizer0);
	Layout();

	m_pListCtrl->SetColumnWidth(0, 0.55*m_pListCtrl->GetSize().x);
	m_pListCtrl->SetColumnWidth(1, 0.40*m_pListCtrl->GetSize().x-10);

	Connect(m_pListCtrl->GetId(), wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler(C_DbPropertyDialog::OnPackListColumnClick));
	Connect(m_pListCtrl->GetId(), wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(C_DbPropertyDialog::OnPackListItemSelect));
	Connect(m_pListCtrl->GetId(), wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler(C_DbPropertyDialog::OnPackListItemSelect));

	Connect(m_pInitButton->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_DbPropertyDialog::OnUpdateUI));
	Connect(m_pOkButton->GetId(), wxEVT_UPDATE_UI, wxUpdateUIEventHandler(C_DbPropertyDialog::OnUpdateUI));
	
	Connect(m_pInitButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_DbPropertyDialog::OnButtonClick));
	Connect(m_pOkButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_DbPropertyDialog::OnButtonClick));
	Connect(m_pCancelButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(C_DbPropertyDialog::OnButtonClick));

	Connect(wxID_ANY, wxEVT_PG_SELECTED, wxPropertyGridEventHandler(C_DbPropertyDialog::OnPropertySelected));
	Connect(wxID_ANY, wxEVT_PG_CHANGED, wxPropertyGridEventHandler(C_DbPropertyDialog::OnPropertyChanged));

	PopulatePackList();
	m_pListCtrl->SortItems(listCompareName, E_Sort_Asc);

	InitialisePropertyGrid();
	PopulatePackProperties();

	EnableCloseButton(false);
}

C_DbPropertyDialog::~C_DbPropertyDialog()
{
	for (PatchHashMap::Iterator it = m_xtPatches.Begin(); it != m_xtPatches.End(); ++it)
	{
		C_XtPatch& patch = it->GetValue();
		delete patch.pModified;
	}
}

void C_DbPropertyDialog::InitialisePropertyGrid()
{
	m_pPropGrid->Freeze();
	m_propertyHashMap.Clear();

	for (PatchHashMap::Iterator it = m_xtPatches.Begin(); it != m_xtPatches.End(); ++it)
	{
		C_XtPatch& patch = it->GetValue();
		delete patch.pModified;
	}
	m_xtPatches.Clear();

	LarvaSchema* pLarvaSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;
	assert(pLarvaSchema);

	LtStruct *pLtASProps = AsStruct(pLarvaSchema->types["ASPackProperties"]);
	if (pLtASProps)
	{
		Btl::Map<Btl::String, wxPropertyCategory*> categoryMap;
		
		//add default category
		wxPropertyCategory *pDefaultPGCategory = new wxPropertyCategory();
		m_pPropGrid->Append(pDefaultPGCategory);

		for(XtUt::List<LarvaField>::Iterator fieldIt(&pLtASProps->fields); fieldIt; ++fieldIt)
		{
			wxPropertyCategory *pPGCategory = 0;
			Btl::String label = fieldIt->label.Text();

			if (label.Length())
			{
				Btl::String category;

				S32 idx = label.Search('|');
				if (idx != -1)
				{
					category = label(0, idx);
					label = label(idx+1, label.Length());
				}

				if (category.Length())
				{
					Btl::Map<Btl::String, wxPropertyCategory*>::Iterator it=categoryMap.Find(category);
					if (it != categoryMap.End())
						pPGCategory = it->GetValue();

					if (!pPGCategory)
					{
						pPGCategory = new wxPropertyCategory(category.Chars());
						m_pPropGrid->Append(pPGCategory);
						categoryMap.Insert(category, pPGCategory);
					}
				}
				else
					pPGCategory = pDefaultPGCategory;
			}		

			if (pPGCategory)
				m_pPropGrid->SetCurrentCategory(pPGCategory);

			//todo: handle nested blocks if/when needed ...
			wxPGProperty *pPGProperty = 0;
			switch(fieldIt->kind)
			{
			case kLfInt:
				pPGProperty = new wxIntProperty(label.Chars(), wxPG_LABEL, 0);
				break;
			case kLfFloat:
				pPGProperty = new wxFloatProperty(label.Chars(), wxPG_LABEL, 0.f);
				break;
			case kLfString:
				pPGProperty = new wxStringProperty(label.Chars(), wxPG_LABEL, "");
				break;
			case kLfBool:
				pPGProperty = new wxBoolProperty(label.Chars(), wxPG_LABEL, false);
				pPGProperty->SetAttribute(wxPG_BOOL_USE_CHECKBOX, 1);
				break;
			default:
				LOGWARN(g_Log, "Unable to display field %s", fieldIt->name.Text());
			}

			if (pPGProperty)
			{
				m_pPropGrid->Append(pPGProperty);
				pPGProperty->SetHelpString((const char*) fieldIt->attrs.GetString("SceneView", "description"));

				m_propertyHashMap[pPGProperty] = fieldIt;
			}
		}

		if (!pDefaultPGCategory->GetChildCount())
			pDefaultPGCategory->Hide(true);

	}
	else
	{
		LOGERROR(g_Log, "Type 'ASPackProperties' was not found");
		Close();
	}

	m_pPropGrid->Thaw();
}

void C_DbPropertyDialog::PopulatePackList()
{
	m_pListCtrl->Freeze();
	Xt::DomDb *pXtDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();
	
	for(Xt::DomDb::Iterator itPack(pXtDb); itPack; ++itPack)
	{
		Xt::DomPack *pPack = itPack;
		wxListItem listItem;
		listItem.SetText(pPack->name.Text());
		long i = m_pListCtrl->InsertItem(listItem);

		time_t aprtime = 0;
		wxDateTime dt(ConvertFileTime(&pPack->filetime.ft));

		m_pListCtrl->SetItem(i, 1, dt.Format("%d/%m/%Y %H:%M"));
		m_pListCtrl->SetItemData(i, pPack->id);
	}

	m_pListCtrl->Thaw();
}

void C_DbPropertyDialog::PopulatePackProperties()
{
	if (m_currentPackId)
	{
		m_pPropGrid->Freeze();
		
		Xt::DomDb *pXtDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();
		Xt::DomPack* pPack = FindPackById(pXtDb, m_currentPackId);

		Xt::DomBlock *pASPropsBlock = 0;
		PatchHashMap::Iterator patchIt=m_xtPatches.Find(m_currentPackId);
		if (patchIt != m_xtPatches.End())
		{
			//used modified copy of property block...
			pASPropsBlock = patchIt->GetValue().pModified;
		}
		else
		{
			Xt::DomBlock *pPropsBlock = &pPack->properties;
			pASPropsBlock = Xt::AsBlock(Xt::At(&pPack->properties, "__ASProperties"));
		}

		if (!pASPropsBlock)
		{
			m_pHelpText->SetLabel("No ActorStudio properties can be found within the pack");
			m_pPropGrid->Show(false);
		}
		else
		{
			for (PropertyHashMap::Iterator propIt = m_propertyHashMap.Begin(); propIt  != m_propertyHashMap.End(); ++propIt )
			{
				wxPGProperty *pPGProperty = propIt->GetKey();
				Xt::DomElement *pElement = Xt::At(pASPropsBlock, propIt->GetValue()->name);
				if (pElement)
				{
					switch (propIt->GetValue()->kind)
					{
					case kLfInt:
						pPGProperty->SetValue(atoi(Xt::AsText(pElement)));
						break;
					case kLfFloat:
						pPGProperty->SetValue(atof(Xt::AsText(pElement)));
						break;
					case kLfString:
						pPGProperty->SetValue(Xt::AsText(pElement).Text());
						break;
					case kLfBool:
						pPGProperty->SetValue(atoi(Xt::AsText(pElement))!=0);
						break;
					default:
						break;
					}
				}
			}

			m_pHelpText->SetLabel("");
			m_pPropGrid->Show(true);
		}
		m_pPropGrid->Thaw();
	}
	else
	{
		m_pHelpText->SetLabel("No Pack Selected");
		m_pPropGrid->Show(false);
	}

	Layout();
	Refresh();
}

void C_DbPropertyDialog::InitialisePackProperties()
{
	if (m_currentPackId)
	{
		Xt::DomDb *pXtDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();
		Xt::DomPack* pPack = FindPackById(pXtDb, m_currentPackId);
		assert(pPack);

		LarvaSchema* pLarvaSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;
		assert(pLarvaSchema);

		//create initial empty block
		Xt::DomBlock *pPropsBlock = &pPack->properties;
		if (!Xt::At(&pPack->properties, "__ASProperties"))
		{
			Xt::DomBlock* pASPropsBlock = Xt::AcquireBlock(&pPack->properties, "__ASProperties");

			//LtStruct *pLtASProperties = AsStruct(pLarvaSchema->types["ASPackProperties"]);
			//if (pLtASProperties)
			//	SetDefaults(pLarvaSchema, pLtASProperties, pASPropsBlock);

			//create our locally modified block
			Xt::DomBlock *pModifiedPropsBlock = new Xt::DomBlock();
			Xt::Clone(pModifiedPropsBlock, pASPropsBlock);

			C_XtPatch patch = {pASPropsBlock, pModifiedPropsBlock, true};
			m_xtPatches[pPack->id] = patch;
		}
	}
}

void C_DbPropertyDialog::OnPackListColumnClick(wxListEvent& e)
{
    int col = e.GetColumn();
	assert(col < 2);
	
	listCtrlCompare compare[] = {listCompareName, listCompareTime};
	static E_Sort sort[] = {E_Sort_Asc, E_Sort_Desc};

	//toggle sorting asc/desc
	sort[col] = (E_Sort)((sort[col]+1)%2);
	m_pListCtrl->SortItems(compare[col], sort[col]);
}

void C_DbPropertyDialog::OnPackListItemSelect(wxListEvent& e)
{
	if (e.GetEventType() == wxEVT_COMMAND_LIST_ITEM_SELECTED)
	{
		long index = m_pListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		assert(index != -1);

		Xt::DomDb *pXtDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();
		Xt::DomPack* pPack = FindPackById(pXtDb, (XtId) m_pListCtrl->GetItemData(index));
		assert(pPack);

		m_currentPackId = pPack->id;
		PopulatePackProperties();
	}
	else //wxEVT_COMMAND_LIST_ITEM_DESELECTED
		m_pPropGrid->CommitChangesFromEditor();
}

void C_DbPropertyDialog::OnPropertySelected(wxPropertyGridEvent& e)
{
	if (m_pPropGrid->IsShown())
	{
		wxPGProperty *pProperty = e.GetProperty();
		if (!pProperty)
			m_pHelpText->SetLabel("");
		else
			m_pHelpText->SetLabel(pProperty->GetHelpString());
	}
}

void C_DbPropertyDialog::OnPropertyChanged(wxPropertyGridEvent& e)
{
	if (m_currentPackId)
	{
		Xt::DomBlock *pASPropsBlock = 0;
		PatchHashMap::Iterator patchIt=m_xtPatches.Find(m_currentPackId);
		if (patchIt != m_xtPatches.End())
		{
			//used modified copy of property block...
			pASPropsBlock = patchIt->GetValue().pModified;
		}
		else
		{
			Xt::DomDb *pXtDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();
			Xt::DomPack* pPack = FindPackById(pXtDb, m_currentPackId);
			assert(pPack);

			Xt::DomBlock *pPropsBlock = &pPack->properties;
			Xt::DomBlock* pOriginalASPropsBlock = Xt::AsBlock(Xt::At(&pPack->properties, "__ASProperties"));
			pASPropsBlock = new Xt::DomBlock();
			Xt::Clone(pASPropsBlock, pOriginalASPropsBlock);

			C_XtPatch patch = {pOriginalASPropsBlock, pASPropsBlock, false};
			m_xtPatches[pPack->id] = patch;
		}

		assert(pASPropsBlock);
		wxPGProperty* pPGProperty = e.GetProperty();
		PropertyHashMap::Iterator propIt = m_propertyHashMap.Find(pPGProperty);
		assert(propIt != m_propertyHashMap.End());
		LarvaField *pLf = propIt->GetValue();

		Xt::DomField *pXtField = Xt::AcquireField(pASPropsBlock, pLf->name);
		switch (pLf->kind)
		{
		case kLfInt:
			Xt::Set(pXtField, (const char*) Format("%d", e.GetPropertyValueAsInt()));
			break;
		case kLfFloat:
			Xt::Set(pXtField, (const char*) Format("%f", (F32)e.GetPropertyValueAsDouble()));
			break;
		case kLfString:
			Xt::Set(pXtField, e.GetPropertyValueAsString().c_str());
			break;
		case kLfBool:
			Xt::Set(pXtField, (const char*) Format("%d", e.GetPropertyValueAsBool() ? 1 : 0));
			break;
		default:
			assert(!"ASPackProperties Error");
			break;
		}
	}
}

void C_DbPropertyDialog::OnButtonClick(wxCommandEvent& e)
{
	if (e.GetId() == m_pOkButton->GetId())
	{
		m_pPropGrid->CommitChangesFromEditor();

		if (m_xtPatches.Size())
		{
			LOGINFO(g_Log, "Applied pack property changes");
			for (PatchHashMap::Iterator it = m_xtPatches.Begin(); it != m_xtPatches.End(); ++it)
			{
				C_XtPatch& patch = it->GetValue();
				Xt::Clone(patch.pOriginal, patch.pModified);
			}
			EditorLib::C_Core::Instance().GetXtManager().Save();
		}
		Close();
	}
	else if (e.GetId() == m_pCancelButton->GetId())
	{
		LOGINFO(g_Log, "Reverted pack property changes");
		for (PatchHashMap::Iterator it = m_xtPatches.Begin(); it != m_xtPatches.End(); ++it)
		{
			C_XtPatch& patch = it->GetValue();
			if (patch.deleteOnRevert)
			{
				Xt::Remove(Xt::AsBlock(patch.pOriginal->parent), patch.pOriginal->parentKey);
				patch.pOriginal = 0;
			}
			Xt::Clone(patch.pOriginal, patch.pModified);
		}
		Close();
	}
	else //if (e.GetId() == m_pInitButton->GetId())
	{
		InitialisePackProperties();
		PopulatePackProperties();
	}
}

void C_DbPropertyDialog::OnUpdateUI(wxUpdateUIEvent &e)
{
	if (e.GetId() == m_pOkButton->GetId())
		e.Enable(m_xtPatches.Size() > 0);
	else if (e.GetId() == m_pInitButton->GetId())
		e.Enable(m_currentPackId && !m_pPropGrid->IsShown());
}



}