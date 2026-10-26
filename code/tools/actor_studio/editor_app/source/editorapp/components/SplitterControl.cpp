#include "stdafx.h"

#include "wx/renderer.h"
#include "wx/toplevel.h"
#include "wx/dcscreen.h"

#include <EditorApp/Components/SplitterControl.h>

#define PROPORTION(x) ((float)x/10000.f)

static U32 GetNumSplitters(E_SplitterLayout layout)
{
	switch (layout)
	{
	case E_SplitterLayout_Unknown:
	case E_SplitterLayout_1:
	case E_SplitterLayout_4:
		return 0;

	case E_SplitterLayout_2Horizontal:
	case E_SplitterLayout_2Vertical:
		return 1;

	case E_SplitterLayout_3Horizontal:
	case E_SplitterLayout_3Vertical:
	case E_SplitterLayout_3Top:
	case E_SplitterLayout_3Bottom:
	case E_SplitterLayout_3Left:
	case E_SplitterLayout_3Right:
		return 2;

	case E_SplitterLayout_4Horizontal:
	case E_SplitterLayout_4Vertical:
	case E_SplitterLayout_4Top:
	case E_SplitterLayout_4Bottom:
	case E_SplitterLayout_4Left:
	case E_SplitterLayout_4Right:
		return 3;
	};

	return 0;
}


//DEFINE_EVENT_TYPE(EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGED)
//DEFINE_EVENT_TYPE(EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGING)

IMPLEMENT_DYNAMIC_CLASS(wxSplitterCtrl, wxWindow)
//IMPLEMENT_DYNAMIC_CLASS(wxSplitterCtrlEvent, wxNotifyEvent)


BEGIN_EVENT_TABLE(wxSplitterCtrl, wxWindow)
EVT_PAINT(wxSplitterCtrl::OnPaint)
EVT_SIZE(wxSplitterCtrl::OnSize)
EVT_LEFT_DOWN(wxSplitterCtrl::OnMouseClick)
EVT_LEFT_UP(wxSplitterCtrl::OnMouseClick)
EVT_MOTION(wxSplitterCtrl::OnMouseMotion)
EVT_LEAVE_WINDOW(wxSplitterCtrl::OnMouseMotion)
EVT_ENTER_WINDOW(wxSplitterCtrl::OnMouseMotion)
EVT_MOUSE_CAPTURE_LOST(wxSplitterCtrl::OnMouseCaptureLost)
END_EVENT_TABLE()

wxSplitterCtrl::wxSplitterCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
:wxWindow(parent, id, pos, size, style, name)
, m_SashCursorWE(wxCursor(wxCURSOR_SIZEWE))
, m_SashCursorNS(wxCursor(wxCURSOR_SIZENS))
, m_SashCursorSize(wxCursor(wxCURSOR_SIZING))
, m_pSashTrackerPen(new wxPen(*wxBLACK, 2, wxSOLID))
, m_Flags(0)
, m_ExpandedWindow(-1)
{
	memset(m_pWindows, 0, sizeof(m_pWindows)/sizeof(m_pWindows[0]));
	memset(m_SplitPoints, 0, sizeof(m_SplitPoints)/sizeof(m_SplitPoints[0]));
}

wxSplitterCtrl::~wxSplitterCtrl()
{
	delete m_pSashTrackerPen;
}

void wxSplitterCtrl::Clear()
{
	//do we need to do anything with our current child windows?
	memset(m_pWindows, 0, sizeof(m_pWindows[0]) * sizeof(m_pWindows)/sizeof(m_pWindows[0]));
	memset(m_SplitPoints, 0, sizeof(m_SplitPoints[0]) * sizeof(m_SplitPoints)/sizeof(m_SplitPoints[0]));
	m_Layout = E_SplitterLayout_Unknown;
}

bool wxSplitterCtrl::Split(
	E_SplitterLayout layout, 
	wxWindow* pWindow0, 
	wxWindow* pWindow1, 
	wxWindow* pWindow2, 
	wxWindow* pWindow3)
{
	Clear();

	if (pWindow0 && pWindow0->GetParent() != this)
		pWindow0->Reparent(this);
	if (pWindow1 && pWindow1->GetParent() != this)
		pWindow1->Reparent(this);
	if (pWindow2 && pWindow2->GetParent() != this)
		pWindow2->Reparent(this);
	if (pWindow3 && pWindow3->GetParent() != this)
		pWindow3->Reparent(this);

	m_pWindows[0] = pWindow0;
	m_pWindows[1] = pWindow1;
	m_pWindows[2] = pWindow2;
	m_pWindows[3] = pWindow3;

	ChangeLayout(layout);
	Refresh();

	return true;
}

