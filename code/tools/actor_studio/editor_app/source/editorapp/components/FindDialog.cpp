#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/NodeId.h>

#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorApp/Frame/Frame.h>
#include <EditorApp/Components/FindDialog.h>
#include <EditorApp/Components/FinderQueries.h>
#include <EditorApp/App.h>

#include <algorithm>

namespace Editor
{
/////////////////////////// C_FinderRow \\\\\\\\\\\\\\\\\\\\\\\\\\\

XtUt::Array<C_FinderRow*> C_FinderRow::rows;

C_FinderRow::C_FinderRow(C_FinderDialog* pFinderDlg) :
	m_pFinderDlg(pFinderDlg),
	m_enabled(true)
{
	m_pEnableBox = new wxCheckBox(m_pFinderDlg, wxID_ANY, "DUMMY");
	m_pFinderDlg->GetFinderRowsSizer()->Add(m_pEnableBox, wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	Connect(
		m_pEnableBox->GetId(), 
		wxEVT_COMMAND_CHECKBOX_CLICKED, 
		wxCommandEventHandler(C_FinderRow::OnEnableBoxClicked));
}

C_FinderRow::~C_FinderRow()
{
}

C_FinderRow* C_FinderRow::AddRow(C_FinderDialog* pFinderDlg, Type type)
{
	C_FinderRow* pRow = 0;
	switch(type)
	{
	case kFindByActorType:
		pRow = new C_FrActorsOfType(pFinderDlg);
		break;
	case kFindByActorName:
		pRow = new C_FrActorsNamed(pFinderDlg);
		break;
	case kFindByActorFieldExpression:
		pRow = new C_FrActorFieldExpression(pFinderDlg);
		break;
	};

	if (pRow)
	{
		pFinderDlg->PushEventHandler(pRow);
		XtUt::AppendEx(&C_FinderRow::rows, pRow);
		pRow->SetEnabled(false);
	}

	return pRow;
}

void C_FinderRow::BuildQuery(Xt::Query* query)
{
	//see derived classes
}

void C_FinderRow::SetEnabled(bool enabled)
{
	if (m_enabled == enabled)
		return;

	m_enabled = enabled;
	m_pEnableBox->SetValue(enabled);
	OnEnableChanged();
}

void C_FinderRow::OnEnableBoxClicked(wxCommandEvent& event)
{
	SetEnabled(!IsEnabled());
}

void C_FinderRow::OnEnableChanged()
{
}

void C_FinderRow::Reset()
{
}

/////////////////////////// C_FrActorsOfType \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_FrActorsOfType::C_FrActorsOfType(C_FinderDialog* pFinderDlg) :
	C_FinderRow(pFinderDlg)	
{
	m_pEnableBox->SetLabel("Only actors of type:");
	m_pTypeChoice =	new C_TypeChoice(m_pFinderDlg, wxID_ANY, wxDefaultPosition, wxSize(460, 20));
	m_pFinderDlg->GetFinderRowsSizer()->Add(m_pTypeChoice, wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
	
	Connect(
		m_pTypeChoice->GetId(), 
		wxEVT_COMMAND_CHOICE_SELECTED, 
		wxCommandEventHandler(C_FrActorsOfType::OnTypeSelected));
}

C_FrActorsOfType::~C_FrActorsOfType()
{
}

void C_FrActorsOfType::BuildQuery(Xt::Query* pQuery)
{
	if(!m_pTypeChoice)
		return;

	if(LtStruct* pType = AsStruct(m_pTypeChoice->GetSelectedType()))
		pQuery->AddElement(new QeByActorType(pType));
}

void C_FrActorsOfType::OnTypeSelected(wxCommandEvent& event)
{
	LtStruct* pType = (LtStruct*)m_pTypeChoice->GetSelectedType();

	for(XtUt::Array<C_FinderRow*>::Iterator it(&C_FinderRow::GetRows()); it; ++it)
	{
		if(((C_FinderRow*)it)->GetType() == kFindByActorFieldExpression)
		{
			C_FrActorFieldExpression* pFieldExp = (C_FrActorFieldExpression*)(C_FinderRow*)it;
			pFieldExp->SetEnabled(false);
			pFieldExp->SetActorType(pType);
		}
	}
}

void C_FrActorsOfType::OnEnableChanged()
{
	if(IsEnabled())
	{
		m_pTypeChoice->Enable(true);
	}
	else
	{
		for(XtUt::Array<C_FinderRow*>::Iterator it(&C_FinderRow::GetRows()); it; ++it)
		{
			if(((C_FinderRow*)it)->GetType() == kFindByActorFieldExpression)
			{
				( (C_FrActorFieldExpression*)(C_FinderRow*)it )->Reset();
				( (C_FrActorFieldExpression*)(C_FinderRow*)it )->SetEnabled(false);
			}
		}
		m_pTypeChoice->Enable(false);
		m_pTypeChoice->SetSelectedType(NULL);
	}
}

void C_FrActorsOfType::Reset()
{
	m_pTypeChoice->SetSelectedType(NULL);
}

/////////////////////////// C_FrActorsNamed \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_FrActorsNamed::C_FrActorsNamed(C_FinderDialog* pFinderDlg) :
	C_FinderRow(pFinderDlg)	
{
	m_pEnableBox->SetLabel("Which are named:");
	m_pNamedText = new wxTextCtrl(m_pFinderDlg, wxID_ANY, "", wxDefaultPosition, wxSize(460, 20));
	m_pFinderDlg->GetFinderRowsSizer()->Add(m_pNamedText, wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));
}

C_FrActorsNamed::~C_FrActorsNamed()
{
}

void C_FrActorsNamed::BuildQuery(Xt::Query* pQuery)
{
	if(!m_pNamedText)
		return;

	wxString val = m_pNamedText->GetValue(); 
	if(val != wxEmptyString)
		pQuery->AddElement(new QeByActorName(val.c_str()));
}

void C_FrActorsNamed::OnEnableChanged()
{
	m_pNamedText->Enable(IsEnabled());
	if(!IsEnabled())
		m_pNamedText->SetValue(wxEmptyString);
}

void C_FrActorsNamed::Reset()
{
	m_pNamedText->SetValue(wxEmptyString);
}

/////////////////////////// C_FrActorFieldExpression \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_FrActorFieldExpression::C_FrActorFieldExpression(C_FinderDialog* pFinderDlg) :
	C_FinderRow(pFinderDlg)
{
	m_pEnableBox->SetLabel("Matching expression:");

	wxBoxSizer* pExprSizer = new wxBoxSizer(wxHORIZONTAL);

	m_pFieldChoice = new C_FieldChoice(m_pFinderDlg, wxID_ANY, wxDefaultPosition, wxSize(180, 20));
	pExprSizer->Add(m_pFieldChoice, 0, wxRIGHT, 10);

	m_pCompareChoice = new C_CompareChoice(m_pFinderDlg, wxID_ANY, wxDefaultPosition, wxSize(90, 20));
	pExprSizer->Add(m_pCompareChoice, 0, wxRIGHT, 10);
	
	m_pLowText = new wxTextCtrl(m_pFinderDlg, wxID_ANY, "", wxDefaultPosition, wxSize(80, 20));
	pExprSizer->Add(m_pLowText, 0, wxRIGHT, 10);

	m_pHighText = new wxTextCtrl(m_pFinderDlg, wxID_ANY, "", wxDefaultPosition, wxSize(80, 20));
	pExprSizer->Add(m_pHighText, wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	m_pRecordCombo = new C_RecordCombo(m_pFinderDlg, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(170, 20), wxCB_READONLY);
	pExprSizer->Add(m_pRecordCombo, 0, wxRIGHT, 10);
	m_pRecordCombo->Hide();

	m_pFinderDlg->GetFinderRowsSizer()->Add(pExprSizer, wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL));

	m_pCompareChoice->Enable(false);
	m_pLowText->Enable(false);
	m_pHighText->Enable(false);

	Connect(
		m_pFieldChoice->GetId(), 
		wxEVT_COMMAND_CHOICE_SELECTED, 
		wxCommandEventHandler(C_FrActorFieldExpression::OnFieldSelected));

	Connect(
		m_pCompareChoice->GetId(), 
		wxEVT_COMMAND_CHOICE_SELECTED, 
		wxCommandEventHandler(C_FrActorFieldExpression::OnCompareSelected));
}

C_FrActorFieldExpression::~C_FrActorFieldExpression()
{
}

void C_FrActorFieldExpression::BuildQuery(Xt::Query* pQuery)
{
	if (!m_pFieldChoice || !m_pCompareChoice || !m_pRecordCombo || !m_pLowText || !m_pHighText)
		return;

	LtStruct* pType = m_pFieldChoice->GetType();
	Xt::FieldPath* pPath = m_pFieldChoice->GetSelectedField();
	Xt::FieldExpressionCmp cmp = m_pCompareChoice->GetSelectedCmp();

	if (!pType || !pPath || (cmp == Xt::kFecNone))
		return;
	
	XtString low	= m_pLowText->GetValue().c_str();
	XtString high	= m_pHighText->GetValue().c_str();

	if(m_pRecordCombo->IsShown())
	{
		low = wxString::Format("%X", m_pRecordCombo->GetSelectedRecordId());
	}

	pQuery->AddElement(new QeByActorFieldExpression(pType, pPath, cmp, low, high));
}

void C_FrActorFieldExpression::SetActorType(LtStruct* pType)
{
	m_pFieldChoice->SetType(pType);
	m_pFieldChoice->SetSelection(wxNOT_FOUND);
	if(IsEnabled())
	{
		m_pFieldChoice->Enable(true);	
	}
}


void C_FrActorFieldExpression::OnFieldSelected(wxCommandEvent& event)
{
	//Reset Input Fields
	m_pCompareChoice->SetSelection(wxNOT_FOUND);
	
	m_pRecordCombo->Hide();
	
	m_pLowText->Show();
	m_pLowText->Enable(false);
	m_pLowText->SetValue(wxEmptyString);
	
	m_pHighText->Show();
	m_pHighText->Enable(false);
	m_pHighText->SetValue(wxEmptyString);
	
	//Enable/Disable CompareChoice
	bool compareEnabled	= false;

	Xt::FieldPath* pPath =	m_pFieldChoice->GetSelectedField();
	
	if (pPath)
	{
		while(pPath->next)
			pPath = pPath->next;
	}
	
	if (pPath && pPath->field)
	{
		m_pCompareChoice->SetField(pPath->field);

		switch(pPath->field->kind)
		{
			case kLfInt:
			case kLfFloat:
				compareEnabled = true;
			break;

			case kLfString:
			case kLfWideString:
				compareEnabled = true;
			break;

			case kLfBool:
				compareEnabled = true;
			break;

			case kLfEnum:
				compareEnabled = true;
			break;

			case kLfHandle:
				compareEnabled = true;
			break;
		}
	}
	else
	{
		m_pCompareChoice->SetField(0);
	}

	m_pCompareChoice->Enable(compareEnabled);

}

void C_FrActorFieldExpression::OnCompareSelected(wxCommandEvent& event)
{
	bool recEnabled  = false;
	bool lowEnabled	 = false;
	bool highEnabled = false;

	Xt::FieldExpressionCmp cmp = m_pCompareChoice->GetSelectedCmp();

	switch(cmp)
	{
		case Xt::kFecEqual:
			{
				if(m_pCompareChoice->GetField()->kind == kLfHandle)
				{
					m_pRecordCombo->SetField(m_pCompareChoice->GetField());
					recEnabled = true;
					break;
				}
				else
				{
					//fall through
				}
			}
		case Xt::kFecNotEqual:
		case Xt::kFecLess:
		case Xt::kFecLessEqual:
		case Xt::kFecGreater:
		case Xt::kFecGreaterEqual:
		case Xt::kFecBetween:
		case Xt::kFecContains:
		case Xt::kFecDoesNotContain:
		case Xt::kFecStartsWith:
		case Xt::kFecEndsWith:
			lowEnabled = true;
		break;
	}

	switch(cmp)
	{
		case Xt::kFecBetween:
			highEnabled = true;
		break;
	}

	if(recEnabled)
	{
		m_pRecordCombo->SetPosition(m_pLowText->GetPosition());
		
		m_pLowText->Hide();
		m_pHighText->Hide();
		m_pRecordCombo->Show();
	}
	else
	{
		m_pRecordCombo->Hide();
		m_pLowText->Show();
		m_pHighText->Show();

		m_pLowText->Enable(lowEnabled);
		m_pHighText->Enable(highEnabled);
	}
}

void C_FrActorFieldExpression::OnRecordSelected(wxCommandEvent& event)
{
	m_pLowText->SetValue(wxString::Format("%x", m_pRecordCombo->GetSelectedRecordId()));
}

void C_FrActorFieldExpression::OnEnableChanged()
{
	if(IsEnabled())
	{
		if(m_pFieldChoice->GetType() != NULL)
		{
			m_pFieldChoice->Enable(true);	
		}
		else
		{
			m_pFieldChoice->Enable(false);
			m_pFieldChoice->SetSelection(0);
		}
	} 
	else
	{
		m_pFieldChoice->Enable(false);
		m_pFieldChoice->SetSelection(wxNOT_FOUND);
		
		m_pCompareChoice->Enable(false);
		m_pCompareChoice->SetSelection(wxNOT_FOUND);
		
		m_pRecordCombo->Hide();
		
		m_pLowText->Show();
		m_pLowText->Enable(false);
		m_pLowText->SetValue(wxEmptyString);
		
		m_pHighText->Show();
		m_pHighText->Enable(false);
		m_pHighText->SetValue(wxEmptyString);
	}
}

void C_FrActorFieldExpression::Reset()
{
	m_pFieldChoice->SetType(NULL);
	m_pCompareChoice->SetField(NULL);
	m_pLowText->SetValue(wxEmptyString);
	m_pHighText->SetValue(wxEmptyString);
}

/////////////////////////// C_TypeChoice \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_TypeChoice::C_TypeChoice(wxWindow* pParent, 
						   wxWindowID id,
						   const wxPoint& pos,
						   const wxSize& size) : 
	wxChoice(pParent, id, pos, size) 
{
	Populate();
}

C_TypeChoice::~C_TypeChoice()
{
}

void C_TypeChoice::SetSelectedType(LarvaType* pType)
{
	for(XtUt::Array<LarvaType*>::Iterator it(&m_types); it; ++it)
	{
		if (it == pType)
		{
			SetSelection(it.index);
			return;
		}
	}

	SetSelection(wxNOT_FOUND);
}

LarvaType* C_TypeChoice::GetSelectedType()
{
	if (LarvaType** type = At(&m_types, GetSelection()))
		return *type;

	return 0;
}

void C_TypeChoice::Populate()
{
	LarvaType* pSel = GetSelectedType();

	XtUt::Free(&m_types);

	Btl::Map<XtString, LtStruct*> nameToType; //this will be sorted by name
	Btl::Map<XtString, LtStruct*>::Iterator nameToTypeIt;

	Xt::DomRecord* pLevelRecord = 
		EditorLib::C_Core::Instance().GetXtManager().FindRecord(
		EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId());

	assert(pLevelRecord);

	LarvaSchema* pSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;

	LtStruct*  pRecordType = Xt::TypeOf(pSchema, pLevelRecord);

	Xt::FieldDom fd = AcquireBySemantic(
		pRecordType, 
		&pLevelRecord->root.data, 
		EditorLib::kXtSemanticAttr, 
		EditorLib::kXtSemanticParam, 
		EditorLib::kXtSemanticLabel_ActorArray);

	Xt::DomArray* pLevelActorArray = Xt::AsArray(fd.dom);

	assert(pLevelActorArray != 0);
	assert(fd.field->kind == kLfArray);

	LfArray* fieldArr = (LfArray*)fd.field;

	for(Xt::DomArray::IndexIterator it(pLevelActorArray); it; ++it)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(it);
		if (!pComponent)
			continue;

		LarvaType* pType = Xt::TypeOf(pSchema, fieldArr->field, pComponent);

		if (!pType)
			continue;

		if(nameToType.Find(pType->name) == nameToType.End())
			nameToType[pType->name] = AsStruct(pType);
	}

