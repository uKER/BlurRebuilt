#include "Core/XtWinPch.h"
#include "GridViewImpl.h"
#include "XtWin/Resource.h"

//
// GridViewDrawContext
//

GridViewDrawContext::GridViewDrawContext() :
	labelFont(Gdiplus::FontFamily::GenericSansSerif(), 10.0f, Gdiplus::FontStyleRegular),
	blackBrush(Gdiplus::Color::Black),
	headerBgBrush(Gdiplus::Color(228, 236, 247)),
	headerLinePen(Gdiplus::Color(158, 182, 206)),
	headerSelectedBgBrush(Gdiplus::Color(255, 213, 141)),
	headerSelectedLinePen(Gdiplus::Color(242, 149, 54)),
	cellLinePen(Gdiplus::Color(208, 215, 229)),
	cellSelectedBgBrush(Gdiplus::Color(234, 236, 245)),
	cellSelectedLinePen(Gdiplus::Color(0, 0, 0), 3.0f),
	focusPen(Gdiplus::Color(54, 149, 242), 1.0f)
{
	float p0[] = { 2.0f, 2.0f };

	focusPen.SetDashPattern(p0, 2);
}

//
// GridViewContext
//

GridViewContext::GridViewContext() :
	_grid(0)
{
}

GridViewContext::~GridViewContext()
{
}

void GridViewContext::ContextMenu(const WinPoint& pt)
{
}

//
// GridView
//

GridView::GridView() :
	_context(0),
	_widthHeader(150.0f),
	_widthTotal(0.0f),
	_heightHeader(17.0f * 2.0f),
	_heightTotal(0.0f),
	_scrollX(0.0f),
	_scrollY(0.0f),
	_selectX0(0),
	_selectX1(0),
	_selectY0(0),
	_selectY1(0),
	_focusX(-1),
	_focusY(-1),
	_dragState(kDragNone),
	_dragTime(0),
	_dragCommitted(false),
	_dragColumn(0),
	_panning(false),
	_panTime(0),
	_panCommitted(false)
{
}

GridView::~GridView()
{
	delete _context;
}

void GridView::SetContext(GridViewContext* context)
{
	delete _context;

	if ((_context = context) != 0)
		_context->_grid = this;
}

void GridView::Clear()
{
	XtUt::Free(&_columns);
	XtUt::Free(&_rows);
}

int GridView::ColumnCount()
{
	return Count(&_columns);
}

GridColumn* GridView::ColumnAt(int x)
{
	return At(&_columns, x);
}

GridColumn* GridView::ColumnAppend(GridColumn* column)
{
	if (column->_view)
		return 0;

	if (!XtUt::AppendEx(&_columns, column))
		return 0;

	column->_view = this;

	return column;
}

GridColumn* GridView::ColumnInsert(int before, GridColumn* column)
{
	if (column->_view)
		return 0;

	if (!XtUt::InsertEx(&_columns, before, column))
		return 0;

	column->_view = this;

	return column;
}

void GridView::ColumnRemove(int x)
{
	XtUt::Remove(&_columns, x);
}

int GridView::RowCount()
{
	return Count(&_rows);
}

GridRow* GridView::RowAt(int y)
{
	return At(&_rows, y);
}

GridRow* GridView::RowAppend(GridRow* row)
{
	if (row->_view)
		return 0;

	if (!XtUt::AppendEx(&_rows, row))
		return 0;

	row->_view = this;

	return row;
}

GridRow* GridView::RowInsert(int before, GridRow* row)
{
	if (row->_view)
		return 0;

	if (!XtUt::InsertEx(&_rows, before, row))
		return 0;

	row->_view = this;

	return row;
}

void GridView::RowRemove(int y)
{
	XtUt::Remove(&_rows, y);
}

void GridView::SetColumnHeaderWidth(float width)
{
	_widthHeader = width;

	InvalidateWindow(*this);
}

void GridView::SetRowHeaderHeight(float height)
{
	_heightHeader = height;

	InvalidateWindow(*this);
}

