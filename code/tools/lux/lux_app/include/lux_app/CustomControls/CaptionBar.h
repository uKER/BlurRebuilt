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
#if !defined(AFX_CAPTIONBAR_H__2454F2F9_A5D1_11D2_8F43_3078302C2030__INCLUDED_)
#define AFX_CAPTIONBAR_H__2454F2F9_A5D1_11D2_8F43_3078302C2030__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//***************************************************************************
// CCaptionBar window
//***************************************************************************
class CCaptionBar : public CButton
{
// Construction
public:
	CCaptionBar();

// Attributes
public:

	HICON		m_hIcon;					

	CFont		m_font;

	BOOL		m_bDoFontShadow;
	BOOL		m_bAutoSizeCaptionBar;

	int			m_nMaxBarHeight;
	
	int			m_nImageWidth;
	int			m_nImageHeight;

	int			m_nTextBorderLeft;
	int			m_nImageBorderRight;

	int			m_nTextShadowOffsetX;
	int			m_nTextShadowOffsetY;

	COLORREF	m_dwColorBorder;
	COLORREF	m_dwColorFace;

	COLORREF	m_dwColorFontFace;
	COLORREF	m_dwColorFontShadow;

// Operations
public:

	BOOL Create( LPCTSTR lpszCaption, 
				 DWORD dwStyle,
				 const RECT& rect, 
				 CWnd* pParentWnd, 
				 UINT nID );

	BOOL SetIcon( UINT nID, 
				  BOOL bBigIcon = TRUE );

	BOOL SetIcon( HICON hIcon, 
				  BOOL  bBigIcon = TRUE );

	BOOL SetFont( const LOGFONT& logfont, BOOL bRedraw = TRUE );

	inline void		SetCaptionHeight( int nHeight );
	inline int		GetCaptionHeight( void );

	inline int		GetCaptionFaceHeight( void );
	
	inline void		SetCaptionWidth( int nWidth );
	inline int		GetCaptionWidth( void );

	inline void	Redraw();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCaptionBar)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	
	//}}AFX_VIRTUAL

protected:

	virtual void InitBarColors( void );
	virtual int  SetOptimalBarHeight( void );

// Implementation
public:

	virtual ~CCaptionBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCaptionBar)
	afx_msg void OnSysColorChange();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT SetIcon( WPARAM wParam, LPARAM lParam );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//***************************************************************************
//***************************************************************************
//	                 I N L I N E    F U N C T I O N S 
//***************************************************************************
//***************************************************************************
inline void CCaptionBar::Redraw(void)
{
	if( m_hWnd != NULL )
		Invalidate();
}

//***************************************************************************
inline int CCaptionBar::GetCaptionHeight( void )
{
	CRect rectCaption;
	GetClientRect( rectCaption );

	return rectCaption.Height();
}

//***************************************************************************
//***************************************************************************
inline int CCaptionBar::GetCaptionFaceHeight( void )
{

	CRect rectCaptionFace;
	GetClientRect( rectCaptionFace );

	rectCaptionFace.DeflateRect( 2, 4 );

	return rectCaptionFace.Height();
}


//***************************************************************************
inline void	CCaptionBar::SetCaptionHeight( int nHeight )
{
    if( IsWindow(GetSafeHwnd()) )
       SetWindowPos( NULL, 0, 0, GetCaptionWidth(), nHeight, SWP_NOZORDER );

}

//***************************************************************************
inline void	CCaptionBar::SetCaptionWidth( int nWidth )
{
    if( IsWindow( GetSafeHwnd() ) )
       SetWindowPos( NULL, 0, 0, nWidth, GetCaptionHeight(), SWP_NOZORDER );
	   
}

//***************************************************************************
inline int CCaptionBar::GetCaptionWidth( void )
{
	CRect rectCaption;
	GetClientRect( rectCaption );

	return rectCaption.Width();
}

//***************************************************************************
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAPTIONBAR_H__2454F2F9_A5D1_11D2_8F43_3078302C2030__INCLUDED_)
