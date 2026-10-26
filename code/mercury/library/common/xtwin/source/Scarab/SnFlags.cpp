#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnFlags::SnFlags(LfFlags* field) :
	_field(field)
{
	_borderVisible	= true;
	_spacing		= 5;
}

SnFlags::~SnFlags()
{
}

bool SnFlags::OnPopulate()
{
	for(XtUt::List<LarvaOption>::Iterator it(&_field->type->options); it; ++it)
		if (!Add(new SnFlagBool(it)))
			return false;

	return true;
}

void SnFlags::OnBind(Xt::DomBlock* parentDom)
{
	Xt::DomBlock* dom = AcquireBlock(parentDom, _field->name);

	SetEnabled(Monitor(&_dom, dom) != 0);

	ScarabNode::OnBind(dom);
}

void SnFlags::OnBind(Xt::DomArray* parentDom, int index)
{
	Xt::DomBlock* dom = AcquireBlock(parentDom, index);

	SetEnabled(Monitor(&_dom, dom) != 0);

	ScarabNode::OnBind(dom);
}

ScarabClipTarget SnFlags::ClipTarget()
{
	return ScarabClipTarget(AsBlock(_dom), _field);
}