void GridView::EnsureVisibleColumn(int x)
{
	if ((_columns.count <= 0) || (_rows.count <= 0))
		return;

	GridColumn*	col = _columns[Clamp(x, 0, (int)_columns.count - 1)];

	DoLayout();

	WinRect rc(*this);

	if (col->_cx < _widthHeader)
	{
		SCROLLINFO si = { 0 };

		si.cbSize	= sizeof(si);
		si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS;

		GetScrollInfo(*this, SB_HORZ, &si);

		si.fMask = SIF_POS;
		si.nPos = Clamp((int)(col->_x - _widthHeader), si.nMin, si.nMax - (int)si.nPage + 1);

		SetScrollInfo(*this, SB_HORZ, &si, TRUE);

		InvalidateWindow(*this);
	}
	else if ((col->_cx + col->_width) > rc.Width())
	{
		SCROLLINFO si = { 0 };

		si.cbSize	= sizeof(si);
		si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS;

		GetScrollInfo(*this, SB_HORZ, &si);

		si.fMask = SIF_POS;
		si.nPos = Clamp((int)(col->_x + col->_width - rc.Width() + 1.0f), si.nMin, si.nMax - (int)si.nPage + 1);

		SetScrollInfo(*this, SB_HORZ, &si, TRUE);

		InvalidateWindow(*this);
	}
}

void GridView::EnsureVisibleRow(int y)
{
	if ((_columns.count <= 0) || (_rows.count <= 0))
		return;

	GridRow*	row = _rows[Clamp(y, 0, (int)_rows.count - 1)];

	DoLayout();

	WinRect rc(*this);

	if (row->_cy < _heightHeader)
	{
		SCROLLINFO si = { 0 };

		si.cbSize	= sizeof(si);
		si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS;

		GetScrollInfo(*this, SB_VERT, &si);

		si.fMask = SIF_POS;
		si.nPos = Clamp((int)(row->_y - _heightHeader), si.nMin, si.nMax - (int)si.nPage + 1);

		SetScrollInfo(*this, SB_VERT, &si, TRUE);

		InvalidateWindow(*this);
	}
	else if ((row->_cy + row->_height) > rc.Height())
	{
		SCROLLINFO si = { 0 };

		si.cbSize	= sizeof(si);
		si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS;

		GetScrollInfo(*this, SB_VERT, &si);

		si.fMask = SIF_POS;
		si.nPos = Clamp((int)(row->_y + row->_height - rc.Height() + 1.0f), si.nMin, si.nMax - (int)si.nPage + 1);

		SetScrollInfo(*this, SB_VERT, &si, TRUE);

		InvalidateWindow(*this);
	}
}

void GridView::EnsureVisible(int x, int y)
{
	EnsureVisibleColumn(x);
	EnsureVisibleRow(y);
}

bool GridView::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	_wndClassRedraw = true;

	return true;
}

