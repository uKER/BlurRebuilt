#include "stdafx.h"
#include "lux_app/CustomControls/DarkDialog.h"
#include "../../../resource.h"       // main symbols

// Create a green brush for the background for the class of controls:
CBrush CDarkDialog::m_brush(RGB(0,128,0));
CBrush CDarkDialog::m_darkbrush(RGB(57,67,69));
CBrush CDarkDialog::m_HollowBrush;

/*
BOOL CDarkDialog::OnChildNotify(UINT message, WPARAM wParam,
                            LPARAM lParam, LRESULT* pLResult)
{
// If "message" is not the message you're after, do default processing:
HDC hdcChild;

	switch (message)
	{
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		// Set the text foreground to red:
		hdcChild = (HDC)wParam;
		SetTextColor(hdcChild, RGB(0,0,255));

		// Set the text background to green:
		SetBkColor(hdcChild, RGB(0,128,0));

		// Send what would have been the return value of OnCtlColor() - the
		// brush handle - back in pLResult:
		*pLResult = (LRESULT)(m_brush.GetSafeHandle());
		return TRUE;
	default:
        return __super::OnChildNotify(message,wParam,lParam,pLResult);
	}

	// Return TRUE to indicate that the message was handled:
    return TRUE;
}
*/
BEGIN_MESSAGE_MAP(CDarkDialog, CDialog)
ON_WM_CTLCOLOR()
ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

HBRUSH CDarkDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// clear background
	HBRUSH brush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

//	if(m_bmpBackground.GetPixelPtr() != 0) {

		switch(nCtlColor)
		{
			case CTLCOLOR_STATIC:
			{
				switch (pWnd->GetDlgCtrlID())
				{
					// here is a list of controls that need their background being filled properly
				case IDC_PICKER_POLYNO:
				case IDC_PICKER_CHARTNO:
					pDC->SetTextColor(RGB(255,255,255));
					pDC->SetBkColor(RGB(57,67,69));
					return HBRUSH(m_darkbrush);

				default:
					// let static controls have the background shine through
					pDC->SetTextColor(RGB(255,255,255));
					pDC->SetBkMode(TRANSPARENT);
					return HBRUSH(m_HollowBrush);
				}
				break;

			case CTLCOLOR_EDIT:
				pDC->SetTextColor(RGB(255,255,255));
				pDC->SetBkColor(RGB(57,67,69));
				return HBRUSH(m_darkbrush);

			default:
				break;
			}
		}
	
	// if we reach this line, we haven't set a brush so far
	return brush;
}

// our dialog has a gradient for the background
BOOL CDarkDialog::OnEraseBkgnd(CDC* pDC) 
{
	CRect rc;
	GetClientRect(rc);
	int x = 0, y = 0;

	// draw the background fill
	pDC->FillSolidRect(rc.left+rc.Width()/2, rc.top, rc.Width()/2+1, rc.Height(), RGB(57,67,69));

	// draw the gradient
	RECT rect;

	GetClientRect(&rect);

	TRIVERTEX tv[] = { 
		{ rect.left, rect.top, 124*256,129*256,132*256 },
		{ rect.left+4, rect.bottom, 77*256,86*256,91*256 }
	};
	TRIVERTEX tv2[] = { 
		{ rect.left+4, rect.top, 77*256,86*256,91*256 },
		{ rect.right/2, rect.bottom, 57*256,67*256,69*256 }
	};
	GRADIENT_RECT gr = { 0, 1 };

    pDC->GradientFill ( tv, 2, &gr, 1, GRADIENT_FILL_RECT_H );
    pDC->GradientFill ( tv2, 2, &gr, 1, GRADIENT_FILL_RECT_H );

	return TRUE;
}