bool wxSplitterCtrl::ChangeLayout(E_SplitterLayout layout)
{
	if (m_Layout == layout)
		return true;

	switch (layout)
	{
	case E_SplitterLayout_1:
		break;

	case E_SplitterLayout_2Horizontal:
		m_SplitPoints[0].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[0].proportion = 5000;
		break;

	case E_SplitterLayout_2Vertical:
		m_SplitPoints[0].axis = E_SplitAxis_Vertical;
		m_SplitPoints[0].proportion = 5000;
		break;

	case E_SplitterLayout_3Horizontal:
		m_SplitPoints[0].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[0].proportion = 3333;
		m_SplitPoints[1].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[1].proportion = 6666;
		break;

	case E_SplitterLayout_3Vertical:
		m_SplitPoints[0].axis = E_SplitAxis_Vertical;
		m_SplitPoints[0].proportion = 3333;
		m_SplitPoints[1].axis = E_SplitAxis_Vertical;
		m_SplitPoints[1].proportion = 6666;
		break;

	case E_SplitterLayout_3Top:
	case E_SplitterLayout_3Bottom:
		m_SplitPoints[0].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[0].proportion = 5000;
		m_SplitPoints[1].axis = E_SplitAxis_Vertical;
		m_SplitPoints[1].proportion = 5000;
		break;

	case E_SplitterLayout_3Left:
	case E_SplitterLayout_3Right:
		m_SplitPoints[0].axis = E_SplitAxis_Vertical;
		m_SplitPoints[0].proportion = 5000;
		m_SplitPoints[1].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[1].proportion = 5000;
		break;

	case E_SplitterLayout_4:
		m_SplitPoints[0].proportionX = 5000;
		m_SplitPoints[0].proportionY = 5000;
		break;

	case E_SplitterLayout_4Horizontal:
		m_SplitPoints[0].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[0].proportion = 2500;
		m_SplitPoints[1].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[1].proportion = 5000;
		m_SplitPoints[2].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[2].proportion = 7500;
		break;

	case E_SplitterLayout_4Vertical:
		m_SplitPoints[0].axis = E_SplitAxis_Vertical;
		m_SplitPoints[0].proportion = 2500;
		m_SplitPoints[1].axis = E_SplitAxis_Vertical;
		m_SplitPoints[1].proportion = 5000;
		m_SplitPoints[2].axis = E_SplitAxis_Vertical;
		m_SplitPoints[2].proportion = 7500;
		break;

	case E_SplitterLayout_4Top:
	case E_SplitterLayout_4Bottom:
		m_SplitPoints[0].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[0].proportion = 5000;
		m_SplitPoints[1].axis = E_SplitAxis_Vertical;
		m_SplitPoints[1].proportion = 3333;
		m_SplitPoints[2].axis = E_SplitAxis_Vertical;
		m_SplitPoints[2].proportion = 6666;
		break;
	
	case E_SplitterLayout_4Left:
	case E_SplitterLayout_4Right:	
		m_SplitPoints[0].axis = E_SplitAxis_Vertical;
		m_SplitPoints[0].proportion = 5000;
		m_SplitPoints[1].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[1].proportion = 3333;
		m_SplitPoints[2].axis = E_SplitAxis_Horizontal;
		m_SplitPoints[2].proportion = 6666;
		break;

	default:
		m_Layout = E_SplitterLayout_Unknown;
		return false;
	}

	m_Layout = layout;
	m_ExpandedWindow = -1;
	SizeWindows();
	return true;
}

void wxSplitterCtrl::SetSplitterProportion(S32 p1, S32 p2, U32 index)
{
	U32 nSplitters = GetNumSplitters(m_Layout);
	assert(index < nSplitters);

	if (m_Layout == E_SplitterLayout_4)
	{
		assert(p1>0 && p2>0);
		m_SplitPoints[index].proportionX = p1;
		m_SplitPoints[index].proportionY = p2;
	}
	else
		m_SplitPoints[index].proportion = p1;

	SizeWindows();
	Refresh();
}

E_SplitterLayout wxSplitterCtrl::GetLayout() const
{
	return m_Layout;
}

bool wxSplitterCtrl::IsExpanded() const
{
	return m_ExpandedWindow != -1;
}

wxWindow* wxSplitterCtrl::GetWindow(S32 i) const
{
	if (i>=0 && i<sizeof(m_pWindows)/sizeof(m_pWindows[0]))
		return m_pWindows[i];
	else
		return NULL;
}