	Clear();

	for(nameToTypeIt = nameToType.Begin(); nameToTypeIt != nameToType.End(); ++nameToTypeIt)
	{
		XtUt::AppendEx(&m_types, (LarvaType*)nameToTypeIt->GetValue());
		Append(nameToTypeIt->GetKey().Text());
	}

	SetSelectedType(pSel);
}

/////////////////////////// C_ResultsGrid \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_ResultsGrid::C_ResultsGrid(
	wxWindow* pParent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :

	wxGrid(pParent, id, pos, size, style, name)
{
	SetRowLabelSize(1);
	SetColLabelSize(20);

	Connect(
		id, 
		wxEVT_GRID_CELL_LEFT_CLICK, 
		wxGridEventHandler(C_ResultsGrid::OnCellLeftClick));
	
	Connect(
		id, 
		wxEVT_GRID_LABEL_LEFT_CLICK, 
		wxGridEventHandler(C_ResultsGrid::OnLabelLeftClick));
}

C_ResultsGrid::~C_ResultsGrid()
{
}

void C_ResultsGrid::OnCellLeftClick(wxGridEvent& event)
{
	wxString actorIdStr = GetCellValue(event.GetRow(), 0);
	EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", actorIdStr.c_str());

	SelectRow(event.GetRow(), false);
	wxGetApp().GetFrame()->SetFocus();
}

