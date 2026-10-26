#include "Common/XtPch.h"
#include "Xt/XtTool.h"

// Xt::LvType

LvType::LvType()	{ }
LvType::~LvType()	{ }

bool LvType::Visit(LtStruct* type)		{ return true; }
bool LvType::Visit(LtAtom* type)		{ return true; }
bool LvType::Visit(LtEnum* type)		{ return true; }
bool LvType::Visit(LtFlags* type)		{ return true; }

// Xt::LvField

LvField::LvField()	{ }
LvField::~LvField()	{ }

bool LvField::Visit(LfStruct* field)		{ return true; }
bool LvField::Visit(LfInt* field)			{ return true; }
bool LvField::Visit(LfFloat* field)			{ return true; }
bool LvField::Visit(LfString* field)		{ return true; }
bool LvField::Visit(LfWideString* field)	{ return true; }
bool LvField::Visit(LfBool* field)			{ return true; }
bool LvField::Visit(LfEnum* field)			{ return true; }
bool LvField::Visit(LfFlags* field)			{ return true; }
bool LvField::Visit(LfStrictPtr* field)		{ return true; }
bool LvField::Visit(LfPtr* field)			{ return true; }
bool LvField::Visit(LfWeakPtr* field)		{ return true; }
bool LvField::Visit(LfHandle* field)		{ return true; }
bool LvField::Visit(LfArray* field)			{ return true; }
bool LvField::Visit(LfLocString* field)		{ return true; }
bool LvField::Visit(LfControl* field)		{ return true; }

// Xt::LvTypeDom

LvTypeDom::LvTypeDom()	{ }
LvTypeDom::~LvTypeDom()	{ }

bool LvTypeDom::Visit(LtStruct* type, Xt::DomBlock* dom)	{ return true; }
bool LvTypeDom::Visit(LtAtom* type, Xt::DomField* dom)		{ return true; }
bool LvTypeDom::Visit(LtEnum* type, Xt::DomField* dom)		{ return true; }
bool LvTypeDom::Visit(LtFlags* type, Xt::DomBlock* dom)		{ return true; }

// Xt::LvFieldDom

LvFieldDom::LvFieldDom()	{ }
LvFieldDom::~LvFieldDom()	{ }

bool LvFieldDom::Visit(LfStruct* field, Xt::DomBlock* dom)			{ return true; }
bool LvFieldDom::Visit(LfInt* field, Xt::DomField* dom)				{ return true; }
bool LvFieldDom::Visit(LfFloat* field, Xt::DomField* dom)			{ return true; }
bool LvFieldDom::Visit(LfString* field, Xt::DomField* dom)			{ return true; }
bool LvFieldDom::Visit(LfWideString* field, Xt::DomField* dom)		{ return true; }
bool LvFieldDom::Visit(LfBool* field, Xt::DomField* dom)			{ return true; }
bool LvFieldDom::Visit(LfEnum* field, Xt::DomField* dom)			{ return true; }
bool LvFieldDom::Visit(LfFlags* field, Xt::DomBlock* dom)			{ return true; }
bool LvFieldDom::Visit(LfStrictPtr* field, Xt::DomComponent* dom)	{ return true; }
bool LvFieldDom::Visit(LfPtr* field, Xt::DomComponent* dom)			{ return true; }
bool LvFieldDom::Visit(LfWeakPtr* field, Xt::DomRef* dom)			{ return true; }
bool LvFieldDom::Visit(LfHandle* field, Xt::DomField* dom)			{ return true; }
bool LvFieldDom::Visit(LfArray* field, Xt::DomArray* dom)			{ return true; }
bool LvFieldDom::Visit(LfLocString* field, Xt::DomBlock* dom)		{ return true; }
bool LvFieldDom::Visit(LfControl* field, Xt::DomElement* dom)		{ return true; }

// Xt::LvObjects

LvObjects::LvObjects()	{ }
LvObjects::~LvObjects()	{ }

bool LvObjects::Visit(struct LtStruct* type, Xt::DomBlock* dom)		{ return true; }

// Xt::LvObjects

LvComponents::LvComponents()	{ }
LvComponents::~LvComponents()	{ }

bool LvComponents::Visit(struct LtStruct* type, Xt::DomComponent* dom)	{ return true; }

// VisitMembers

bool VisitMembers(LtStruct* type, Xt::DomBlock* dom, LvFieldDom* visitor, Xt::IndexMethod method)
{
	if (!visitor)
		return false;

	if (type)
	{
		if (type->base && !VisitMembers(type->base->type, dom, visitor, method))
			return false;

		for(XtUt::List<LarvaField>::Iterator it(&type->fields); it; ++it)
		{
			if (!it->Accept(visitor, dom, method))
				return false;
		}
	}

	return true;
}

// VisitArrayItems

bool VisitArrayItems(LfArray* field, Xt::DomArray* dom, LvFieldDom* visitor, Xt::IndexMethod method)
{
	if (!visitor)
		return false;

	if (field && dom)
	{
		for(Xt::DomArray::IndexIterator it(dom); it; ++it)
		{
			if (!field->field->Accept(visitor, it, method))
				return false;
		}
	}

	return true;
}