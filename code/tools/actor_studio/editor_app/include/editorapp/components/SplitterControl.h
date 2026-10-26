#ifndef EDITOR_SPLITTER_CTRL
#define EDITOR_SPLITTER_CTRL

#ifdef _MSC_VER
#pragma once
#endif

#include "wx/event.h"
#include "wx/window.h"     
#include "wx/dcclient.h"
#include "wx/containr.h"                   
// for common constants
#include "wx/splitter.h"

//1-4 way splitter control. Loosely based on wxFourWaySplitter on sourceforge

enum E_SplitterLayout
{
	E_SplitterLayout_Unknown,
	E_SplitterLayout_1 = 10001,		//HACK: also used as eventIDs in menu
	E_SplitterLayout_2Horizontal,
	E_SplitterLayout_2Vertical,

	E_SplitterLayout_3Horizontal,
	E_SplitterLayout_3Vertical,
	E_SplitterLayout_3Top,
	E_SplitterLayout_3Bottom,
	E_SplitterLayout_3Left,
	E_SplitterLayout_3Right,

	E_SplitterLayout_4,
	E_SplitterLayout_4Horizontal,
	E_SplitterLayout_4Vertical,
	E_SplitterLayout_4Top,
	E_SplitterLayout_4Bottom,
	E_SplitterLayout_4Left,
	E_SplitterLayout_4Right
};

enum E_SplitAxis
{
	E_SplitAxis_Unknown,
	E_SplitAxis_Horizontal,
	E_SplitAxis_Vertical,
	E_SplitAxis_Both

};

#define wxSP_SASH_BORDERS      0x0800

class wxSplitterCtrl: public wxWindow
{
public:
	DECLARE_DYNAMIC_CLASS(wxSplitterCtrl)

	wxSplitterCtrl(	wxWindow *parent = NULL, wxWindowID id = wxID_ANY,
					const wxPoint& pos = wxDefaultPosition,
					const wxSize& size = wxDefaultSize, long style = wxNO_BORDER|wxSP_SASH_BORDERS,
					const wxString& name = "splitterCtrl");
	~wxSplitterCtrl();

	void Clear();

	bool Split(
		E_SplitterLayout layout,
		wxWindow* pWindow0, 
		wxWindow* pWindow1=0, 
		wxWindow* pWindow2=0, 
		wxWindow* pWindow3=0);
	bool ChangeLayout(E_SplitterLayout layout);
	void SetSplitterProportion(S32 p1, S32 p2=-1, U32 index=0);

	E_SplitterLayout GetLayout() const;

	wxWindow* GetWindow(S32 i) const;
	void SetWindow(S32 i, wxWindow *pWindow);
	void SwapWindows(wxWindow *pWindow1, wxWindow *pWindow2);
	void ExpandWindow(S32 i);
	void ExpandWindow(wxWindow *pWindow);
	void UnexpandWindow();
	bool IsExpanded() const;

	void OnPaint(wxPaintEvent &e);
	void OnSize(wxSizeEvent &e);
	void OnMouseClick(wxMouseEvent &e); 
	void OnMouseMotion(wxMouseEvent &e);
	void OnMouseCaptureLost(wxMouseCaptureLostEvent& e);


private:

	void DrawSplitter(wxClientDC& dc);
	void DrawUpdateSplitter(wxPoint& pt);

	void SizeWindows();
	void SetWindowVisibility();
	S32 GetBorderSize() const;
	S32 GetSashSize() const;
	S32 GetActiveSplitter(const wxPoint &pt, E_SplitAxis *pAxis = 0) const;
	void UpdateSplitterClampPoints() const;
	void UpdateLayout();

	union SplitPoint
	{
		struct
		{
			E_SplitAxis axis;
			S32 proportion;		//in range 0-10000
		};

		struct
		{	//special case for E_SplitterLayout_4
			S32 proportionX;
			S32 proportionY;
		};
	};

	wxWindow			*m_pWindows[4];

	//The interpretation of these splitting points is dependent upon the current E_SplitterLayout.
	SplitPoint			m_SplitPoints[3];
	E_SplitterLayout	m_Layout;
	S32					m_ExpandedWindow;
	S32					m_Flags;
	S32					m_ActiveSplitter;
	wxPoint				m_ActiveSplitterPos;

	wxCursor	m_SashCursorWE;
	wxCursor	m_SashCursorNS;
	wxCursor	m_SashCursorSize;
	wxPen		*m_pSashTrackerPen;

	//splitter position clamping
	mutable	wxPoint m_Min;
	mutable wxPoint m_Max;

	static const S32 Active		= 0x00000001;
	static const S32 Horizontal = 0x00000002;
	static const S32 Vertical	= 0x00000004;


	DECLARE_EVENT_TABLE()
};


/*
TODO: Fire events when changing...
BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_EVENT_TYPE(EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGED, wxNewEventType())
	DECLARE_EVENT_TYPE(EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGING, wxNewEventType())
END_DECLARE_EVENT_TYPES()

class wxSplitterCtrlEvent : public wxNotifyEvent
{
	wxSplitterCtrlEvent(wxEventType type = wxEVT_NULL,
		wxSplitterCtrl *pSplitter = (wxSplitterCtrl*) NULL)
		: wxNotifyEvent(type)
	{
		SetEventObject(pSplitter);
		if (pSplitter) 
			m_id = pSplitter->GetId();
	}

	// SASH_POS_CHANGED methods
	// sash position is a point rather than an S32 since we can have up to 2 splits
	void SetSashPosition(const wxPoint& pt)
	{
		wxASSERT( GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGED
			|| GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGING);
		m_Point = pt;
	}

	wxPoint GetSashPosition() const
	{
		wxASSERT( GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGED
			|| GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGING);
		return m_Point;
	}

	// Sash index being altered
	void SetSashIndex(S32 idx)
	{
		wxASSERT( GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGED
			|| GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGING);
		m_Mode = idx;
	}

	S32 GetSashIdx(void)
	{
		wxASSERT( GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGED
			|| GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGING);		
		return m_SashIndex;
	}

	void SetMode(S32 m)
	{
		wxASSERT( GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGED
			|| GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGING);
		m_Mode = m;
	}

	S32 GetMode() const
	{
		wxASSERT( GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGED
			|| GetEventType() == EVT_COMMAND_SPLITTERCTRL_SASH_POINT_CHANGING);
		return m_Mode;
	}

private:
	wxPoint m_Point;	// position for SASH_POINT_CHANGED event
	S32 m_SashIndex;
	S32 m_Mode;			// wxHORIZONTAL, wxVERTICAL, wxBOTH
	DECLARE_DYNAMIC_CLASS_NO_COPY(wxSplitterCtrlEvent)

	friend class wxSplitterCtrl;
};
*/
#endif