void wxSplitterCtrl::SetWindow(S32 i, wxWindow *pWindow)
{
	if (i>=0 && i<sizeof(m_pWindows)/sizeof(m_pWindows[0]))
	{
		if (pWindow->GetParent() != this)
			pWindow->Reparent(this);

		for (S32 ii=0; ii<4; ++ii)
		{
			if (m_pWindows[ii] == pWindow && ii!=i)
			{
				//swap
				m_pWindows[ii] = m_pWindows[i];
				m_pWindows[i] = pWindow;
				return;
			}
		}
		m_pWindows[i] = pWindow;
		SizeWindows();
	}
}

void wxSplitterCtrl::SwapWindows(wxWindow *pWindow1, wxWindow *pWindow2)
{
	S32 i1 = -1, i2 = -1;

	for (S32 i=0; i<4; ++i)
		if (m_pWindows[i] == pWindow1)
			i1 = i;
		else if (m_pWindows[i] == pWindow2)
			i2 = i;

	if (i1!=-1 && i2!=-1)
	{
		wxWindow *pTmp = m_pWindows[i1];
		m_pWindows[i1] = m_pWindows[i2];
		m_pWindows[i2] = pTmp;
	}
	else
		wxASSERT_MSG(false, _T("windows are not under splitter's control"));

	SizeWindows();
}

void wxSplitterCtrl::ExpandWindow(S32 i)
{
	if (i>=0 && i<sizeof(m_pWindows)/sizeof(m_pWindows[0]))
		m_ExpandedWindow = i;

	SizeWindows();
}

void wxSplitterCtrl::ExpandWindow(wxWindow *pWindow)
{
	for (S32 i=0; i<sizeof(m_pWindows)/sizeof(m_pWindows[0]); ++i)
		if (pWindow == m_pWindows[i])
			m_ExpandedWindow = i;

	SizeWindows();
}

void wxSplitterCtrl::UnexpandWindow()
{
	if (IsExpanded())
	{
		m_ExpandedWindow = -1;
		SizeWindows();
	}
}

void wxSplitterCtrl::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);

	wxColour backColour = GetBackgroundColour();
	dc.SetBrush(wxBrush(backColour, wxSOLID));
	dc.SetPen(wxPen(backColour));
	dc.Clear();

	DrawSplitter(dc);
}

void wxSplitterCtrl::OnSize(wxSizeEvent& e)
{
	wxTopLevelWindow* parent = static_cast<wxTopLevelWindow*>(wxGetTopLevelParent(this));
	if (parent->IsIconized())
		return e.Skip();

	SizeWindows();
	Refresh();
}

void wxSplitterCtrl::OnMouseClick(wxMouseEvent &e)
{
	if (!IsEnabled())
		return;

	if (e.LeftDown())
	{
		if (!(m_Flags & Active))
		{
			E_SplitAxis axis;
			m_ActiveSplitter = GetActiveSplitter(e.GetPosition(), &axis);
			if (m_ActiveSplitter != -1)
			{
				UpdateSplitterClampPoints();
				m_ActiveSplitterPos = e.GetPosition();
				m_Flags |= Active;

				if (axis == E_SplitAxis_Horizontal)
					m_Flags |= Horizontal;
				else if (axis == E_SplitAxis_Vertical)
					m_Flags |= Vertical;
				else if (axis == E_SplitAxis_Both)
					m_Flags |= Horizontal|Vertical;

				CaptureMouse();
				DrawUpdateSplitter(m_ActiveSplitterPos);
			}
		}
	}

	else if (e.LeftUp())
	{
		if (wxWindow::GetCapture() == this)
			ReleaseMouse();

		//Splitter lines are drawn using an invert function
		//Drawing this again will remove the previous drawn line
		DrawUpdateSplitter(m_ActiveSplitterPos);

		UpdateLayout();
		SizeWindows();
		m_Flags = 0;
	}
}

