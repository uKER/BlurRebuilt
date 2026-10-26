#ifndef XTWIN_GRIDVIEWIMPL_H
#define XTWIN_GRIDVIEWIMPL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/Core.h"
#include "XtWin/GridView.h"

//
// GridViewDrawContext
//

struct GridViewDrawContext
{
	Gdiplus::Font		labelFont;
	Gdiplus::SolidBrush	blackBrush;

	Gdiplus::SolidBrush	headerBgBrush;
	Gdiplus::Pen		headerLinePen;

	Gdiplus::SolidBrush	headerSelectedBgBrush;
	Gdiplus::Pen		headerSelectedLinePen;

	Gdiplus::Pen		cellLinePen;

	Gdiplus::SolidBrush	cellSelectedBgBrush;
	Gdiplus::Pen		cellSelectedLinePen;

	Gdiplus::Pen		focusPen;

public:
	GridViewDrawContext();
};

#endif // XTWIN_GRIDVIEWIMPL_H