void C_ResultsGrid::OnLabelLeftClick(wxGridEvent& event)
{
	int col = event.GetCol();
	static bool isAsc = true;
	C_ResultsGridTableBase* pTable = (C_ResultsGridTableBase*)GetTable();
	if(pTable)
	{
		pTable->Sort(col, isAsc);
		ForceRefresh();
		isAsc = !isAsc;
	}
}

/////////////////////////// C_ResultsGridTableBase \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_ResultsGridTableBase::C_ResultsGridTableBase(XtUt::List<FinderResult>& results) 
{
	for(XtUt::List<FinderResult>::Iterator it(&results); it; ++it)
	{
		Xt::DomComponent* pComponent = ComponentOf(it->dom);
		LtStruct* pComponentType = it->type;

		Xt::DomField* pNameField  = Xt::AsField(AcquireBySemantic(
			pComponentType,
			&pComponent->data,
			EditorLib::kXtSemanticAttr,
			EditorLib::kXtSemanticParam,
			EditorLib::kXtSemanticLabel_Name).dom);
		
		EditorLib::C_NodeId componentId(pComponent->id, EditorLib::kNoChildId);
		
		Result res;
		res.actorID = wxString::Format("%s", componentId.AsString().Chars());
		res.actorName = pNameField->value.Text();
		res.actorType = pComponentType->name.Text();

		m_sortedResults.PushBack(res);
	}
}

