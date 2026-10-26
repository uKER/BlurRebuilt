#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnPtr::SnPtr(LfPtr* field) :
	_field(field),
	_header(0),
	_body(0)
{
	LarvaTypeLayout layout = (field->layout == kLtlDefault) ? field->baseType->layout : field->layout;
	LarvaTypeBorder border = (field->border == kLtbDefault) ? field->baseType->border : field->border;
	
	switch(layout)
	{
		case kLtlDefault:
			_layoutMethod = IsCompact() ? kSlHorizontal : kSlVertical;
		break;

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

SnPtr::~SnPtr()
{
}

bool SnPtr::OnPopulate()
{
	assert(!_header && !_body);

	bool compact = IsCompact();

	if (!compact)
	{
		if ((_header = Add(new SnPtrHeader(this))) == 0)
			return false;
	}

	if ((_body = Add(new SnPtrContainer)) == 0)
		return false;

	_body->_layoutMethod = kSlInline;

	if (compact)
	{
		if ((_header = Add(new SnPtrCompactHeader(this, IsCompactHyperLink()))) == 0)
			return false;
	}

	return true;
}

void SnPtr::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireComponent(parentDom, _field->name));
}

void SnPtr::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireComponent(parentDom, index));
}

void SnPtr::OnMeasure(HDC ic)
{
	ScarabNode::OnMeasure(ic);

	if (IsCompact() && _body && (_body->_nodes.count > 0))
	{
		if ((_layoutMethod == kSlHorizontal) && !_borderVisible)
			_anchor.x = _labelAlignMargin;
	}
}

ScarabClipTarget SnPtr::ClipTarget()
{
	return ScarabClipTarget(AsComponent(_dom), _field);
}

void SnPtr::SetDom(Xt::DomComponent* dom)
{
	SetEnabled(Monitor(&_dom, dom) != 0);

	GenerateIdForComponent(dom, false);

	PopulateBody();

	ScarabNode::OnBind(dom);

	Panel()->DirtyLayout();
}

void SnPtr::OnTypeChanged()
{
	PopulateBody();

	_body->Bind(AsComponent(_dom));

	Panel()->DirtyLayout();
}

bool SnPtr::PopulateBody()
{
	if (!_body)
		return false;

	_body->Clear();

	if (LtStruct* type = TypeOf(Panel()->Db()->schema, _field->baseType, AsComponent(_dom)))
	{
		_borderColour = type->GetColour("Border", "colour", 0);

		if (!ScarabBuild(_body, type, false, ""))
			return false;

		if (!_body->Populate())
			return false;
	}
	else
		_borderColour = _field->baseType->GetColour("Border", "colour", 0);

	return true;
}

bool SnPtr::IsCompact()
{
	return _field->attrs.FindAttr("CompactPtr") || (_field->baseType && _field->baseType->FindAttr("CompactPtr"));
}

bool SnPtr::IsCompactHyperLink()
{
	if (LarvaAttr* attr = _field->attrs.FindAttr("CompactPtr"))
		return attr->GetBool("hyperLink", false);

	if (_field->baseType && _field->baseType->FindAttr("CompactPtr"))
		return _field->baseType->attrs.GetBool("CompactPtr", "hyperLink", false);

	return false;
}

void SnPtrContainer::OnBind(Xt::DomComponent* parentDom)
{
	ScarabNode::OnBind(parentDom ? &parentDom->data : 0);
}