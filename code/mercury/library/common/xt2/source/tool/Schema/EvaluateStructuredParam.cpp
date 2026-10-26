#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "Xt/Impl/Grimoire.h"

// "{value actor}"
// "{summary actor}"
// "{if {lt {value size} 2} {value size} 2}"

const int kEstMaxLength			= 512;
const int kEstMaxRecursionDepth	= 8;

// EstFormatter

EstFormatter::EstFormatter()
{
}

EstFormatter::~EstFormatter()
{
}

bool EstFormatter::GetFormatByType(LtStruct* type, XtString& format)
{
	return false;
}

bool EstFormatter::GetFormatByOption(LarvaOption* option, XtString& format)
{
	return false;
}

void EstFormatter::CacheComponentText(Xt::DomRecord* record, XtId id, const char* text)
{
}

bool EstFormatter::FindComponentText(Xt::DomRecord* record, XtId id, XtString& text)
{
	return false;
}

// EstContext

struct EstContext
{
	Xt::DomDb*			db;
	LarvaSchema*		schema;
	EstFormatter*		formatter;
	int					recursionDepth;
	char*				outPtr;
	char*				outEnd;

public:
	EstContext(Xt::DomDb* db_, LarvaSchema* schema_, EstFormatter* formatter_, char* outPtr_, char* outEnd_) :
		db(db_),
		schema(schema_),
		formatter(formatter_),
		recursionDepth(0),
		outPtr(outPtr_),
		outEnd(outEnd_)
	{
	}

public:
	bool Append(const char* text)
	{
		while(*text != '\0')
		{
			if (outPtr >= outEnd)
				return false;

			*outPtr++ = *text++;
		}

		return true;
	}

	bool Append(char ch)
	{
		if (outPtr >= outEnd)
			return false;

		*outPtr++ = ch;

		return true;
	}
};

// EstField

bool Est_DoElement(EstContext* ctx, LtStruct* type, Xt::DomBlock* dom, const char* element, const char* format);
bool Est_DoString(EstContext* ctx, LtStruct* type, Xt::DomBlock* dom, const char* format);

struct EstField : public LvField
{
	EstContext*			_ctx;
	Xt::FieldDom*		_fd;
	const char*			_format;

public:
	EstField(EstContext* ctx, Xt::FieldDom* fd, const char* format) :
		_ctx(ctx),
		_fd(fd),
		_format(format)
	{
	}

public:
	virtual bool Visit(LfStruct* field)
	{
		XtString typeFormat;

		if (_ctx->formatter && _ctx->formatter->GetFormatByType(field->type, typeFormat))
			return Est_DoString(_ctx, field->type, AsBlock(_fd->dom), typeFormat);

		return _ctx->Append("(") && _ctx->Append(field->type->label) && _ctx->Append(")");
	}

	virtual bool Visit(LfInt* field)
	{
		const XtString& text = AsText(_fd->dom);

		if (*_format == 'i') return _ctx->Append(Format("%i", atoi(text)));
		if (*_format == 'x') return _ctx->Append(Format("%*x", atoi(_format + 1), atoi(text)));
		if (*_format == 'X') return _ctx->Append(Format("%*X", atoi(_format + 1), atoi(text)));

		return _ctx->Append(text);
	}

	virtual bool Visit(LfFloat* field)
	{
		const XtString& text = AsText(_fd->dom);

		if (*_format == 'f') return _ctx->Append(Format("%.*f", atoi(_format + 1), atof(text)));
		if (*_format == 'g') return _ctx->Append(Format("%.*g", atoi(_format + 1), atof(text)));

		return _ctx->Append(text);
	}

	virtual bool Visit(LfString* field)
	{
		const XtString& text = AsText(_fd->dom);

		if (*_format == 'n')
			return _ctx->Append(SplitFileExt(text.Text()));

		return _ctx->Append(text);
	}

	virtual bool Visit(LfWideString* field)
	{
		const XtString& text = AsText(_fd->dom);

		if (*_format == 'n')
			return _ctx->Append(SplitFileExt(text.Text()));

		return _ctx->Append(text);
	}

	virtual bool Visit(LfBool* field)
	{
		return _ctx->Append((atoi(AsText(_fd->dom)) != 0) ? "true" : "false");
	}

	virtual bool Visit(LfEnum* field)
	{
		LarvaOption* op = FindOptionByName(field->type, AsText(AsField(_fd->dom)));

		if (!op && (field->type->options.count > 0))
			op = field->type->options[0];

		if (op)
		{
			XtString optionFormat;

			if (_ctx->formatter && _ctx->formatter->GetFormatByOption(op, optionFormat))
				return _ctx->Append(optionFormat);

			return _ctx->Append(op->name);
		}

		return _ctx->Append("(") && _ctx->Append(field->type->label) && _ctx->Append(")");
	}

