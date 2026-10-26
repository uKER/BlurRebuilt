#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DeskWindow.h"
#include "XtWin/Resource.h"
#include "XtWin/Scarab.h"

DwTab::DwTab(DwHost* host, HWND window) :
	DwItem(kDwtTab, host),
	_window(window)
{
}

DwTab::~DwTab()
{
}

int DwTab::CountWindows()
{
	return _window ? 1 : 0;
}

WinSize DwTab::RequiredSize()
{
	return WinSize(16, 16);
}

void DwTab::OnLayout(const WinRect& location, HDWP dwp, bool scaling)
{
	if (!_window)
		return;

	XtString text = SzGetWindowText(_window);

	WinRect rc(location);

	if (text.Length() > 0)
		rc.top += 18;

	rc.Shrink(1);

	DeferWindowPos(dwp, _window, 0, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void DwTab::OnPaint(DwDrawContext& dc)
{
	WinRect rc = Location();

	XtString text = SzGetWindowText(_window);

	SelectObject(dc, dc.borderPen);
	SelectObject(dc, GetStockObject(HOLLOW_BRUSH));

	if (text.Length() > 0)
	{
		// Label

		SetTextColor(dc, dc.labelColour);
		SetBkColor(dc, dc.faceColour);
		SelectObject(dc, dc.labelFont);

		ExtTextOut(dc, rc.left + 4, rc.top + 2, ETO_OPAQUE, &WinRect(rc.left, rc.top, rc.right, rc.top + 18), text, (int)text.Length(), 0);

		Rectangle(dc, rc.left, rc.top, rc.right, rc.top + 19);

		rc.top += 18;
	}

	// Tab outline

	Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
}