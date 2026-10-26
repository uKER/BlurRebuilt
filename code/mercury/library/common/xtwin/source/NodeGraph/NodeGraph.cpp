#include "Core/XtWinPch.h"
#include "NgImpl.h"

//
// NodeGraphContext
//

NodeGraphContext::~NodeGraphContext()
{
}

void NodeGraphContext::ContextMenu(const WinPoint& pt)
{
}

void NodeGraphContext::SelectionChanged()
{
}

void NodeGraphContext::CheckPoint()
{
}

void NodeGraphContext::Undo()
{
}

void NodeGraphContext::Redo()
{
}

void NodeGraphContext::Delete()
{
}

void NodeGraphContext::Cut()
{
}

void NodeGraphContext::Copy()
{
}

void NodeGraphContext::Paste(const NgVector& targetPt)
{
}

//
// NodeGraph
//

NodeGraph::NodeGraph(NodeGraphContext* context) :
	_diagram(this),
	_context(context),
	_panning(false),
	_panCommitted(false),
	_panTime(0),
	_dragging(kDragNone),
	_dragCommitted(false),
	_dragTime(0),
	_zoomIn(false),
	_zoomOut(false)
{
	assert(_context);
}

NodeGraph::~NodeGraph()
{
	Diagram()->Clear();

	delete _context;
}

bool NodeGraph::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	_wndClassRedraw = true;

	return true;
}

