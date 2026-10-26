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
#include "stdafx.h"
#include "CaptionView.h"
#include "CaptionBar.h"

#include "AfxPriv.h"
//#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//***************************************************************************
// CCaptionView
//***************************************************************************
IMPLEMENT_DYNCREATE(CCaptionView, CView)


//***************************************************************************
//***************************************************************************
BEGIN_MESSAGE_MAP(CCaptionView, CView)
	//{{AFX_MSG_MAP(CCaptionView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	// Standard printing commands
END_MESSAGE_MAP()


//***************************************************************************
// CCaptionView construction/destruction
//***************************************************************************
CCaptionView::CCaptionView() : 
m_pCaption(NULL),
m_pCtrl(NULL)
{
	//
	// The default constructor should never be called!
	//
	ASSERT( FALSE );

}

//***************************************************************************
//***************************************************************************
//  Derived classed pass in the RTTI of the window.
//***************************************************************************
CCaptionView::CCaptionView( CRuntimeClass* prtti ) :
m_pCaption(NULL),
m_pCtrl(NULL)
{
	m_pCaption = new (CCaptionBar);
	ASSERT_VALID( m_pCaption );

	m_pCtrl = STATIC_DOWNCAST( CWnd,  prtti->CreateObject() );
	ASSERT_VALID( m_pCtrl );
}


//***************************************************************************
//***************************************************************************
CCaptionView::~CCaptionView()
{
	if( m_pCaption )
	{
		delete m_pCaption;
		m_pCaption = NULL;
	}
}


//***************************************************************************
//***************************************************************************
BOOL CCaptionView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPCHILDREN;

	return CView::PreCreateWindow(cs);
}

//***************************************************************************
// CCaptionView drawing
//***************************************************************************
void CCaptionView::OnDraw(CDC* pDC)
{
	UNUSED_ALWAYS( pDC );

	// TODO: add draw code for native data here
}


//***************************************************************************
//
//***************************************************************************
int CCaptionView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	int nResult = 0;

	nResult = CView::OnCreate(lpCreateStruct);

	if( -1 == nResult )
		return nResult;

	nResult = m_pCaption->Create( NULL, 
							    WS_VISIBLE 
							   |WS_CHILD 
							   |BS_OWNERDRAW, 
							    CRect(0,0,0,0), 
							    this, 
								1 );

	if( -1 == nResult )
	{
		TRACE0(_T("FAILED TO CREATE CAPTION-BAR WINDOW!\n"));
		return nResult;
	}

	nResult = m_pCtrl->Create( NULL, 
				               NULL, 
							   WS_CHILD,
							   CRect(0,0,0,0), 
							   this, 
							   2 );

	m_pCtrl->SetOwner( this );
	m_pCaption->SetOwner( this );

	if( -1 == nResult )
	{
		TRACE0(_T("FAILED TO CREATE CUSTOM CONTROL WINDOW!\n"));
		return nResult;
	}

    return nResult;
}


//***************************************************************************
//
//***************************************************************************
void CCaptionView::OnSize(UINT nType, int cx, int cy) 
{
	//
	//  1 - Make sure windows are valid...
	//
	ASSERT( IsWindow( m_pCaption->GetSafeHwnd() ) );
	ASSERT( IsWindow( m_pCtrl->GetSafeHwnd()  ) );

	CRect rectCaption;
	m_pCaption->GetClientRect( rectCaption );

	HDWP dwp = BeginDeferWindowPos( 2 ); // number of windows

	if( NULL != dwp )
	{
		dwp = DeferWindowPos(  dwp,
							   m_pCaption->GetSafeHwnd(),
							   HWND_BOTTOM,
							   0,
							   0,
							   cx,
							   rectCaption.bottom,
							   SWP_NOZORDER );

		dwp = DeferWindowPos(  dwp,
							   m_pCtrl->GetSafeHwnd(),
							   HWND_BOTTOM,
							   0,
							   rectCaption.bottom,
							   cx,
							   cy - rectCaption.bottom,
							   SWP_NOZORDER );

		VERIFY
		( 
			EndDeferWindowPos( dwp ) 
		);
	}

	CView::OnSize(nType, cx, cy);
}


//***************************************************************************
//
//***************************************************************************
BOOL CCaptionView::OnEraseBkgnd(CDC* pDC) 
{
	UNUSED_ALWAYS(pDC);

	return TRUE;
}

//***************************************************************************
//***************************************************************************
LRESULT CCaptionView::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT lResult = 0;

	switch( message )
	{
	case WM_NOTIFY :
		{
			NMHDR*	pNMHDR = (NMHDR*)lParam;
			int		nCode  = pNMHDR->code;

			AFX_NOTIFY notify;

			notify.pResult = &lResult;
			notify.pNMHDR  = pNMHDR;

			CWnd::OnCmdMsg( 0, 
				            MAKELONG(nCode, WM_REFLECT_BASE+WM_NOTIFY), 
							&notify, 
							NULL );
		}
		break;

	case WM_SETTEXT :
		{
			lResult = m_pCaption->SendMessage( message, wParam, lParam );
		}
		break;

	case WM_SETICON :
		{
			lResult = m_pCaption->SendMessage( message, wParam, lParam );
		}

	default : 
		
		lResult = CView::DefWindowProc(message, wParam, lParam);

		break;
	}
	
	return lResult;
}

//***************************************************************************
//***************************************************************************
void CCaptionView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rectClient;
	GetClientRect( rectClient );

	dc.FillSolidRect( rectClient, GetSysColor(COLOR_APPWORKSPACE) );
	
	// Do not call CView::OnPaint() for painting messages
}


//***************************************************************************
// CCaptionView diagnostics
//***************************************************************************
#ifdef _DEBUG
void CCaptionView::AssertValid() const
{
	CView::AssertValid();
}
#endif //_DEBUG

//***************************************************************************
//***************************************************************************
#ifdef _DEBUG
void CCaptionView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

