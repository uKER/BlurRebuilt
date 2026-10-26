#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnStruct::SnStruct(LfStruct* field) :
	_field(field)
{
	LarvaTypeLayout layout = (field->layout == kLtlDefault) ? field->type->layout : field->layout;
	LarvaTypeBorder border = (field->border == kLtbDefault) ? field->type->border : field->border;
	
	switch(layout)
	{
		case kLtlDefault:
		case kLtlVertical:
			_layoutMethod = kSlVertical;
		break;

		case kLtlHorizontal:
			_layoutMethod = kSlHorizontal;
		break;

		case kLtlInline:
			_layoutMethod = kSlInline;
		break;
	}

	switch(border)
	{
		case kLtbNone:
			_borderVisible = false;
		break;

		case kLtbDefault:
			_borderVisible = _layoutMethod != kSlInline;
		break;

		case kLtbNormal:
			_borderVisible = true;
		break;
	}
}

SnStruct::~SnStruct()
{
}

bool SnStruct::OnPopulate()
{
	return ScarabBuild(this, _field->type, false, "") && ScarabNode::OnPopulate();
}

void SnStruct::OnBind(Xt::DomBlock* parentDom)
{
	Xt::DomBlock* dom = AcquireBlock(parentDom, _field->name);

	SetEnabled(Monitor(&_dom, dom) != 0);

	ScarabNode::OnBind(dom);
}

void SnStruct::OnBind(Xt::DomArray* parentDom, int index)
{
	Xt::DomBlock* dom = AcquireBlock(parentDom, index);

	SetEnabled(Monitor(&_dom, dom) != 0);

	ScarabNode::OnBind(dom);
}

void SnStruct::OnBind(Xt::DomComponent* parentDom)
{
	ScarabNode::OnBind(parentDom ? &parentDom->data : 0);
}

void SnStruct::OnMeasure(HDC ic)
{
	ScarabNode::OnMeasure(ic);

	if (_nodes.count > 0)
	{
		LarvaTypeLayout layout = (_field->layout == kLtlDefault) ? _field->type->layout : _field->layout;
		LarvaTypeBorder border = (_field->border == kLtbDefault) ? _field->type->border : _field->border;

		if ((layout == kLtlHorizontal) && (border == kLtbNone))
			_anchor.x = _labelAlignMargin;
	}
}

ScarabClipTarget SnStruct::ClipTarget()
{
	return ScarabClipTarget(AsBlock(_dom), _field);
}