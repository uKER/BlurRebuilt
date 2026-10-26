#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DeskWindow.h"
#include "XtWin/Resource.h"

//
// DwDrawContext
//

DwDrawContext::DwDrawContext() :
	borderColour(GetSysColor(COLOR_3DSHADOW)),
	backgroundColour(GetSysColor(COLOR_MENUBAR)),
	faceColour(RGB(204, 199, 186)),
	labelColour(RGB(0, 0, 0))
{
	borderPen		= CreatePen(PS_SOLID, 0, borderColour);
	borderBrush		= CreateSolidBrush(borderColour);

	backgroundPen	= CreatePen(PS_SOLID, 0, backgroundColour);
	backgroundBrush	= CreateSolidBrush(backgroundColour);

	labelFont		= (HFONT)GetStockObject(DEFAULT_GUI_FONT);
}

DwDrawContext::~DwDrawContext()
{
	if (borderPen != 0)
		DeleteObject(borderPen);

	if (borderBrush != 0)
		DeleteObject(borderBrush);

	if (backgroundPen != 0)
		DeleteObject(backgroundPen);

	if (backgroundBrush != 0)
		DeleteObject(backgroundBrush);
}

//
// DwHost
//

DwHost::DwHost() :
	_root(this),
	_window(0)
{
}

DwHost::~DwHost()
{
}

void DwHost::SetWindow(HWND window)
{
	_window = window;
}

void DwHost::SetLocation(const WinRect& location)
{
	_location = location;
}

void DwHost::PerformLayout(bool scaling)
{
	if (!_window)
		return;

	int count = _root.CountWindows();

	if (count <= 0)
		return;

	HDWP dwp = BeginDeferWindowPos(count);

	if (dwp == 0)
		return;

	_root.PerformLayout(_location, dwp, scaling);

	EndDeferWindowPos(dwp);

	InvalidateWindow(_window);
}

void DwHost::Paint(HDC hdc, const WinRect& rcPaint)
{
	if (!_window)
		return;

	DwDrawContext dc;

	dc.hdc		= hdc;
	dc.rcPaint	= rcPaint;

	_root.OnPaint(dc);

	SelectObject(dc.hdc, GetStockObject(WHITE_BRUSH));
	SelectObject(dc.hdc, GetStockObject(BLACK_PEN));
	SelectObject(dc.hdc, GetStockObject(DEFAULT_GUI_FONT));
}