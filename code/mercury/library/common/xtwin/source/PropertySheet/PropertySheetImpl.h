#ifndef XTWIN_PROPERTYSHEETIMPL_H
#define XTWIN_PROPERTYSHEETIMPL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/Core.h"
#include "XtWin/PropertySheet.h"

//
// PropertySheetDrawContext
//

struct PropertySheetDrawContext
{
	Gdiplus::SolidBrush	blackBrush;
	Gdiplus::SolidBrush	whiteBrush;

	Gdiplus::Font		labelFont;
	Gdiplus::Pen		labelPen;
	Gdiplus::SolidBrush	labelBrush;

	Gdiplus::Pen		gridPen;
	Gdiplus::SolidBrush	gridBrush;

	Gdiplus::Pen		selectedPen;
	Gdiplus::SolidBrush	selectedBrush;

	Gdiplus::Font		multiFont;
	Gdiplus::Pen		multiPen;
	Gdiplus::SolidBrush	multiBrush;

public:
	PropertySheetDrawContext();
};

#endif // XTWIN_PROPERTYSHEETIMPL_H