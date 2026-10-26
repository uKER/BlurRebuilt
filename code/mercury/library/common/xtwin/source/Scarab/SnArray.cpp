#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnArray::SnArray(LfArray* field) :
	_field(field),
	_header(0),
	_body(0),
	_builtBody(false)
{
	_dom.SetCallback(DomCallback, this);

	_borderVisible = true;
}

SnArray::~SnArray()
{
}

bool SnArray::OnPopulate()
{
	assert(!_header && !_body);

	if ((_header = Add(new SnArrayHeader(this))) == 0)
		return false;

	if ((_body = Add(new ScarabNode)) == 0)
		return false;

	if (!_header->Populate())
		return false;

	SyncBody();

	return true;
}

void SnArray::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireArray(parentDom, _field->name));
}

void SnArray::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireArray(parentDom, index));
}

ScarabClipTarget SnArray::ClipTarget()
{
	return ScarabClipTarget(AsArray(_dom), _field);
}

void SnArray::OnIndexChanged()
{
	if (!_body)
		return;

	SyncBody();

	if (Xt::DomArray* array = AsArray(_dom))
		GenerateIdForComponent(AsComponent(At(array, _header->Index())), false);
}

void SnArray::SetDom(Xt::DomArray* array)
{
	SetEnabled(Monitor(&_dom, array) != 0);

	_header->SyncControls(true);

	OnIndexChanged();
}

void SnArray::SyncBody()
{
	Xt::DomArray* arr = AsArray(_dom);

	if (_field->attrs.FindAttr("CompactArray") && (Count(arr) <= 0))
	{
		_header->SetMiniMode(true);

		_borderVisible = false;

		if (_builtBody)
		{
			_body->Clear();
			_builtBody = false;

			Panel()->DirtyLayout();
		}

		return;
	}

	if (_builtBody)
	{
		_body->Bind(AsArray(_dom), _header->Index());
		return;
	}

	_header->SetMiniMode(false);

	_borderVisible = true;

	_builtBody = true;

	if (ScarabBuild(_body, _field->field))
	{
		for(XtUt::List<ScarabNode>::Iterator it(&_body->_nodes); it; ++it)
		{
			it->_borderVisible	= false;
			it->_labelVisible	= false;
		}

		_body->Populate();
	}

	_body->Bind(AsArray(_dom), _header->Index());

	Panel()->DirtyLayout();
}

void SnArray::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnArray* node)
{
	if ((event->from == monitor->element) && (event->code == Xt::kDomRemoved))
		node->SetDom(0);
	else if (*monitor == event->from)
	{
		node->_header->SyncControls(true);
		node->OnIndexChanged();
	}
}