C_ResultsGridTableBase::~C_ResultsGridTableBase()
{
}

int C_ResultsGridTableBase::GetNumberRows()
{
	return m_sortedResults.Size();
}

int C_ResultsGridTableBase::GetNumberCols()
{
	return 3;
}

bool C_ResultsGridTableBase::IsEmptyCell(int row, int col)
{
	return false;
}

wxString C_ResultsGridTableBase::GetValue(int row, int col)
{
	wxString retVal;
	switch(col)
	{
	case 0:
		retVal = m_sortedResults[row].actorID;
		break;
	case 1:
		retVal = m_sortedResults[row].actorName;
		break;
	case 2:
		retVal = m_sortedResults[row].actorType;
		break;
	}
	
	return retVal;
}

void C_ResultsGridTableBase::SetValue(int row, int col, const wxString& value)
{
}

wxString C_ResultsGridTableBase::GetColLabelValue(int col)
{
	wxString retVal;
	switch(col)
	{
	case 0:
		retVal = "Actor ID";
		break;
	case 1:
		retVal = "Actor Name";
		break;
	case 2:
		retVal = "Actor Type";
		break;
	}
	return retVal;
}

void C_ResultsGridTableBase::Sort(int col, bool isAsc)
{
	C_SortPredicate pred(col, isAsc);
	std::sort(m_sortedResults.Begin(), m_sortedResults.End(), pred); 
}

