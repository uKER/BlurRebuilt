
// For compilers that support precompilation, includes "wx/wx.h".
#include "stdafx.h"

#include "wx/image.h"

#include "NodeGraph.h"

#include "wx\datetime.h"
#include "NodeExample.h"
#include "system/misc/random.h"

// DragImage overloaded redraw function
bool DragImage::UpdateBackingFromWindow(wxDC& windowDC, wxMemoryDC& destDC, const wxRect& sourceRect, const wxRect& destRect) const
{
	destDC.SetClippingRegion(destRect);
	destDC.SetBackground(*wxLIGHT_GREY_BRUSH);
	destDC.Clear();

	m_pSchematic->PrepareDC(destDC);
	m_pSchematic->DrawGrid(destDC);
    m_pSchematic->DrawNodes(destDC, false);
	
	return true;
}

bool DragImage::ForceShow()
{
	m_isShown = false;
	return Show();
}


// C_NodeGraph

IMPLEMENT_CLASS(C_NodeGraph, wxWindow)

BEGIN_EVENT_TABLE(C_NodeGraph, wxWindow)
  EVT_PAINT(C_NodeGraph::OnPaint)
  EVT_SIZE(C_NodeGraph::OnSize)
  EVT_ERASE_BACKGROUND(C_NodeGraph::OnEraseBackground)
  EVT_MOUSE_EVENTS(C_NodeGraph::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(C_NodeGraph::OnMouseCaptureLostEvent)
END_EVENT_TABLE()

C_NodeGraph::C_NodeGraph( wxWindow *parent, wxWindowID id,
                    const wxPoint &pos, const wxSize &size)
        : wxWindow( parent, id, pos, size, wxNO_BORDER )
{
    SetBackgroundColour(* wxLIGHT_GREY);
    SetCursor(wxCursor(wxCURSOR_ARROW));

	m_dragMode = E_DragMode_None;
    m_pDragNode = NULL;
    m_dragImage = NULL;
	m_pSelectedNode = NULL;
    m_pTargetNode = NULL;
}

C_NodeGraph::~C_NodeGraph()
{
    DeleteNodes();

    if(m_dragImage)
        delete m_dragImage;
}

void C_NodeGraph::Initialise()
{
	// create a random number of example draggable shapes
	Util::C_Random random;
	wxDateTime dateTime;

	random.Seed((U32)dateTime.GetTimeNow());

	S32 numNodes = random.Rand(5) + 5;

	for(S32 i = 0; i < numNodes; ++i)
	{
		C_NodeExample* pNode = new C_NodeExample(0);
		pNode->SetPosition(wxPoint(10 + random.Rand(500), 10 + random.Rand(500)));
		pNode->SetSize(wxSize(32 + random.Rand(128), 32 + random.Rand(128)));
		pNode->GenerateBitmap();
		m_Nodes.PushBack(pNode);
	}
}

void C_NodeGraph::OnSize(wxSizeEvent& WXUNUSED(event))
{
	DrawBuffered();
}

void C_NodeGraph::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
	wxPaintDC dc( this );
	Paint(dc);
}

void C_NodeGraph::Paint(wxDC& dc)
{
	// setup transform from scrolled window
	PrepareDC(dc);

	DrawGrid(dc);
	DrawNodes(dc, true);
	Draw(dc);

	dc.SetDeviceOrigin(0, 0);
}

void C_NodeGraph::DrawBuffered()
{
	// get client dc
	wxClientDC dc(this);
	
	// get offscreen buffer
	wxMemoryDC dc2;
	wxSize sz = GetClientSize();
	wxBitmap bitmap(sz.x, sz.y);
	dc2.SelectObject(bitmap);
	// clear offscreen
	dc2.SetBackground(*wxLIGHT_GREY_BRUSH);
	dc2.Clear();

	Paint(dc2);

	dc.Blit(0, 0, sz.x, sz.y, & dc2, 0, 0);
}

void C_NodeGraph::DrawNode(const I_Node* pNode)
{
	// get client dc
	wxClientDC dc(this);
	pNode->PrepareDC(dc);
	pNode->Draw(dc);
}

void C_NodeGraph::Draw(wxDC& dc) const
{
}

void C_NodeGraph::DrawNodes(wxDC& dc, bool bDrawDragSrc) const
{
	for(S32 i = 0; i < m_Nodes.Size(); ++i)
	{
		I_Node* pNode = m_Nodes[i];
     
		pNode->Draw(dc);
    }

	if(bDrawDragSrc && m_pDragNode)
	{
		const I_Node* pParent = m_pDragNode->GetParent();
		S32 x, y;

		if(pParent)
		{
			wxPoint position = pParent->GetPositionAbsolute();
			dc.GetDeviceOrigin(&x, &y);
			dc.SetDeviceOrigin(x + position.x, y + position.y);
		}

		wxPoint oldPosition = m_pDragNode->GetPositionAbsolute();
		m_pDragNode->ToggleVisible();
		m_pDragNode->SetPositionAbsolute(oldPosition + m_MousePosition - m_DragPosition);
		m_pDragNode->ToggleVisible();
		m_pDragNode->SetPositionAbsolute(oldPosition);

		if(pParent)
		{
			// restore device origin
			dc.SetDeviceOrigin(x, y);
		}
	}
}

