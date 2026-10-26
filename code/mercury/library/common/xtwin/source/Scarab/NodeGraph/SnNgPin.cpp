#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/NodeGraph.h"
#include "../ScarabImpl.h"
#include "SnNgImpl.h"

SnNgPin::SnNgPin(ScarabDomNodeGraph* ng, LfWeakPtr* weak, Xt::DomRef* ref, LarvaField* field, LtStruct* parentType, Xt::DomBlock* parentDom) :
	_ng(ng),
	_weak(weak),
	_dom(DomCallback, this),
	_parentDom(DomCallback, this),
	_ignoreNotifications(0),
	_field(field),
	_parentType(parentType)
{
	Monitor(&_dom, ref);
	Monitor(&_parentDom, parentDom);

	if (weak)
	{
		if (LarvaParam* param = weak->attrs.FindParam("NodeGraph", "pinLabel"))
			SetLabel(MbToWc(GetPinLabelText(param->AsString())));
		else if (!*weak->label)
			SetLabel(MbToWc(weak->baseType->label));
		else
			SetLabel(MbToWc(weak->label));

		SetColour(weak->baseType->GetColour("NodeGraph", "pinColour", Colour()));

		SetAlign((_stricmp(weak->attrs.GetString("NodeGraph", "pinAlign", "left"), "right") == 0) ? kNgAlignRight : kNgAlignLeft);

		if (_weak && _weak->attrs.GetBool("NodeGraph", "pinArrow", false))
		{
			SetFromArrow(true);
			SetToArrow(true);
		}
	}

	_ng->MegaUndoHack_CheckPoint();
}

SnNgPin::~SnNgPin()
{
	_ng->MegaUndoHack_CheckPoint();
}

Xt::DomRef* SnNgPin::Field()
{
	return AsRef(_dom);
}

void SnNgPin::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnNgPin* pin)
{
	if ((event->code == Xt::kDomRemoved) && (event->from == pin->_dom))
		delete pin;
	else
	{
		if (pin->_weak)
		{
			if (LarvaParam* param = pin->_weak->attrs.FindParam("NodeGraph", "pinLabel"))
				pin->SetLabel(MbToWc(pin->GetPinLabelText(param->AsString())));
		}
	}
}

XtString SnNgPin::GetPinLabelText(const XtString& format)
{
	if (Xt::DomDb* db = DbOf(_dom))
	{
		if (Xt::DomRef* ref = AsRef(_dom))
		{
			XtString text = EvaluateStructuredText(db, db->schema, _parentType, AsBlock(_parentDom), format, &SnNgFormatter("pinLabel"));

			return text;
		}
	}

	if (!*_weak->label)
		return _weak->baseType->label;

	return _weak->label;
}

NgPin* SnNgPin::OnDrag()
{
	if (_weak == 0)
		return this;

	NgConnection* con = 0;

	for(XtUt::LinkList<NgConnection>::Iterator it(Connections()); it; ++it)
	{
		con = it;
		break;
	}

	NgPin* start = this;

	if (con)
	{
		if (con->From() == this)
		{
			start = con->To();

			if (Xt::DomRef* ref = AsRef(_dom))
				Set(ref, 0);
		}
	}

	return start;
}

bool SnNgPin::OnTryConnect(NgPin* to)
{
	if (!to)
		return false;

	bool lhsOut = _weak == 0;
	bool rhsOut = ((SnNgPin*)to)->_weak == 0;

	if (!(lhsOut ^ rhsOut))
		return false;

	LfWeakPtr*	weak;
	LtStruct*	type;

	if (_weak == 0)
	{
		type = ((SnNgNode*)Node())->_type;
		weak = ((SnNgPin*)to)->_weak;
	}
	else
	{
		type = ((SnNgNode*)to->Node())->_type;
		weak = _weak;
	}

	return IsBaseOf(weak->baseType, type);
}

void SnNgPin::OnConnect(NgPin* to)
{
	if (!OnTryConnect(to))
		return;

	SnNgPin*	fromPin;
	SnNgNode*	toNode;

	if (_weak == 0)
	{
		fromPin	= (SnNgPin*)to;
		toNode	= (SnNgNode*)Node();
	}
	else
	{
		fromPin	= this;
		toNode	= (SnNgNode*)to->Node();
	}

	if (!fromPin || !toNode)
		return;

	if (Xt::DomRef* ref = AsRef(fromPin->_dom))
	{
		Set(ref, toNode->_id);
	}
}

void SnNgPin::CreateConnections()
{
	NgConnection* con = 0;

	if (ConnectionCount() > 0)
	{
		for(XtUt::LinkList<NgConnection>::Iterator it(Connections()); it; ++it)
		{
			if (it->From() == this)
			{
				con = it;
				break;
			}
		}
	}

	if (Xt::DomRef* ref = AsRef(_dom))
	{
		SnNgNode* to = _ng->FindNode(AsId(ref));

		if (to)
		{
			bool newlyCreated = con == 0;

			if (!con && ((con = Diagram()->AddConnection(new SnNgConnection(_ng))) == 0))
				return;

			if ((con->From() != this) || (con->To() != to->_out))
			{
				_ng->MegaUndoHack_CheckPoint();
			}

			con->SetFrom(this);
			con->SetTo(to->_out);

			con->SetColour(_weak->attrs.GetColour("NodeGraph", "connectionColour", 0x000000));

			if (newlyCreated)
				con->SetVirtual(_weak->attrs.GetBool("NodeGraph", "connectionVirtual", false) ? kNgVirtualFrom : kNgVirtualNone);
		}
		else
			delete con;
	}
	else
		delete con;
}