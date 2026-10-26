#include "stdafx.h"

#include <DialogueMan/Actions/AddRows.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/ViewDisabler.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Misc/MacroExpander.h>

#include <DialogueMan/App.h>

#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>

#include <System/Misc/StringUtil.h>

namespace DMan
{
namespace AddRows
{

BEGIN_EVENT_TABLE(C_Options, wxDialog)
	EVT_CHOICE(ID_ROWTYPE, C_Options::OnRowTypeSelected)
	EVT_CHOICE(ID_LOCTABLE, C_Options::OnLocTableSelected)
	EVT_RADIOBOX(ID_POSITION, C_Options::OnPositionRadioSelected)
END_EVENT_TABLE()

int C_Options::defaultRowTypeSel  = 0;
int	C_Options::defaultLocTableSel = 0;
int C_Options::defaultPositionSel = 0;

C_Options::C_Options(wxWindow *parent)
:
wxDialog(
	parent,
	wxID_ANY,
	wxString(wxT("Add Row options"))),
m_pRowType(NULL),
m_pLocTable(NULL)
{	
    wxBoxSizer* pMainSizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* pGridSizer = new wxFlexGridSizer(2, 5, 5);

	pGridSizer->Add(new wxStaticText(this, wxID_ANY, _T("Row type:")),
                   wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	m_pRowTypeChoice = new wxChoice(this, ID_ROWTYPE, wxDefaultPosition, wxSize(350, 20));
	PopulateRowTypeChoice();
	
    pGridSizer->Add(m_pRowTypeChoice,
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
	
	pGridSizer->Add(new wxStaticText(this, wxID_ANY, _T("Loc Table:")),
                   wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL));

	m_pLocTableChoice = new wxChoice(this, ID_LOCTABLE, wxDefaultPosition, wxSize(350, 20));
	PopulateLocTableChoice();

    pGridSizer->Add(m_pLocTableChoice,
                   wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	pMainSizer->Add( pGridSizer, 0, wxALL, 8 );

	wxString availablePos[] = {"Before currently selected row", "After currently selected row"};

	m_pPositionRadio = new wxRadioBox(this, 
		ID_POSITION, "Insert", wxDefaultPosition, wxDefaultSize, 2, availablePos, 2, wxVERTICAL);
	m_pPositionRadio->SetSelection(defaultPositionSel);

	DisablePositionRadioIf();

	pMainSizer->Add(m_pPositionRadio, 0, wxEXPAND | wxDOWN | wxRIGHT | wxLEFT, 8);

	wxBoxSizer* pButtonsSizer = new wxBoxSizer(wxHORIZONTAL);

	pButtonsSizer->Add(new wxButton(this, wxID_OK, wxT("Ok")),
					   0,
					   wxRIGHT | wxALIGN_RIGHT,
					   2);

	pButtonsSizer->Add(new wxButton(this, wxID_CANCEL, wxT("Cancel")),
					   0,
					   wxALIGN_RIGHT,
					   0);

	pMainSizer->Add( pButtonsSizer, 0, wxALL | wxALIGN_RIGHT, 8 );
	
	pMainSizer->SetSizeHints(this);
    pMainSizer->Fit(this);

    SetSizer(pMainSizer);
	Layout();
}

void C_Options::PopulateRowTypeChoice()
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);
	
	C_RowSet* pRowSet = selection.GetCurrentRowSet();
	if (!pRowSet)
		return;
	
	LfArray* pLarvaArray = (LfArray*)pRowSet->GetXtType().AsField();
	LtStruct* pBaseItemType = (LtStruct*)pLarvaArray->field->Type();
		   
	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&wxGetApp().GetDb()->schema->types); it; ++it)
	{
		if (it->kind != kLtStruct)
			continue;
		
		LtStruct* pDerivedItemType = (LtStruct*)(LarvaType*)it;
		
		if (IsBaseOf(pBaseItemType, pDerivedItemType) && pBaseItemType != pDerivedItemType)
		{
			m_pRowTypeChoice->Append((const char*)pDerivedItemType->label, pDerivedItemType);
		}
	}

	U32 choiceCount = m_pRowTypeChoice->GetCount();
	if(choiceCount == 0) //If we couldn't find derived types, just add the base one
	{
		m_pRowTypeChoice->Append((const char*)pBaseItemType->label, pBaseItemType);
	}

	m_pRowType = (LarvaType*)m_pRowTypeChoice->GetClientData(0);
	m_pRowTypeChoice->SetSelection(defaultRowTypeSel < (int)choiceCount ? defaultRowTypeSel : 0);
}

