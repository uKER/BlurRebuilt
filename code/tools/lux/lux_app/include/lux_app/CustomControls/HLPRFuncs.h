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
#if !defined(HLPRFUNCS_H_INCLUDED_)
#define HLPRFUNCS_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//***************************************************************************
struct CLogFont : public LOGFONT
{
public:
	CLogFont() { ZeroMemory( this, sizeof(LOGFONT) ); }
};

//***************************************************************************
//
//
//***************************************************************************
BOOL DrawVerticalText
( 
	CDC* pDC, 
	const CRect& rectWindow, 
	LPCSTR szcText, 
	const DWORD dwStyle 
);

//***************************************************************************
//
//
//***************************************************************************
void GradientFill
( 
	CDC* pDC, 
	const CRect& rectWindow,
	const COLORREF crFace,
	const COLORREF crShadow,
	const BOOL bHorizontal 
);

//***************************************************************************
//
//
//***************************************************************************
void GetWindowBitmap( CWnd* pWnd, CBitmap* bmpWindow );


//***************************************************************************
//
//
//***************************************************************************
BYTE TransitionColor
( 
	const BYTE bySource, 
	const BYTE byDestination, 
	const int nIndex, 
	const float nDelta 
);


#endif // HLPRFUNCS_H_INCLUDED_




