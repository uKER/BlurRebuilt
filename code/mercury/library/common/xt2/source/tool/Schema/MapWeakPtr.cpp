#include "Common/XtPch.h"
#include "Xt/XtTool.h"

namespace
{
	struct MapWeakPtrsVisitor : public LvFieldDom
	{
		LarvaSchema*	_schema;
		Xt::IdMap*		_map;

	public:
		MapWeakPtrsVisitor(LarvaSchema* schema, Xt::IdMap* map) :
			_schema(schema),
			_map(map)
		{
		}

	public:
		virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)			{ return VisitMembers(field->type, dom, this, Xt::kIndexAt);									}
		virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)	{ return VisitMembers(field->type, &dom->data, this, Xt::kIndexAt);								}
		virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)			{ return VisitMembers(TypeOf(_schema, field->baseType, dom), &dom->data, this, Xt::kIndexAt);	}
		virtual bool Visit(LfArray* field, Xt::DomArray* dom)			{ return VisitArrayItems(field, dom, this, Xt::kIndexAt);										}

		virtual bool Visit(LfWeakPtr* field, Xt::DomRef* dom)
		{
			if (XtId id = AsId(dom))
			{
				if (XtId* newId = At(_map, id))
					Set(dom, *newId);
			}

			return true;
		}
	};
}

// MapWeakPtrs

void MapWeakPtrs(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, Xt::IdMap* map)
{
	if (!schema || !type || !block || !map)
		return;

	VisitMembers(type, block, &MapWeakPtrsVisitor(schema, map), Xt::kIndexAt);
}

void MapWeakPtrs(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, Xt::IdMap* map)
{
	if (!schema || !type || !component || !map)
		return;

	VisitMembers(type, &component->data, &MapWeakPtrsVisitor(schema, map), Xt::kIndexAt);
}

void MapWeakPtrs(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, Xt::IdMap* map)
{
	if (!schema || !field || !element || !map)
		return;

	field->Accept(&MapWeakPtrsVisitor(schema, map), element, Xt::kIndexSelf);
}