void C_NodeGraph::DrawGrid(wxDC& dc) const
{
	wxPen pen(wxColour(176, 176, 176));
	dc.SetPen(pen);

	wxSize size = GetClientSize();

	wxPoint move = m_gridOffset;
	move.x = move.x % 20;
	move.y = move.y % 20;

	for(S32 x = 0; x < size.GetWidth(); x += 20)
		dc.DrawLine(x + move.x, 0, x + move.x, size.GetHeight());

	for(S32 y = 0; y < size.GetHeight(); y += 20)
		dc.DrawLine(0, y + move.y, size.GetWidth(), y + move.y);
}

void C_NodeGraph::OnEraseBackground(wxEraseEvent& event)
{
	event.Skip(); // The official way of doing it
}

// mouse control

void C_NodeGraph::OnMouseCaptureLostEvent(wxMouseCaptureLostEvent& event)
{
	EndDrag(m_DragPosition);
}

bool C_NodeGraph::SelectNode(I_Node* pSelect)
{
	I_Node* pOldSelection = m_pSelectedNode;
	m_pSelectedNode = pSelect;

	bool bRedraw = false;

	if(pOldSelection != m_pSelectedNode)
	{
		if(pOldSelection)
			bRedraw = pOldSelection->SetSelected(false) || bRedraw;
		if(m_pSelectedNode)
			bRedraw = m_pSelectedNode->SetSelected(true) || bRedraw;
	}

	return bRedraw;
}

void C_NodeGraph::OnMouseEvent(wxMouseEvent& event)
{
	// get the device and logical m_dragCurrentPositions
	m_MousePosition = event.GetPosition();
	
	if (event.LeftDown())
    {
		I_Node* pOldSelection = m_pSelectedNode;

        if(I_Node* pNode = FindDraggable(m_MousePosition))
        {
			BringToFront(pNode);

			if(SelectNode(pNode))
				DrawBuffered();
		
			if(pNode->IsDraggable())
			{
				m_dragMode = E_DragMode_ShapeStart;
				m_DragPosition = m_MousePosition;
				m_pDragNode = pNode;
				m_pDragNode->ToggleVisible();
				m_pDragNode->SetDragSrc(true);
			}

			if(pNode->MouseClick())
				DrawBuffered();
        }
		else
		{
			if(SelectNode(NULL))
				DrawBuffered();
		}

		
    }
	else if(event.MiddleDown())
	{
		m_dragMode = E_DragMode_CanvasDragging;
		m_DragPosition = m_MousePosition;
		CaptureMouse();
		SetCursor(wxCursor(wxCURSOR_HAND));
	}
    else if ((event.LeftUp() || event.MiddleUp()))
    {
		EndDrag(m_MousePosition);
	}
    else if (event.Dragging() && m_dragMode != E_DragMode_None)
    {
		if (m_dragMode == E_DragMode_ShapeStart)
        {
            // We will start dragging if we've moved beyond a couple of pixels
            int tolerance = 2;
            int dx = abs(m_MousePosition.x - m_DragPosition.x);
            int dy = abs(m_MousePosition.y - m_DragPosition.y);
            if (dx <= tolerance && dy <= tolerance)
               return;

			// Start the drag.
            m_dragMode = E_DragMode_ShapeDragging;

			// if we don't redraw while dragging redraw a single time now
			if(!m_pDragNode->GetDragRedraw())
				DrawBuffered();

            if (m_dragImage)
			{
				delete m_dragImage;
				wxASSERT_MSG( false, wxT("DragImage should not exist here") );
			}

           	m_dragImage = new DragImage(this, m_pDragNode, m_pDragNode->GetBitmap(), wxCursor(wxCURSOR_HAND));
            m_dragImage->BeginDrag(m_DragPosition - m_pDragNode->GetPositionAbsolute(), this, false);
            m_dragImage->Move(m_DragPosition);
            m_dragImage->Show();
        }
        else if (m_dragMode == E_DragMode_ShapeDragging)
        {
            // We're currently dragging. See if we're over another node.
			I_Node* pTargetNode = FindDraggable(m_MousePosition, m_pDragNode);
			
			// if we were previously over another node set drag target as false
			if(m_pTargetNode && m_pTargetNode != pTargetNode)
			{
				// returns true if node needs to be redawn
				if(m_pTargetNode->SetDragDest(false))
				{
					DrawBuffered();
					m_dragImage->ForceShow();
				}
			}

			if(pTargetNode)
			{
				if(m_pTargetNode != pTargetNode)
				{
					m_pTargetNode = pTargetNode;
					if(m_pTargetNode->SetDragDest(true))
					{
						DrawBuffered();
						m_dragImage->ForceShow();
					}
				}
			}
			else
				m_pTargetNode = NULL;

			// see if we need to update whole screen while dragging
			if(m_pDragNode->GetDragRedraw())
			{
				m_pDragNode->SetPositionAbsolute(m_pDragNode->GetPositionAbsolute() + m_MousePosition - m_DragPosition);
				m_DragPosition = m_MousePosition;
				m_pDragNode->ToggleVisible();
				DrawBuffered();
				m_pDragNode->ToggleVisible();
			}
			else
			{
				m_dragImage->Move(m_MousePosition);			
			}
	    }
		else if(m_dragMode == E_DragMode_CanvasDragging)
        {
			// move nodes and offset grid etc
			wxPoint move = m_MousePosition - m_DragPosition;
			MoveNodes(move);
			m_gridOffset += move;
			m_DragPosition = m_MousePosition;

			DrawBuffered();
		}

    }
	else if(event.RightDown())
	{
		m_MousePosition = event.GetPosition();
		ShowMenu(event.GetPosition());
	}
	
}