LRESULT NodeGraph::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
			SetTimer(*this, WM_APP, 10, 0);

			_diagram.SetClientRect(NgRect(WinRect(*this)));
		break;

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
					g2.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

					g2.TranslateTransform(-(float)rcPaint.left, -(float)rcPaint.top);

					_diagram.Render(&g2);
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
			if (_dragging == kDragMoveNodes)
				SetCursor(LoadCursor(0, IDC_SIZEALL));
			else
				SetCursor(LoadCursor(0, IDC_ARROW));
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

			_diagram.Zoom(_diagram.ToCanvas(pt), factor);
		}
		return 0;

		// Mouse movement

		case WM_MOUSEMOVE:
		{
			WinPoint pt = WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if ((_dragging == kDragNone) || (_dragging == kDragPin))
			{
				NgPin*	pin		= _diagram.HitTestPin(_diagram.ToCanvas(pt));
				NgNode*	node	= _diagram.HitTestNode(_diagram.ToCanvas(pt));

				if (pin && node)
				{
					if (pin->Node()->IsBehind(node))
						pin = 0;
				}

				_diagram.SetHotPin(pin);
			}
			else
				_diagram.SetHotPin(0);

			if (_panning)
				DoPan(pt);

			if (_dragging != kDragNone)
				DoDrag(pt);
		}
		return 0;

		// Left mouse button

		case WM_LBUTTONDOWN:
		{
			if (_dragging == kDragNone)
			{
				WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				// Select nodes

				NgNode* node = _diagram.HitTestNode(_diagram.ToCanvas(pt));

				DragState drag = node ? kDragIgnore : kDragSelection;

				if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
					_diagram.Select(node, kNgSelectRemove);
				else if ((GetKeyState(VK_SHIFT) & 0x8000))
					_diagram.Select(node, kNgSelectAdd);
				else if (GetKeyState(VK_CONTROL) & 0x8000)
					_diagram.Select(node, kNgSelectToggle);
				else
				{
					if (node)
					{
						if ((_diagram.SelectionCount() == 1) || !node->IsSelected())
							_diagram.Select(node, kNgSelectSingle);
					}
					else
						_diagram.ClearSelection();
				}

				if (node && node->IsSelected())
					drag = kDragMoveNodes;

				// Start dragging

				DoSetCapture();

				_dragging = drag;

				_diagram.SetHotPin(0);

				if (_dragging == kDragSelection)
					_diagram.RectSelectionBegin(_diagram.ToCanvas(pt));

				_dragCommitted	= false;
				_dragTime		= GetTickCount();
				_dragStart		= pt;
				_dragLast		= pt;
			}
		}
		return 0;

		case WM_LBUTTONDBLCLK:
		{
			if (_dragging == kDragNone)
			{
				WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				NgNode* node = _diagram.HitTestNode(_diagram.ToCanvas(pt));

				if (node)
					node->OnDoubleClicked();
			}
		}
		return 0;

		case WM_LBUTTONUP:
			if ((_dragging == kDragIgnore) || (_dragging == kDragSelection) || (_dragging == kDragMoveNodes))
			{
				DragState drag = _dragging;

				DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

				_dragging = kDragNone;

				DoReleaseCapture();

				_diagram.RectSelectionEnd();

				if (drag == kDragSelection)
				{
					NgSelectMode mode = kNgSelectAdd;

					if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
						mode = kNgSelectRemove;
					else if ((GetKeyState(VK_SHIFT) & 0x8000))
						mode = kNgSelectAdd;
					else if (GetKeyState(VK_CONTROL) & 0x8000)
						mode = kNgSelectToggle;
					else
						_diagram.ClearSelection();

					_diagram.SelectRect(_diagram.RectSelection(), mode);
				}

				_context->CheckPoint();
			}
		return 0;

		// Middle mouse button

		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		{
			if (_dragging == kDragNone)
			{
				WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				// Select pins

				NgPin*	pin		= _diagram.HitTestPin(_diagram.ToCanvas(pt));
				NgNode*	node	= _diagram.HitTestNode(_diagram.ToCanvas(pt));

				if (pin && node)
				{
					if (pin->Node()->IsBehind(node))
						pin = 0;
				}

				DragState drag = pin ? kDragPin : kDragIgnorePin;

				// Start dragging

				DoSetCapture();

				_dragging = drag;

				if (_dragging == kDragPin)
					_diagram.ConnectionDragBegin(pin ? pin->OnDrag() : 0, pin);

				_dragCommitted	= false;
				_dragTime		= GetTickCount();
				_dragStart		= pt;
				_dragLast		= pt;
			}
		}
		return 0;

		case WM_MBUTTONUP:
			if ((_dragging == kDragIgnorePin) || (_dragging == kDragPin))
			{
				DoDrag(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

				_dragging = kDragNone;

				DoReleaseCapture();

				_diagram.ConnectionDragEnd();

				NgPin* from	= _diagram.ConnectionDragFrom();
				NgPin* to	= _diagram.ConnectionDragTo();

				if (from && to)
				{
					if (from->OnTryConnect(to))
						from->OnConnect(to);
				}
			}
		return 0;

		// Right mouse button

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			DoSetCapture();

			_panning		= true;
			_panCommitted	= false;
			_panTime		= GetTickCount();
			_panStart		= WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			_panLast		= _panStart;
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
				ClientToScreen(*this, &pt);

				PostMessage(*this, WM_CONTEXTMENU, (WPARAM)(HWND)*this, MAKELONG(pt.x, pt.y));
			}
		}
		return 0;

		// Additional mouse buttons

		case WM_XBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
			if (HIWORD(wParam) == XBUTTON1)
			{
				DoSetCapture();

				_zoomIn = true;
			}
			else if (HIWORD(wParam) == XBUTTON2)
			{
				DoSetCapture();

				_zoomOut = true;
			}
		break;

		case WM_XBUTTONUP:
			if (HIWORD(wParam) == XBUTTON1)
			{
				_zoomIn = false;

				DoReleaseCapture();
			}
			else if (HIWORD(wParam) == XBUTTON2)
			{
				_zoomOut = false;

				DoReleaseCapture();
			}
		break;

		// Capture

		case WM_CAPTURECHANGED:
			_panning		= false;
			_dragging		= kDragNone;
			_zoomIn			= false;
			_zoomOut		= false;

			_diagram.RectSelectionEnd();
			_diagram.ConnectionDragEnd();
		break;

		// Timer

		case WM_TIMER:
			if (wParam == WM_APP)
			{
				if (_zoomIn ^ _zoomOut)
				{
					DWORD mp = GetMessagePos();

					WinPoint pt(GET_X_LPARAM(mp), GET_Y_LPARAM(mp));

					if (ScreenToClient(*this, &pt))
					{
						float factor = (_zoomIn ? 0.1f : 0.0f) - (_zoomOut ? 0.1f : 0.0f);

						if (factor < 0)
							factor = 1.0f / Clamp(1.0f + fabs(factor * 0.4f), 1.0f, 2.0f);
						else
							factor = Clamp(1.0f + fabs(factor * 0.4f), 1.0f, 2.0f);

						_diagram.Zoom(_diagram.ToCanvas(pt), factor);
					}
				}
			}
		break;

		// Keyboard

		case WM_CHAR:
		return 0;

		case WM_KEYDOWN:
			if ((wParam == 'Z') && (GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
				_context->Redo();
			else if ((wParam == 'Z') && (GetKeyState(VK_CONTROL) & 0x8000))
				_context->Undo();
			else if (wParam == VK_DELETE)
				_context->Delete();
			else if ((wParam == 'X') && (GetKeyState(VK_CONTROL) & 0x8000))
				_context->Cut();
			else if ((wParam == 'C') && (GetKeyState(VK_CONTROL) & 0x8000))
				_context->Copy();
			else if ((wParam == 'V') && (GetKeyState(VK_CONTROL) & 0x8000))
			{
				DWORD mp = GetMessagePos();

				WinPoint pt(GET_X_LPARAM(mp), GET_Y_LPARAM(mp));

				_context->Paste(_diagram.ToCanvas(pt.ScreenToClient(*this)));
			}
		return 0;

		case WM_KEYUP:
		return 0;

		// Context menu

		case WM_CONTEXTMENU:
			_context->ContextMenu(WinPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		return 0;

		// Dialog

		case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

		// Size

		case WM_SIZE:
			_diagram.SetClientRect(NgRect(WinRect(*this)));
		break;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void NodeGraph::DoPan(const WinPoint& pt)
{
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

	if ((pt.x == _panLast.x) && (pt.y == _panLast.y))
		return;

	_diagram.Pan(_diagram.ToCanvas(pt) - _diagram.ToCanvas(_panLast));

	_panLast = pt;
}

void NodeGraph::DoDrag(const WinPoint& pt)
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

	if ((pt.x == _dragLast.x) && (pt.y == _dragLast.y))
		return;

	switch(_dragging)
	{
		case kDragSelection:
			_diagram.RectSelectionUpdate(_diagram.ToCanvas(pt));
		break;

		case kDragMoveNodes:
			if (!_panning)
				_diagram.MoveSelectedBy(_diagram.ToCanvas(pt) - _diagram.ToCanvas(_dragLast));
		break;

		case kDragPin:
		{
			NgPin*	pin		= _diagram.HitTestPin(_diagram.ToCanvas(pt));
			NgNode*	node	= _diagram.HitTestNode(_diagram.ToCanvas(pt));

			if (pin && node)
			{
				if (pin->Node()->IsBehind(node))
					pin = 0;
			}

			if (pin)
				_diagram.ConnectionDragUpdate(pin);
			else
				_diagram.ConnectionDragUpdate(_diagram.ToCanvas(pt));
		}
		break;
	}

	_dragLast = pt;
}

void NodeGraph::DoSetCapture()
{
	if ((_dragging == kDragNone) && !_panning && !_zoomIn && !_zoomOut)
		SetCapture(*this);
}

void NodeGraph::DoReleaseCapture()
{
	if ((_dragging == kDragNone) && !_panning && !_zoomIn && !_zoomOut)
		ReleaseCapture();
}