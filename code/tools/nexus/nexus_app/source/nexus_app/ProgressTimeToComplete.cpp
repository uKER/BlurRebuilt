#include <nexus_app/stdafx.h>
#include <time.h>
#include <math.h>
#include <nexus_app/ProgressTimeToComplete.h>

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(TProgressTimeToComplete, CProgressCtrl)
    //{{AFX_MSG_MAP(TProgressTimeToComplete)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

TProgressTimeToComplete::TProgressTimeToComplete()
{
    LOGFONT logFount;

    m_lfPercent = 0.0;
    m_start     = clock();

    // create a horizontal fount
    m_fountHorz.CreatePointFont(90, "MS Sans", NULL);

    // create a vertical fount the same as the horizontal one
    m_fountHorz.GetLogFont(&logFount);
    logFount.lfEscapement  = 900;
    logFount.lfOrientation = 900;
    m_fountVert.CreateFontIndirect(&logFount);
}

TProgressTimeToComplete::~TProgressTimeToComplete()
{
}

void TProgressTimeToComplete::ResetStartTime(void)
{
    m_start = clock();
}

void TProgressTimeToComplete::OnPaint() 
{
    int      nRange, nLower, nUpper;
    CBrush   brush(::GetSysColor(COLOR_HIGHLIGHT));
    CPaintDC dcPaint(this);
    CMemDC   dc(&dcPaint);
    CRect    rcClient, rcBlock, rcComplete;
    LONG     lStyle;
    int      nGapWidth = 2;
    int      nLoop, nBlocks;
    clock_t  now = clock();
    CString  strRemaining;

    this->GetRange(nLower, nUpper);
    this->GetClientRect(&rcClient);
    rcBlock = rcClient;

    nRange      = nUpper - nLower;
    m_lfPercent = (double)this->GetPos() / (double)nRange;
    lStyle      = ::GetWindowLong(*this, GWL_STYLE);

    if ((lStyle & PBS_VERTICAL) == PBS_VERTICAL)
        rcBlock.top = rcBlock.bottom - (int)(rcBlock.Height() * m_lfPercent);
    else
        rcBlock.right = rcBlock.left + (int)(rcBlock.Width() * m_lfPercent);
    rcComplete = rcBlock;

    if ((lStyle & PBS_SMOOTH) == PBS_SMOOTH)
        nBlocks = 1;
    else
    {
        #define BLOCKS 18.0
        rcBlock = rcClient;
        if ((lStyle & PBS_VERTICAL) == PBS_VERTICAL)
            rcBlock.top = rcBlock.bottom - (int)((rcClient.Height()-(9*nGapWidth)) / BLOCKS);
        else
            rcBlock.right = rcBlock.left + (int)((rcClient.Width()-(9*nGapWidth)) / BLOCKS);
        nBlocks = (int)((m_lfPercent*BLOCKS)+0.9);
    }

    if (this->GetParent() != NULL)
        ::FillRect(dc, &rcClient, (HBRUSH)this->GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)(HDC)dc, (LPARAM)this->GetSafeHwnd()));

    for (nLoop=1; nLoop<=nBlocks; nLoop++)
    {
        ::FillRect(dc, &rcBlock, brush);
        if ((lStyle & PBS_VERTICAL) == PBS_VERTICAL)
        {
            rcComplete.top = rcBlock.top;
            rcBlock.OffsetRect(0, -(rcBlock.Height()+nGapWidth));
        }
        else
        {
            rcComplete.right = rcBlock.right;
            rcBlock.OffsetRect(rcBlock.Width()+nGapWidth, 0);
        }
    }

    if (m_lfPercent > 0.02)
    {
        CFont  *pFount;
        double  lfSecsSoFar = (double)(now - m_start) / CLOCKS_PER_SEC;
        strRemaining = this->GetRemainingText(m_lfPercent, (lfSecsSoFar / m_lfPercent) * (1.0 - m_lfPercent));
        dc.SetBkMode(TRANSPARENT);

        if ((lStyle & PBS_VERTICAL) == PBS_VERTICAL)
        {
            CSize  szExt;
            CPoint ptText;

            // draw the vertical text centred in the control
            pFount = dc.SelectObject(&m_fountVert);
            szExt  = dc.GetTextExtent(strRemaining);
            ptText.x = (rcClient.Width() - szExt.cy) >> 1;
            ptText.y = rcClient.bottom - ((rcClient.Height() - szExt.cx) >> 1);

            dc.SaveDC();
            dc.IntersectClipRect(rcComplete);
            dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
            dc.ExtTextOut(ptText.x, ptText.y, ETO_CLIPPED, rcClient, strRemaining, NULL);
            dc.RestoreDC(-1);

            dc.SaveDC();
            dc.ExcludeClipRect(rcComplete);
            dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
            dc.ExtTextOut(ptText.x, ptText.y, ETO_CLIPPED, rcClient, strRemaining, NULL);
            dc.RestoreDC(-1);

            dc.SelectObject(pFount);
        }
        else
        {
            // horizontal text is easier
            pFount = dc.SelectObject(&m_fountHorz);

            dc.SaveDC();
            dc.IntersectClipRect(rcComplete);
            dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
            dc.DrawText(strRemaining, rcClient, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_CENTER);
            dc.RestoreDC(-1);

            dc.SaveDC();
            dc.ExcludeClipRect(rcComplete);
            dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
            dc.DrawText(strRemaining, rcClient, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_CENTER);
            dc.RestoreDC(-1);

            dc.SelectObject(pFount);
        }
    }
}

BOOL TProgressTimeToComplete::OnEraseBkgnd(CDC* pDC) 
{
    return TRUE;
}

CString TProgressTimeToComplete::GetRemainingText(double lfPercent, double lfSecsRemaining)
{
    CString str;
    int     nSeconds = (int)fmod(lfSecsRemaining, 60.0);

    if (lfSecsRemaining < 60)
    {
        if (nSeconds < 1)
            str = "Less than a second";
        else
            str.Format("%d second%s remaining", nSeconds, nSeconds==1? "":"s");
    }
    else
    {
        int nMinutes = (int)(lfSecsRemaining/60.0);
        str.Format("%d minute%s, %d second%s remaining", nMinutes, nMinutes==1? "":"s",
                                                         nSeconds, nSeconds==1? "":"s");
    }

    return str;
}