void C_NodeGraph::EndDrag(wxPoint logicalPosition)
{
	if(m_dragMode != E_DragMode_None)
	{
		// Finish dragging
		if(m_dragMode == E_DragMode_ShapeStart)
		{
			m_pDragNode->ToggleVisible();
			m_pDragNode->SetDragSrc(false);
			m_pDragNode = NULL;
		}
		else if(m_dragMode == E_DragMode_ShapeDragging)
		{
			if (!m_pDragNode || !m_dragImage)
				return;

			bool bRedraw = false;

			if(m_pTargetNode)
				bRedraw = m_pTargetNode->SetDragDest(false) || bRedraw;

			m_pDragNode->ToggleVisible();
			bRedraw = m_pDragNode->SetDragSrc(false) || bRedraw;

			// end the drag, returns true if drag image should disappear, false if it stays put
			if(m_pDragNode->EndDrag(m_pTargetNode, m_pDragNode->GetPositionAbsolute() + logicalPosition - m_DragPosition))
				m_dragImage->Hide();
			
			m_dragImage->EndDrag();
			delete m_dragImage;
			m_dragImage = NULL;

			m_pTargetNode = NULL;
			m_pDragNode = NULL;

			if(bRedraw)
				DrawBuffered();
		}
		else
		{
			#ifdef __WXMSW__
				// Under Windows we can be pretty sure this test will give
				// the correct results
			if (wxWindow::GetCapture() == this)
			#endif
				ReleaseMouse();

			SetCursor(wxCursor(wxCURSOR_ARROW));
		}

		// end dragging
		m_dragMode = E_DragMode_None;
	}
}

void C_NodeGraph::SelectionChanged(I_Node* pOld, I_Node* pNew)
{
}


void C_NodeGraph::ShowMenu(const wxPoint& pt)
{
	
}

void C_NodeGraph::Clean(I_Node* pNode)
{
	if(m_pSelectedNode == pNode)
		m_pSelectedNode = NULL;
}

void C_NodeGraph::MoveNodes(wxPoint move)
{
	for(S32 i = 0; i < m_Nodes.Size(); ++i)
	{
		I_Node* pNode = m_Nodes[i];

		pNode->SetPosition(pNode->GetPositionAbsolute() + move);
	}
}

void C_NodeGraph::DeleteNodes()
{
	for(S32 i = 0; i < m_Nodes.Size(); ++i)
	{
		I_Node* pNode = m_Nodes[i];
        delete pNode;
    }

	m_Nodes.Clear();
}

I_Node* C_NodeGraph::FindDraggable(const wxPoint& pt, I_Node* pExclude)
{
	for(S32 i = m_Nodes.Size() - 1; i >= 0; --i )
	{
		I_Node* pNode = m_Nodes[i];
		
		if(I_Node* pHit = pNode->HitTest(pt, pExclude))
			return pHit;
	}

    return NULL;
}

void C_NodeGraph::BringToFront(I_Node* pNode)
{
	// look for draggable and move to end of array so it gets drawn last
	S32 remove = -1;

	for(S32 i = 0; i < m_Nodes.Size(); ++i )
	{
		if(pNode == m_Nodes[i])
		{
			remove = i;
			break;
		}
	}

	if(remove != -1)
	{
		m_Nodes.EraseByIndex(remove);
		m_Nodes.PushBack(pNode);
	}
}

void C_NodeGraph::LayoutNodes()
{
	
}

