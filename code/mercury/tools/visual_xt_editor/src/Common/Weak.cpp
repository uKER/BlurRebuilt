#include "stdafx.h"

#include "Xt/XtTool.h"
#include "Weak.h"

//
// LvWeakType::Visit
//

bool LvWeakType::Visit(LtStruct* type)
{
	Xt::DomBlock* block = AsBlock(element);

	if (labelField && labelWatch)
		ctx->node->AddItem(type, block, labelField, labelWatch);

	for(XtUt::List<LarvaBaseType>::Iterator it(&type->bases); it; ++it)
		ctx->VisitType(it->type, block, 0, 0);

	for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
		ctx->VisitField(it, block, 0, 0);

	return true;
}

bool LvWeakType::Visit(LtInt* type)
{
	return true;
}

bool LvWeakType::Visit(LtFloat* type)
{
	return true;
}

bool LvWeakType::Visit(LtString* type)
{
	return true;
}

bool LvWeakType::Visit(LtBool* type)
{
	return true;
}

bool LvWeakType::Visit(LtEnum* type)
{
	return true;
}

bool LvWeakType::Visit(LtFlags* type)
{
	return true;
}

bool LvWeakType::Visit(LtLocString* type)
{
	return true;
}

//
// LvWeakField::Visit
//

bool LvWeakField::Visit(LfStruct* field)
{
	Xt::DomBlock*	parent		= AsBlock(element);
	Xt::DomBlock*	block		= *field->name ? (parent ? AsBlock(At(parent, field->name)) : 0) : parent;

	return ctx->VisitType(field->type, block, labelField, labelWatch);
}

bool LvWeakField::Visit(LfInt* field)
{
	return true;
}

bool LvWeakField::Visit(LfFloat* field)
{
	return true;
}

bool LvWeakField::Visit(LfString* field)
{
	return true;
}

bool LvWeakField::Visit(LfBool* field)
{
	return true;
}

bool LvWeakField::Visit(LfEnum* field)
{
	return true;
}

bool LvWeakField::Visit(LfFlags* field)
{
	return true;
}

bool LvWeakField::Visit(LfPtr* field)
{
	Xt::DomBlock*	parent		= AsBlock(element);
	Xt::DomBlock*	block		= *field->name ? (parent ? AsBlock(At(parent, field->name)) : 0) : parent;

	if (block)
	{
		if (LarvaType* type = TypeOf(ctx->schema, block))
		{
			ctx->VisitType(type, block, field, block);
		}
	}

	return true;
}

bool LvWeakField::Visit(LfWeakPtr* field)
{
	Xt::DomBlock* pBlock = AsBlock(element);
	Xt::DomField* pField = pBlock ? AsField(At(pBlock, field->name)) : AsField(element);

	if(pField)
	{
		if(pField->value == ctx->node->m_StructId)
			ctx->node->AddWeakPtr(field, pField);
	}

	return true;
}

bool LvWeakField::Visit(LfHandle* field)
{
	return true;
}

bool LvWeakField::Visit(LfArray* field)
{
	Xt::DomBlock* parent	= AsBlock(element);
	Xt::DomArray* arr		= parent ? AsArray(At(parent, field->name)) : AsArray(element);

	if (arr)
	{
		for(int i = 0; i < (int)arr->elements.count; i++)
		{
			Xt::DomElement* el = At(arr, i);

			ctx->VisitField(field->field, el, field->field, AsBlock(el));
		}
	}

	return true;
}

bool LvWeakField::Visit(LfLocString* field)
{
	return true;
}

bool LvWeakField::Visit(LfControl* field)
{
	return true;
}

