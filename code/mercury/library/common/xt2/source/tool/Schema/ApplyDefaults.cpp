#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "Xt/Impl/Grimoire.h"
#include <Xt/Impl/CryptRandom.h>

struct SdFieldDom : public LvFieldDom
{
	LarvaSchema* _schema;

public:
	SdFieldDom(LarvaSchema* schema) :
		_schema(schema)
	{
	}

public:
	bool SetFieldDefault(Xt::DomField* dom, const XtString& defValue)
	{
		if (dom && !*AsText(dom))
			Set(dom, defValue);

		return true;
	}

	bool SetBlockDefaults(LarvaField* field, Xt::DomBlock* dom)
	{
		if (field && dom)
		{
			if (LarvaAttr* attr = field->attrs.FindAttr("Default"))
			{
				for(XtUt::List<LarvaParam>::Iterator it(&attr->params); it; ++it)
					SetByDottedPath(dom, it->key, it->AsString());
			}
		}

		return true;
	}

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)
	{
		if (!VisitMembers(field->type, dom, this, Xt::kIndexAcquire))
			return false;

		return !dom || SetBlockDefaults(field, dom);
	}

	virtual bool Visit(LfInt* field, Xt::DomField* dom)
	{
		if (dom)
		{
			if (field->defValue == "uid")
			{
				CryptRandom cr;

				if (cr.Initialise())
				{
					char buf[128] = "uid";

					switch(field->type->atomType)
					{
						case kXtI8:		_i64toa_s((XtI8)cr(), buf, sizeof(buf), (int)field->radix);		break;
						case kXtI16:	_i64toa_s((XtI16)cr(), buf, sizeof(buf), (int)field->radix);	break;
						case kXtI32:	_i64toa_s((XtI32)cr(), buf, sizeof(buf), (int)field->radix);	break;
						case kXtI64:	_i64toa_s((XtI64)cr(), buf, sizeof(buf), (int)field->radix);	break;

						case kXtU8:		_ui64toa_s((XtU8)cr(), buf, sizeof(buf), (int)field->radix);	break;
						case kXtU16:	_ui64toa_s((XtU16)cr(), buf, sizeof(buf), (int)field->radix);	break;
						case kXtU32:	_ui64toa_s((XtU32)cr(), buf, sizeof(buf), (int)field->radix);	break;
						case kXtU64:	_ui64toa_s((XtU64)cr(), buf, sizeof(buf), (int)field->radix);	break;
					}

					return SetFieldDefault(dom, buf);
				}
			}
		}

		return SetFieldDefault(dom, field->defValue);
	}

	virtual bool Visit(LfFloat* field, Xt::DomField* dom)		{ return SetFieldDefault(dom, field->defValue); }
	virtual bool Visit(LfString* field, Xt::DomField* dom)		{ return SetFieldDefault(dom, field->defValue); }
	virtual bool Visit(LfWideString* field, Xt::DomField* dom)	{ return SetFieldDefault(dom, field->defValue); }
	virtual bool Visit(LfEnum* field, Xt::DomField* dom)		{ return SetFieldDefault(dom, field->defValue); }

	virtual bool Visit(LfBool* field, Xt::DomField* dom)
	{
		return SetFieldDefault(dom, (atoi(field->defValue) != 0) ? "1" : "0");
	}

	virtual bool Visit(LfFlags* field, Xt::DomBlock* dom)
	{
		for(XtUt::List<LarvaOption>::Iterator it(&field->type->options); it; ++it)
			SetFieldDefault(AcquireField(dom, it->name), (atoi(it->defValue) != 0) ? "1" : "0");

		return true;
	}

	virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)
	{
		if (!VisitMembers(field->type, &dom->data, this, Xt::kIndexAcquire))
			return false;

		return !dom || SetBlockDefaults(field, &dom->data);
	}

	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)
	{
		if (dom)
		{
			LtStruct* type = TypeOf(_schema, field->baseType, dom);

			if (!type)
			{
				type = AsStruct(FindTypeByName(_schema, field->defValue));

				if (!type)
					type = AsStruct(FindTypeByName(_schema, field->baseType->GetString("DefaultDerivedType", "0")));

				if (IsBaseOf(field->baseType, type))
				{
					GenerateIdForComponent(dom, false);
					SetType(dom, type->name);
				}
				else
					type = 0;
			}

			if (type)
			{
				if (!VisitMembers(type, &dom->data, this, Xt::kIndexAcquire))
					return false;

				return SetBlockDefaults(field, &dom->data);
			}
		}

		return true;
	}

	virtual bool Visit(LfArray* field, Xt::DomArray* dom)
	{
		return VisitArrayItems(field, dom, this, Xt::kIndexAcquire);
	}
};

bool SetDefaults(LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component)
{
	if (!schema || !type || !component)
		return false;

	return VisitMembers(type, &component->data, &SdFieldDom(schema), Xt::kIndexAcquire);
}