LRESULT GridView::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
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

					g2.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
					g2.SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
					g2.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

					g2.TranslateTransform(-(float)rcPaint.left, -(float)rcPaint.top);

					OnPaint(&g2);
				}

				Gdiplus::CachedBitmap cbm(&bm, &g);

				g.DrawCachedBitmap(&cbm, rcPaint.left, rcPaint.top);
			}

			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_ERASEBKGND:
		return 0;

		// Mouse movement

		case WM_MOUSEMOVE:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			DoDrag(pt);
			DoPan(pt);
		}
		return 0;

		case WM_SETCURSOR:
			DoCursor();
		return 0;

		// Left mouse button

		case WM_LBUTTONDOWN:
		{
			SetFocus(*this);

			if (_dragState == kDragNone)
			{
				WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				DoSetCapture();

				GridHitResult result = HitTest(pt);

				switch(result.kind)
				{
					case GridHitResult::kColumnHeader:
						_dragState = kDragSelectColumns;

						DoLayout();

						_selectX0 = _selectX1 = Clamp(result.column, 0, Max((int)_columns.count - 1, 0));
						_selectY0 = 0;
						_selectY1 = Max((int)_rows.count - 1, 0);

						InvalidateWindow(*this);
					break;

					case GridHitResult::kColumnSplitter:
						_dragState	= kDragColumnSplitter;
						_dragColumn	= result.column;

						InvalidateWindow(*this);
					break;

					case GridHitResult::kRowHeader:
						_dragState = kDragSelectRows;

						DoLayout();

						_selectX0 = 0;
						_selectX1 = Max((int)_columns.count - 1, 0);
						_selectY0 = _selectY1 = Clamp(result.row, 0, Max((int)_rows.count - 1, 0));

						InvalidateWindow(*this);
					break;

					case GridHitResult::kRowEdge:
						_dragState = kDragRowEdge;

						InvalidateWindow(*this);
					break;

					case GridHitResult::kCells:
					{
						bool control = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

						_dragState = control ? kDragIgnore : kDragSelectCells;

						DoLayout();

						_focusX = result.column;
						_focusY = result.row;

						if (!control)
						{
							_selectX0 = _selectX1 = _focusX;
							_selectY0 = _selectY1 = _focusY;
						}

						InvalidateWindow(*this);
					}
					break;

					default:
						_dragState = kDragIgnore;
					break;
				}

				_dragStart		= pt;
				_dragLast		= pt;
				_dragTime		= GetTickCount();
				_dragCommitted	= false;

				DoCursor();
			}
		}
		return 0;

		case WM_LBUTTONDBLCLK:
		{
			SetFocus(*this);

			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			_focusX = Clamp(HitTestColumn(pt.x), 0, Max((int)_columns.count - 1, 0));
			_focusY = Clamp(HitTestRow(pt.y), 0, Max((int)_rows.count - 1, 0));

			if (GridColumn* col = At(&_columns, _focusX))
			{
				if (GridRow* row = At(&_rows, _focusY))
				{
					row->DoubleClicked(col);
				}
			}
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (_dragState != kDragNone)
			{
				DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

				_dragState = kDragNone;

				DoReleaseCapture();

				DoCursor();
			}
		}
		return 0;

		// Right mouse button

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			SetFocus(*this);

			DoSetCapture();

			_panning		= true;
			_panStart		= pt;
			_panLast		= _panStart;
			_panCommitted	= false;
			_panTime		= GetTickCount();

			DoCursor();
		}
		return 0;

		case WM_RBUTTONUP:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if (_panning)
				DoPan(pt);

			_panning = false;

			DoReleaseCapture();

			if (!_panCommitted)
			{
				_focusX = Clamp(HitTestColumn(pt.x), 0, Max((int)_columns.count - 1, 0));
				_focusY = Clamp(HitTestRow(pt.y), 0, Max((int)_rows.count - 1, 0));

				InvalidateWindow(*this);

				ClientToScreen(*this, &pt);

				PostMessage(*this, WM_CONTEXTMENU, (WPARAM)(HWND)*this, MAKELONG(pt.x, pt.y));
			}

			InvalidateWindow(*this);
		}
		return 0;

		// Other mouse buttons

		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			SetFocus(*this);
		return 0;

		case WM_MBUTTONUP:
		return 0;

		// Context menu

		case WM_CONTEXTMENU:
			if (_context)
			{
				WinPoint pt;
				GetCursorPos(&pt);

				_context->ContextMenu(pt);
			}
		return 0;

		// Lost capture

		case WM_CAPTURECHANGED:
			_dragState	= kDragNone;
			_panning	= false;
		return 0;

		// Mouse wheel

		case WM_MOUSEWHEEL:
		{
			int delta = (GET_WHEEL_DELTA_WPARAM(wParam) * 64) / WHEEL_DELTA;

			SCROLLINFO si = { 0 };

			si.cbSize	= sizeof(si);
			si.fMask	= SIF_PAGE | SIF_RANGE | SIF_POS;

			GetScrollInfo(*this, SB_VERT, &si);

			si.fMask = SIF_POS;
			si.nPos = Clamp(si.nPos - delta, si.nMin, si.nMax - (int)si.nPage + 1);

			SetScrollInfo(*this, SB_VERT, &si, TRUE);

			DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)).ScreenToClient(*this));

			InvalidateWindow(*this);
		}
		return 0;

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

			_scrollX = (float)x;
			_scrollY = (float)y;

			InvalidateWindow(*this);
		}
		return 0;

		// Keys

		case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

		case WM_KEYDOWN:
			if ((_rows.count > 0) && (_columns.count > 0))
			{
				switch(wParam)
				{
					case VK_LEFT:
						_focusX = Clamp<int>(_focusX - 1, 0, _columns.count - 1);
						InvalidateWindow(*this);
					break;

					case VK_RIGHT:
						_focusX = Clamp<int>(_focusX + 1, 0, _columns.count - 1);
						InvalidateWindow(*this);
					break;

					case VK_UP:
						_focusY = Clamp<int>(_focusY - 1, 0, _rows.count - 1);
						InvalidateWindow(*this);
					break;

					case VK_DOWN:
						_focusY = Clamp<int>(_focusY + 1, 0, _rows.count - 1);
						InvalidateWindow(*this);
					break;

					case VK_PRIOR:
						_focusY = Clamp<int>(_focusY - 10, 0, _rows.count - 1); // TODO: HACK, calculate actual page up position
						InvalidateWindow(*this);
					break;

					case VK_NEXT:
						_focusY = Clamp<int>(_focusY + 10, 0, _rows.count - 1); // TODO: HACK, calculate actual page down position
						InvalidateWindow(*this);
					break;

					case VK_HOME:
						_focusX = _focusY = 0;
						InvalidateWindow(*this);
					break;

					case VK_END:
						_focusX = _columns.count - 1;
						_focusY = _rows.count - 1;

						InvalidateWindow(*this);
					break;
				}

				bool shift	= (GetKeyState(VK_SHIFT) & 0x8000) != 0;
				bool ctrl	= (GetKeyState(VK_CONTROL) & 0x8000) != 0;

				if (shift && ctrl)
				{
					_selectX0 = _focusX;
					_selectY0 = _focusY;
				}
				else if (shift)
				{
					_selectX1 = _focusX;
					_selectY1 = _focusY;
				}
				else if (!ctrl)
				{
					_selectX0 = _selectX1 = _focusX;
					_selectY0 = _selectY1 = _focusY;
				}

				EnsureVisible(_focusX, _focusY);
			}
		return 0;

		case WM_CHAR:
			if (GridColumn* col = At(&_columns, _focusX))
			{
				if (GridRow* row = At(&_rows, _focusY))
				{
					row->KeyPressed(col, wParam);
				}
			}
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void GridView::DoVirtualLayout()
{
	// Width

	float x = _widthHeader;

	for(int iCol = 0; iCol < (int)_columns.count; iCol++)
	{
		GridColumn* col = _columns[iCol];

		col->_x = x;

		x += col->_width;
	}

	_widthTotal = x;

	// Height

	float y = _heightHeader;

	for(int iRow = 0; iRow < (int)_rows.count; iRow++)
	{
		GridRow* row = _rows[iRow];

		row->_y = y;

		y += row->_height;
	}

	_heightTotal = y;
}

