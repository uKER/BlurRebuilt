#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include <Xt/Impl/CryptRandom.h>

struct EuuField : public LvFieldDom
{
	EuuParams*			_params;
	LarvaSchema*		_schema;
	CryptRandom			_cr;
	XtUt::Set<XtU64>	_ids;

public:
	EuuField() :
		_params(0),
		_schema(0)
	{
	}

	bool Initialise()
	{
		return _cr.Initialise();
	}

public:
	bool GenerateUid(LfInt* field, Xt::DomField* dom)
	{
		if (!field || !dom)
			return false;

		for(int i = 0; i < 32; i++)
		{
			XtU64 id = 0;

			switch(field->type->atomType)
			{
				case kXtI8:		id = (XtI8)_cr(); break;
				case kXtI16:	id = (XtI16)_cr(); break;
				case kXtI32:	id = (XtI32)_cr(); break;
				case kXtI64:	id = (XtI64)_cr(); break;

				case kXtU8:		id = (XtU8)_cr(); break;
				case kXtU16:	id = (XtU16)_cr(); break;
				case kXtU32:	id = (XtU32)_cr(); break;
				case kXtU64:	id = (XtU64)_cr(); break;
			}

			if (!id || XtUt::Exists(&_ids, id))
				continue;

			char buf[128];

			switch(field->type->atomType)
			{
				default:
				case kXtI8:
				case kXtI16:
				case kXtI32:
				case kXtI64:
					_i64toa_s(id, buf, sizeof(buf), (int)field->radix);
				break;

				case kXtU8:
				case kXtU16:
				case kXtU32:
				case kXtU64:
					_ui64toa_s(id, buf, sizeof(buf), (int)field->radix);
				break;
			}

			if (!Set(dom, buf))
				return false;

			if (!XtUt::Insert(&_ids, id))
				return false;

			if (_params->ensureGloballyUnique && !XtUt::Insert(&_params->globalIds, id))
				return false;

			return true;
		}

		return false;
	}

public:
	virtual bool Visit(LfInt* field, Xt::DomField* dom)
	{
		if (field->defValue != "uid")
			return true;

		XtU64 id = 0;

		switch(field->type->atomType)
		{
			case kXtI8:
			case kXtI16:
			case kXtI32:
			case kXtI64:
				id = _strtoi64(AsText(dom), 0, (int)field->radix);
			break;

			case kXtU8:
			case kXtU16:
			case kXtU32:
			case kXtU64:
				id = _strtoui64(AsText(dom), 0, (int)field->radix);
			break;
		}

		bool collision = id && XtUt::Exists(&_ids, id);

		if (!collision && id && _params->ensureGloballyUnique)
			collision = XtUt::Exists(&_params->globalIds, id);

		if (!id)
			_params->missingCount++;

		if (collision)
			_params->collisionCount++;

		if (!id || collision)
		{
			_params->totalCount++;

			if (!GenerateUid(field, dom))
			{
				DebugLn("GenerateUid FAILED!");
				return true;
			}
		}
		else
		{
			if (!XtUt::Insert(&_ids, id))
			{
				DebugLn("EvuField::Visit::Insert FAILED!");
			}

			if (_params->ensureGloballyUnique && !XtUt::Insert(&_params->globalIds, id))
			{
				DebugLn("EvuField::Visit::Insert FAILED!");
			}
		}

		return true;
	}
};

struct EuuObjects : public LvObjects
{
	EuuField v;

public:
	virtual bool Visit(LtStruct* type, Xt::DomBlock* dom)
	{
		return VisitMembers(type, dom, &v, Xt::kIndexAcquire);
	}
};

// EnsureUniqueUids

bool EnsureUniqueUids(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, EuuParams* params)
{
	if (!schema || !type || !component || !params)
		return false;

	EuuObjects v;

	if (!v.v.Initialise())
		return false;

	v.v._params = params;
	v.v._schema = schema;

	return VisitAllObjects(schema, type, component, &v, true);
}

//
// RegenerateUniqueIds
//

struct RuiField : public LvFieldDom
{
	LarvaSchema*	_schema;
	CryptRandom		_cr;

public:
	RuiField() :
		_schema(0)
	{
	}

	bool Initialise()
	{
		return _cr.Initialise();
	}

public:
	bool GenerateUid(LfInt* field, Xt::DomField* dom)
	{
		if (!field || !dom)
			return false;

		XtU64 id = 0;

		do
		{
			switch(field->type->atomType)
			{
				case kXtI8:		id = (XtI8)_cr(); break;
				case kXtI16:	id = (XtI16)_cr(); break;
				case kXtI32:	id = (XtI32)_cr(); break;
				case kXtI64:	id = (XtI64)_cr(); break;

				case kXtU8:		id = (XtU8)_cr(); break;
				case kXtU16:	id = (XtU16)_cr(); break;
				case kXtU32:	id = (XtU32)_cr(); break;
				case kXtU64:	id = (XtU64)_cr(); break;
			}
		}
		while(!id);

		char buf[128];

		switch(field->type->atomType)
		{
			default:
			case kXtI8:
			case kXtI16:
			case kXtI32:
			case kXtI64:
				_i64toa_s(id, buf, sizeof(buf), (int)field->radix);
			break;

			case kXtU8:
			case kXtU16:
			case kXtU32:
			case kXtU64:
				_ui64toa_s(id, buf, sizeof(buf), (int)field->radix);
			break;
		}

		return Set(dom, buf);
	}

public:
	virtual bool Visit(LfInt* field, Xt::DomField* dom)
	{
		if (field->defValue != "uid")
			return true;

		if (!GenerateUid(field, dom))
		{
			DebugLn("GenerateUid FAILED!");
		}

		return true;
	}
};

struct RuiObjects : public LvObjects
{
	RuiField v;

public:
	virtual bool Visit(LtStruct* type, Xt::DomBlock* dom)
	{
		return VisitMembers(type, dom, &v, Xt::kIndexAcquire);
	}
};

void RegenerateUniqueIds(LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block)
{
	if (!schema || !type || !block)
		return;

	RuiObjects v;

	if (!v.v.Initialise())
		return;

	v.v._schema = schema;

	VisitAllObjects(schema, type, block, &v, true);
}

void RegenerateUniqueIds(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component)
{
	if (!schema || !type || !component)
		return;

	RuiObjects v;

	if (!v.v.Initialise())
		return;

	v.v._schema = schema;

	VisitAllObjects(schema, type, component, &v, true);
}

void RegenerateUniqueIds(LarvaSchema* schema, LarvaField* field, Xt::DomElement* element)
{
	if (!schema || !field || !element)
		return;

	RuiObjects v;

	if (!v.v.Initialise())
		return;

	v.v._schema = schema;

	VisitAllObjects(schema, field, element, &v, true);
}
