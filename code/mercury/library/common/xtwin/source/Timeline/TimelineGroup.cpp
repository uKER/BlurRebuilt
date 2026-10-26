#include "Core/XtWinPch.h"
#include "TimelineImpl.h"

TlGroup::TlGroup() :
	_anchor(this),
	_diagram(0),
	_linkDiagram(this),
	_colour(0xFFAABBDD),
	_y(0.0f),
	_height(10.0f)
{
}

TlGroup::~TlGroup()
{
	Clear();

	SetDirty(true);
}

void TlGroup::SetLabel(const Sz16& label)
{
	if (_label != label)
	{
		_label = label;

		SetDirty(true);
	}
}

void TlGroup::SetColour(XtU32 colour)
{
	if ((colour & 0xFFFFFF) != (_colour & 0xFFFFFF))
	{
		_colour = (colour & 0xFFFFFF) | 0xFF000000;

		SetDirty(false);
	}
}

void TlGroup::Detach()
{
	if (TlDiagram* diagram = _diagram)
	{
		XtUt::Detach(&_linkDiagram);

		_diagram = 0;

		diagram->SetDirty(true);
	}
}

TlRow* TlGroup::AddRow(TlRow* row)
{
	if (!row)
		return 0;

	row->_group = this;

	XtUt::PushBack(&_rows, &row->_linkGroup);

	SetDirty(true);

	return row;
}

TlRow* TlGroup::InsertRow(TlRow* row, TlRow* after)
{
	if (!row)
		return 0;

	row->_group = this;

	if (after && (after->_group == this))
		XtUt::AttachAfter(&row->_linkGroup, &after->_linkGroup);
	else
		XtUt::PushFront(&_rows, &row->_linkGroup);

	SetDirty(true);

	return row;
}

void TlGroup::Clear()
{
	Abandon(&_rows);
}

void TlGroup::SetDirty(bool redoLayout)
{
	if (_diagram)
		_diagram->SetDirty(redoLayout);
}