	virtual bool Visit(LfFlags* field)
	{
		return _ctx->Append("(") && _ctx->Append(field->type->label) && _ctx->Append(")"); // TODO: Expand out true flags
	}

	virtual bool Visit(LfStrictPtr* field)
	{
		XtString typeFormat;

		if (Xt::DomComponent* component = AsComponent(_fd->dom))
		{
			if (_ctx->formatter && _ctx->formatter->GetFormatByType(field->type, typeFormat))
				return Est_DoString(_ctx, field->type, &component->data, typeFormat);

			return _ctx->Append("[") && _ctx->Append(field->type->label) && _ctx->Append("-") && _ctx->Append(Format("%08X", component->id)) && _ctx->Append("]");
		}

		return _ctx->Append("[") && _ctx->Append(field->type->label) && _ctx->Append("-00000000]");
	}

	virtual bool Visit(LfPtr* field)
	{
		if (Xt::DomComponent* component = AsComponent(_fd->dom))
		{
			if (LtStruct* type = TypeOf(_ctx->schema, field->baseType, component))
			{
				XtString typeFormat;

				if (_ctx->formatter && _ctx->formatter->GetFormatByType(type, typeFormat))
					return Est_DoString(_ctx, type, &component->data, typeFormat);

				return _ctx->Append("[") && _ctx->Append(type->label) && _ctx->Append("-") && _ctx->Append(Format("%08X", component->id)) && _ctx->Append("]");
			}
		}

		return _ctx->Append("(null)");
	}

	virtual bool Visit(LfWeakPtr* field)
	{
		if (XtId id = AsId(_fd->dom))
		{
			if (Xt::DomRecord* record = RecordOf(_fd->dom))
			{
				XtString cached;

				if (_ctx->formatter && _ctx->formatter->FindComponentText(record, id, cached))
					return _ctx->Append(cached);

				if (Xt::TypeComponent tc = FindComponentById(_ctx->schema, record, id))
				{
					if (IsBaseOf(field->baseType, tc.type))
					{
						bool		result	= false;
						const char*	currPtr	= _ctx->outPtr;

						XtString typeFormat;

						if (_ctx->formatter && _ctx->formatter->GetFormatByType(tc.type, typeFormat))
							result = Est_DoString(_ctx, tc.type, &tc.component->data, typeFormat);
						else
							result = _ctx->Append(Format("[%s-%08X]", tc.type->name.Text(), id));

						*_ctx->outPtr = '\0';

						if (result && _ctx->formatter)
							_ctx->formatter->CacheComponentText(record, id, currPtr);

						return result;
					}
				}
			}
		}

		return _ctx->Append("(null)");
	}

	virtual bool Visit(LfHandle* field)
	{
		if (Xt::DomRecord* record = FindRecordById(_ctx->db, Xt::ParseId(AsText(_fd->dom))))
		{
			if (LtStruct* type = TypeOf(_ctx->schema, record))
			{
				XtString recordFormat;

				if (_ctx->formatter && _ctx->formatter->GetFormatByType(type, recordFormat))
					return Est_DoString(_ctx, type, &record->root.data, recordFormat);

				if (*_format == 'n')
				{
					int index = LastIndexOf(record->name, ".");

					if (index >= 0)
						return _ctx->Append(Substring(record->name, index + 1));
				}

				return _ctx->Append(record->name);
			}
		}

		return _ctx->Append("(null)");
	}

	virtual bool Visit(LfArray* field)
	{
		return _ctx->Append("(") && _ctx->Append(field->label) && _ctx->Append(")");
	}

	virtual bool Visit(LfLocString* field)
	{
		if (Xt::DomBlock* block = AsBlock(_fd->dom))
		{
			Xt::DomField* fieldRec	= AsField(At(block, "record"));
			Xt::DomField* fieldId	= AsField(At(block, "id"));

			if (fieldRec && fieldId)
			{
				if (Xt::DomRecord* record = FindRecordById(_ctx->db, Xt::ParseId(AsText(fieldRec))))
				{
					if (Xt::DomArray* strings = AsArray(At(&record->root.data, "strings")))
					{
						if (Xt::DomBlock* entry = AsBlock(At(&strings->elements, Xt::ParseId(AsText(fieldId)))))
						{
							return _ctx->Append(AsText(At(entry, "name")));
						}
					}
				}
			}
		}

		return _ctx->Append("(null)");
	}

