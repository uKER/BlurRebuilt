#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DeskWindow.h"
#include "XtWin/Resource.h"

DwPane::DwPane(DwHost* host) :
	DwItem(kDwtPane, host)
{
}

DwPane::~DwPane()
{
}

DwTab* DwPane::Add(DwTab* tab)
{
	return _tabs.Append(tab);
}

int DwPane::CountWindows()
{
	int count = 0;

	for(int i = 0; i < _tabs.Length(); i++)
		count += _tabs[i].CountWindows();

	return count;
}

WinSize DwPane::RequiredSize()
{
	WinSize sz;

	for(int i = 0; i < _tabs.Length(); i++)
	{
		WinSize childSz = _tabs[i].RequiredSize();

		sz.cx = Max(sz.cx, childSz.cx);
		sz.cy = Max(sz.cy, childSz.cy);
	}

	return sz;
}

void DwPane::OnLayout(const WinRect& location, HDWP dwp, bool scaling)
{
	for(int i = 0; i < _tabs.Length(); i++)
		_tabs[i].PerformLayout(location, dwp, scaling);
}

void DwPane::OnPaint(DwDrawContext& dc)
{
	for(int i = 0; i < _tabs.Length(); i++)
		_tabs[i].OnPaint(dc);
}