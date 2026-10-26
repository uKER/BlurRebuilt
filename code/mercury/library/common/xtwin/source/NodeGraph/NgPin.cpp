#include "Core/XtWinPch.h"
#include "NgImpl.h"

/*
	TODO: Allow user to filter/hide nodes, when this is done any connections to hidden nodes show up like this:
	____
		\
		|
		O-- - - - HiddenNodeName
		|
	____/
*/

NgPin::NgPin() :
	_anchor(this),
	_node(0),
	_link(this),
	_colour(0xFFD0D0D0),
	_align(kNgAlignRight),
	_fromArrow(false),
	_toArrow(false)
{
}

NgPin* NgPin::OnDrag()
{
	return this;
}

bool NgPin::OnTryConnect(NgPin* to)
{
	return false;
}

void NgPin::OnConnect(NgPin* to)
{
}

NgPin::~NgPin()
{
	while(NgConnection* connection = XtUt::PopFront(&_connections))
		delete connection;

	SetDirty(true);
}

NgDiagram* NgPin::Diagram()
{
	return _node ? _node->Diagram() : 0;
}

void NgPin::SetLabel(const Sz16& label)
{
	_label = label;

	SetDirty(true);
}

void NgPin::SetColour(XtU32 colour)
{
	_colour = (colour & 0xFFFFFF) | 0xFF000000;

	SetDirty(false);
}

void NgPin::SetAlign(NgAlign align)
{
	_align = align;

	SetDirty(true);
}

void NgPin::SetFromArrow(bool enabled)
{
	_fromArrow = enabled;

	SetDirty(false);
}

void NgPin::SetToArrow(bool enabled)
{
	_toArrow = enabled;

	SetDirty(false);
}

int NgPin::ConnectionCount() const
{
	int count = 0;	

	for(XtUt::LinkList<NgConnection>::ConstIterator itConnection(&_connections); itConnection; ++itConnection)
		count++;

	return count;
}

void NgPin::SetDirty(bool redoLayout)
{
	if (_node)
		_node->SetDirty(redoLayout);
}

void NgPin::Measure(NgRenderContext* context)
{
	_labelSize = context->Measure(_label, &context->pinFont);
}

NgAlign NgPin::NodeAlign() const
{
	if (_node->_mirrored)
		return (_align == kNgAlignLeft) ? kNgAlignRight : kNgAlignLeft;

	return _align;
}