void GridView::DoLayout()
{
	DoVirtualLayout();
	DoScrollBars();

	// Calculate translated positions

	for(int iCol = 0; iCol < (int)_columns.count; iCol++)
	{
		GridColumn* col = _columns[iCol];

		col->_cx = col->_x - _scrollX;
	}

	for(int iRow = 0; iRow < (int)_rows.count; iRow++)
	{
		GridRow* row = _rows[iRow];

		row->_cy = row->_y - _scrollY;
	}
}

void GridView::DoScrollBars()
{
	WinRect rc(*this);

	SCROLLINFO si = { sizeof(SCROLLINFO) };

	// Set ranges

	si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;

	si.nMin		= 0;
	si.nMax		= (int)_heightTotal;
	si.nPage	= rc.Height();

	SetScrollInfo(*this, SB_VERT, &si, TRUE);

	si.nMin		= 0;
	si.nMax		= (int)_widthTotal;
	si.nPage	= rc.Width();

	SetScrollInfo(*this, SB_HORZ, &si, TRUE);

	// Get position

	si.fMask = SIF_POS;

	GetScrollInfo(*this, SB_VERT, &si);

	_scrollY = Clamp((float)si.nPos, 0.0f, _heightTotal);

	GetScrollInfo(*this, SB_HORZ, &si);

	_scrollX = Clamp((float)si.nPos, 0.0f, _widthTotal);
}

