#include "Core/XtWinPch.h"
#include "DopeSheetImpl.h"

DsKey::DsKey() :
	_anchor(this),
	_row(0),
	_linkRow(this),
	_isSelected(0),
	_linkSelection(this),
	_subRow(0),
	_time(0.0f),
	_duration(0.0f),
	_colour(0xFF202020)
{
}

DsKey::~DsKey()
{
	if (_isSelected)
		Deselect();

	SetDirty(true);
}

void DsKey::OnTimeChanged()
{
}

void DsKey::SetTime(float time)
{
	if (time != _time)
	{
		_time = time;

		SetDirty(true);
	}
}

void DsKey::SetDuration(float duration)
{
	if (duration != _duration)
	{
		_duration = duration;

		SetDirty(true);
	}
}

void DsKey::SetColour(XtU32 colour)
{
	_colour = (colour & 0xFFFFFF) | 0xFF000000;

	SetDirty(false);
}

void DsKey::Detach()
{
	if (DsRow* row = _row)
	{
		XtUt::Detach(&_linkRow);

		_row = 0;

		row->SetDirty(true);
	}
}

DsDiagram* DsKey::Diagram() const
{
	return _row ? _row->Diagram() : 0;
}

void DsKey::MoveBy(const NgVector& offset)
{
	_time += offset.vx;

	if (DsDiagram* diagram = Diagram())
	{
		if (diagram->_quantise)
		{
			float q = diagram->_quantisation;

			_time = ((int)((_time / q) + _copysign(0.5f, _time))) * q;
		}
	}

	OnTimeChanged();

	SetDirty(true);	
}

void DsKey::Select()
{
	if (_isSelected)
		return;

	DsDiagram* diagram = Diagram();

	if (!diagram)
		return;

	_isSelected = true;

	XtUt::PushFront(&diagram->_selectionKeys, &_linkSelection);

	//BringToTop();

	SetDirty(false);

	if (DsDiagram* diagram = Diagram())
		diagram->DopeSheet()->Context()->SelectionChanged();
}

void DsKey::Deselect()
{
	if (!_isSelected)
		return;

	DsDiagram* diagram = Diagram();

	if (!diagram)
		return;

	_isSelected = false;

	XtUt::Detach(&_linkSelection);

	SetDirty(false);

	if (DsDiagram* diagram = Diagram())
		diagram->DopeSheet()->Context()->SelectionChanged();
}

void DsKey::SetDirty(bool redoLayout)
{
	if (_row)
		_row->SetDirty(redoLayout);
}