C_ResultsGridTableBase::C_SortPredicate::C_SortPredicate(int col, bool isAsc) :
	m_col(col), m_isAsc(isAsc)
{}

bool C_ResultsGridTableBase::C_SortPredicate::operator() (const Result& lhs, const Result& rhs)
{
	bool retVal = false;
	switch(m_col)
	{
	case 0:
		retVal = m_isAsc ? lhs.actorID < rhs.actorID : lhs.actorID > rhs.actorID;
		break;
	case 1:
		retVal = m_isAsc ? lhs.actorName < rhs.actorName : lhs.actorName > rhs.actorName;
		break;
	case 2:
		retVal = m_isAsc ? lhs.actorType < rhs.actorType : lhs.actorType > rhs.actorType;
		break;
	}
	return retVal;
}

/////////////////////////// C_FinderDialog \\\\\\\\\\\\\\\\\\\\\\\\\\\


C_FinderDialog* C_FinderDialog::m_pFinderDlg = NULL; 

void C_FinderDialog::OnShowDialog()
{
	if(!m_pFinderDlg)
	{
		m_pFinderDlg = new C_FinderDialog(wxGetApp().GetFrame());
	}
	m_pFinderDlg->Show();
}

void C_FinderDialog::OnClose(wxCloseEvent& event)
{
	Destroy();
	m_pFinderDlg = NULL;
}

C_FinderDialog::C_FinderDialog(
	wxWindow* pParent, 
	wxWindowID id,
	const wxString& title,
	const wxPoint& pos,
	const wxSize& size,
	long style)
	: 
	wxDialog(pParent, id, title, pos, size, style)
{
	wxBoxSizer* pMainSizer = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer* pTopSizer = new wxBoxSizer(wxHORIZONTAL);
	
	m_pRowsSizer = new wxFlexGridSizer(2, 5, 5);

	C_FinderRow::AddRow(this, C_FinderRow::kFindByActorType);
	C_FinderRow::AddRow(this, C_FinderRow::kFindByActorName);	
	C_FinderRow::AddRow(this, C_FinderRow::kFindByActorFieldExpression);
	C_FinderRow::AddRow(this, C_FinderRow::kFindByActorFieldExpression);
	C_FinderRow::AddRow(this, C_FinderRow::kFindByActorFieldExpression);

	wxBoxSizer* pButtonsSizer = new wxBoxSizer(wxVERTICAL);
	m_pFindNowBtn = new wxButton(this, wxID_ANY, wxT("Find Now"));
	pButtonsSizer->Add(m_pFindNowBtn, 0, wxALL, 0);
	m_pNewSearchBtn = new wxButton(this, wxID_ANY, wxT("New Search"));
	pButtonsSizer->Add(m_pNewSearchBtn, 0, wxALL, 0);

	//-----------------

	pTopSizer->Add(m_pRowsSizer, 0, wxALL, 8);
	pTopSizer->Add(pButtonsSizer, 0, wxALL, 8);

	pMainSizer->Add(pTopSizer, 0, wxALL, 0);

	m_pResultsGrid = new C_ResultsGrid(this, wxID_ANY, wxDefaultPosition, wxSize(675, 300));
	m_pResultsGrid->EnableEditing(false);
	m_pResultsGrid->SetCellHighlightPenWidth(0);
	m_pResultsGrid->DisableDragRowSize();
	m_pResultsGrid->SetTable(new C_ResultsGridTableBase(m_results), true, wxGrid::wxGridSelectRows);
	m_pResultsGrid->SetColSize(0, 100);
	m_pResultsGrid->SetColSize(1, 287);
	m_pResultsGrid->SetColSize(2, 287);
	pMainSizer->Add(m_pResultsGrid, 1, wxALL, 8);

	pMainSizer->SetSizeHints(this);
    pMainSizer->Fit(this);

    SetSizer(pMainSizer);
	Layout();


	//-------------------

	Connect(
		m_pFindNowBtn->GetId(), 
		wxEVT_COMMAND_BUTTON_CLICKED, 
		wxCommandEventHandler(C_FinderDialog::OnFindNowClick));

	Connect(
		m_pNewSearchBtn->GetId(), 
		wxEVT_COMMAND_BUTTON_CLICKED, 
		wxCommandEventHandler(C_FinderDialog::OnNewSearchClick));

	Connect(
		id, 
		wxEVT_CLOSE_WINDOW, 
		wxCloseEventHandler(C_FinderDialog::OnClose));
}