void GridView::OnPaint(Gdiplus::Graphics* g)
{
	DoLayout();

	GridViewDrawContext dc;
	Gdiplus::GraphicsState s0;

	WinRect rcClient(*this);

	// Calculate Selection

	int sx0 = _selectX0;
	int sx1 = _selectX1;
	int sy0 = _selectY0;
	int sy1 = _selectY1;

	if (sx0 > sx1)
		Swap(sx0, sx1);

	if (sy0 > sy1)
		Swap(sy0, sy1);

	GridColumn*	selCol0 = At(&_columns, Clamp(sx0, 0, (int)_columns.count - 1));
	GridColumn*	selCol1 = At(&_columns, Clamp(sx1, 0, (int)_columns.count - 1));
	GridRow*	selRow0 = At(&_rows, Clamp(sy0, 0, (int)_rows.count - 1));
	GridRow*	selRow1 = At(&_rows, Clamp(sy1, 0, (int)_rows.count - 1));

	// Cell selection background

	if (selCol0 && selCol1 && selRow0 && selRow1)
	{
		NgRect rc;
		
		rc.pt.vx = selCol0->_cx;
		rc.pt.vy = selRow0->_cy;
		rc.sz.vx = selCol1->_cx + selCol1->_width - selCol0->_cx;
		rc.sz.vy = selRow1->_cy + selRow1->_height - selRow0->_cy;

		g->FillRectangle(&dc.cellSelectedBgBrush, Inflate(rc, -2.0f));
	}

	// Cells

	s0 = g->Save();

	for(int iRow = 0; iRow < (int)_rows.count; iRow++)
	{
		GridRow* row = _rows[iRow];

		if (((row->_cy + row->_height) < _heightHeader) || (row->_cy > rcClient.Height()))
			continue;

		for(int iCol = 0; iCol < (int)_columns.count; iCol++)
		{
			GridColumn* col = _columns[iCol];

			if (((col->_cx + col->_width) < _widthHeader) || (col->_cx > rcClient.Width()))
				continue;

			NgRect rc(col->_cx, row->_cy, col->_width, row->_height);

			g->SetClip(rc);

			DrawCell(g, &dc, row, col, rc);
		}
	}

	// Cell lines

	g->SetClip(NgRect(_widthHeader, _heightHeader, _widthTotal - _widthHeader, _heightTotal - _heightHeader));

	for(int iCol = 0; iCol < (int)_columns.count; iCol++)
	{
		GridColumn* col = _columns[iCol];

		if (((col->_cx + col->_width) < _widthHeader) || (col->_cx > rcClient.Width()))
			continue;

		g->DrawLine(&dc.cellLinePen, col->_cx, _heightHeader, col->_cx, _heightTotal);
	}

	for(int iRow = 0; iRow < (int)_rows.count; iRow++)
	{
		GridRow* row = _rows[iRow];

		if (((row->_cy + row->_height) < _heightHeader) || (row->_cy > rcClient.Height()))
			continue;

		g->DrawLine(&dc.cellLinePen, _widthHeader, row->_cy, _widthTotal, row->_cy);
	}

	// Selection box

	if (selCol0 && selCol1 && selRow0 && selRow1)
	{
		NgRect rc;
		
		rc.pt.vx = selCol0->_cx;
		rc.pt.vy = selRow0->_cy;
		rc.sz.vx = selCol1->_cx + selCol1->_width - selCol0->_cx;
		rc.sz.vy = selRow1->_cy + selRow1->_height - selRow0->_cy;

		g->DrawRectangle(&dc.cellSelectedLinePen, rc);
	}

	// Focus cell

	if ((_focusX >= 0) && (_focusY >= 0))
	{
		GridColumn*	col = _columns[Clamp(_focusX, 0, (int)_columns.count - 1)];
		GridRow*	row = _rows[Clamp(_focusY, 0, (int)_rows.count - 1)];

		NgRect rc;

		rc.pt.vx = col->_cx;
		rc.pt.vy = row->_cy;
		rc.sz.vx = col->_width;
		rc.sz.vy = row->_height;

		g->DrawRectangle(&dc.focusPen, rc);
		g->DrawRectangle(&dc.focusPen, Inflate(rc, -1.0f));
		g->DrawRectangle(&dc.focusPen, Inflate(rc, +1.0f));
	}

	g->Restore(s0);

	// Column headers

	s0 = g->Save();

	for(int iCol = 0; iCol < (int)_columns.count; iCol++)
	{
		GridColumn* col = _columns[iCol];

		if (((col->_cx + col->_width) < _widthHeader) || (col->_cx > rcClient.Width()))
			continue;

		NgRect rc(col->_cx, 0.0f, col->_width, _heightHeader);

		g->SetClip(rc);

		bool selected = (iCol >= sx0) && (iCol <= sx1);

		Gdiplus::Color c0 = Gdiplus::Color(249, 252, 253);
		Gdiplus::Color c1 = Gdiplus::Color(211, 219, 233);

		if (selected)
		{
			c0 = Gdiplus::Color(249, 217, 159);
			c1 = Gdiplus::Color(241, 193, 95);
		}

		Gdiplus::LinearGradientBrush b(rc, c0, c1, Gdiplus::LinearGradientModeVertical);

		//g->FillRectangle(selected ? &dc.headerSelectedBgBrush : &dc.headerBgBrush, Inflate(rc, 1.0f));
		g->FillRectangle(&b, Inflate(rc, 1.0f));

		DrawHeader(g, &dc, col->Label(), rc, selected);
	}

	g->SetClip(NgRect(_widthHeader, 0.0f, _widthTotal, _heightHeader + 1.0f));

	for(int iCol = 0; iCol < (int)_columns.count; iCol++)
	{
		GridColumn* col = _columns[iCol];

		if (((col->_cx + col->_width) < _widthHeader) || (col->_cx > rcClient.Width()))
			continue;

		bool vertSelected = (iCol >= sx0) && (iCol <= (sx1 + 1));
		bool horzSelected = (iCol >= sx0) && (iCol <= sx1);

		g->DrawLine(vertSelected ? &dc.headerSelectedLinePen : &dc.headerLinePen, col->_cx, 0.0f, col->_cx, _heightHeader);
		g->DrawLine(horzSelected ? &dc.headerSelectedLinePen : &dc.headerLinePen, col->_cx + 1.0f, _heightHeader, col->_cx + col->_width - 1.0f, _heightHeader);
	}

	g->Restore(s0);

	// Row headers

	s0 = g->Save();

	for(int iRow = 0; iRow < (int)_rows.count; iRow++)
	{
		GridRow* row = _rows[iRow];

		if (((row->_cy + row->_height) < _heightHeader) || (row->_cy > rcClient.Height()))
			continue;

		NgRect rc(0.0f, row->_cy, _widthHeader, row->_height);

		g->SetClip(rc);

		bool selected = (iRow >= sy0) && (iRow <= sy1);

		g->FillRectangle(selected ? &dc.headerSelectedBgBrush : &dc.headerBgBrush, Inflate(rc, 1.0f));

		DrawHeader(g, &dc, row->Label(), rc, selected);
	}

	g->SetClip(NgRect(0.0f, _heightHeader, _widthHeader + 1.0f, _heightTotal));

	for(int iRow = 0; iRow < (int)_rows.count; iRow++)
	{
		GridRow* row = _rows[iRow];

		if (((row->_cy + row->_height) < _heightHeader) || (row->_cy > rcClient.Height()))
			continue;

		bool horzSelected = (iRow >= sy0) && (iRow <= (sy1 + 1));
		bool vertSelected = (iRow >= sy0) && (iRow <= sy1);

		g->DrawLine(horzSelected ? &dc.headerSelectedLinePen : &dc.headerLinePen, 0.0f, row->_cy, _widthHeader, row->_cy);
		g->DrawLine(vertSelected ? &dc.headerSelectedLinePen : &dc.headerLinePen, _widthHeader, row->_cy + 1.0f, _widthHeader, row->_cy + row->_height - 1.0f);
	}

	g->Restore(s0);

	// Empty square

	g->FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(169, 196, 233)), NgRect(-1.0f, -1.0f, _widthHeader + 1.0f, _heightHeader + 1.0f));

	g->DrawLine(&dc.headerLinePen, 0.0f, _heightHeader, _widthHeader, _heightHeader);
	g->DrawLine(&dc.headerLinePen, _widthHeader, 0.0f, _widthHeader, _heightHeader);
}

