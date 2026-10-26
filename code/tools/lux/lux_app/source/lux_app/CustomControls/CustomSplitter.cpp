#include "stdafx.h"
#include "customsplitter.h"

IMPLEMENT_DYNAMIC(CCustomSplitterWnd, CSplitterWnd)

BEGIN_MESSAGE_MAP(CCustomSplitterWnd, CSplitterWnd)
    ON_WM_NCHITTEST()
END_MESSAGE_MAP()

// CSplitOverride message handlers
afx_msg UINT CCustomSplitterWnd::OnNcHitTest(CPoint /*point*/)
{
    return HTNOWHERE;
}

void CCustomSplitterWnd::RecalcLayout()
{
	RECT rc;
	int w,h;

	GetWindowRect(&rc);
	w = rc.right - rc.left;
	h = rc.bottom - rc.top;

	switch (m_iLayoutMode)
	{
	case LAYOUT_MODE_SINGLE:
		SetNoSplitterGap(true);

		SetRowInfo( 0, 0, 0 );
		SetColumnInfo( 0, 0, 0 );
		SetColumnInfo( 1, 0, 0 );

		break;
	case LAYOUT_MODE_QUAD:
		SetNoSplitterGap(false);

		SetRowInfo( 0, h/2, 0 );
		SetColumnInfo( 0, w/2, 0 );
		SetColumnInfo( 1, 0, 0 );
		break;
	}

	__super::RecalcLayout();
}