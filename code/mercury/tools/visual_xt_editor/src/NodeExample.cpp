#include "stdafx.h"

#include "NodeExample.h"


C_NodeExample::~C_NodeExample()
{
}

// sets the correct position of child widgets and also the size of this widget
void C_NodeExample::SetPositions()
{
}

bool C_NodeExample::EndDrag(I_Node* pTarget, wxPoint point)
{
	SetPosition(point);

	return false;
}

void C_NodeExample::PrepareDC(wxDC& dc) const
{
	wxColour colour;

	if(m_bDragSrc)
		colour = wxColour(128, 160, 128);
	else if(m_bDragDest)
		colour = wxColour(160, 128, 128);
	else
		colour = wxColour(128, 128, 160);

	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(wxBrush(colour));
}

void C_NodeExample::Draw(wxDC& dc) const
{
	float indent = 0;

	if(!m_bVisible)
		return;

	PrepareDC(dc);
	
	// draw window
	{
		dc.DrawRectangle(m_Position.x + indent, m_Position.y, m_Size.x - indent, m_Size.y);
	}

	// set device origin for children
	S32 x, y;
	dc.GetDeviceOrigin(&x, &y);
	dc.SetDeviceOrigin(x + m_Position.x, y + m_Position.y);

	// restore device origin
	dc.SetDeviceOrigin(x, y);

	for(int i = 0; i < m_Children.Size(); ++i)
	{
		m_Children[i]->Draw(dc);
	}
}