void GridView::DrawHeader(Gdiplus::Graphics* g, GridViewDrawContext* dc, const Sz16& label, const NgRect& rc, bool selected)
{
	Gdiplus::StringFormat f;
	f.SetTrimming(Gdiplus::StringTrimmingNone);
	f.SetAlignment(Gdiplus::StringAlignmentCenter);

	g->DrawString(label, label.Length(), &dc->labelFont, rc, &f, &dc->blackBrush);
}

void GridView::DrawCell(Gdiplus::Graphics* g, GridViewDrawContext* dc, GridRow* row, GridColumn* column, const NgRect& rc)
{
	Sz16 content = row->Content(column);

	g->DrawString(content, content.Length(), &dc->labelFont, rc.pt, &dc->blackBrush);
}

void GridView::DoDrag(const WinPoint& pt)
{
	if (_dragState == kDragNone)
		return;

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

	if ((pt.x == _dragLast.x) && (pt.y == _dragLast.y))
		return;

	switch(_dragState)
	{
		case kDragSelectColumns:
			DoLayout();

			_selectX1 = HitTestColumn(pt.x);

			InvalidateWindow(*this);
		break;

		case kDragSelectRows:
			DoLayout();

			_selectY1 = HitTestRow(pt.y);

			InvalidateWindow(*this);
		break;

		case kDragSelectCells:
			DoLayout();

			_focusX = _selectX1 = HitTestColumn(pt.x);
			_focusY = _selectY1 = HitTestRow(pt.y);

			EnsureVisible(_focusX, _focusY);

			InvalidateWindow(*this);
		break;

		case kDragRowEdge:
			_widthHeader = Clamp((float)pt.x, 8.0f, 512.0f);

			InvalidateWindow(*this);
		break;

		case kDragColumnSplitter:
			if (GridColumn* column = At(&_columns, _dragColumn))
			{
				column->_width = Clamp(pt.x - column->_cx, 8.0f, 512.0f);

				InvalidateWindow(*this);
			}
		break;
	}

	_dragLast = pt;
}