void C_Options::PopulateLocTableChoice()
{
	LarvaType* pLarvaType = wxGetApp().GetDb()->schema->types["XtLocTable"];
	Btl::Map<XtString, Xt::DomRecord*> sortedlocTables;
	Btl::Map<XtString, Xt::DomRecord*>::Iterator sortedlocTablesIt;
	
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(wxGetApp().GetDb()->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator recordIt(&(pPack->records)); recordIt; ++recordIt)
		{
			Xt::DomRecord* pRec = recordIt;
			LarvaType* pCurrentType = Xt::TypeOf(wxGetApp().GetDb()->schema, pRec);
			if (pCurrentType != pLarvaType)
				continue;
						
			sortedlocTables[pRec->name] = pRec;
		}
	}

	for(sortedlocTablesIt = sortedlocTables.Begin(); sortedlocTablesIt != sortedlocTables.End(); ++sortedlocTablesIt)
	{
		m_pLocTableChoice->Append(sortedlocTablesIt->GetKey().Text(), sortedlocTablesIt->GetValue());
	}
	
	U32 choiceCount = m_pLocTableChoice->GetCount();
	m_pLocTable = (Xt::DomRecord*)m_pLocTableChoice->GetClientData(0);
	m_pLocTableChoice->SetSelection(defaultLocTableSel < (int)choiceCount ? defaultLocTableSel : 0);
}

void C_Options::DisablePositionRadioIf()
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);
	C_Selection::C_Item item = selection.GetItems().Back();
	if(item.components.Size() == 0) //no row selected
	{
		m_pPositionRadio->Disable();
	}
}

void C_Options::OnRowTypeSelected(wxCommandEvent& event)
{
	int selection = m_pRowTypeChoice->GetSelection();
	m_pRowType = (LarvaType*)m_pRowTypeChoice->GetClientData(selection);
	defaultRowTypeSel = selection;
}

void C_Options::OnLocTableSelected(wxCommandEvent& event)
{
	int selection = m_pLocTableChoice->GetSelection();
	m_pLocTable = (Xt::DomRecord*)m_pLocTableChoice->GetClientData(selection);
	defaultLocTableSel = selection;
}

void C_Options::OnPositionRadioSelected(wxCommandEvent& event)
{
	defaultPositionSel = m_pPositionRadio->GetSelection();
}

bool Do()
{
	wxBusyCursor wait; //Display an HourGlass cursor

	C_ViewDisabler viewDisabler;
	
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if ((selection.GetMode() != C_Selection::E_Mode_Row) &&
		(selection.GetMode() != C_Selection::E_Mode_Record))
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select a Record or Row and try again\n"), 
			wxT("Add Rows"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return false;
	}

	C_RowSet* pRowSet = selection.GetCurrentRowSet();
	if (!pRowSet)
		return false;

	if (selection.GetItems().Size() == 0)
	{
		assert(!"Must have valid selection!");
		return false;
	}

	C_Selection::C_Item item = selection.GetItems().Back();
	C_Record* pDialogueRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);

	Btl::Vector<U32> components = item.components;

	C_Options options(NULL);
	U32 insertAt = 0;
    if (options.ShowModal() == wxID_OK )
	{
		Btl::Map<wxString, C_MutantValue> emptyRow;
		if(components.Size() > 0) //add in the middle
		{
			if(options.GetPosition() == 0)
			{
				insertAt = components[components.Size() - 1];
			}
			else
			{
				insertAt = components[components.Size() - 1] + 1;
			}

			Add(options.GetRowType(), pArray, options.GetLocTable(), emptyRow, insertAt); 
		}
		else //add at the begining because no row was selected, only a record
		{
			insertAt = 0;
			Add(options.GetRowType(), pArray, options.GetLocTable(), emptyRow, insertAt); 
		}

		//select the added row
		selection.Clear();
		components.Clear();
		components.PushBack(insertAt);
	
		selection.Add(C_Selection::E_Mode_Row, item.id, components);
		wxGetApp().SetSelection(selection);	 

		return true;
	}
	else
	{
		LOGINFO(wxGetApp().GetLogger(), "Nothing added.\n");
		return true;
	}

	return false;
}

