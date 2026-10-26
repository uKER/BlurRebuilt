#include "stdafx.h"

#include "Record.h"

#include "../Graph.h"
#include "src/Record/Record.h"
#include "src/DynamicForm/Form.h"

using namespace RecordGraph;

C_RecordNode::C_RecordNode(LtStruct* pType, C_NodeGraph* pGraph) :
	C_GenericNode(E_NodeType_Record),
	m_pType(pType)
{
	m_Colour = wxColour(128, 160, 128);
	m_Size = wxSize(128, 64);
	m_bDragRedraw = true;

	m_pGraph = pGraph;
}

bool C_RecordNode::SetSelected(const bool bSelected)
{
	if(bSelected)
	{
		C_Graph* pGraph = (C_Graph*)m_pGraph;
		pGraph->GetRecord()->Select();
	}

	return I_Node::SetSelected(bSelected);
}

bool C_RecordNode::OnPopulate()
{
	for(XtUt::List<LarvaField>::Iterator it(&m_pType->fields); it; ++it)
		if (!BuildNode(this, it))
			return false;

	return C_GenericNode::OnPopulate();
}

