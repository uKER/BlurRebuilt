#include "stdafx.h"

#include "GenerateId.h"

#include <Xt/XtTool.h>
#include <Xt/Impl/CryptRandom.h>

//
// GenerateIdForBlock
//

struct GenerateIdVisitor : public LvField
{
public:
	GenerateIdVisitor() :
		_name("id")
	{
	}

public:
	virtual bool Visit(LfStruct* field)		{ _name = field->type->name; return true; }
	virtual bool Visit(LfInt* field)		{ _name = field->type->name; return true; }
	virtual bool Visit(LfFloat* field)		{ _name = field->type->name; return true; }
	virtual bool Visit(LfString* field)		{ _name = field->type->name; return true; }
	virtual bool Visit(LfBool* field)		{ _name = field->type->name; return true; }
	virtual bool Visit(LfEnum* field)		{ _name = field->type->name; return true; }
	virtual bool Visit(LfFlags* field)		{ _name = field->type->name; return true; }
	virtual bool Visit(LfPtr* field)		{ _name = field->type->name; return true; }
	virtual bool Visit(LfWeakPtr* field)	{ _name = field->type->name; return true; }
	virtual bool Visit(LfHandle* field)		{ _name = field->type->name; return true; }

	virtual bool Visit(LfArray* field)
	{
		return field->field->Accept(this);
	}

	virtual bool Visit(LfLocString* field)	{ _name = "locstring";			return true; }
	virtual bool Visit(LfControl* field)	{ _name = "control";			return true; }

public:
	XtString _name;
};

void GenerateIdForBlock(Xt::DomBlock* block, LarvaField* field)
{
	if (!block)
		return;

	Xt::DomField* id = Xt::AcquireField(block, "__id");

	if (!id || *Xt::AsText(id))
		return;

	GenerateIdVisitor v;

	if (field)
		field->Accept(&v);

	CryptRandom cr;

	if (cr.Initialise())
	{
		char buf[128];

		_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%.4s%08X", (const char*)v._name, (int)cr());

		Set(id, buf);
	}
}