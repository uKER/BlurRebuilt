#include "Common/XtPch.h"
#include "Xt/XtTool.h"

namespace
{
	struct VaoFieldDom : public LvFieldDom
	{
		LarvaSchema*	_schema;
		LvObjects*		_visitor;
		bool			_useAcquire;

	public:
		VaoFieldDom(LarvaSchema* schema, LvObjects* visitor, bool useAcquire) :
			_schema(schema),
			_visitor(visitor),
			_useAcquire(useAcquire)
		{
		}

	public:
		virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)
		{
			return _visitor->Visit(field->type, dom) && VisitMembers(field->type, dom, this, _useAcquire ? Xt::kIndexAcquire : Xt::kIndexAt);
		}

		virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)
		{
			return !dom || _visitor->Visit(field->type, &dom->data) && VisitMembers(field->type, &dom->data, this, _useAcquire ? Xt::kIndexAcquire : Xt::kIndexAt);
		}

		virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)
		{
			if (LtStruct* type = TypeOf(_schema, field->baseType, dom))
				return _visitor->Visit(type, &dom->data) && VisitMembers(type, &dom->data, this, _useAcquire ? Xt::kIndexAcquire : Xt::kIndexAt);

			return true;
		}

		virtual bool Visit(LfArray* field, Xt::DomArray* dom)
		{
			return VisitArrayItems(field, dom, this, _useAcquire ? Xt::kIndexAcquire : Xt::kIndexAt);
		}
	};
}

bool VisitAllObjects(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, LvObjects* visitor, bool useAcquire)
{
	if (!schema || !type || !block || !visitor)
		return false;

	return visitor->Visit(type, block) && VisitMembers(type, block, &VaoFieldDom(schema, visitor, useAcquire), useAcquire ? Xt::kIndexAcquire : Xt::kIndexAt);
}

bool VisitAllObjects(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, LvObjects* visitor, bool useAcquire)
{
	if (!schema || !type || !component || !visitor)
		return false;

	return visitor->Visit(type, &component->data) && VisitMembers(type, &component->data, &VaoFieldDom(schema, visitor, useAcquire), useAcquire ? Xt::kIndexAcquire : Xt::kIndexAt);
}

bool VisitAllObjects(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, LvObjects* visitor, bool useAcquire)
{
	if (!schema || !field || !element || !visitor)
		return false;

	return field->Accept(&VaoFieldDom(schema, visitor, useAcquire), element, Xt::kIndexSelf);
}