C_FinderDialog::~C_FinderDialog()
{
	for(XtUt::Array<C_FinderRow*>::Iterator it(&C_FinderRow::GetRows()); it; ++it)
	{
		PopEventHandler(true); //this will also delete each C_FinderRow instance
	}
	C_FinderRow::RemoveRows();
}

void C_FinderDialog::AddResult(const Xt::QueryResult* pResult)
{
	if (!pResult)
		return;

	XtUt::Ptr<FinderResult> r = new FinderResult;

	if (!r)
		return;

	Monitor(&r->dom, pResult->dom);

	r->path		= pResult->path;
	r->field	= pResult->field;
	r->type		= pResult->type;

	XtUt::AppendEx(&m_results, XtUt::Pop(&r));
}

void C_FinderDialog::ExecuteQuery()
{
	LarvaSchema* pSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;

	Free(&m_results);

	Xt::Query q;

	q.AddElement(new QeAllActors());

	for(XtUt::Array<C_FinderRow*>::Iterator it(&C_FinderRow::GetRows()); it; ++it)
	{
		if (!((C_FinderRow*)it)->IsEnabled())
			continue;

		((C_FinderRow*)it)->BuildQuery(&q);
	}

	q.AddElement(new SinkActorResults(this));
	q.Execute(EditorLib::C_Core::Instance().GetXtManager().GetDb());

	m_pResultsGrid->SetTable(new C_ResultsGridTableBase(m_results), true, wxGrid::wxGridSelectRows);
	m_pResultsGrid->SetColSize(0, 100);
	m_pResultsGrid->SetColSize(1, 287);
	m_pResultsGrid->SetColSize(2, 287);
	m_pResultsGrid->ForceRefresh();
}

void C_FinderDialog::OnFindNowClick(wxCommandEvent& event)
{
	ExecuteQuery();
}

void C_FinderDialog::OnNewSearchClick(wxCommandEvent& event)
{
	for(XtUt::Array<C_FinderRow*>::Iterator it(&C_FinderRow::GetRows()); it; ++it)
	{
		((C_FinderRow*)it)->Reset();
		((C_FinderRow*)it)->SetEnabled(false);
	}

	Free(&m_results);
	m_pResultsGrid->SetTable(new C_ResultsGridTableBase(m_results), true, wxGrid::wxGridSelectRows);
	m_pResultsGrid->SetColSize(0, 100);
	m_pResultsGrid->SetColSize(1, 287);
	m_pResultsGrid->SetColSize(2, 287);
	m_pResultsGrid->ForceRefresh();
}

/////////////////////////// C_FieldChoice \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_FieldChoice::C_FieldChoice(wxWindow* pParent,
             wxWindowID id,
             const wxPoint& pos,
			 const wxSize& size): wxChoice(pParent, id, pos, size), m_pType(0) 
{
	Append("Please choose actor type first");
	SetSelection(wxNOT_FOUND);
}

C_FieldChoice::~C_FieldChoice()
{
}

void C_FieldChoice::SetType(LtStruct* pType)
{
	if (m_pType == pType)
		return;

	m_pType = pType;

	if(m_pType == NULL)
	{
		Clear();
		Append("Please choose actor type first");
		SetSelection(wxNOT_FOUND);
	}
	else
	{
		Populate();
	}
}

LtStruct* C_FieldChoice::GetType()
{
	return m_pType;
}

void C_FieldChoice::SetSelectedField(Xt::FieldPath* pPath)
{
	for(XtUt::List<Item>::Iterator it(&m_fields); it; ++it)
	{
		if (Equal(&it->path, pPath))
		{
			SetSelection(it.index);
			return;
		}
	}

	SetSelection(wxNOT_FOUND);
}

Xt::FieldPath* C_FieldChoice::GetSelectedField()
{
	if (Item* item = At(&m_fields, GetSelection()))
		return &item->path;

	return 0;
}

