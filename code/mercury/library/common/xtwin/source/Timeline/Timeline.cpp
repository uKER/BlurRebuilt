#include "Core/XtWinPch.h"
#include "TimelineImpl.h"

//
// TimelineContext
//

TimelineContext::TimelineContext()
{
}

TimelineContext::~TimelineContext()
{
}

void TimelineContext::SelectionChanged()
{
}

//
// TimelineWnd
//

TimelineWnd::TimelineWnd(TimelineContext* context) :
	_diagram(new TlDiagram(this)),
	_context(context),
	_dragging(kDragNone),
	_dragCommitted(false),
	_panning(false),
	_adjusting(false)
{
}

TimelineWnd::~TimelineWnd()
{
	delete _diagram;
	delete _context;
}

bool TimelineWnd::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN | WS_VSCROLL;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	_wndClassRedraw = true;

	return true;
}

LRESULT TimelineWnd::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			if (ChildWindow::WndProc(msg, wParam, lParam) < 0)
				return -1;

			_diagram->SetClientRect(WinRect(*this));
			_diagram->Layout();
		}
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

					g2.Clear(Gdiplus::Color::White);

					g2.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
					g2.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
					g2.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

					g2.TranslateTransform(-(float)rcPaint.left, -(float)rcPaint.top);

					WinRect rcClient(*this);

					_diagram->Render(&g2);
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
			DoCursor();
		return 0;

		// Mouse activate

		case WM_MOUSEACTIVATE:
			SetFocus(*this);
		return MA_ACTIVATE;

		// Mouse wheel

		case WM_MOUSEWHEEL:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			ScreenToClient(*this, &pt);

			float factor = (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;

			if (factor < 0)
				factor = 1.0f / Clamp(1.0f + fabs(factor * 0.4f), 1.0f, 2.0f);
			else
				factor = Clamp(1.0f + fabs(factor * 0.4f), 1.0f, 2.0f);

			DoDrag(pt);

			_diagram->Zoom(pt, factor);
		}
		return 0;

		// Mouse movement

		case WM_MOUSEMOVE:
			DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		return 0;

		// Left mouse button

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			/*if (_dragging == kDragNone)
			{
				WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				DoSetCapture();

				_dragging		= kDragIgnore;
				_dragCommitted	= false;
				_dragPt			= pt;
				_dragStart		= pt;
				_dragTime		= GetTickCount();

				if (!_adjusting)
				{
					switch(_diagram->HitTest(pt))
					{
						case kTlHrHeader:
							_dragging = kDragHeader;
						break;

						case kTlHrRowLabel:
						{
							NgVector rowPt = _diagram->ToRowArea(pt);

							if (TlRow* row = _diagram->HitTestRow(rowPt))
							{
								_dragging = kDragRowLabel;

								if (row->CanExpand() && (row->HitTest(rowPt) == kTlHrRowExpand))
								{
									_dragging = kDragIgnore;

									row->SetExpanded(!row->IsExpanded());
								}
								else
								{
									if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
										_diagram->Select(row, kTlSelectRemove);
									else if ((GetKeyState(VK_SHIFT) & 0x8000))
										_diagram->Select(row, kTlSelectAdd);
									else if (GetKeyState(VK_CONTROL) & 0x8000)
										_diagram->Select(row, kTlSelectToggle);
									else
									{
										if ((_diagram->RowSelectionCount() == 1) || !row->IsSelected())
											_diagram->Select(row, kTlSelectSingle);
									}
								}
							}
							else
							{
								if (!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000))
									_diagram->ClearSelection();
							}
						}
						break;

						case kTlHrRowContent:
						{
							NgVector rowPt		= _diagram->ToRowArea(pt);
							NgVector contentPt	= _diagram->ToContentArea(pt);

							if (TlRow* row = _diagram->HitTestRow(rowPt))
							{
								_dragging = kDragRowContent;

								if (TlKey* key = row->HitTestKey(contentPt))
								{
									if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
										_diagram->Select(key, kTlSelectRemove);
									else if ((GetKeyState(VK_SHIFT) & 0x8000))
										_diagram->Select(key, kTlSelectAdd);
									else if (GetKeyState(VK_CONTROL) & 0x8000)
										_diagram->Select(key, kTlSelectToggle);
									else
									{
										if ((_diagram->KeySelectionCount() == 1) || !key->IsSelected())
											_diagram->Select(key, kTlSelectSingle);
									}

									if (key->IsSelected())
									{
										_dragging = kDragMoveKeys;

										_diagram->BeginMoveKeys();
									}
								}
								else
								{
									_dragging = kDragRectSelection;

									_diagram->RectSelectionBegin(contentPt);
								}
							}
							else
							{
								_dragging = kDragRectSelection;

								_diagram->RectSelectionBegin(contentPt);
							}
						}
						break;
					}
				}
			}*/
		return 0;

		case WM_LBUTTONUP:
		{
			// If drag start ~= drag end, and was clicking on row label, then single select row

			if (_dragging != kDragNone)
			{
				DragState drag = _dragging;

				DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

				_dragging = kDragNone;

				DoReleaseCapture();

				_diagram->EndMoveKeys();
				_diagram->RectSelectionEnd();

				if (drag == kDragRectSelection)
				{
					TlSelectMode mode = kTlSelectAdd;

					if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
						mode = kTlSelectRemove;
					else if ((GetKeyState(VK_SHIFT) & 0x8000))
						mode = kTlSelectAdd;
					else if (GetKeyState(VK_CONTROL) & 0x8000)
						mode = kTlSelectToggle;
					else
						_diagram->ClearSelection();

					_diagram->SelectRect(_diagram->RectSelection(), mode);
				}
			}
		}
		return 0;

		// Right mouse button

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			if (!_panning)
			{
				WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				DoSetCapture();

				_panning	= true;
				_panPt		= pt;
			}
		return 0;

		case WM_RBUTTONUP:
			if (_panning)
			{
				DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

				_panning = false;

				DoReleaseCapture();
			}
		return 0;

		// Other mouse buttons

		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			if (!_adjusting && (_dragging == kDragNone))
			{
				WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				DoSetCapture();

				_adjusting	= true;
				_adjustPt	= pt;

				DoCursor();
			}
		return 0;

		case WM_MBUTTONUP:
			if (_adjusting)
			{
				DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

				_adjusting = false;

				DoReleaseCapture();

				DoCursor();
			}
		return 0;

		// Capture

		case WM_CAPTURECHANGED:
			_dragging	= kDragNone;
			_panning	= false;
			_adjusting	= false;

			_diagram->EndMoveKeys();
			_diagram->RectSelectionEnd();

			DoCursor();
		break;

		// Context menu

		case WM_CONTEXTMENU:
		return 0;

		// Dialog

		case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

		// Size

		case WM_SIZE:
			_diagram->SetClientRect(WinRect(*this));
			_diagram->Layout();
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

			_diagram->SetScrollY((float)si.nPos);
		}
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void TimelineWnd::DoDrag(const WinPoint& pt)
{
	if (_panning)
	{
		WinPoint delta = pt - _panPt;

		_panPt = pt;

		_diagram->Pan(delta);
	}

	if (_adjusting)
	{
		if (!_panning)
			_diagram->MoveSelectedKeysBy((_diagram->ToContentArea(pt) - _diagram->ToContentArea(_adjustPt)) / NgVector(_diagram->ZoomX(), 1.0f));

		_adjustPt = pt;
	}

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
			case kDragHeader:
			break;

			case kDragRowLabel:
			break;

			case kDragRowContent:
			break;

			case kDragMoveKeys:
				if (!_panning)
				{
					_diagram->MoveSelectedKeysBy((_diagram->ToContentArea(pt) - _diagram->ToContentArea(_dragPt)) / NgVector(_diagram->ZoomX(), 1.0f));
				}
			break;

			case kDragRectSelection:
				_diagram->RectSelectionUpdate(_diagram->ToContentArea(pt));
			break;
		}

		_dragPt = pt;
	}
}