bool Add(LarvaType* pRowType, Xt::DomArray* pDialogueArray, Xt::DomRecord* pLocTable, Btl::Map<wxString, C_MutantValue>& row, int index)
{
	if(!pRowType || !pDialogueArray || !pLocTable)
		return false;

	if(index >= (int)Xt::Count(pDialogueArray)) //we should append
		index = -1;
			
	bool dataAdded = false;
	bool blankRow = row.Size() == 0;
				
	Xt::DomComponent* pArrayEntry = new Xt::DomComponent;
	if (!GenerateIdForComponent(pArrayEntry, false)) 
	{
		delete pArrayEntry;
		return false;
	}
	
	C_ColumnSet* pColSet = wxGetApp().GetDbManager().GetTypeColumnSet(pRowType);
	Btl::Map<wxString, C_MutantValue>::Iterator itCol;
	C_MacroExpander expander;
	C_MutantValue line;

	//set default values from schema
	//SetDefaults(wxGetApp().GetDb()->schema, (LtStruct*)pRowType, pArrayEntry);

	for(itCol = row.Begin(); itCol != row.End(); ++itCol)
	{
		const C_Column* pCol = pColSet->GetColumn(itCol->GetKey());
		if(!pCol || !pCol->GetLarvaField()) 
			continue;

		C_MutantValue value = itCol->GetValue();
		//For the LocID field
		if (pCol->GetLarvaField()->kind == kLfLocString)
		{
			line = value;		  
		}
		else
		{
			Xt::DomField* pField = Xt::AcquireField(&pArrayEntry->data, pCol->GetLarvaField()->name);
			Xt::Set( pField, wxString(value).c_str() );
			expander.Insert((const char*)pCol->GetLarvaField()->name, wxString(value).c_str());
		}
	}

	wxString name;
	//For the LocID field
	{
		//Insert entry into selected LocTable
		Xt::DomArray* pStringArray = Xt::AsArray(Xt::At(&pLocTable->root.data, "strings"));
		Xt::DomBlock* pLocTableArrayEntry = new Xt::DomBlock;

		Xt::DomField* pField = Xt::AcquireField(pLocTableArrayEntry, "english");
		Xt::Set( pField, wxString(line).c_str() );
		expander.Insert("TextID", wxString(line).c_str());

		XtId id = Xt::GenerateUniqueArrayIdForLoc(pStringArray);
		XtString strId = Format("%08X", id);
		
		//Get the currently selected record
		C_Selection selection;
		wxGetApp().GetSelection(selection);
		const C_Selection::ItemArray& arr = selection.GetItems();
		const C_Selection::C_Item& item = arr.Back();
		C_Record* pRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

		//Set record values for macro
		Xt::DomRecord* pRec = FindRecordById(wxGetApp().GetDb(), pRecord->GetRecordId());
		Xt::DomField* pNextUidField = Xt::AcquireField(&pRec->root.data, "nextUid");
		XtString nextUidStr = Xt::AsText(pNextUidField);
		if(nextUidStr == "")
			nextUidStr = "0";

		U32 nextUid = ToU32(nextUidStr.Text());
		
		expander.Insert("DIALOGUE RECORD", RemoveFoldersFromRecordName((const char*)pRecord->GetName()));
		expander.Insert("UID", nextUidStr.Text());

		Xt::DomField* pUidField = Xt::AcquireField(&pArrayEntry->data, "uid");
		Xt::Set( pUidField, nextUidStr );

		Xt::Set(pNextUidField, ToString(++nextUid));

		//Expand the locid macro from the config file
		DialogueManConfig& config = wxGetApp().GetConfig();
		Util::C_String locIdMacro = config.locId; 
		expander.Expand(locIdMacro);
		locIdMacro = RemoveUnexpandedMacros(locIdMacro);
		locIdMacro = locIdMacro.Replace(" ", "_"); 

		if(blankRow)
		   locIdMacro = "NEW_ROW";

		Xt::Set( pLocTableArrayEntry, "name", locIdMacro );
		name = locIdMacro;

		Xt::DomElement* pElAppended = Xt::AppendWithId(pStringArray, id, pLocTableArrayEntry);
		
		//Set the LocID fields in the Dialogue Record
		Xt::DomBlock* pTextIdBlock = Xt::AcquireBlock(&pArrayEntry->data, "TextId");
		
		pField = Xt::AcquireField(pTextIdBlock, "id");
		Xt::Set(pField, strId);
		
		XtString locTableStrId = Format("%08X", pLocTable->id);
		pField = Xt::AcquireField(pTextIdBlock, "record");
		Xt::Set(pField, locTableStrId);
					   	
		dataAdded = true;
	}
	
	Xt::SetType(pArrayEntry, pRowType->name);

	if(dataAdded)
	{
		if(index == -1)
		{
			Xt::Append(pDialogueArray, pArrayEntry);
		}
		else
		{
			Xt::Insert(pDialogueArray, index, pArrayEntry);
		}

		LOGINFO(wxGetApp().GetLogger(), "Added row: %s\n", name);
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Failed to add row at: %i\n", index);
	}
		
	return dataAdded;
}

} //namespace AddRows
} //namespace DMan
