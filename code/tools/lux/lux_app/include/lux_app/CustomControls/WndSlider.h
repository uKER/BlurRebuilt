//***************************************************************************
// Copyright 1999                                  
//***************************************************************************
// @Author: Christopher Brown
// @E-Mail: AllKnowing@EarthLink.net
// @ICQ:	875098
// @WEB:	http://home.earthlink.net/~allknowing
//
// @Cedits:
//
//	  Giving credit where credit is due...  Thanks for the help!!!
//    -----------------------------------------------------------
//		  Iuri Apollonio - warch@dnet.it
//		  Michael Dunn   - mdunn@inreach.com
//		  Dave Buches	 - ??
//
//		  Anyone else?
//		  Sorry if I forgot to include your name... 
//    -----------------------------------------------------------
//
// @Blah, blah, blah...
//
// THIS CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
// You may use and distribute this code provided that you do not
// remove this title header and that you do not charge money for it.  
//
//***************************************************************************
#if !defined(AFX_WNDSLIDER_H__90B1F5F3_B5AC_11D2_877B_0000C042C40D__INCLUDED_)
#define AFX_WNDSLIDER_H__90B1F5F3_B5AC_11D2_877B_0000C042C40D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//***************************************************************************
// WndSlider.h : header file
//***************************************************************************

#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define SOB_ERROR			0xffffffff // == -1

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
const int SOB_BAR_DEFAULT_FIRST_VIEW_ID	 = 0xDEAD;

/////////////////////////////////////////////////////////////////////////////
// Increase or decrease value to adjust button height.
/////////////////////////////////////////////////////////////////////////////
const int SOB_BAR_DEFAULT_BTN_HEIGHT	 = 19;

/////////////////////////////////////////////////////////////////////////////
// Increase this value to slow down the animation.  
/////////////////////////////////////////////////////////////////////////////
const int SOB_BAR_DEFAULT_ANIMATION_SPEED =	10;

/////////////////////////////////////////////////////////////////////////////
// Increase or decrease this value to to limit the amount of time spent 
// animating...
/////////////////////////////////////////////////////////////////////////////
const int SOB_BAR_DEFAULT_ANIMATION_LIMIT =	200;


/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
const DWORD SOB_VIEW_HORZ		=	0x00000001;
const DWORD SOB_VIEW_VERT		=	0x00000002;
const DWORD SOB_VIEW_ANIMATE	=	0x00000004;

const DWORD SOB_BTN_ANIMATE		=	0x00001000;
const DWORD SOB_BTN_FANCYFACE	=   0x00002000;


/////////////////////////////////////////////////////////////////////////////
// Constance used by CWndSliderView & CSliderButton.  
/////////////////////////////////////////////////////////////////////////////
typedef const enum tagButtonState
{ 
	SOB_BTN_UP   = 0x00000010,
	SOB_BTN_HIT  = 0x00000020, 
	SOB_BTN_OVER = 0x00000040 

} BUTTON_STATE, * LPBUTTON_STATE;

//***************************************************************************
//
//***************************************************************************
class CSliderButton : public CObject
{
	DWORD		m_dwButtonStyle;

	CWnd*		m_pWndParent;
	CWnd		*m_pWndContents;

	CString		m_strButtonTitle;

	BOOL		m_bUseFancyFaceButton;
	BOOL		m_bUpdateBitmaps;

	CRect		m_rectButton;

	CBitmap		m_bmpNormalFace;
	CBitmap		m_bmpPressedFace;
	int			m_idView;

public:

	CSliderButton();
	~CSliderButton(); 

	CWnd* GetParentWnd( void );
	void SetParentWnd( CWnd* pWnd );
	  
	DWORD GetButtonStyle( void ) const;
	void  ModifyButtonStyle( const DWORD dwRemove, const DWORD dwAdd );

	void   SetButtonTitle( LPCSTR szcTitle );
	LPCSTR GetButtonTitle( void ) const;

protected:

	friend class CWndSliderView;

	void Draw( CDC* pDC, const CRect& rectScreen, const DWORD dwFlags = SS_CENTER );
	void Size( const CRect& rectNewSize, const DWORD dwStyle );

	void UpdateBitmaps( CDC* pDC, DWORD dwFlags );
	void OnSysColorChange( void );
};

//***************************************************************************
typedef CArray< CSliderButton*, CSliderButton* > CSliderButtonArray; 


//***************************************************************************
// The main CWndSliderView window
//***************************************************************************
//
//
//***************************************************************************
class CWndSliderView : public CWnd
{
protected:

	DECLARE_DYNCREATE(CWndSliderView)

// Construction
public:
	CWndSliderView();

// Attributes
public:

	int				m_nButtonHeight;

protected:

	CSliderButtonArray  m_buttonArray;

	DWORD			m_dwViewStyle;

	int				m_nViewCount;
	int				m_nActiveView;

	int				m_nMouseOnButton;
	BOOL			m_bLButtonDown;

	BOOL			m_bTrackLeave;
	HCURSOR			m_hHandCursor;

	static int		m_idViewCounter;

private:

	int				m_nPriviousButton;
	BOOL			m_bTrackingMouse;

// Operations
public:

	int CreateView( LPCSTR szcViewTitle,
		            CRuntimeClass* pViewClass, 
				    CCreateContext* pContext,
				    const DWORD dwStyle = WS_CHILD|WS_VISIBLE );

	// for creating a view containing a dialog box
	int		InsertPage(const char* caption, UINT nIDTemplate);
	int		InsertPage(const char* caption, UINT nIDTemplate, CRuntimeClass* rtc);
	int		InsertPage(const char* caption, CDialog* wndtemplate, UINT nIDTemplate);
	int		_InsertPage(LPCSTR szcViewTitle, CDialog* pwndTemplate, int idx, BOOL bAutoDestroyTpl);


	void RecalcLayout( const BOOL bRepaint = TRUE );
	
	int	  GetViewID( const int nID ) const;
	CWnd* GetView( const int nView ) const;
	BOOL  RemoveView( const int nView );

	int	  GetViewCount() const;
	void  FixButtons();

	DWORD GetBarStyle( void ) const;
	void  ModifyViewStyle( const DWORD dwRemove, const DWORD dwAdd, const BOOL bRedraw );

	int	  ButtonHitTest( const CPoint& point );

	CSliderButton* GetButton( const int nButton ) const;

	int	  GetActiveView( void ) const;
	int   SetActiveView( const int nNewView, const BOOL bRedraw = TRUE );

	CRect GetButtonRect( const int nButton, const CRect& rectClient ) const;
	CRect GetButtonRect( const int nButton ) const;

	CRect GetInsideRect( const int nButton ) const;

	void  InvalidateButton( const int nButton );

protected:

	void ViewAnimation( const int nNewView, const int nPreviousView );
	void OnPrintClient( HDC hDC, UINT uFlags );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndSliderView)
	public:
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWndSliderView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWndSliderView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//***************************************************************************


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDSLIDER_H__90B1F5F3_B5AC_11D2_877B_0000C042C40D__INCLUDED_)
