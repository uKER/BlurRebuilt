#include "stdafx.h"
#include "lux_app/CustomControls\fixedwidthtoolbar.h"


/////////////////////////////////////////////////////////////////////////////
// CFixedWidthToolbar

IMPLEMENT_DYNAMIC(CFixedWidthToolbar, CExtControlBar)

CFixedWidthToolbar::CFixedWidthToolbar()
{
}

CFixedWidthToolbar::~CFixedWidthToolbar()
{
}
/*
CSize CFixedWidthToolbar::OnQueryFixedBarNewSize(
	CExtPanelControlBar::e_fixed_bar_size_query_t eWhichSizeNeedToKnow
	) const
{
	ASSERT_VALID( this );
	switch( eWhichSizeNeedToKnow )
	{
	case CExtPanelControlBar::e_fixed_bar_size_query_t::__FSQ_DOCKED_H:
		return CSize(150,50);
	case CExtPanelControlBar::e_fixed_bar_size_query_t::__FSQ_DOCKED_V:
		return CSize(50,150);
	default:
		return CSize(150,150);
	} // switch( eWhichSizeNeedToKnow )
}
*/
BEGIN_MESSAGE_MAP(CFixedWidthToolbar, CExtControlBar)
	//{{AFX_MSG_MAP(CFixedWidthToolbar)
	//}}AFX_MSG_MAP
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
//LRESULT CFixedWidthToolbar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
//{
//	if (message == WM_PAINT)
//	{
//		CRect rect;
//		GetWindowRect(rect);
//		if (rect.Width() != 400)
//		{
//			rect.right = rect.left + 400;
//			MoveWindow(rect);
//		}
//	}
//
//	return __super::WindowProc(message, wParam, lParam);
//}

void CFixedWidthToolbar::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CExtControlBar::OnWindowPosChanging(lpwndpos);

//	lpwndpos->flags |= SWP_NOSIZE;
//	lpwndpos->cx = 400;
}

CSize CFixedWidthToolbar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize size;
	size.cx = 300;//(bStretch && bHorz ? 32767 : 0);
	size.cy = (bStretch && !bHorz ? 32767 : 0);
	return size;
}
