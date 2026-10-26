#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

//
// ScarabDrawContext
//

ScarabDrawContext::ScarabDrawContext() :
	backgroundColour(RGB(255, 255, 255)),
	borderColour(RGB(169, 169, 169)),
	headerColour(RGB(236, 236, 236))
{
	backgroundBrush	= CreateSolidBrush(backgroundColour);

	borderPen		= CreatePen(PS_SOLID, 0, borderColour);
	borderBrush		= CreateSolidBrush(borderColour);

	headerPen		= CreatePen(PS_SOLID, 0, headerColour);
	headerBrush		= CreateSolidBrush(headerColour);

	labelFont		= (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	LOGFONT lf;

	GetObject(labelFont, sizeof(lf), &lf);

	lf.lfWeight = FW_BOLD;

	boldFont	= CreateFontIndirect(&lf);

	lf.lfHeight		= (int)(lf.lfHeight * 1.4f);
	lf.lfWeight		= FW_BOLD;
	lf.lfCharSet	= DEFAULT_CHARSET;
	lf.lfQuality	= ANTIALIASED_QUALITY;

	_snprintf_s(lf.lfFaceName, LF_FACESIZE, _TRUNCATE, "Wingdings 3");

	iconFont	= CreateFontIndirect(&lf);

	_snprintf_s(lf.lfFaceName, LF_FACESIZE, _TRUNCATE, "Wingdings 2");

	iconFont2	= CreateFontIndirect(&lf);

	{
		HDC ic = CreateIC(_T("DISPLAY"), 0, 0, 0);

		SelectObject(ic, labelFont);

		WinSize sz;
		GetTextExtentPoint32(ic, _T("W"), 1, &sz);

		labelSizeY = sz.cy;

		DeleteDC(ic);
	}
}

ScarabDrawContext::~ScarabDrawContext()
{
	if (backgroundBrush != 0)
		DeleteObject(backgroundBrush);

	if (borderPen != 0)
		DeleteObject(borderPen);

	if (borderBrush != 0)
		DeleteObject(borderBrush);

	if (headerPen != 0)
		DeleteObject(borderPen);

	if (headerBrush != 0)
		DeleteObject(borderBrush);

	if (boldFont != 0)
		DeleteObject(boldFont);

	if (iconFont != 0)
		DeleteObject(boldFont);
}

//
// ScarabFormContext
//

ScarabFormContext::ScarabFormContext()
{
}

ScarabFormContext::~ScarabFormContext()
{
}

void ScarabFormContext::OpenRecord(Xt::DomRecord* record)
{
}

void ScarabFormContext::BeginDropper(LarvaField* field, Xt::DomElement* element)
{
}