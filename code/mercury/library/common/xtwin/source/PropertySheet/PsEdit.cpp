#include "Core/XtWinPch.h"
#include "PropertySheetImpl.h"

PsEdit::PsEdit()
{
}

PsEdit::~PsEdit()
{
}

void PsEdit::SetText(const Sz16& text)
{
	if (text != _text)
	{
		_text = text;

		SetDirty(true);
	}
}

void PsEdit::Clicked()
{
	// TODO: Create temporary edit control upon click for editing?
}

void PsEdit::Draw(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect)
{
	Gdiplus::StringFormat sf;

	sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	sf.SetTrimming(Gdiplus::StringTrimmingNone);
	sf.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

	g->DrawString(_text.Text(), _text.Length(), &dc->labelFont, rect, &sf, &dc->labelBrush);
	//g->DrawString(L"(multiple)", -1, &dc->multiFont, rect, &sf, &dc->multiBrush);
}