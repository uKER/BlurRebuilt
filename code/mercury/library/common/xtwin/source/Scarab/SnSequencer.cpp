#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnSequencer::SnSequencer(LfControl* field) :
	SnControl(field),
	_domRoot(DomCallback, this),
	_domBody(DomCallback, this),
	_header(0),
	_body(0)
{
	_layoutMethod	= kSlHorizontal;
	_borderVisible	= true;
}

SnSequencer::~SnSequencer()
{
}

bool SnSequencer::OnPopulate()
{
	assert(!_header && !_body);

	if ((_header = Add(new SnSequencerHeader(this))) == 0)
		return false;

	if ((_body = Add(new ScarabNode)) == 0)
		return false;

	// Populate body?

	return ScarabNode::OnPopulate();
}

void SnSequencer::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnSequencer::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

void SnSequencer::SetBody(Xt::DomElement* element, int index, LarvaField* field)
{
	_body->Clear();

	ScarabBuild(_body, field);

	_body->Populate();

	if (index == -1)
		_body->Bind(AsBlock(element));
	else
		_body->Bind(AsArray(element), index);

	Panel()->DirtyLayout();
}

void SnSequencer::SetDom(Xt::DomBlock* block)
{
	SetEnabled(Monitor(&_domRoot, block) != 0);

	if (_header)
		_header->Bind(block);
}

void SnSequencer::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnSequencer* seq)
{
	if ((event->code == Xt::kDomRemoved) && (event->from == seq->_domRoot))
		seq->SetDom(0);
}