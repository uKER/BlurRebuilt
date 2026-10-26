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
//***************************************************************************
// CaptionView.h : interface of the CCaptionView class
//***************************************************************************
//
//***************************************************************************
#if !defined(AFX_CAPTIONVIEW_H__2454F2F1_A5D1_11D2_8F43_3078302C2030__INCLUDED_)
#define AFX_CAPTIONVIEW_H__2454F2F1_A5D1_11D2_8F43_3078302C2030__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CaptionBar.h"

//***************************************************************************
//***************************************************************************
class CCaptionView : public CView
{

	CCaptionView();

protected: // create from serialization only

	CCaptionView( CRuntimeClass* prtti ); 

	DECLARE_DYNCREATE(CCaptionView)

// Attributes
public:

// Operations
public:

	inline CCaptionBar* GetCaptionBar( void );
	inline void GetCaptionRect( LPRECT lpRect ) ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCaptionView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCaptionView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCaptionView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
    CCaptionBar*	m_pCaption;
	CWnd*			m_pCtrl;
};


//***************************************************************************
//***************************************************************************
//			          I N L I N E   F U N C T I O N S 
//***************************************************************************
//***************************************************************************

//***************************************************************************
inline CCaptionBar* CCaptionView::GetCaptionBar( void )
{
	return m_pCaption; 
}

//***************************************************************************
//***************************************************************************
inline void CCaptionView::GetCaptionRect( LPRECT lpRect ) 
{
	CRect rectClient;
	CWnd::GetClientRect( lpRect );

	int height = m_pCaption->GetCaptionHeight();

	lpRect->top += height;
}



//***************************************************************************

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately 
// before the previous line.

#endif // !defined(AFX_CAPTIONVIEW_H__2454F2F1_A5D1_11D2_8F43_3078302C2030__INCLUDED_)
