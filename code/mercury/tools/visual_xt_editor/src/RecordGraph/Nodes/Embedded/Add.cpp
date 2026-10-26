#include "stdafx.h"

#include "Add.h"
#include "../Array.h"
#include "../Type.h"

using namespace RecordGraph;

// event node
C_AddIcon::C_AddIcon() : I_Node(E_NodeType_AddIcon)
{
	m_Size = wxSize(8, 8);
	m_bDraggable = false;
}

bool C_AddIcon::MouseClick()
{
	if(m_pParent && m_pParent->GetType() == E_NodeType_Array)
	{
		C_ArrayNode* pArrayNode = (C_ArrayNode*)m_pParent;
		pArrayNode->Add();
		return true;
	}
	else
		return false;
}

void C_AddIcon::Draw(wxDC& dc) const
{
	wxRect rect = GetRect();
	wxPen pen = *wxBLACK_PEN;
	
	pen.SetWidth(2);
	dc.SetPen(pen);

	dc.DrawLine(rect.x, rect.y + rect.height / 2, rect.x + rect.width, rect.y + rect.height / 2);
	dc.DrawLine(rect.x + rect.width / 2, rect.y, rect.x + rect.width / 2, rect.y + rect.height);
}

