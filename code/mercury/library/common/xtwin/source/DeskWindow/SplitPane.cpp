#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DeskWindow.h"
#include "XtWin/Resource.h"

SplitPane::SplitPane(DwSplit& split) :
	_split(split),
	_capture(false)
{
}

SplitPane::~SplitPane()
{
}

bool SplitPane::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style |= WS_TABSTOP;

	return true;
}

LRESULT SplitPane::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);
			SetBkColor(ps.hdc, GetSysColor(COLOR_3DFACE));
			ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, 0, 0, 0);
			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_SETCURSOR:
		{
			if (_capture || ((GetKeyState(VK_LBUTTON) & 0x8000) == 0))
			{
				int cursor = _split.Vertical() ? IDC_XTWIN_SPLITH : IDC_XTWIN_SPLITV;

				HCURSOR h = LoadCursor(GetXtWinIconDll(), MAKEINTRESOURCE(cursor));

				if (!h)
					h = LoadCursor(0, IDC_ARROW);

				SetCursor(h);
			}
			else
				SetCursor(LoadCursor(0, IDC_ARROW));
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			WinPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			MapWindowPoints(*this, GetParent(*this), &point, 1);

			_capture	= true;
			_position	= _split.Vertical() ? point.x : point.y;
			SetCapture(*this);

			SendMessage(*this, WM_SETCURSOR, 0, 0);
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (_capture)
				ReleaseCapture();

			SendMessage(*this, WM_SETCURSOR, 0, 0);
		}
		return 0;

		case WM_CAPTURECHANGED:
			_capture = false;
		return 0;

		case WM_MOUSEMOVE:
			if (_capture)
			{
				WinPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				MapWindowPoints(*this, GetParent(*this), &point, 1);

				int position = _split.Vertical() ? point.x : point.y;

				if (_position != position)
				{
					_split.SetPosition(_position = position);

					_split.Host()->PerformLayout(false);
				}
			}
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}