void C_FieldChoice::Populate()
{
	Xt::FieldPath* pSel = GetSelectedField();

	XtUt::Free(&m_fields);

	BuildList(m_pType, 0, &m_fields);

	for(XtUt::List<Item>::Iterator it(&m_fields); it; ++it)
		it->text = ToString(&it->path);

	int sel = GetSelection();

	Clear();

	for(XtUt::List<Item>::Iterator it(&m_fields); it; ++it)
	{
		Append(it->text.Text());	
	}

	SetSelectedField(pSel);
}

Xt::FieldPath* C_FieldChoice::AddToList(XtUt::List<Item>* pList, Xt::FieldPath* pPath)
{
	if (pPath->next)
	{
		Xt::FieldPath* pRoot = AddToList(pList, pPath->next);

		if (!pRoot)
			return 0;

		pRoot->next = new Xt::FieldPath(pPath->field);

		return pRoot->next;
	}

	Item* pItem = XtUt::AppendEx(pList, new Item);

	if (!pItem)
		return 0;

	pItem->path.field = pPath->field;

	return &pItem->path;
}


void C_FieldChoice::BuildList(LtStruct* pType, Xt::FieldPath* pRoot, XtUt::List<Item>* pList)
{
	if (!pType || !pList)
		return;

	if (pType->base)
		BuildList(pType->base->type, pRoot, pList);

	for(LtStruct::FieldIterator itField(&pType->fields); itField; ++itField)
	{
		Xt::FieldPath path(itField, pRoot);

		switch(itField->kind)
		{
			case kLfStruct:
				BuildList(((LfStruct*)&*itField)->type, &path, pList);
			break;

			case kLfInt:
			case kLfFloat:
			case kLfString:
			case kLfWideString:
			case kLfBool:
			case kLfEnum:
			//case kLfFlags:
			//case kLfStrictPtr:
			//case kLfPtr:
			//case kLfWeakPtr:
			case kLfHandle:
			//case kLfArray:
			//case kLfLocString:
				AddToList(pList, &path);
			break;
		}
	}
}  

/////////////////////////// C_CompareChoice \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_CompareChoice::C_CompareChoice(wxWindow* pParent,
             wxWindowID id,
             const wxPoint& pos,
			 const wxSize& size): 
	wxChoice(pParent, id, pos, size), m_pField(0) 
{
}

C_CompareChoice::~C_CompareChoice()
{

}

void C_CompareChoice::SetField(LarvaField* pField)
{
	if (m_pField == pField)
		return;

	m_pField = pField;

	Populate();
}

LarvaField* C_CompareChoice::GetField()
{
	return m_pField;
}

void C_CompareChoice::SetSelectedCmp(Xt::FieldExpressionCmp cmp)
{
	for(XtUt::Array<Xt::FieldExpressionCmp>::Iterator it(&m_cmps); it; ++it)
	{
		if (it == cmp)
		{
			SetSelection(it.index);
			return;
		}
	}

	SetSelection(wxNOT_FOUND);
}

Xt::FieldExpressionCmp C_CompareChoice::GetSelectedCmp()
{
	if (Xt::FieldExpressionCmp* cmp = At(&m_cmps, GetSelection()))
		return *cmp;

	return Xt::kFecNone;
}

void C_CompareChoice::Populate()
{
	Xt::FieldExpressionCmp sel = GetSelectedCmp();

	XtUt::Free(&m_cmps);
	
	if (m_pField)
	{
		switch(m_pField->kind)
		{
			case kLfInt:
			case kLfFloat:
				XtUt::AppendEx(&m_cmps, Xt::kFecBetween);
				XtUt::AppendEx(&m_cmps, Xt::kFecEqual);
				XtUt::AppendEx(&m_cmps, Xt::kFecNotEqual);
				XtUt::AppendEx(&m_cmps, Xt::kFecLess);
				XtUt::AppendEx(&m_cmps, Xt::kFecLessEqual);
				XtUt::AppendEx(&m_cmps, Xt::kFecGreater);
				XtUt::AppendEx(&m_cmps, Xt::kFecGreaterEqual);
			break;

			case kLfString:
			case kLfWideString:
				XtUt::AppendEx(&m_cmps, Xt::kFecContains);
				XtUt::AppendEx(&m_cmps, Xt::kFecDoesNotContain);
				XtUt::AppendEx(&m_cmps, Xt::kFecStartsWith);
				XtUt::AppendEx(&m_cmps, Xt::kFecEndsWith);
				XtUt::AppendEx(&m_cmps, Xt::kFecEqual);
				XtUt::AppendEx(&m_cmps, Xt::kFecNotEqual);
			break;

			case kLfBool:
				XtUt::AppendEx(&m_cmps, Xt::kFecIsTrue);
				XtUt::AppendEx(&m_cmps, Xt::kFecIsFalse);
			break;

			case kLfEnum:
				XtUt::AppendEx(&m_cmps, Xt::kFecEqual);
				XtUt::AppendEx(&m_cmps, Xt::kFecNotEqual);
			break;

			case kLfHandle:
				XtUt::AppendEx(&m_cmps, Xt::kFecIsNull);
				XtUt::AppendEx(&m_cmps, Xt::kFecIsNotNull);
				XtUt::AppendEx(&m_cmps, Xt::kFecEqual);
			break;
		}
	}

	Clear();

	for(XtUt::Array<Xt::FieldExpressionCmp>::Iterator it(&m_cmps); it; ++it)
	{
		const char* text = "?cmp?";

		switch(*it)
		{
			case Xt::kFecEqual:				text = "==";				break;
			case Xt::kFecNotEqual:			text = "!=";				break;
			case Xt::kFecLess:				text = "<";					break;
			case Xt::kFecLessEqual:			text = "<=";				break;
			case Xt::kFecGreater:			text = ">";					break;
			case Xt::kFecGreaterEqual:		text = ">=";				break;
			case Xt::kFecBetween:			text = "between";			break;
			case Xt::kFecIsTrue:			text = "is true";			break;
			case Xt::kFecIsFalse:			text = "is false";			break;
			case Xt::kFecContains:			text = "contains";			break;
			case Xt::kFecDoesNotContain:	text = "does not contain";	break;
			case Xt::kFecStartsWith:		text = "starts with";		break;
			case Xt::kFecEndsWith:			text = "ends with";			break;
			case Xt::kFecIsNull:			text = "is null";			break;
			case Xt::kFecIsNotNull:			text = "is not null";		break;
		}

		if(strcmp(text, "?cmp?") != 0)
		{
			Append(text);
		}
	}

	SetSelectedCmp(sel);
}

