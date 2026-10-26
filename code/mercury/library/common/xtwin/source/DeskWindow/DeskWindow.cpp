#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DeskWindow.h"
#include "XtWin/Resource.h"

/*class FloatingPalette : public Window
{
};*/

DeskWindow::DeskWindow()
{
}

DeskWindow::~DeskWindow()
{
}

bool DeskWindow::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!Window::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle	&= ~WS_EX_CLIENTEDGE;

	return true;
}

LRESULT DeskWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
			if (Window::WndProc(msg, wParam, lParam) < 0)
				return -1;

			_host.SetWindow(*this);
		return 0;

		case WM_DESTROY:
			_host.SetWindow(0);
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);
			_host.Paint(ps.hdc, ps.rcPaint);
			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_SIZE:
			_host.SetLocation(WinRect(*this));
			_host.PerformLayout(false);
		return 0;
	}

	return Window::WndProc(msg, wParam, lParam);
}