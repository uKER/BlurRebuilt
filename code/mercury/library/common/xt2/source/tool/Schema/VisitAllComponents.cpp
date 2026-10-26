#include "Common/XtPch.h"
#include "Xt/XtTool.h"

namespace
{
	struct VacFieldDom : public LvFieldDom
	{
		LarvaSchema*	_schema;
		LvComponents*	_visitor;

	public:
		VacFieldDom(LarvaSchema* schema, LvComponents* visitor) :
			_schema(schema),
			_visitor(visitor)
		{
		}

	public:
		virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)
		{
			return VisitMembers(field->type, dom, this, Xt::kIndexAcquire);
		}

		virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)
		{
			return !dom || _visitor->Visit(field->type, dom) && VisitMembers(field->type, &dom->data, this, Xt::kIndexAcquire);
		}

		virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)
		{
			if (LtStruct* type = TypeOf(_schema, field->baseType, dom))
				return _visitor->Visit(type, dom) && VisitMembers(type, &dom->data, this, Xt::kIndexAcquire);

			return true;
		}

		virtual bool Visit(LfArray* field, Xt::DomArray* dom)
		{
			return VisitArrayItems(field, dom, this, Xt::kIndexAcquire);
		}
	};
}

bool VisitAllComponents(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, LvComponents* visitor)
{
	if (!schema || !type || !block || !visitor)
		return false;

	return VisitMembers(type, block, &VacFieldDom(schema, visitor), Xt::kIndexAcquire);
}

bool VisitAllComponents(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, LvComponents* visitor)
{
	if (!schema || !type || !component || !visitor)
		return false;

	return visitor->Visit(type, component) && VisitMembers(type, &component->data, &VacFieldDom(schema, visitor), Xt::kIndexAcquire);
}

bool VisitAllComponents(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, LvComponents* visitor)
{
	if (!schema || !field || !element || !visitor)
		return false;

	return field->Accept(&VacFieldDom(schema, visitor), element, Xt::kIndexSelf);
}