/////////////////////////// C_RecordCombo \\\\\\\\\\\\\\\\\\\\\\\\\\\

const int wxListViewComboPopup::LIST_VIEW_ID = 12321;

BEGIN_EVENT_TABLE(wxListViewComboPopup, wxListView)
	EVT_LIST_ITEM_FOCUSED(LIST_VIEW_ID, wxListViewComboPopup::OnFocused)
END_EVENT_TABLE()

C_RecordCombo::C_RecordCombo(wxWindow* pParent, 
			wxWindowID id, 
			const wxString& value, 
			const wxPoint& pos, 
			const wxSize& size,
			long style) : 
	wxComboCtrl(pParent, id, value, pos, size, style) 
{
}

C_RecordCombo::~C_RecordCombo()
{
}

void C_RecordCombo::SetField(LarvaField* pField)
{
	if (m_pField == pField)
		return;

	m_pField = pField;

	Populate();
}

LarvaField* C_RecordCombo::GetField()
{
	return m_pField;
}

void C_RecordCombo::SetSelectedRecordId(XtId recordId)
{
	wxListViewComboPopup* popUpCtrl = (wxListViewComboPopup*)GetPopupControl();

	for(int i = 0; i < popUpCtrl->GetItemCount(); i++)
	{
		Xt::DomRecord* pRec = (Xt::DomRecord*)popUpCtrl->GetItemData(i);
		if(pRec && pRec->id == recordId)
		{
			popUpCtrl->Select(i);	
		}
	}
}

XtId C_RecordCombo::GetSelectedRecordId()
{
	wxListViewComboPopup* popUpCtrl = (wxListViewComboPopup*)GetPopupControl();
	Xt::DomRecord* pRec = (Xt::DomRecord*)popUpCtrl->GetItemData(popUpCtrl->GetFirstSelected());
	if(pRec)
		return pRec->id;	
		
	return 0;
}

void C_RecordCombo::Populate()
{
	wxListViewComboPopup* popUpCtrl = new wxListViewComboPopup();
	wxComboPopup* popUpCtrlAsCombo = (wxComboPopup*)popUpCtrl; 
	SetPopupControl(popUpCtrl);
	XtId sel = GetSelectedRecordId();

	int maxItemCharLen = popUpCtrl->GetMaxItemCharLen();
	if (m_pField && m_pField->kind == kLfHandle)
	{
		for(Xt::DomDb::Iterator itPack(EditorLib::C_Core::Instance().GetXtManager().GetDb()); itPack; ++itPack)
		{
			for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
			{
				Xt::DomRecord* pRec = itRecord;
				const char* recordType = pRec->root.type.Text();
				const char* fieldType  = m_pField->Type()->name.Text();
				if(strcmp(recordType, fieldType) == 0)
				{
					wxListItem info;
					info.SetText(pRec->name.Text());
					info.SetMask(wxLIST_MASK_TEXT);
					info.SetId(popUpCtrl->GetItemCount());
					info.SetData((wxUIntPtr)pRec);

					popUpCtrl->InsertItem(info);
										
					int recCharLen = strlen(pRec->name.Text());
					if(recCharLen > maxItemCharLen)
						maxItemCharLen = recCharLen;
				}
			}
		}
	}
	SetSelectedRecordId(sel);
	popUpCtrl->SetMaxItemCharLen(maxItemCharLen);
}

} //namespace Editor 
