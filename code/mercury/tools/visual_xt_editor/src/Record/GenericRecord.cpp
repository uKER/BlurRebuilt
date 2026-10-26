
#include "stdafx.h"

#include "GenericRecord.h"

#include "../Frame.h"
#include "../RecordGraph/Graph.h"
#include "../DynamicForm/Form.h"
#include "../Explorer/Explorer.h"

#include "wx/aui/aui.h"

C_GenericRecord::~C_GenericRecord()
{
}

bool C_GenericRecord::Create()
{
	m_pExplorerItemId = new wxTreeItemId;
	*m_pExplorerItemId = C_Frame::Instance().GetExplorer()->AddItem(this);

	S32 index = 0;

	return true;
}

void C_GenericRecord::Destroy()
{
	C_Frame::Instance().GetExplorer()->Delete(*m_pExplorerItemId);
	delete m_pExplorerItemId;
}

bool C_GenericRecord::Open()
{
	OpenGraph();

	return true;
}

void C_GenericRecord::Close()
{
	CloseGraph();
}

void C_GenericRecord::Select()
{
	if(m_pGraph)
	{
		wxAuiNotebook* pNotebook = C_Frame::Instance().GetRecordGraphNotebook();
		S32 index = pNotebook->GetPageIndex(m_pGraph);
		if(index != wxNOT_FOUND && index != pNotebook->GetSelection())
		{
			pNotebook->SetSelection(index);
		}
	}

	{
		C_Explorer* pExplorer = C_Frame::Instance().GetExplorer();
		wxTreeItemId selection = pExplorer->GetSelection();

		if(pExplorer->GetSelection() != *m_pExplorerItemId)
		{
			if(selection.IsOk())
				pExplorer->Unselect();
			pExplorer->SelectItem(*m_pExplorerItemId);
		}
	}

	m_pForm->LoadRecord(m_pDomRecord);
}
	

void C_GenericRecord::OpenGraph()
{
	if(!m_pGraph)
	{
		if(m_pGraph = new RecordGraph::C_Graph(&C_Frame::Instance(), wxID_ANY, wxDefaultPosition, wxDefaultSize, this))
		{
			m_pGraph->Initialise();
			
			wxAuiNotebook* pNotebook = C_Frame::Instance().GetRecordGraphNotebook();
			pNotebook->AddPage( m_pGraph, m_Name );
			pNotebook->SetSelection(pNotebook->GetPageCount() - 1);
		}

	}
	else
	{
		Select();
	}
}

void C_GenericRecord::CloseGraph()
{
	if(m_pGraph)
	{
		S32 index = C_Frame::Instance().GetRecordGraphNotebook()->GetPageIndex(m_pGraph);
		if(index != wxNOT_FOUND)
		{
			C_Frame::Instance().GetRecordGraphNotebook()->RemovePage(index);
		}

		delete m_pGraph;
		m_pGraph = NULL;
	}
}