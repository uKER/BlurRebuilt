
#include "stdafx.h"

#include "Form.h"
#include "wx/aui/aui.h"
#include "Nodes/Record.h"
#include "../Frame.h"
#include "Xt\XtTool.h"

using namespace DynamicForm;

C_Form::C_Form(C_Frame* pFrame,
				wxWindow *pParent,
				const wxWindowID id,
				const wxPoint& pos,
				const wxSize& size,
				long style) :
	wxScrolledWindow(pParent, id, pos, size, style),
	m_pFrame(pFrame),
	m_pRoot(NULL),
	m_pBlock(NULL)
{
	m_DomMonitor.SetCallback(DomCallback, this);
}

C_Form::~C_Form()
{

}

void C_Form::LoadRecord(Xt::DomRecord* pRecord)
{
	Xt::DomBlock* pBlock = &pRecord->root;

	m_pDomDb = Xt::DbOf(pRecord);

	if (LarvaType* pType = m_pDomDb->larvaSchema->types[pRecord->type])
	{
		if (pType->behaviour == kLtStruct)
		{
			LoadBlock(pBlock, (LtStruct*)pType);
		}
	}
}

void C_Form::LoadBlock(Xt::DomBlock* pBlock, LtStruct* pLtStruct) 
{
	if(pBlock != m_pBlock)
	{
		wxString type((const char*)pLtStruct->name);
	
		Freeze();

		if(!m_pRoot || type != m_PreviousType)
		{
			ClearForm();

			m_pRoot = new C_RecordNode(pLtStruct, this);

			// populate graph with record structure
			m_pRoot->Populate();
		}

		m_PreviousType = type;

		Xt::DomRecord* pRecord;

		pRecord = Xt::RecordOf(pBlock);
		Xt::Monitor(&m_DomMonitor, pRecord);

		// bind data to record
		m_pRoot->Bind(pBlock);

		// build UI elements
		m_pRoot->Build();

		UpdateLayout();
		Thaw();
	
		m_pBlock = pBlock;
	}
}

void C_Form::UpdateLayout()
{
	if(m_pRoot)
		m_pRoot->Layout();

	wxSize size = GetBestVirtualSize();
	SetVirtualSize(size);
	SetScrollRate(10, 10);

	//size.IncBy(25, 0);
	//SetMaxSize(size);
	
	//m_pAuiManager->Update();

	Layout();
}

void C_Form::ClearForm()
{
	if(m_pRoot)
	{
		Freeze();
		delete m_pRoot;
		m_pRoot = NULL;

		Xt::Monitor(&m_DomMonitor, NULL);

		if(wxSizer* pSizer = GetSizer())
		{
			SetSizer(NULL);
		}

		UpdateLayout();

		Thaw();
	}
}

void C_Form::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, C_Form* doc)
{
	if (Xt::DomRecord* record = Xt::AsRecord(doc->m_DomMonitor))
		doc->GetFrame()->NetworkAddRecord(record);
}

