#include "Core/XtWinPch.h"
#include "PropertySheetImpl.h"
#include "XtWin/Resource.h"

//
// PropertySheetDrawContext
//

PropertySheetDrawContext::PropertySheetDrawContext() :
	blackBrush(Gdiplus::Color::Black),
	whiteBrush(Gdiplus::Color::White),
	labelFont(Gdiplus::FontFamily::GenericSansSerif(), 8.0, Gdiplus::FontStyleRegular),
	labelPen(Gdiplus::Color::Black),
	labelBrush(Gdiplus::Color::Black),
	gridPen(Gdiplus::Color(0xFFE0E0E0)),
	gridBrush(Gdiplus::Color(0xFFE0E0E0)),
	selectedPen(Gdiplus::Color::Blue),
	selectedBrush(Gdiplus::Color::Blue),
	multiFont(Gdiplus::FontFamily::GenericSansSerif(), 8.0, Gdiplus::FontStyleItalic),
	multiPen(Gdiplus::Color::DarkGray),
	multiBrush(Gdiplus::Color::DarkGray)
{
}

//
// PropertySheet
//

PropertySheet::PropertySheet() :
	_diagram(this),
	_dragging(kDragNone),
	_dragTime(0),
	_dragCommitted(false)
{
}

PropertySheet::~PropertySheet()
{
}

bool PropertySheet::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN | WS_VSCROLL;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT | WS_EX_CLIENTEDGE;

	_wndClassRedraw = true;

	return true;
}

LRESULT PropertySheet::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		// Creation

		case WM_CREATE:
		break;

		case WM_DESTROY:
		break;

		// Painting

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

					g2.Clear(Gdiplus::Color::White);

					g2.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
					g2.SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
					g2.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
					g2.SetSmoothingMode(Gdiplus::SmoothingModeNone);
					g2.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

					g2.TranslateTransform(-(float)rcPaint.left, -(float)rcPaint.top);

					PropertySheetDrawContext ctx;

					_diagram.Render(&g2, &ctx);
				}

				Gdiplus::CachedBitmap cbm(&bm, &g);

				g.DrawCachedBitmap(&cbm, rcPaint.left, rcPaint.top);
			}

			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_ERASEBKGND:
		return 0;

		// Cursor

		case WM_SETCURSOR:
		{
			DWORD pos = GetMessagePos();

			DoCursor(WinPoint(GET_X_LPARAM(pos), GET_Y_LPARAM(pos)).ScreenToClient(*this));
		}
		return 0;

		// Mouse activate

		case WM_MOUSEACTIVATE:
			SetFocus(*this);
		return MA_ACTIVATE;

		// Mouse wheel

		case WM_MOUSEWHEEL:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		return 0;

		// Mouse movement

		case WM_MOUSEMOVE:
			DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			DoCursor(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		return 0;

		// Left mouse button

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			if (_dragging == kDragNone)
			{
				WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				DoSetCapture();

				_dragging		= kDragIgnore;
				_dragCommitted	= false;
				_dragPt			= pt;
				_dragStart		= pt;
				_dragTime		= GetTickCount();

				PsHitResult hit = _diagram.HitTest(_diagram.ToCanvas(pt));

				switch(hit.kind)
				{
					case PsHitResult::kExpander:
						if (hit.row && hit.row->CanExpand())
							hit.row->SetExpanded(!hit.row->IsExpanded());
					break;

					case PsHitResult::kLabel:
						if (msg == WM_LBUTTONDBLCLK)
						{
							if (hit.row && hit.row->CanExpand())
								hit.row->SetExpanded(!hit.row->IsExpanded());
						}
						else
							_dragging = kDragLabel;
					break;

					case PsHitResult::kSplitter:
						_dragging = kDragSplitter;
					break;

					case PsHitResult::kContent:
						if (hit.control)
							hit.control->Clicked();
					break;
				}

				DoCursor(pt);
			}
		return 0;

		case WM_LBUTTONUP:
		{
			if (_dragging != kDragNone)
			{
				DragState drag = _dragging;

				DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

				_dragging = kDragNone;

				DoReleaseCapture();
			}
		}
		return 0;

		// Capture

		case WM_CAPTURECHANGED:
		{
			_dragging = kDragNone;

			DWORD pos = GetMessagePos();

			DoCursor(WinPoint(GET_X_LPARAM(pos), GET_Y_LPARAM(pos)).ScreenToClient(*this));
		}
		break;

		// Dialog

		case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

		// Size

		case WM_SIZE:
		{
			WinRect rc(*this);

			_diagram._clientSize = rc.Size();

			_diagram.SetDirty(true);
		}
		break;

		// Scroll bars

		case WM_VSCROLL:
		{
			SCROLLINFO si = { 0 };

			si.cbSize	= sizeof(si);
			si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS | SIF_TRACKPOS;

			GetScrollInfo(*this, SB_VERT, &si);

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

			SetScrollInfo(*this, SB_VERT, &si, TRUE);

			Diagram()->SetScrollY(si.nPos);
		}
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void PropertySheet::DoDrag(const WinPoint& pt)
{
	if (!_dragCommitted)
	{
		WinPoint delta = _dragStart - pt;

		if ((GetTickCount() - _dragTime) < 250)
		{
			if ((abs(delta.x) < 4) && (abs(delta.y) < 4))
				return;
		}

		_dragCommitted = true;
	}

	if ((pt.x != _dragPt.x) || (pt.y != _dragPt.y))
	{
		switch(_dragging)
		{
			case kDragLabel:
			break;

			case kDragSplitter:
				_diagram._splitterX = Clamp<int>(pt.x, 16, Max(_diagram._clientSize.x - 16, 16));

				_diagram.SetDirty(true);
			break;
		}

		_dragPt = pt;
	}
}

void PropertySheet::DoSetCapture()
{
	if (_dragging == kDragNone)
		SetCapture(*this);
}

void PropertySheet::DoReleaseCapture()
{
	if (_dragging == kDragNone)
		ReleaseCapture();
}

void PropertySheet::DoCursor(const XwVector& pt)
{
	PsHitResult hit = _diagram.HitTest(_diagram.ToCanvas(pt));

	if ((_dragging == kDragSplitter) || (hit.kind == PsHitResult::kSplitter))
		SetCursor(LoadCursor(GetXtWinIconDll(), MAKEINTRESOURCE(IDC_XTWIN_SPLITH)));
	else
		SetCursor(LoadCursor(0, IDC_ARROW));
}

void PropertySheet::DoScrollBars()
{
	WinRect rc(*this);

	SCROLLINFO si = { sizeof(SCROLLINFO) };

	// Set range

	si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;

	si.nMin		= 0;
	si.nMax		= Diagram()->Root()->_bottom;
	si.nPage	= Max(rc.Height(), 1);

	SetScrollInfo(*this, SB_VERT, &si, TRUE);
}

void PropertySheet::DoScrollPos()
{
	WinRect rc(*this);

	SCROLLINFO si = { sizeof(SCROLLINFO) };

	// Set pos

	si.fMask	= SIF_POS;
	si.nPos		= _diagram.ScrollY();

	SetScrollInfo(*this, SB_VERT, &si, TRUE);
}
