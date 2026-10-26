#include "Core/XtWinPch.h"
#include "PropertySheetImpl.h"

PsButton::PsButton()
{
	SetDesiredWidth(20);
}

PsButton::~PsButton()
{
}

void PsButton::SetText(const Sz16& text)
{
	if (text != _text)
	{
		_text = text;

		PropertySheetDrawContext dc;

		Gdiplus::Graphics g(*Row()->Diagram()->Sheet());

		Gdiplus::RectF bb;

		g.MeasureString(_text.Text(), _text.Length(), &dc.labelFont, Gdiplus::PointF(0, 0), &bb);

		SetDesiredWidth((int)bb.Width + 5);

		SetDirty(true);
	}
}

void PsButton::Clicked()
{
	DebugLn("%S", _text.Text());
}

void PsButton::Draw(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect)
{
	g->FillRectangle(&dc->gridBrush, rect.pt.x, rect.pt.y, rect.sz.x, rect.sz.y);

	Gdiplus::StringFormat sf;

	sf.SetAlignment(Gdiplus::StringAlignmentCenter);
	sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	sf.SetTrimming(Gdiplus::StringTrimmingNone);
	sf.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

	g->DrawString(_text.Text(), _text.Length(), &dc->labelFont, rect, &sf, &dc->labelBrush);
}