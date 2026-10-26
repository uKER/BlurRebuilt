#include "Core/XtWinPch.h"
#include "TimelineImpl.h"

TlKey::TlKey() :
	_anchor(this),
	_row(0),
	_linkRow(this),
	_isSelected(0),
	_linkSelection(this),
	_time(0.0f),
	_duration(0.0f),
	_colour(0xFF202020)
{
}

TlKey::~TlKey()
{
	if (_isSelected)
		Deselect();

	SetDirty(true);
}

void TlKey::OnTimeChanged()
{
}

void TlKey::SetTime(float time)
{
	if (time != _time)
	{
		_time = time;

		SetDirty(true);
	}
}

void TlKey::SetDuration(float duration)
{
	if (duration != _duration)
	{
		_duration = duration;

		SetDirty(true);
	}
}

void TlKey::SetLabel(const Sz16& label)
{
	if (_label != label)
	{
		_label = label;

		SetDirty(false);
	}
}

void TlKey::SetColour(XtU32 colour)
{
	XtU32 newColour = (colour & 0xFFFFFF) | 0xFF000000;

	if (_colour != newColour)
	{
		_colour = newColour;

		SetDirty(false);
	}
}

void TlKey::Detach()
{
	if (TlRow* row = _row)
	{
		XtUt::Detach(&_linkRow);

		_row = 0;

		row->SetDirty(true);
	}
}

TlDiagram* TlKey::Diagram() const
{
	return _row ? _row->Diagram() : 0;
}

void TlKey::MoveBy(const NgVector& offset)
{
	_time += offset.vx;

	if (TlDiagram* diagram = Diagram())
	{
		if (diagram->_quantise)
		{
			float q = diagram->_quantisation;

			_time = ((int)((_time /q) + 0.5f)) * q;
		}
	}

	OnTimeChanged();

	SetDirty(true);	
}

void TlKey::Select()
{
	if (_isSelected)
		return;

	TlDiagram* diagram = Diagram();

	if (!diagram)
		return;

	_isSelected = true;

	XtUt::PushFront(&diagram->_selectionKeys, &_linkSelection);

	//BringToTop();

	SetDirty(false);

	if (TlDiagram* diagram = Diagram())
		diagram->Timeline()->Context()->SelectionChanged();
}

void TlKey::Deselect()
{
	if (!_isSelected)
		return;

	TlDiagram* diagram = Diagram();

	if (!diagram)
		return;

	_isSelected = false;

	XtUt::Detach(&_linkSelection);

	SetDirty(false);

	if (TlDiagram* diagram = Diagram())
		diagram->Timeline()->Context()->SelectionChanged();
}

void TlKey::SetDirty(bool redoLayout)
{
	if (_row)
		_row->SetDirty(redoLayout);
}