void TimelineWnd::DoSetCapture()
{
	if ((_dragging == kDragNone) && !_panning && !_adjusting)
		SetCapture(*this);
}

void TimelineWnd::DoReleaseCapture()
{
	if ((_dragging == kDragNone) && !_panning && !_adjusting)
		ReleaseCapture();
}

void TimelineWnd::DoScrollBars()
{
	WinRect rc(*this);

	SCROLLINFO si = { sizeof(SCROLLINFO) };

	// Set range

	si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;

	si.nMin		= 0;
	si.nMax		= (int)_diagram->TotalHeight();
	si.nPage	= Max(rc.Height() - (int)_diagram->HeaderY(), 1);

	SetScrollInfo(*this, SB_VERT, &si, TRUE);

	// Get position

	si.fMask = SIF_POS;

	GetScrollInfo(*this, SB_VERT, &si);

	//_scrollY = Clamp((float)si.nPos, 0.0f, _heightTotal);
}

void TimelineWnd::DoScrollBarPos()
{
	WinRect rc(*this);

	SCROLLINFO si = { sizeof(SCROLLINFO) };

	// Set pos

	si.fMask = SIF_POS;

	si.nPos = (int)_diagram->ScrollY();

	SetScrollInfo(*this, SB_VERT, &si, TRUE);
}

void TimelineWnd::DoCursor()
{
	if (_adjusting)
	{
		if (_diagram->KeySelectionCount() <= 0)
			SetCursor(LoadCursor(0, IDC_NO));
		else
			SetCursor(LoadCursor(0, IDC_SIZEWE));
	}
	else
		SetCursor(LoadCursor(0, IDC_ARROW));
}