void wxSplitterCtrl::OnMouseMotion(wxMouseEvent &e)
{
	if (HasFlag(wxSP_NOSASH))
		return e.Skip();

	if (e.GetEventType() == wxEVT_LEAVE_WINDOW)
	{
		SetCursor(*wxSTANDARD_CURSOR);
		return e.Skip();
	}

	if (m_Flags & Active)
	{
		wxSize size = GetSize();
		S32 sashSize = GetSashSize();

		wxPoint posPrev = m_ActiveSplitterPos;
		m_ActiveSplitterPos = e.GetPosition();
		m_ActiveSplitterPos.x = Maths::Clamp(m_ActiveSplitterPos.x, m_Min.x, m_Max.x);
		m_ActiveSplitterPos.y = Maths::Clamp(m_ActiveSplitterPos.y, m_Min.y, m_Max.y);
		DrawUpdateSplitter(posPrev);
		DrawUpdateSplitter(m_ActiveSplitterPos);
	}




	//set cursor
	if (m_Flags & Active)
	{
		if ((m_Flags & Vertical) && (m_Flags & Horizontal))
			SetCursor(m_SashCursorSize);
		else if (m_Flags & Vertical)
			SetCursor(m_SashCursorWE);
		else
			SetCursor(m_SashCursorNS);
	}
	else
	{
		E_SplitAxis axis;
		GetActiveSplitter(e.GetPosition(), &axis);
		switch (axis)
		{
		case E_SplitAxis_Horizontal:
			SetCursor(m_SashCursorNS);
			break;
		case E_SplitAxis_Vertical:
			SetCursor(m_SashCursorWE);
			break;
		case E_SplitAxis_Both:
			SetCursor(m_SashCursorSize);
			break;
		default:
			SetCursor(*wxSTANDARD_CURSOR);
		}
	}

	e.Skip();
}

void wxSplitterCtrl::OnMouseCaptureLost(wxMouseCaptureLostEvent &e)
{
	m_Flags &= ~Active;
	if (wxWindow::GetCapture() == this)
		ReleaseMouse();
}


void wxSplitterCtrl::DrawSplitter(wxClientDC &dc)
{
	if (GetWindowStyle() & wxSP_SASH_BORDERS && !IsExpanded())
	{
		dc.SetPen(wxPen(wxColour(159,156,150,255)));
		S32 sashSize = GetSashSize();
		wxSize size = GetSize();

		switch (m_Layout)
		{
		case E_SplitterLayout_1:
			break;

		case E_SplitterLayout_2Horizontal:
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));
			break;

		case E_SplitterLayout_2Vertical:
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, size.y));
			break;

		case E_SplitterLayout_3Horizontal:
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize));
			break;

		case E_SplitterLayout_3Vertical:
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, size.y));
			break;

		case E_SplitterLayout_3Top:
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));

			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, size.y));

			break;

		case E_SplitterLayout_3Bottom:
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));

			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+1));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			break;

		case E_SplitterLayout_3Left:
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, size.y));

			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize));
			break;

		case E_SplitterLayout_3Right:
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, size.y));

			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+1, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize));
			break;

		case E_SplitterLayout_4:
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportionY)*size.y),
						wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x, PROPORTION(m_SplitPoints[0].proportionY)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportionY)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportionY)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportionY)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x, PROPORTION(m_SplitPoints[0].proportionY)*size.y+sashSize));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportionY)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportionY)*size.y+sashSize));

			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x, PROPORTION(m_SplitPoints[0].proportionY)*size.y+1));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x, PROPORTION(m_SplitPoints[0].proportionY)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x, size.y));						
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportionY)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportionY)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x+sashSize, size.y));
			break;

		case E_SplitterLayout_4Horizontal:
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[2].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[2].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize));
			break;

		case E_SplitterLayout_4Vertical:
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, size.y));
			break;

		case E_SplitterLayout_4Top:
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));

			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));

			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, size.y));
			break;

		case E_SplitterLayout_4Bottom:
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y));

			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize));

			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+1));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x, PROPORTION(m_SplitPoints[0].proportion)*size.y+1));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y));
			break;

		case E_SplitterLayout_4Left:
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[2].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, size.y));

			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[2].proportion)*size.y),
						wxPoint(size.x, PROPORTION(m_SplitPoints[2].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize),
						wxPoint(size.x, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize));

			break;

		case E_SplitterLayout_4Right:
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, PROPORTION(m_SplitPoints[2].proportion)*size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, size.y));
			dc.DrawLine(wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, size.y));

			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+1, PROPORTION(m_SplitPoints[1].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[2].proportion)*size.y),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+1, PROPORTION(m_SplitPoints[2].proportion)*size.y));
			dc.DrawLine(wxPoint(0, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize),
						wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize));
			break;

		default:
			break;
		}
	}

}

