#include "Core/XtWinPch.h"
#include "XtWin/WndCore.h"
#include "XtWin/Core.h"
#include "XtWin/HyperView.h"
#include "XtWin/NodeGraph.h"

//
// HyperViewNgContext
//

class HyperViewNgContext : public NodeGraphContext
{
	HyperViewForm* _hv;

public:
	HyperViewNgContext(HyperViewForm* hv) :
		_hv(hv)
	{
	}

	virtual ~HyperViewNgContext()
	{
	}

public:
	virtual void ContextMenu(const WinPoint& pt)
	{
		_hv->GoBack();
	}

public:
	virtual void SelectionChanged()
	{
	}

	virtual void CheckPoint()
	{
	}

public:
	virtual void Undo()
	{
	}

	virtual void Redo()
	{
	}

	virtual void Delete()
	{
	}
};

//
// HyperViewForm
//

HyperViewForm::HyperViewForm(Xt::DomDb* db) :
	_db(db),
	_graph(0)
{
}

HyperViewForm::~HyperViewForm()
{
	delete _graph;
}

bool HyperViewForm::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->dwExStyle |= WS_EX_CLIENTEDGE;

	return true;
}

LRESULT HyperViewForm::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			//SendMessage(*this, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_SORCERY)));

			WinRect rcClient(*this);

			if ((_graph = new NodeGraph(new HyperViewNgContext(this))) == 0)
				return -1;

			if (!_graph->Create(rcClient, *this))
				return -1;

			//UpdateWindowTitle();

			Populate();
		}
		break;

		case WM_SIZE:
		{
			WinRect rcClient(*this);

			SetWindowPos(*_graph, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		}
		break;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

void HyperViewForm::SetDom(Xt::DomElement* element)
{
	switch (element->kind)
	{
	case(Xt::kDomRecord):
		{
			Monitor(&_component, &Xt::AsRecord(element)->root);
			break;
		}
	case (Xt::kDomComponent):
		{
			Monitor(&_component, Xt::AsComponent(element));
			break;
		}
	default:
		Monitor(&_component, 0);
	}

	Free(&_history);

	Populate();
}

void HyperViewForm::GoForward(XtId recordId)
{
	if (!recordId)
		return;

	Xt::DomDb* db = _db;

	Xt::DomRecord* record = FindRecordById(db, recordId);

	if (record)
	{
		if (Xt::DomComponent* existing = Xt::AsComponent(_component))
			XtUt::AppendEx(&_history, Item(existing->id, _graph->Diagram()->TransformOffset(), _graph->Diagram()->TransformScale()));

		Monitor(&_component, record);

		Populate();

		_graph->Diagram()->Frame();
	}
}

void HyperViewForm::GoBack()
{
	Xt::DomDb* db = _db;

	for(;;)
	{
		int count = Count(&_history);

		if (count < 1)
			return;

		Item* pItem = At(&_history, count - 1);

		if (!pItem)
			return;

		Item item = *pItem;

		XtUt::Remove(&_history, _history.count - 1);

		/*
		if (Xt::DomComponent* record = FindRecordById(db, item.id))
		{
			Monitor(&_element, record);

			Populate();

			_graph->Diagram()->SetTransformOffset(item.offset);
			_graph->Diagram()->SetTransformScale(item.scale);

			break;
		}
		*/
	}
}

void HyperViewForm::AdjustSizeToForm()
{
}

void HyperViewForm::UpdateWindowTitle()
{
	SetWindowText(*this, "(experimental)");
}

//
// Populate
//

XtU32 GenerateColourFromString(const char* text)
{
	XtU32 rgb = 0x9E3779B9;

	for(const char* p = text; *p != '\0'; p++)
	{
		XtU32 v = (XtU32)*p;

		rgb ^= ((rgb * 16127) ^ (rgb * 37));
		rgb += (v * 3571) ^ (v * 16127);
		rgb -= (rgb >> 12) ^ (rgb << 12);
		rgb ^= rgb >> 14;
	}

	int r = Clamp<int>(80 + ((rgb & 0xFF) / 2), 0, 255);
	int g = Clamp<int>(80 + (((rgb & 0xFF00) >> 8) / 2), 0, 255);
	int b = Clamp<int>(80 + (((rgb & 0xFF0000) >> 16) / 2), 0, 255);

	NgColour c(r / 255.0f, g / 255.0f, b / 255.0f);

	c = RgbToHsv(c);

	c.g = 0.5f;
	c.b = 0.9f;

	c = HsvToRgb(c);

	r = Clamp((int)(c.r * 255), 0, 255);
	g = Clamp((int)(c.g * 255), 0, 255);
	b = Clamp((int)(c.b * 255), 0, 255);

	return RGB(r, g, b);
}

class HvNode : public NgNode
{
	HyperViewForm*	_hv;

public:
	HvNode(HyperViewForm* hv, XtId componentId) :
		_hv(hv),
		_componentId(componentId)
	{
	}

	void OnDoubleClicked()
	{
		_hv->GoForward(_componentId);
	}

	XtId				_componentId;
};

struct HyperViewBuilder
{
	HyperViewForm*	_hv;
	Xt::DomDb*			_db;
	LarvaSchema*		_schema;
	NodeGraph*			_graph;
	float _y;

public:
	HyperViewBuilder(HyperViewForm* hv, Xt::DomDb* db, LarvaSchema* schema, NodeGraph* graph) :
		_hv(hv),
		_db(db),
		_schema(schema),
		_graph(graph),
		_y(0.0f)
	{
	}

public:
	NgNode* CreateNode(NgPin* parent, LtStruct* type, Xt::DomComponent* component, XtId componentId, XtU32 conColour)
	{
		NgNode* node = _graph->Diagram()->AddNode(new HvNode(_hv, componentId));

		if (!node)
			return 0;

		node->SetLabel(MbToWc(EvaluateSummaryText(_db, _schema, type, component, (componentId != 0) ? "{__name}" : "{__type}")));
		node->SetColour(GenerateColourFromString(type->name));
		//node->SetCompact(componentId != 0);

		if (parent)
		{
			NgVector pos = parent->Node()->Location().pt;

			pos.vx += 250.0f;
			pos.vy = _y;

			node->SetPosition(pos);
		}
		else
			node->SetPosition(NgVector(0.0f, _y));

		_y += 45.0f;

		if (parent)
		{
			if (NgPin* pin = node->AddPin(new NgPin))
			{
				pin->SetAlign(kNgAlignLeft);
				pin->SetColour(parent->Colour());

				if (NgConnection* connection = _graph->Diagram()->AddConnection(new NgConnection()))
				{
					connection->SetFrom(parent);
					connection->SetTo(pin);
					connection->SetColour(conColour);
				}
			}
		}

		return node;
	}
};

struct HyperViewColourVisitor : public LvField
{
	XtU32 colour;

public:
	HyperViewColourVisitor() :
		colour(0xD0D0D0)
	{
	}

public:
	virtual bool Visit(LfStruct* field)			{ colour = 0x2040FF; return true; }
	virtual bool Visit(LfInt* field)			{ colour = 0x40FF20; return true; }
	virtual bool Visit(LfFloat* field)			{ colour = 0x40FF20; return true; }
	virtual bool Visit(LfString* field)			{ colour = 0x40FF20; return true; }
	virtual bool Visit(LfWideString* field)		{ colour = 0x40FF20; return true; }
	virtual bool Visit(LfBool* field)			{ colour = 0x40FF20; return true; }
	virtual bool Visit(LfEnum* field)			{ colour = 0x40FF20; return true; }
	virtual bool Visit(LfFlags* field)			{ colour = 0x40FF20; return true; }
	virtual bool Visit(LfStrictPtr* field)		{ colour = 0x2040FF; return true; }
	virtual bool Visit(LfPtr* field)			{ colour = 0x2040FF; return true; }
	virtual bool Visit(LfWeakPtr* field)		{ colour = 0x40FF20; return true; }
	virtual bool Visit(LfHandle* field)			{ colour = 0xFF4020; return true; }
	virtual bool Visit(LfArray* field)			{ return field->field->Accept(this); }
	virtual bool Visit(LfLocString* field)		{ colour = 0x40FF20; return true; }
};

struct HyperViewSourceVisitor : public LvFieldDom
{
	HyperViewBuilder*	_builder;
	NgNode*				_parent;
	NgPin*				_pin;
	int					_levels;
	XtU32				_conColour;

public:
	HyperViewSourceVisitor(HyperViewBuilder* builder, NgNode* parent) :
		_builder(builder),
		_parent(parent),
		_pin(0),
		_levels(0),
		_conColour(0)
	{
	}

public:
	void PushPin(LarvaField* field)
	{
		if (++_levels <= 1)
		{
			if ((_pin = _parent->AddPin(new NgPin)) != 0)
			{
				_pin->SetLabel(MbToWc(field->name));

				HyperViewColourVisitor v;

				field->Accept(&v);

				_pin->SetColour(v.colour);

				_conColour = GenerateColourFromString(field->name);
			}
		}
	}

	void PopPin()
	{
		if (--_levels <= 0)
			_pin = 0;
	}

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)
	{
		return VisitMembers(field->type, dom, this, Xt::kIndexAt);
	}

	virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)
	{
		PushPin(field);

		if (dom)
		{
			NgNode* node = _builder->CreateNode(_pin, field->type, dom, 0, _conColour);

			VisitMembers(field->type, &dom->data, &HyperViewSourceVisitor(_builder, node), Xt::kIndexAt);
		}

		PopPin();

		return true;
	}

	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)
	{
		PushPin(field);

		if (LtStruct* type = TypeOf(_builder->_schema, field->baseType, dom))
		{
			NgNode* node = _builder->CreateNode(_pin, type, dom, 0, _conColour);

			VisitMembers(type, &dom->data, &HyperViewSourceVisitor(_builder, node), Xt::kIndexAt);
		}

		PopPin();

		return true;
	}

	virtual bool Visit(LfHandle* field, Xt::DomField* dom)
	{
		PushPin(field);

		if (Xt::DomRecord* record = FindRecordById(_builder->_db, Xt::ParseId(AsText(dom))))
		{
			if (LtStruct* type = TypeOf(_builder->_schema, record))
			{
				_builder->CreateNode(_pin, type, &record->root, record->id, _conColour);
			}
		}

		PopPin();

		return true;
	}

	virtual bool Visit(LfArray* field, Xt::DomArray* dom)
	{
		PushPin(field);

		VisitArrayItems(field, dom, this, Xt::kIndexAt);

		PopPin();

		return true;
	}
};

