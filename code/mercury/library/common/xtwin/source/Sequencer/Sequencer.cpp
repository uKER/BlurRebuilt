#include "Core/XtWinPch.h"
#include "SequencerImpl.h"

SequencerLayoutContext::SequencerLayoutContext() :
	width(0.0f),
	x(0.0f),
	y(0.0f)
{
}

Sequencer::Sequencer() :
	_isDirty(true),
	_scrollHeight(0.0f),
	_scrollOffset(0.0f)
{
	_root._sequencer	= this;
	_root._indent		= 0.0f;
	_root._height		= 0.0f;
	_root._gap			= 0.0f;
}

Sequencer::~Sequencer()
{
}

bool Sequencer::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN | WS_VSCROLL | WS_BORDER;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	_wndClassRedraw = true;

	return true;
}

LRESULT Sequencer::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
			if (ChildWindow::WndProc(msg, wParam, lParam) < 0)
				return -1;

			DoScrollBars();
		return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);

			{
				Gdiplus::Graphics g(ps.hdc);

				WinRect rcPaint = ps.rcPaint;

				Gdiplus::Bitmap bm(rcPaint.Width(), rcPaint.Height(), &g);

				{
					Gdiplus::Graphics g2(&bm);

					g2.Clear(Gdiplus::Color::DarkGray);

					g2.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
					g2.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
					g2.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

					g2.TranslateTransform(-(float)rcPaint.left, -(float)rcPaint.top);

					Render(&g2);
				}

				Gdiplus::CachedBitmap cbm(&bm, &g);

				g.DrawCachedBitmap(&cbm, rcPaint.left, rcPaint.top);
			}

			EndPaint(_hwnd, &ps);
		}
		return 0;

		// Mouse move

		case WM_MOUSEMOVE:
		return 0;

		// Left mouse button

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			SetFocus(*this);

			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if (SequencerItem* item = HitTest(pt))
				item->OnClick();
		}
		return 0;

		case WM_LBUTTONUP:
			SetFocus(*this);
		return 0;

		// Middle mouse button

		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONUP:
			SetFocus(*this);
		return 0;

		// Right mouse button

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONUP:
			SetFocus(*this);
		return 0;

		// Mouse wheel

		case WM_MOUSEWHEEL:
		{
			WinRect rc;
			GetWindowRect(*this, &rc);

			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if (PtInRect(&rc, pt))
			{
				int delta = (GET_WHEEL_DELTA_WPARAM(wParam) * 64) / WHEEL_DELTA;

				SCROLLINFO si = { 0 };

				si.cbSize	= sizeof(si);
				si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS;

				GetScrollInfo(*this, SB_VERT, &si);

				si.fMask = SIF_POS;
				si.nPos = Clamp(si.nPos - delta, si.nMin, si.nMax - (int)si.nPage + 1);

				SetScrollInfo(*this, SB_VERT, &si, TRUE);

				_scrollOffset = (float)si.nPos;

				//DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)).ScreenToClient(*this));

				InvalidateWindow(*this);

				return 0;
			}
		}
		break;

		// Scroll bars

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

			//_scrollX = (float)x;
			_scrollOffset = (float)y;

			InvalidateWindow(*this);
		}
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

SequencerItem* Sequencer::HitTest(const NgVector& pt)
{
	return _root.DoHitTest(pt + NgVector(0.0f, _scrollOffset));
}

void Sequencer::Layout()
{
	if (!_isDirty)
		return;

	_isDirty = false;

	WinRect rc(*this);

	SequencerLayoutContext ctx;

	ctx.width	= (float)rc.Width() - 2.0f;
	ctx.x		= 1.0f;
	ctx.y		= 1.0f;

	_root.DoLayout(&ctx);

	_scrollHeight = ctx.y;

	DoScrollBars();
}

void Sequencer::DoScrollBars()
{
	WinRect rc(*this);

	SCROLLINFO si = { sizeof(SCROLLINFO) };

	// Set ranges

	si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;

	si.nMin		= 0;
	si.nMax		= (int)_scrollHeight;
	si.nPage	= rc.Height();

	SetScrollInfo(*this, SB_VERT, &si, TRUE);

	// Get position

	si.fMask = SIF_POS;

	GetScrollInfo(*this, SB_VERT, &si);

	_scrollOffset = Clamp((float)si.nPos, 0.0f, _scrollHeight);
}

void Sequencer::Render(Gdiplus::Graphics* g)
{
	Layout();

	g->TranslateTransform(0.0f, -_scrollOffset);

	_root.DoRender(g);
}

void Sequencer::SetDirty(bool redoLayout)
{
	_isDirty = _isDirty || redoLayout;

	InvalidateWindow(*this);
}