void wxSplitterCtrl::DrawUpdateSplitter(wxPoint& pt)
{
	wxScreenDC dc;

	dc.SetLogicalFunction(wxINVERT);
	dc.SetPen(*m_pSashTrackerPen);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	if (m_Flags & Horizontal)
	{
		wxPoint p1 = m_Min; p1.y = pt.y;
		wxPoint p2 = m_Max; p2.y = pt.y;

		p1 = ClientToScreen(p1);
		p2 = ClientToScreen(p2);

		dc.DrawLine(p1, p2);
	}

	if (m_Flags & Vertical)
	{
		wxPoint p1 = m_Min; p1.x = pt.x;
		wxPoint p2 = m_Max; p2.x = pt.x;

		p1 = ClientToScreen(p1);
		p2= ClientToScreen(p2);

		dc.DrawLine(p1, p2);
	}

	dc.SetLogicalFunction(wxCOPY);
}

void wxSplitterCtrl::SizeWindows()
{
	wxSize size = GetSize();
	S32 borderSize = GetBorderSize();
	S32 sashSize = GetSashSize();
	size.x -= 2*borderSize;
	size.y -= 2*borderSize;

	//make space for our custom sash borders
	if (GetWindowStyle() & wxSP_SASH_BORDERS)
		sashSize += 1;

	for (S32 i=0; i<4; ++i)
		if (m_pWindows[i])
			m_pWindows[i]->Freeze();

	if (IsExpanded())
	{
		m_pWindows[m_ExpandedWindow]->SetSize(wxRect(
				wxPoint(0, 0), 
				wxPoint(size.x-1, size.y-1)));
	}
	else
	{
		switch (m_Layout)
		{
		case E_SplitterLayout_1:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0), 
					wxPoint(size.x, size.y)));
			break;

		case E_SplitterLayout_2Horizontal:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_2Vertical:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0), 
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, size.y)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_3Horizontal:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0),
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[1].proportion)*size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_3Vertical:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0),
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0), 
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, 0), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_3Top:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0),
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize), 
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_3Bottom:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
					wxPoint(size.x-1, size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(0, 0), 
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, 0), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			break;

		case E_SplitterLayout_3Left:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0),
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[1].proportion)*size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_3Right:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0),
					wxPoint(size.x-1, size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(0, 0), 
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, PROPORTION(m_SplitPoints[1].proportion)*size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize), 
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_4:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0),
					wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x-1, PROPORTION(m_SplitPoints[0].proportionY)*size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x+sashSize, 0), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[0].proportionY)*size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[0].proportionY)*size.y+sashSize), 
					wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x-1, size.y-1)));
			if (m_pWindows[3])
				m_pWindows[3]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportionX)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportionY)*size.y+sashSize), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_4Horizontal:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0),
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[1].proportion)*size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[2].proportion)*size.y-1)));
			if (m_pWindows[3])
				m_pWindows[3]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_4Vertical:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0),
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0), 
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, 0), 
					wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[3])
				m_pWindows[3]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, 0), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_4Top:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0),
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize), 
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize), 
					wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[3])
				m_pWindows[3]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_4Bottom:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize),
					wxPoint(size.x-1, size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(0, 0), 
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize, 0), 
					wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			if (m_pWindows[3])
				m_pWindows[3]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize, 0), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[0].proportion)*size.y-1)));
			break;

		case E_SplitterLayout_4Left:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(0, 0),
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[1].proportion)*size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, PROPORTION(m_SplitPoints[2].proportion)*size.y-1)));
			if (m_pWindows[3])
				m_pWindows[3]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize), 
					wxPoint(size.x-1, size.y-1)));
			break;

		case E_SplitterLayout_4Right:
			if (m_pWindows[0])
				m_pWindows[0]->SetSize(wxRect(
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize, 0),
					wxPoint(size.x-1, size.y-1)));
			if (m_pWindows[1])
				m_pWindows[1]->SetSize(wxRect(
					wxPoint(0, 0), 
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, PROPORTION(m_SplitPoints[1].proportion)*size.y-1)));
			if (m_pWindows[2])
				m_pWindows[2]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize), 
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, PROPORTION(m_SplitPoints[2].proportion)*size.y-1)));
			if (m_pWindows[3])
				m_pWindows[3]->SetSize(wxRect(
					wxPoint(0, PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize), 
					wxPoint(PROPORTION(m_SplitPoints[0].proportion)*size.x-1, size.y-1)));

			break;

		default:
			break;
		}
	}

	SetWindowVisibility();

	for (S32 i=0; i<4; ++i)
		if (m_pWindows[i])
			m_pWindows[i]->Thaw();

	Refresh();
}

