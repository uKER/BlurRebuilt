#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/NodeGraph.h"
#include "../ScarabImpl.h"
#include "SnNgImpl.h"

class SnNgRootBuilder : public LvFieldDom
{
	ScarabDomNodeGraph* _graph;

public:
	SnNgRootBuilder(ScarabDomNodeGraph* graph) :
		_graph(graph)
	{
	}

public:
	bool DoField(LarvaField* field, Xt::DomElement* dom)
	{
		if (field->attrs.GetBool("NodeGraph", "root"))
		{
			if (AsStruct(field->Type()))
			{
				if (!XtUt::AppendEx(&_graph->_rootElements, new ScarabDomNodeGraph::RootElement(dom, field)))
					return false;
			}
		}

		return true;
	}

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)
	{
		return VisitMembers(field->type, dom, this, Xt::kIndexAcquire);
	}

	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)
	{
		return DoField(field, dom);
	}

	virtual bool Visit(LfArray* field, Xt::DomArray* dom)
	{
		return DoField(field, dom);
	}
};

void ScarabDomNodeGraph::PopulateRoots(LtStruct* type, Xt::DomBlock* dom)
{
	Free(&_rootElements);

	VisitMembers(type, dom, &SnNgRootBuilder(this), Xt::kIndexAcquire); 
}

bool ScarabDomNodeGraph::CreateNode(LarvaSchema* schema, LarvaField* field, Xt::DomComponent* component, Xt::Indexer* indexer, Xt::DomElement* parentDom)
{
	if (component->id)
	{
		if (LtStruct* type = AsStruct(TypeOf(schema, field, component)))
		{
			if (!FindNode(component->id))
				Diagram()->AddNode(new SnNgNode(this, field, type, component, indexer->Clone(), parentDom));
		}
	}

	return true;
}

void ScarabDomNodeGraph::PopulateNodes()
{
	LarvaSchema* schema = _form->Db()->schema;

	for(XtUt::List<RootElement>::Iterator itRoot(&_rootElements); itRoot; ++itRoot)
	{
		switch(itRoot->field->kind)
		{
			case kLfPtr:
			{
				if (Xt::DomComponent* component = AsComponent(itRoot->dom))
				{
					CreateNode(schema, itRoot->field, component, &Xt::NullIndexer(), 0); // TODO: Right indexer...
				}
			}
			break;

			case kLfArray:
			{
				if (Xt::DomArray* array = AsArray(itRoot->dom))
				{
					for(Xt::DomArray::IndexIterator itArr(array); itArr; ++itArr)
					{
						if (Xt::DomComponent* component = AsComponent(itArr))
						{
							CreateNode(schema, ((LfArray*)itRoot->field)->field, component, &Xt::ArrayIndexer(itArr.Key()), array);
						}
					}
				}
			}
			break;
		}
	}
}

struct SnNgPinBuilderContext
{
	ScarabDomNodeGraph*		ng;
	LarvaSchema*			schema;
	SnNgNode*				node;
	XtUt::WeakRef<NgPin>	lastPin;

public:
	SnNgPinBuilderContext(ScarabDomNodeGraph* ng_, LarvaSchema* schema_, SnNgNode* node_) :
		ng(ng_),
		schema(schema_),
		node(node_)
	{
	}
};

class SnNgPinBuilder : public LvFieldDom
{
	SnNgPinBuilderContext*	_ctx;
	LtStruct*				_parentType;
	Xt::DomBlock*			_parentDom;

public:
	SnNgPinBuilder(SnNgPinBuilderContext* ctx, LtStruct* parentType, Xt::DomBlock* parentDom) :
		_ctx(ctx),
		_parentType(parentType),
		_parentDom(parentDom)
	{
	}

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)
	{
		return !dom || VisitMembers(field->type, dom, &SnNgPinBuilder(_ctx, field->type, dom), Xt::kIndexAcquire);
	}

	virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)
	{
		return !dom || VisitMembers(field->type, &dom->data, &SnNgPinBuilder(_ctx, field->type, &dom->data), Xt::kIndexAcquire);
	}

	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)
	{
		if (LtStruct* type = TypeOf(_ctx->schema, field->baseType, dom))
			return VisitMembers(type, &dom->data, &SnNgPinBuilder(_ctx, type, &dom->data), Xt::kIndexAcquire);

		return true;
	}

	virtual bool Visit(LfArray* field, Xt::DomArray* dom)
	{
		return !dom || VisitArrayItems(field, dom, this, Xt::kIndexAcquire);
	}

	virtual bool Visit(LfWeakPtr* field, Xt::DomRef* dom)
	{
		if (dom)
		{
			bool createIt = true;

			for(XtUt::LinkList<NgPin>::Iterator itPin(_ctx->node->Pins()); itPin; ++itPin)
			{
				if (((SnNgPin*)(NgPin*)itPin)->Field() == dom)
				{
					createIt = false;

					_ctx->lastPin = itPin->Anchor();

					break;
				}
			}

			if (createIt)
			{
				if (_ctx->ng->FindPossiblePlug(field->baseType))
				{
					if (field->attrs.GetBool("NodeGraph", "pinEnabled", true))
					{
						NgPin* pin = _ctx->node->InsertPin(new SnNgPin(_ctx->ng, field, dom, field, _parentType, _parentDom), _ctx->lastPin);

						if (pin)
							_ctx->lastPin = pin->Anchor();
					}
				}
			}
		}

		return true;
	}
};

void ScarabDomNodeGraph::PopulatePins()
{
	LarvaSchema* schema = _form->Db()->schema;

	for(XtUt::LinkList<SnNgNode>::Iterator itNode(&_nodes); itNode; ++itNode)
	{
		if (Xt::DomComponent* component = AsComponent(itNode->_dom))
		{
			if (LtStruct* type = AsStruct(TypeOf(schema, itNode->_field, component)))
			{
				SnNgPinBuilderContext ctx(this, schema, itNode);

				VisitMembers(type, &component->data, &SnNgPinBuilder(&ctx, type, &component->data), Xt::kIndexAcquire);
			}
		}
	}

	for(XtUt::LinkList<SnNgNode>::Iterator itNode(&_nodes); itNode; ++itNode)
	{
		for(XtUt::LinkList<NgPin>::Iterator itPin(itNode->Pins()); itPin; ++itPin)
			((SnNgPin*)(NgPin*)itPin)->CreateConnections();
	}
}