	virtual bool Visit(LfControl* field)
	{
		return _ctx->Append("(") && _ctx->Append(field->label) && _ctx->Append(")");
	}
};

// Est_DoElement

bool Est_DoElementSub(EstContext* ctx, LtStruct* type, Xt::DomBlock* dom, const char* element, const char* format)
{
	if (strcmp(element, "__id") == 0)
	{
		if (Xt::DomComponent* component = ComponentOf(dom))
		{
			return ctx->Append(Format("%08X", component->id));
		}

		return ctx->Append("00000000");
	}

	if (strcmp(element, "__name") == 0)
	{
		if (Xt::DomRecord* record = RecordOf(dom))
		{
			if (*format == 'n')
			{
				int idx = LastIndexOf(record->name, ".");

				if (idx >= 0)
					return ctx->Append(Substring(record->name, idx + 1, record->name.Length()));
			}

			return ctx->Append(record->name);
		}

		return ctx->Append("(null)");
	}

	if (strcmp(element, "__type") == 0)
		return ctx->Append(type->label);

	Xt::FieldDom fd = GetByDottedPath(type, dom, element);

	if (fd.field)
		return fd.field->Accept(&EstField(ctx, &fd, format));

	return ctx->Append("(?") && ctx->Append(element) && ctx->Append("?)");
}

bool Est_DoElement(EstContext* ctx, LtStruct* type, Xt::DomBlock* dom, const char* element, const char* format)
{
	if (ctx->recursionDepth > kEstMaxRecursionDepth)
		return ctx->Append("(...)");

	ctx->recursionDepth++;

	bool result = Est_DoElementSub(ctx, type, dom, element, format);

	ctx->recursionDepth--;

	return result;
}

// Est_DoString

bool Est_DoString(EstContext* ctx, LtStruct* type, Xt::DomBlock* dom, const char* format)
{
	if (!ctx || !type || !dom || !format)
		return false;

	for(; *format != '\0'; format++)
	{
		if (*format != '{')
		{
			if (!ctx->Append(*format))
				return false;

			continue;
		}

		const char* open	= format;
		const char* colon	= 0;

		for(; *format != '}'; format++)
		{
			if (*format == '\0')
				return ctx->Append("(?missing brace?)");

			if (!colon && (*format == ':'))
				colon = format;
		}

		if (open >= format)
			continue;

		XtString tag;
		XtString fmt;

		if (colon)
		{
			tag.Set(open + 1, (int)((colon - open) - 1));
			fmt.Set(colon + 1, (int)((format - colon) - 1));
		}
		else
			tag.Set(open + 1, (int)((format - open) - 1));

		if (tag.Length() > 0)
		{
			if (!Est_DoElement(ctx, type, dom, tag, fmt))
				return false;
		}
	}

	return true;
}

// EvaluateStructuredText

XtString EvaluateStructuredText(Xt::DomDb* db, LarvaSchema* schema, LtStruct* type, Xt::DomBlock* block, const char* format, EstFormatter* formatter)
{
	if (!schema || !type || !block || !format || !*format)
		return kXtNullString;

	char buffer[kEstMaxLength];

	EstContext ctx(db, schema, formatter, buffer, buffer + kEstMaxLength - 1);

	if (!Est_DoString(&ctx, type, block, format))
	{
		DebugLn("EvaluateStructuredText overflow: %s", format);
	}

	*ctx.outPtr = '\0';

	return XtsStripBoth(buffer, ' ');
}

XtString EvaluateStructuredText(Xt::DomDb* db, LarvaSchema* schema, LtStruct* type, Xt::DomComponent* component, const char* format, EstFormatter* formatter)
{
	if (!component)
		return kXtNullString;

	return EvaluateStructuredText(db, schema, type, &component->data, format, formatter);
}

XtString EvaluateStructuredText(Xt::DomDb* db, LarvaSchema* schema, LarvaField* field, Xt::DomElement* element, EstFormatter* formatter)
{
	if (!schema || !field || !element)
		return kXtNullString;

	char buffer[kEstMaxLength];

	EstContext ctx(db, schema, formatter, buffer, buffer + kEstMaxLength - 1);

	if (!field->Accept(&EstField(&ctx, &Xt::FieldDom(field, element), "")))
	{
		DebugLn("EvaluateStructuredText overflow: *field*");
	}

	*ctx.outPtr = '\0';

	return XtsStripBoth(buffer, ' ');
}