void GridView::DoPan(const WinPoint& pt)
{
	if (!_panning)
		return;

	if (!_panCommitted)
	{
		WinPoint delta = _panStart - pt;

		if ((GetTickCount() - _panTime) < 400)
		{
			if ((abs(delta.x) < 6) && (abs(delta.y) < 6))
				return;
		}

		_panCommitted = true;
	}

	WinRect rc(*this);

	if (pt.x != _panLast.x)
	{
		SCROLLINFO si = { sizeof(SCROLLINFO) };

		// Get

		si.fMask = SIF_POS;

		GetScrollInfo(*this, SB_HORZ, &si);

		_scrollX = Clamp((float)si.nPos - (pt.x - _panLast.x), 0.0f, _widthTotal);

		// Set

		si.nPos = (int)_scrollX;

		SetScrollInfo(*this, SB_HORZ, &si, TRUE);

		InvalidateWindow(*this);
	}

	if (pt.y != _panLast.y)
	{
		SCROLLINFO si = { sizeof(SCROLLINFO) };

		// Get

		si.fMask = SIF_POS;

		GetScrollInfo(*this, SB_VERT, &si);

		_scrollY = Clamp((float)si.nPos - (pt.y - _panLast.y), 0.0f, _heightTotal);

		// Set

		si.nPos = (int)_scrollY;

		SetScrollInfo(*this, SB_VERT, &si, TRUE);

		InvalidateWindow(*this);
	}

	_panLast = pt;
}

