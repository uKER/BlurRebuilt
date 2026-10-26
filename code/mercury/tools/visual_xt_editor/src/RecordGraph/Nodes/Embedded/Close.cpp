#include "stdafx.h"

#include "Close.h"
#include "../Type.h"

using namespace RecordGraph;

// event node
C_CloseIcon::C_CloseIcon() : I_Node(E_NodeType_CloseIcon)
{
	m_Size = wxSize(8, 8);
	m_bDraggable = false;
}

bool C_CloseIcon::MouseClick()
{
	if(m_pParent)
	{
		m_pParent->Close();
		return true;
	}
	else
		return false;
}

void C_CloseIcon::Draw(wxDC& dc) const
{
	wxRect rect = GetRect();
	wxPen pen = *wxBLACK_PEN;
	
	pen.SetWidth(2);
	dc.SetPen(pen);

	dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.width);
	dc.DrawLine(rect.x, rect.y + rect.height, rect.x + rect.width, rect.y);
}

