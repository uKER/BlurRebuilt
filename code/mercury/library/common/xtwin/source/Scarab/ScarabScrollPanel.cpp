#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

ScarabScrollPanel::ScarabScrollPanel(ScarabForm* form) :
	_form(form),
	_panel(0)
{
}

ScarabScrollPanel::~ScarabScrollPanel()
{
}

void ScarabScrollPanel::SetPanel(ScarabPanel* panel)
{
	_panel = panel;

	CalculateScrollBar();
}

void ScarabScrollPanel::Refresh()
{
	CalculateScrollBar();
}

bool ScarabScrollPanel::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	return true;
}

LRESULT ScarabScrollPanel::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
			CalculateScrollBar();
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);

			ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, 0, 0, 0);

			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_SIZE:
			CalculateScrollBar();
		return 0;

		case WM_VSCROLL:
		case WM_HSCROLL:
		{
			SCROLLINFO si = { 0 };

			si.cbSize	= sizeof(si);
			si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS | SIF_TRACKPOS;

			GetScrollInfo(*this, (msg == WM_HSCROLL) ? SB_HORZ : SB_VERT, &si);

			int max = si.nMax - si.nPage + 1;

			switch(LOWORD(wParam))
			{
				case SB_LINEUP:			si.nPos = si.nPos - 50;				break;
				case SB_LINEDOWN:		si.nPos = si.nPos + 50;				break;
				case SB_PAGEUP:			si.nPos = si.nPos - (int)si.nPage;	break;
				case SB_PAGEDOWN:		si.nPos = si.nPos + (int)si.nPage;	break;
				case SB_THUMBPOSITION:	si.nPos = si.nTrackPos;				break;
				case SB_THUMBTRACK:		si.nPos = si.nTrackPos;				break;
				case SB_TOP:			si.nPos = si.nMin;					break;
				case SB_BOTTOM:			si.nPos = max;						break;
			}

			si.fMask = SIF_POS;
			si.nPos = Clamp(si.nPos, si.nMin, max);

			SetScrollInfo(*this, (msg == WM_HSCROLL) ? SB_HORZ : SB_VERT, &si, TRUE);

			SCROLLINFO si2 = { 0 };

			si2.cbSize	= sizeof(si2);
			si2.fMask	= SIF_POS;

			GetScrollInfo(*this, (msg == WM_HSCROLL) ? SB_VERT : SB_HORZ, &si2);

			int x = (msg == WM_HSCROLL) ? si.nPos : si2.nPos;
			int y = (msg == WM_HSCROLL) ? si2.nPos : si.nPos;

			if (_panel && *_panel)
				SetWindowPos(*_panel, 0, -x, -y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
		}
		return 0;

		case WM_NCHITTEST:
		{
			LRESULT result = ChildWindow::WndProc(msg, wParam, lParam);

			if (result == HTCLIENT)
				return HTTRANSPARENT;

			return result;
		}
		break;

		case WM_MOUSEWHEEL:
			DoMouseWheel(wParam);
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void ScarabScrollPanel::CalculateScrollBar()
{
	SCROLLINFO siv = { 0 };
	SCROLLINFO sih = { 0 };

	siv.cbSize	= sizeof(siv);
	siv.fMask	= SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;

	sih.cbSize	= sizeof(sih);
	sih.fMask	= SIF_PAGE | SIF_RANGE;// | SIF_DISABLENOSCROLL;

	if (_panel && *_panel)
	{
		WinRect rcClient(*this);
		WinRect rcForm(*_panel);
		
		siv.nMin	= 0;
		siv.nMax	= rcForm.Height();
		siv.nPage	= rcClient.Height();

		sih.nMin	= 0;
		sih.nMax	= rcForm.Width();
		sih.nPage	= rcClient.Width();
	}
	else
	{
		siv.nMin	= 0;
		siv.nMax	= 0;
		siv.nPage	= 0;

		sih.nMin	= 0;
		sih.nMax	= 0;
		sih.nPage	= 0;
	}

	if (sih.nPage <= 0)
		sih.nMin = sih.nMax = sih.nPage = 0;

	if (siv.nPage <= 0)
		siv.nMin = siv.nMax = siv.nPage = 0;

	SetScrollInfo(*this, SB_VERT, &siv, TRUE);
	SetScrollInfo(*this, SB_HORZ, &sih, TRUE);

	if (_panel)
	{
		siv.fMask = SIF_POS;
		sih.fMask = SIF_POS;

		GetScrollInfo(*this, SB_VERT, &siv);
		GetScrollInfo(*this, SB_HORZ, &sih);

		SetWindowPos(*_panel, 0, -sih.nPos, -siv.nPos, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	}
}

void ScarabScrollPanel::DoMouseWheel(WPARAM wParam)
{
	int delta = (GET_WHEEL_DELTA_WPARAM(wParam) * 64) / WHEEL_DELTA;

	SCROLLINFO si = { 0 };

	si.cbSize	= sizeof(si);
	si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS;

	GetScrollInfo(*this, SB_VERT, &si);

	si.fMask = SIF_POS;
	si.nPos = Clamp(si.nPos - delta, si.nMin, si.nMax - (int)si.nPage + 1);

	SetScrollInfo(*this, SB_VERT, &si, TRUE);

	SCROLLINFO sih = { 0 };
	sih.cbSize	= sizeof(sih);
	sih.fMask	= SIF_POS;
	GetScrollInfo(*this, SB_HORZ, &sih);

	if (_panel && *_panel)
		SetWindowPos(*_panel, 0, -sih.nPos, -si.nPos, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}