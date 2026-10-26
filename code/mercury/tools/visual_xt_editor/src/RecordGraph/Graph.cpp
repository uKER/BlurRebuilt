
// For compilers that support precompilation, includes "wx/wx.h".
#include "stdafx.h"

#include "Graph.h"
#include "../Record/Record.h"
#include "Nodes/Nodes.h"
#include "system/misc/random.h"

#include "Xt\XtTool.h"

using namespace RecordGraph;


// event table

IMPLEMENT_CLASS(C_Graph, C_NodeGraph)

BEGIN_EVENT_TABLE(C_Graph, C_NodeGraph)
  EVT_PAINT(C_NodeGraph::OnPaint)
  EVT_SIZE(C_NodeGraph::OnSize)
  EVT_ERASE_BACKGROUND(C_NodeGraph::OnEraseBackground)
  EVT_MOUSE_EVENTS(C_NodeGraph::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(C_NodeGraph::OnMouseCaptureLostEvent)
END_EVENT_TABLE()

C_Graph::C_Graph( wxWindow *parent, wxWindowID id,
                    const wxPoint &pos, const wxSize &size, C_Record* pRecord)
					: C_NodeGraph( parent, id, pos, size), m_pRecord(pRecord), m_pSelection(NULL)
{
}

C_Graph::~C_Graph()
{
}

void C_Graph::Initialise()
{
	Util::C_Random random;
	wxDateTime dateTime;

	LoadRecord(m_pRecord->GetDomRecord());
}

void C_Graph::LoadRecord(Xt::DomRecord* pRecord)
{
	C_RecordNode* pNode;

	Xt::DomDb* db = Xt::DbOf(pRecord);

	if (LarvaType* pType = db->larvaSchema->types[pRecord->type])
	{
		if (pType->behaviour == kLtStruct)
		{
			pNode = new C_RecordNode((LtStruct*)pType, this);
		}
	}

	pNode->SetPosition(wxPoint(10, 10));
	m_Nodes.PushBack(pNode);

	// populate graph with record structure
	pNode->Populate();

	// bind data to record
	pNode->Bind(&pRecord->root);

	pNode->Layout(0);
	pNode->Layout2(0);
}

void C_Graph::LoadBlock(Xt::DomBlock* pBlock, LarvaType* pType)
{
	C_StructNode* pNode;

	Xt::DomDb* db = Xt::DbOf(pBlock);

	if(pType->behaviour == kLfStruct)
		pNode = new C_StructNode((LfStruct*)pType, this);

	SetPosition(wxPoint(10, 10));
	m_Nodes.PushBack(pNode);

	pNode->Populate();

	pNode->Bind(pBlock);
}

void C_Graph::LayoutNodes()
{
	C_GenericNode* pNode = (C_GenericNode*)m_Nodes[0];
	pNode->Layout(0);
	pNode->Layout2(0);
}

C_GenericNode* C_Graph::GetRootNode()
{
	C_GenericNode* pNode = (C_GenericNode*)m_Nodes[0];
	return pNode;
}


void C_Graph::Draw(wxDC& dc) const
{
	
}