void wxSplitterCtrl::SetWindowVisibility()
{
	if (!IsExpanded())
	{
		S32 nWindows = 0;
		switch (m_Layout)
		{
		case E_SplitterLayout_1:
			nWindows = 1;
			break;

		case E_SplitterLayout_2Horizontal:
		case E_SplitterLayout_2Vertical:
			nWindows = 2;
			break;

		case E_SplitterLayout_3Horizontal:
		case E_SplitterLayout_3Vertical:
		case E_SplitterLayout_3Top:
		case E_SplitterLayout_3Bottom:
		case E_SplitterLayout_3Left:
		case E_SplitterLayout_3Right:
			nWindows = 3;
			break;

		case E_SplitterLayout_4:
		case E_SplitterLayout_4Horizontal:
		case E_SplitterLayout_4Vertical:
		case E_SplitterLayout_4Top:
		case E_SplitterLayout_4Bottom:
		case E_SplitterLayout_4Left:
		case E_SplitterLayout_4Right:
			nWindows = 4;
			break;
		}

		for (S32 i=0; i<nWindows; ++i)
			if (m_pWindows[i]) m_pWindows[i]->Show(true);
		for (S32 i=nWindows; i<4; ++i)
			if (m_pWindows[i]) m_pWindows[i]->Show(false);
	}
	else
	{
		for (S32 i=0; i<4; ++i)
			if (i == m_ExpandedWindow)
				m_pWindows[i]->Show(true);
			else
				m_pWindows[i]->Show(false);
	}
}

S32 wxSplitterCtrl::GetBorderSize() const
{
	return wxRendererNative::Get().GetSplitterParams(this).border;
}

S32 wxSplitterCtrl::GetSashSize() const
{
	if (HasFlag(wxSP_NOSASH))
		return 0;
	else
		return wxRendererNative::Get().GetSplitterParams(this).widthSash;
}

