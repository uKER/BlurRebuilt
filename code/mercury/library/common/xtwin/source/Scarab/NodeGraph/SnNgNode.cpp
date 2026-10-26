#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/NodeGraph.h"
#include "../ScarabImpl.h"
#include "SnNgImpl.h"

SnNgNode::SnNgNode(ScarabDomNodeGraph* ng, LarvaField* field, LtStruct* type, Xt::DomComponent* component, Xt::Indexer* indexer, Xt::DomElement* parentDom) :
	_link(this),
	_ng(ng),
	_field(field),
	_type(type),
	_dom(DomCallback, this),
	_ignoreNotifications(0),
	_out(0),
	_indexer(indexer)
{
	// TODO: Remove _type and replace with lookup function?

	Monitor(&_dom, component);
	Monitor(&_parentDom, parentDom);

	_id = component->id;

	XtUt::PushBack(&ng->_nodes, &_link);

	if (type->GetBool("NodeGraph", "pinEnabled", true))
	{
		if ((_out = AddPin(new SnNgPin(_ng, 0, 0, 0, 0, 0))) != 0)
		{
			_out->SetLabel(MbToWc(type->GetString("NodeGraph", "pinLabel", "Out")));

			_out->SetColour(type->GetColour("NodeGraph", "pinColour", _out->Colour()));
			_out->SetAlign((_stricmp(type->GetString("NodeGraph", "pinAlign", "right"), "left") == 0) ? kNgAlignLeft : kNgAlignRight);

			if (_type && _type->GetBool("NodeGraph", "pinArrow", false))
			{
				_out->SetFromArrow(true);
				_out->SetToArrow(true);
			}
		}
	}

	LoadPosition();
	LoadName();

	SetColour(type->GetColour("NodeGraph", "nodeColour", Colour()));

	_ng->MegaUndoHack_CheckPoint();
}

SnNgNode::~SnNgNode()
{
	_ng->MegaUndoHack_CheckPoint();

	delete _indexer;
}

void SnNgNode::OnPositionChanged()
{
	SavePosition();
}

void SnNgNode::OnDoubleClicked()
{
	DoDblClkAction(_type, AsComponent(_dom));
}

bool SnNgNode::DoDblClkAction(LtStruct* type, Xt::DomComponent* com)
{
	if (!type || !com)
		return false;

	for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
	{
		if (it->attrs.GetString("NodeGraph", "action") == "open")
		{
			XtId id = Xt::ParseId(AsText(At(&com->data, it->name)));

			if (Xt::DomRecord* record = FindRecordById(_ng->_form->Db(), id))
				_ng->_form->FormContext()->OpenRecord(record);

			return true;
		}
	}

	if (type->base)
	{
		if (DoDblClkAction(type->base->type, com))
			return true;
	}

	return false;
}

void SnNgNode::LoadPosition()
{
	if (OnlyOnce once = _ignoreNotifications)
	{
		if (Xt::DomComponent* component = AsComponent(_dom))
		{
			NgVector pt(
					(float)atof(AsText(At(&component->data, "__x"))),
					(float)atof(AsText(At(&component->data, "__y")))
				);

			SetPosition(pt);

			if (_type)
			{
				bool mirrored = atoi(AsText(At(&component->data, "__flipped"))) != 0;

				SetMirrored(mirrored ^ _type->GetBool("NodeGraph", "nodeFlip", false));
			}
		}
	}
}

void SnNgNode::LoadName()
{
	if (OnlyOnce once = _ignoreNotifications)
	{
		if (Xt::DomComponent* component = AsComponent(_dom))
		{
			if (Xt::DomDb* db = _ng->_form->Db())
			{
				{
					SnNgFormatter fmtr("nodeLabel");

					XtString format;

					if (!fmtr.GetFormatByType(_type, format))
						format = "{__type}";

					XtString text = EvaluateStructuredText(db, db->schema, _type, component, format, &fmtr);

					SetLabel(MbToWc(text));
				}

				if (_out)
				{
					XtString format	= _type->GetString("NodeGraph", "pinLabel", "Out");
					XtString text	= EvaluateStructuredText(db, db->schema, _type, component, format, &SnNgFormatter("pinLabel"));

					_out->SetLabel(MbToWc(text));
				}
			}
			else
			{
				SetLabel(MbToWc(_type->label));

				if (_out)
					_out->SetLabel(MbToWc(_type->GetString("NodeGraph", "pinLabel", "Out")));
			}
		}
	}
}

void SnNgNode::LoadType()
{
	if (OnlyOnce once = _ignoreNotifications)
	{
		if (Xt::DomDb* db = _ng->_form->Db())
		{
			if (Xt::DomComponent* component = AsComponent(_dom))
			{
				_type = AsStruct(TypeOf(db->schema, _field, component));
			}
		}
	}
}

void SnNgNode::SavePosition()
{
	if (OnlyOnce once = _ignoreNotifications)
	{
		if (Xt::DomComponent* component = AsComponent(_dom))
		{
			const NgRect& l = Location();

			Set(&component->data, "__x", Format("%g", l.pt.vx));
			Set(&component->data, "__y", Format("%g", l.pt.vy));
		}
	}
}

void SnNgNode::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnNgNode* node)
{
	node->LoadType();

	if (node->_type == 0)
	{
		delete node;
		return;
	}

	node->LoadPosition();
	node->LoadName();

	if ((event->code == Xt::kDomRemoved) && (event->from == node->_dom))
		delete node;
}