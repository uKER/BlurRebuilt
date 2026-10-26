#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "Xt/FindDependencies.h"

struct FindDependenciesContext
{
	LarvaSchema*		schema;
	XtId				dependsOnId;
	Xt::QueryElement*	queryElement;
	Xt::QueryContext*	queryContext;
};

struct FdField;

struct FdType : public LvTypeDom
{
	FindDependenciesContext*	ctx;
	FdField*					lastField;

public:
	FdType(FindDependenciesContext* ctx_, FdField* lastField_) :
		ctx(ctx_),
		lastField(lastField_)
	{
	}

public:
	virtual bool Visit(LtStruct* type, Xt::DomBlock* dom);
};

struct FdField : public LvFieldDom
{
	FindDependenciesContext*	ctx;
	FdField*					lastField;
	XtString					name;

public:
	FdField(FindDependenciesContext* ctx_, FdField* lastField_, XtString name_) :
		ctx(ctx_),
		lastField(lastField_),
		name(name_)
	{
	}

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom);
	virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom);
	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom);
	virtual bool Visit(LfHandle* field, Xt::DomField* dom);
	virtual bool Visit(LfArray* field, Xt::DomArray* dom);
};

// FdBuildPath

XtString FdBuildPath(FdField* field)
{
	if (!field)
		return "/";

	return FdBuildPath(field->lastField) + field->name + "/";
}

// FdType

bool FdType::Visit(LtStruct* type, Xt::DomBlock* dom)
{
	if (type->base)
		type->base->type->Accept(&FdType(ctx, lastField), dom);

	for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
		it->Accept(&FdField(ctx, lastField, it->name), dom, Xt::kIndexAcquire);

	return true;
}

// FdField

bool FdField::Visit(LfStruct* field, Xt::DomBlock* dom)
{
	return field->type->Accept(&FdType(ctx, this), dom);
}

bool FdField::Visit(LfStrictPtr* field, Xt::DomComponent* dom)
{
	if (dom)
		field->type->Accept(&FdType(ctx, this), &dom->data);

	return true;
}

bool FdField::Visit(LfPtr* field, Xt::DomComponent* dom)
{
	if (dom)
	{
		if (LarvaType* type = TypeOf(ctx->schema, field->baseType, dom))
			type->Accept(&FdType(ctx, this), &dom->data);
	}

	return true;
}

bool FdField::Visit(LfHandle* field, Xt::DomField* dom)
{
	if (dom)
	{
		XtId recordId = Xt::ParseId(dom->value);

		if (ctx->dependsOnId == recordId)
		{
			ctx->queryElement->Result(ctx->queryContext, Xt::QueryResult(field, 0, dom, FdBuildPath(lastField) + name));
		}
	}

	return true;
}

bool FdField::Visit(LfArray* field, Xt::DomArray* dom)
{
	if (dom)
	{
		for(Xt::DomArray::IndexIterator it(dom); it; ++it)
		{
			char idx[16];
			_snprintf_s(idx, sizeof(idx), _TRUNCATE, "#%i", it.index);

			field->field->Accept(&FdField(ctx, this, idx), it, Xt::kIndexAcquire);
		}
	}

	return true;
}

// FindDependencies

void FindDependencies(LarvaSchema* schema, XtId dependsOnId, Xt::DomRecord* record, Xt::QueryElement* queryElement, Xt::QueryContext* queryContext)
{
	FindDependenciesContext ctx;

	ctx.schema			= schema;
	ctx.dependsOnId		= dependsOnId;
	ctx.queryElement	= queryElement;
	ctx.queryContext	= queryContext;

	if (LtStruct* type = TypeOf(schema, record))
		type->Accept(&FdType(&ctx, 0), &record->root.data);
}