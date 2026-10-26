#include "Core/XtWinPch.h"
#include "NgImpl.h"

NgConnection::NgConnection() :
	_anchor(this),
	_diagram(0),
	_linkDiagram(this),
	_from(0),
	_linkFrom(this),
	_to(0),
	_linkTo(this),
	_colour(0xFF000000),
	_virtual(kNgVirtualNone)
{
}

NgConnection::~NgConnection()
{
}

bool NgConnection::IsConflicted()
{
	return false;
}

bool NgConnection::IsHot()
{
	return false;
}

void NgConnection::SetColour(XtU32 colour)
{
	_colour = (colour & 0xFFFFFF) | 0xFF000000;

	if (_diagram)
		_diagram->SetDirty(false);
}

void NgConnection::SetVirtual(NgVirtual v)
{
	if (_virtual != v)
	{
		_virtual = v;

		if (_diagram)
			_diagram->SetDirty(false);
	}
}

bool NgConnection::SetFrom(NgPin* from)
{
	if (!from || !_diagram || (from->Diagram() != _diagram))
		return false;

	_from = from;

	XtUt::PushBack(&from->_connections, &_linkFrom);

	_diagram->SetDirty(true);

	return true;
}

bool NgConnection::SetTo(NgPin* to)
{
	if (!to || !_diagram || (to->Diagram() != _diagram))
		return false;

	_to = to;

	XtUt::PushBack(&to->_connections, &_linkTo);

	_diagram->SetDirty(true);

	return true;
}