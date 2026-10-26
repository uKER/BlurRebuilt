#include "Core/XtWinPch.h"
#include "TimelineImpl.h"

TlRow::TlRow() :
	_anchor(this),
	_diagram(0),
	_linkDiagram(this),
	_group(0),
	_linkGroup(this),
	_isSelected(false),
	_linkSelection(this),
	_colour(0xFFAABBDD),
	_y(0.0f),
	_height(20.0f)
{
}

TlRow::~TlRow()
{
	if (_isSelected)
		Deselect();

	SetDirty(true);
}

void TlRow::SetLabel(const Sz16& label)
{
	if (_label != label)
	{
		_label = label;

		SetDirty(true);
	}
}

void TlRow::SetColour(XtU32 colour)
{
	if ((colour & 0xFFFFFF) != (_colour & 0xFFFFFF))
	{
		_colour = (colour & 0xFFFFFF) | 0xFF000000;

		SetDirty(false);
	}
}

void TlRow::Detach()
{
	if (TlDiagram* diagram = _diagram)
	{
		XtUt::Detach(&_linkDiagram);

		_diagram = 0;

		diagram->SetDirty(true);
	}
}

TlKey* TlRow::AddKey(TlKey* key)
{
	if (!key || (key->_row != 0))
		return 0;

	key->_row = this;

	XtUt::PushBack(&_keys, &key->_linkRow);

	SetDirty(true);

	return key;
}

TlKey* TlRow::InsertKey(TlKey* key, TlKey* after)
{
	if (!key || (key->_row != 0))
		return 0;

	key->_row = this;

	if (after && (after->_row == this))
		XtUt::AttachAfter(&key->_linkRow, &after->_linkRow);
	else
		XtUt::PushFront(&_keys, &key->_linkRow);

	SetDirty(true);

	return key;
}

void TlRow::Clear()
{
	Free(&_keys);
}

void TlRow::Select()
{
	if (_isSelected || !_diagram)
		return;

	_isSelected = true;

	XtUt::PushFront(&_diagram->_selectionRows, &_linkSelection);

	SetDirty(false);

	_diagram->Timeline()->Context()->SelectionChanged();
}

void TlRow::Deselect()
{
	if (!_isSelected || !_diagram)
		return;

	_isSelected = false;

	XtUt::Detach(&_linkSelection);

	SetDirty(false);

	_diagram->Timeline()->Context()->SelectionChanged();
}

void TlRow::SetDirty(bool redoLayout)
{
	if (_diagram)
		_diagram->SetDirty(redoLayout);
}