void GridView::DoSetCapture()
{
	if ((_dragState == kDragNone) && !_panning)
		SetCapture(*this);
}

void GridView::DoReleaseCapture()
{
	if ((_dragState == kDragNone) && !_panning)
		ReleaseCapture();
}

GridHitResult GridView::HitTest(const WinPoint& pt)
{
	WinRect rc(*this);

	// Control boundries

	if (!PtInRect(&rc, pt))
		return GridHitResult(GridHitResult::kNone, -1, -1);

	// Row edge

	if (fabsf(pt.x - _widthHeader) < 4.0f)
		return GridHitResult(GridHitResult::kRowEdge, -1, -1);

	// Column splitters

	int column	= HitTestColumn(pt.x);
	int row		= HitTestRow(pt.y);

	if (pt.y < _heightHeader)
	{
		if (GridColumn* col = At(&_columns, column))
		{
			int dl = (int)fabsf(pt.x - col->_cx);
			int dr = (int)fabsf(pt.x - (col->_cx + col->_width));

			if (dr < 4)
				return GridHitResult(GridHitResult::kColumnSplitter, column, -1);

			if ((column > 0) && (dl < 4))
				return GridHitResult(GridHitResult::kColumnSplitter, column - 1, -1);
		}
	}

	// Empty square

	if ((pt.x < _widthHeader) && (pt.y < _heightHeader))
		return GridHitResult(GridHitResult::kNone, -1, -1);

	// Row headers

	if (pt.x < _widthHeader)
		return GridHitResult(GridHitResult::kRowHeader, -1, row);

	// Column headers

	if (pt.y < _heightHeader)
		return GridHitResult(GridHitResult::kColumnHeader, column, -1);

	// Cells

	return GridHitResult(GridHitResult::kCells, column, row);
}

int GridView::HitTestColumn(int x)
{
	for(int iCol = 0; iCol < (int)_columns.count; iCol++)
	{
		GridColumn*	col	= _columns[iCol];
		int			tx	= (int)(x - col->_cx);

		if ((tx >= 0) && (tx < col->_width))
			return iCol;
	}

	if (_columns.count > 0)
	{
		if (x < _columns[0]->_cx)
			return 0;

		if (x > _columns[_columns.count - 1]->_cx)
			return _columns.count - 1;
	}

	return -1;
}

int GridView::HitTestRow(int y)
{
	for(int iRow = 0; iRow < (int)_rows.count; iRow++)
	{
		GridRow*	row	= _rows[iRow];
		int			ty	= (int)(y - row->_cy);

		if ((ty >= 0) && (ty < row->_height))
			return iRow;
	}

	if (_rows.count > 0)
	{
		if (y < _rows[0]->_cy)
			return 0;

		if (y > _rows[_rows.count - 1]->_cy)
			return _rows.count - 1;
	}

	return -1;
}

void GridView::DoCursor()
{
	WinPoint		pt		= WinPoint::FromLastMessage().ScreenToClient(*this);
	GridHitResult	result	= HitTest(pt);

	switch(result.kind)
	{
		case GridHitResult::kColumnHeader:
			SetCursor(LoadCursor(GetXtWinIconDll(), MAKEINTRESOURCE(IDC_XTWIN_GRIDDOWN)));
		break;

		case GridHitResult::kColumnSplitter:
			SetCursor(LoadCursor(GetXtWinIconDll(), MAKEINTRESOURCE(IDC_XTWIN_GRIDSIZEX)));
		break;

		case GridHitResult::kRowHeader:
			SetCursor(LoadCursor(GetXtWinIconDll(), MAKEINTRESOURCE(IDC_XTWIN_GRIDRIGHT)));
		break;

		case GridHitResult::kRowEdge:
			SetCursor(LoadCursor(GetXtWinIconDll(), MAKEINTRESOURCE(IDC_XTWIN_GRIDSIZEX)));
		break;

		case GridHitResult::kCells:
			SetCursor(LoadCursor(GetXtWinIconDll(), MAKEINTRESOURCE(IDC_XTWIN_CROSS)));
		break;

		default:
			SetCursor(LoadCursor(0, IDC_ARROW));
		break;
	}
}