struct HyperViewDestinationVisitor : public LvFieldDom
{
	HyperViewBuilder*	_builder;
	Xt::DomComponent*	_source;
	NgNode*				_destination;
	XtU32				_conColour;

public:
	HyperViewDestinationVisitor(HyperViewBuilder* builder, NgNode* destination, Xt::DomComponent* source) :
		_builder(builder),
		_source(source),
		_destination(destination),
		_conColour(0)		
	{
	}

public:
	void AddPin(LarvaField* field, NgNode* source)
	{
		if (NgPin* sourcePin = source->AddPin(new NgPin))
		{
			sourcePin->SetLabel(MbToWc(field->name));

			HyperViewColourVisitor v;

			field->Accept(&v);

			sourcePin->SetColour(v.colour);

			_conColour = GenerateColourFromString(field->name);

			sourcePin->SetAlign(kNgAlignRight);

			if (NgPin* destinationPin = _destination->AddPin(new NgPin))
			{
				destinationPin->SetAlign(kNgAlignLeft);
				destinationPin->SetColour(_destination->Colour());
			
				if (NgConnection* connection = _builder->_graph->Diagram()->AddConnection(new NgConnection()))
				{
					connection->SetFrom(sourcePin);
					connection->SetTo(destinationPin);
					connection->SetColour(_conColour);
				}
			}
		}
	}

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)
	{
		return VisitMembers(field->type, dom, &HyperViewDestinationVisitor(_builder, _destination, _source), Xt::kIndexAt);
	}

	virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)
	{
		if (dom)
		{
			if (_source && dom->id == ((HvNode*)_destination)->_componentId)
			{
				// <THIS CODE IS TEMPORARY!>
				// Recurse up until we find an actor node. This will be changed to look for some sort of semantic tag in the schema, and moved into a separate function.
				Xt::DomElement* parent = _source->parent;
				while (parent)
				{
					Xt::DomComponent* parentComponent = Xt::AsComponent(parent);
					if (parentComponent && IsBaseOf(AsStruct(FindTypeByName(_builder->_db->schema, "ActorCreationData")), AsStruct(FindTypeByName(_builder->_db->schema, parentComponent->type))) )
					{
						NgNode* node = _builder->CreateNode(0, AsStruct(FindTypeByName(_builder->_db->schema, parentComponent->type)), parentComponent, parentComponent->id, _conColour);
						AddPin(field, node);
						break;
					}
					parent = parent->parent;
				}
				// </THIS CODE IS TEMPORARY!>
			}
			
			VisitMembers(field->type, &dom->data, &HyperViewDestinationVisitor(_builder, _destination, dom), Xt::kIndexAt);
		}

		return true;
	}

	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)
	{
		if (LtStruct* type = TypeOf(_builder->_schema, field->baseType, dom))
		{
			if (_source && dom->id == ((HvNode*)_destination)->_componentId)
			{
				// <THIS CODE IS TEMPORARY!>
				// Recurse up until we find an actor node. This will be changed to look for some sort of semantic tag in the schema, and moved into a separate function.
				Xt::DomElement* parent = _source->parent;
				while (parent)
				{
					Xt::DomComponent* parentComponent = Xt::AsComponent(parent);
					if (parentComponent && IsBaseOf(AsStruct(FindTypeByName(_builder->_db->schema, "ActorCreationData")), AsStruct(FindTypeByName(_builder->_db->schema, parentComponent->type))) )
					{
						NgNode* node = _builder->CreateNode(0, AsStruct(FindTypeByName(_builder->_db->schema, parentComponent->type)), parentComponent, parentComponent->id, _conColour);
						AddPin(field, node);
						break;
					}
					parent = parent->parent;
				}
				// </THIS CODE IS TEMPORARY!>
			}
			
			VisitMembers(type, &dom->data, &HyperViewDestinationVisitor(_builder, _destination, dom), Xt::kIndexAt);
		}

		return true;
	}

	virtual bool Visit(LfHandle* field, Xt::DomField* dom)
	{
		if (_source && Xt::ParseId(dom->value) == ((HvNode*)_destination)->_componentId)
		{
			// <THIS CODE IS TEMPORARY!>
			// Recurse up until we find an actor node. This will be changed to look for some sort of semantic tag in the schema, and moved into a separate function.
			Xt::DomElement* parent = _source->parent;
			while (parent)
			{
				Xt::DomComponent* parentComponent = Xt::AsComponent(parent);
				if (parentComponent && IsBaseOf(AsStruct(FindTypeByName(_builder->_db->schema, "ActorCreationData")), AsStruct(FindTypeByName(_builder->_db->schema, parentComponent->type))) )
				{
					NgNode* node = _builder->CreateNode(0, AsStruct(FindTypeByName(_builder->_db->schema, parentComponent->type)), parentComponent, parentComponent->id, _conColour);
					AddPin(field, node);
					break;
				}
				parent = parent->parent;
			}
			// </THIS CODE IS TEMPORARY!>
		}

		return true;
	}

	virtual bool Visit(LfWeakPtr* field, Xt::DomRef* dom)
	{
		if (_source && dom->value == ((HvNode*)_destination)->_componentId)
		{
			// <THIS CODE IS TEMPORARY!>
			// Recurse up until we find an actor node. This will be changed to look for some sort of semantic tag in the schema, and moved into a separate function.
			Xt::DomElement* parent = _source->parent;
			while (parent)
			{
				Xt::DomComponent* parentComponent = Xt::AsComponent(parent);
				if (parentComponent && IsBaseOf(AsStruct(FindTypeByName(_builder->_db->schema, "ActorCreationData")), AsStruct(FindTypeByName(_builder->_db->schema, parentComponent->type))) )
				{
					NgNode* node = _builder->CreateNode(0, AsStruct(FindTypeByName(_builder->_db->schema, parentComponent->type)), parentComponent, parentComponent->id, _conColour);
					AddPin(field, node);
					break;
				}
				parent = parent->parent;
			}
			// </THIS CODE IS TEMPORARY!>
		}

		return true;
	}

	virtual bool Visit(LfArray* field, Xt::DomArray* dom)
	{
		VisitArrayItems(field, dom, &HyperViewDestinationVisitor(_builder, _destination, _source), Xt::kIndexAt);

		return true;
	}
};

void HyperViewForm::Populate()
{
	Xt::DomDb*		db		= _db;
	LarvaSchema*	schema	= db->schema;
	Xt::DomComponent* component = Xt::AsComponent(_component);
	LtStruct*		type	= component ? AsStruct(FindTypeByName(schema, component->type)) : 0;

	if (!schema || !component || !type)
		return;

	_graph->Diagram()->Clear();

	HyperViewBuilder builder(this, db, schema, _graph);
	NgNode* root = builder.CreateNode(0, type, component, component->id, 0);
	VisitMembers(type, &component->data, &HyperViewSourceVisitor(&builder, root), Xt::kIndexAt);
	VisitMembers(Xt::TypeOf(schema, Xt::RecordOf(component)), &Xt::RecordOf(component)->root.data, &HyperViewDestinationVisitor(&builder, root, 0), Xt::kIndexAt);
}