S32 wxSplitterCtrl::GetActiveSplitter(const wxPoint& pt, E_SplitAxis* pAxis) const
{
	const S32 TOLERANCE = 5;
	wxSize size = GetSize();
	S32 borderSize = GetBorderSize();
	S32 sashSize = GetSashSize();

	size.x -= 2*borderSize;
	size.y -= 2*borderSize;

	E_SplitAxis axis = E_SplitAxis_Unknown;
	S32 splitterIndex = -1;

	if (IsExpanded())
	{
		if (pAxis)
			*pAxis = axis;
		return splitterIndex;
	}

	switch(m_Layout)
	{
	case E_SplitterLayout_2Horizontal:
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 0;
		}
		break;

	case E_SplitterLayout_2Vertical:
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 0;
		}
		break;

	case E_SplitterLayout_3Horizontal:
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 0;
		}

		if (pt.y > PROPORTION(m_SplitPoints[1].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 1;
		}
		break;

	case E_SplitterLayout_3Vertical:
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 0;
		}
		if (pt.x > PROPORTION(m_SplitPoints[1].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 1;
		}
		break;

	case E_SplitterLayout_3Top:
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize+TOLERANCE)
		{
			if (pAxis) *pAxis = E_SplitAxis_Horizontal;
			splitterIndex = 0;
		}
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y &&
			pt.x > PROPORTION(m_SplitPoints[1].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 1;
		}
		break;

	case E_SplitterLayout_3Bottom:
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 0;
		}
		if (pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y &&
			pt.x > PROPORTION(m_SplitPoints[1].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 1;
		}
		break;

	case E_SplitterLayout_3Left:
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 0;
		}
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x &&
			pt.y > PROPORTION(m_SplitPoints[1].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 1;
		}
		break;

	case E_SplitterLayout_3Right:
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 0;
		}
		if (pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x &&
			pt.y > PROPORTION(m_SplitPoints[1].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 1;
		}
		break;

	case E_SplitterLayout_4:
		if (pt.x > PROPORTION(m_SplitPoints[0].proportionX)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[0].proportionX)*size.x+sashSize+TOLERANCE)
			axis = E_SplitAxis_Vertical;

		if (pt.y > PROPORTION(m_SplitPoints[0].proportionY)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[0].proportionY)*size.y+sashSize+TOLERANCE)
			axis = (axis == E_SplitAxis_Vertical) ? E_SplitAxis_Both : E_SplitAxis_Horizontal;

		if (axis)
			splitterIndex = 0;
		break;

	case E_SplitterLayout_4Horizontal:
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 0;
		}

		if (pt.y > PROPORTION(m_SplitPoints[1].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 1;
		}

		if (pt.y > PROPORTION(m_SplitPoints[2].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 2;
		}
		break;

	case E_SplitterLayout_4Vertical:
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 0;
		}
		if (pt.x > PROPORTION(m_SplitPoints[1].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 1;
		}
		if (pt.x > PROPORTION(m_SplitPoints[2].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 2;
		}
		break;

	case E_SplitterLayout_4Top:
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize+TOLERANCE)
		{
			if (pAxis) *pAxis = E_SplitAxis_Horizontal;
			splitterIndex = 0;
		}
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y &&
			pt.x > PROPORTION(m_SplitPoints[1].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 1;
		}
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y &&
			pt.x > PROPORTION(m_SplitPoints[2].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 2;
		}
		break;

	case E_SplitterLayout_4Bottom:
		if (pt.y > PROPORTION(m_SplitPoints[0].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 0;
		}
		if (pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y &&
			pt.x > PROPORTION(m_SplitPoints[1].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[1].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 1;
		}
		if (pt.y < PROPORTION(m_SplitPoints[0].proportion)*size.y &&
			pt.x > PROPORTION(m_SplitPoints[2].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[2].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 2;
		}
		break;

	case E_SplitterLayout_4Left:
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 0;
		}
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x &&
			pt.y > PROPORTION(m_SplitPoints[1].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 1;
		}
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x &&
			pt.y > PROPORTION(m_SplitPoints[2].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 2;
		}
		break;

	case E_SplitterLayout_4Right:
		if (pt.x > PROPORTION(m_SplitPoints[0].proportion)*size.x-TOLERANCE &&
			pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Vertical;
			splitterIndex = 0;
		}
		if (pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x &&
			pt.y > PROPORTION(m_SplitPoints[1].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[1].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 1;
		}
		if (pt.x < PROPORTION(m_SplitPoints[0].proportion)*size.x &&
			pt.y > PROPORTION(m_SplitPoints[2].proportion)*size.y-TOLERANCE &&
			pt.y < PROPORTION(m_SplitPoints[2].proportion)*size.y+sashSize+TOLERANCE)
		{
			axis = E_SplitAxis_Horizontal;
			splitterIndex = 2;
		}
	}

	if (pAxis)
		*pAxis = axis;
	return splitterIndex;
}

void wxSplitterCtrl::UpdateSplitterClampPoints() const
{
	wxSize size = GetSize();
	S32 borderSize = GetBorderSize();
	S32 sashSize = GetSashSize();

	size.x -=2*borderSize;
	size.y -=2*borderSize;

	m_Min.x = 0; m_Min.y = 0;
	m_Max.x = size.x-sashSize; m_Max.y = size.y-sashSize;

	switch (m_Layout)
	{
	case E_SplitterLayout_3Horizontal:
		if (m_ActiveSplitter > 0)
			m_Min.y = PROPORTION(m_SplitPoints[m_ActiveSplitter-1].proportion)*size.y+sashSize;
		if (m_ActiveSplitter < 1)
			m_Max.y = PROPORTION(m_SplitPoints[m_ActiveSplitter+1].proportion)*size.y-sashSize;
		break;

	case E_SplitterLayout_3Vertical:
		if (m_ActiveSplitter > 0)
			m_Min.x = PROPORTION(m_SplitPoints[m_ActiveSplitter-1].proportion)*size.x+sashSize;
		if (m_ActiveSplitter < 1)
			m_Max.x = PROPORTION(m_SplitPoints[m_ActiveSplitter+1].proportion)*size.x-sashSize;
		break;

	case E_SplitterLayout_3Top:
		if (m_ActiveSplitter > 0)
			m_Min.y = PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize;
		break;

	case E_SplitterLayout_3Bottom:
		if (m_ActiveSplitter > 0)
			m_Max.y = PROPORTION(m_SplitPoints[0].proportion)*size.y-sashSize;
		break;

	case E_SplitterLayout_3Left:
		if (m_ActiveSplitter > 0)
			m_Min.x = PROPORTION(m_SplitPoints[0].proportion)*size.x+sashSize;
		break;

	case E_SplitterLayout_3Right:
		if (m_ActiveSplitter > 0)
			m_Max.x = PROPORTION(m_SplitPoints[0].proportion)*size.x-sashSize;
		break;

	case E_SplitterLayout_4Horizontal:
		if (m_ActiveSplitter > 0)
			m_Min.y = PROPORTION(m_SplitPoints[m_ActiveSplitter-1].proportion)*size.y+sashSize;
		if (m_ActiveSplitter < 2)
			m_Max.y = PROPORTION(m_SplitPoints[m_ActiveSplitter+1].proportion)*size.y-sashSize;
		break;

	case E_SplitterLayout_4Vertical:
		if (m_ActiveSplitter > 0)
			m_Min.x = PROPORTION(m_SplitPoints[m_ActiveSplitter-1].proportion)*size.x+sashSize;
		if (m_ActiveSplitter < 2)
			m_Max.x = PROPORTION(m_SplitPoints[m_ActiveSplitter+1].proportion)*size.x-sashSize;
		break;

	case E_SplitterLayout_4Top:
		if (m_ActiveSplitter > 0)
			m_Min.y = PROPORTION(m_SplitPoints[0].proportion)*size.y+sashSize;
		if (m_ActiveSplitter > 1)
			m_Min.x = PROPORTION(m_SplitPoints[m_ActiveSplitter-1].proportion)*size.x+sashSize;
		if (m_ActiveSplitter > 0 && m_ActiveSplitter < 2)
			m_Max.x = PROPORTION(m_SplitPoints[m_ActiveSplitter+1].proportion)*size.x-sashSize;
		break;

	case E_SplitterLayout_4Bottom:
		if (m_ActiveSplitter > 0)
			m_Max.y = PROPORTION(m_SplitPoints[0].proportion)*size.y-sashSize;
		if (m_ActiveSplitter > 1)
			m_Min.x = PROPORTION(m_SplitPoints[m_ActiveSplitter-1].proportion)*size.x+sashSize;
		if (m_ActiveSplitter > 0 && m_ActiveSplitter < 2)
			m_Max.x = PROPORTION(m_SplitPoints[m_ActiveSplitter+1].proportion)*size.x-sashSize;
		break;

	case E_SplitterLayout_4Left:
		if (m_ActiveSplitter > 0)
			m_Min.x = PROPORTION(m_SplitPoints[0].proportion)*size.x;
		if (m_ActiveSplitter > 1)
			m_Min.y = PROPORTION(m_SplitPoints[m_ActiveSplitter-1].proportion)*size.y+sashSize;
		if (m_ActiveSplitter > 0 && m_ActiveSplitter < 2)
			m_Max.y = PROPORTION(m_SplitPoints[m_ActiveSplitter+1].proportion)*size.y-sashSize;
		break;

	case E_SplitterLayout_4Right:
		if (m_ActiveSplitter > 0)
			m_Max.x = PROPORTION(m_SplitPoints[0].proportion)*size.x;
		if (m_ActiveSplitter > 1)
			m_Min.y = PROPORTION(m_SplitPoints[m_ActiveSplitter-1].proportion)*size.y+sashSize;
		if (m_ActiveSplitter > 0 && m_ActiveSplitter < 2)
			m_Max.y = PROPORTION(m_SplitPoints[m_ActiveSplitter+1].proportion)*size.y-sashSize;
		break;


	default:
		break;
	};
}

void wxSplitterCtrl::UpdateLayout()
{
	wxSize size = GetSize();
	S32 borderSize = GetBorderSize();

	size.x -= 2*borderSize;
	size.y -= 2*borderSize;

	if (m_ActiveSplitter == -1)
		return;

	SplitPoint& split = m_SplitPoints[m_ActiveSplitter];
	switch (m_Layout)
	{
	case E_SplitterLayout_2Horizontal:
	case E_SplitterLayout_2Vertical:
	case E_SplitterLayout_3Horizontal:
	case E_SplitterLayout_3Vertical:
	case E_SplitterLayout_3Top:
	case E_SplitterLayout_3Bottom:
	case E_SplitterLayout_3Left:
	case E_SplitterLayout_3Right:
	case E_SplitterLayout_4Horizontal:
	case E_SplitterLayout_4Vertical:
	case E_SplitterLayout_4Top:
	case E_SplitterLayout_4Bottom:
	case E_SplitterLayout_4Left:
	case E_SplitterLayout_4Right:

		if (split.axis == E_SplitAxis_Horizontal)
			split.proportion = S32 (10000 * (F32)m_ActiveSplitterPos.y/(F32)size.y);
		else
			split.proportion = S32 (10000 * (F32)m_ActiveSplitterPos.x/(F32)size.x);
		split.proportion = Maths::Clamp(split.proportion, 0, 10000);
		break;

	case E_SplitterLayout_4:
		if (m_Flags & Horizontal)
			split.proportionY = S32 (10000 * (F32)m_ActiveSplitterPos.y/(F32)size.y);			
		if (m_Flags & Vertical)
			split.proportionX = S32 (10000 * (F32)m_ActiveSplitterPos.x/(F32)size.x);


		break;
	}
}