#include "Core/XtWinPch.h"
#include "PropertySheetImpl.h"

PsControl::PsControl() :
	_anchor(this),
	_row(0),
	_linkRow(this),
	_desiredWidth(-1),
	_x(0),
	_width(0)
{
}

PsControl::~PsControl()
{
}

void PsControl::SetDesiredWidth(int desiredWidth)
{
	if (desiredWidth != _desiredWidth)
	{
		_desiredWidth = desiredWidth;

		SetDirty(true);
	}
}

void PsControl::SetDirty(bool performLayout)
{
	if (_row)
		_row->SetDirty(performLayout);
}

void PsControl::Clicked()
{
}

void PsControl::Draw(Gdiplus::Graphics* g, PropertySheetDrawContext* dc, const XwRect& rect)
{
}