#ifndef WEAK_TYPE_H
#define WEAK_TYPE_H

#pragma once

#include "WeakInterface.h"

//
// WeakContext
//

struct WeakContext;

struct LvWeakType : public LvType
{
	WeakContext*	ctx;
	Xt::DomElement*	element;
	LarvaField*		labelField;
	Xt::DomBlock*	labelWatch;

public:
	LvWeakType(WeakContext* ctx_, Xt::DomElement* element_, LarvaField* labelField_, Xt::DomBlock* labelWatch_) :
		ctx(ctx_),
		element(element_),
		labelField(labelField_),
		labelWatch(labelWatch_)
	{
	}

public:
	virtual bool Visit(LtStruct* type);
	virtual bool Visit(LtInt* type);
	virtual bool Visit(LtFloat* type);
	virtual bool Visit(LtString* type);
	virtual bool Visit(LtBool* type);
	virtual bool Visit(LtEnum* type);
	virtual bool Visit(LtFlags* type);
	virtual bool Visit(LtLocString* type);
};

struct LvWeakField : public LvField
{
	WeakContext*	ctx;
	Xt::DomElement*	element;
	LarvaField*		labelField;
	Xt::DomBlock*	labelWatch;

public:
	LvWeakField(WeakContext* ctx_, Xt::DomElement* element_, LarvaField* labelField_, Xt::DomBlock* labelWatch_) :
		ctx(ctx_),
		element(element_),
		labelField(labelField_),
		labelWatch(labelWatch_)
	{
	}

public:
	virtual bool Visit(LfStruct* field);
	virtual bool Visit(LfInt* field);
	virtual bool Visit(LfFloat* field);
	virtual bool Visit(LfString* field);
	virtual bool Visit(LfBool* field);
	virtual bool Visit(LfEnum* field);
	virtual bool Visit(LfFlags* field);
	virtual bool Visit(LfPtr* field);
	virtual bool Visit(LfWeakPtr* field);
	virtual bool Visit(LfHandle* field);
	virtual bool Visit(LfArray* field);
	virtual bool Visit(LfLocString* field);
	virtual bool Visit(LfControl* field);
};

struct WeakContext
{
	I_WeakInterface*		node;
	LarvaSchema*	schema;

public:
	WeakContext(I_WeakInterface* node_, LarvaSchema* schema_) :
		node(node_),
		schema(schema_)
	{
	}

public:
	bool VisitType(LarvaType* type, Xt::DomElement* element, LarvaField* labelField, Xt::DomBlock* labelWatch)
	{
		return type->Accept(&LvWeakType(this, element, labelField, labelWatch));
	}

	bool VisitField(LarvaField* field, Xt::DomElement* element, LarvaField* labelField, Xt::DomBlock* labelWatch)
	{
		return field->Accept(&LvWeakField(this, element, labelField, labelWatch));
	}
};

#endif // WEAK_TYPE_H
