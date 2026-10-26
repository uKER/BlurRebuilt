#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "XtFlaskHelpers.h"

const ExportString* Find(const ExportStringTable* st, const XtString& string, ExportStringEncoding encoding)
{
	return XtUt::At(&const_cast<ExportStringTable*>(st)->entries, ExportStringKey(string, encoding));
}

const ExportString* Add(ExportStringTable* st, const XtString& string, ExportStringEncoding encoding)
{
	if (const ExportString* entry = Find(st, string, encoding))
		return entry;

	ExportString* entry = new ExportString;

	if (!XtUt::Insert(&st->entries, ExportStringKey(string, encoding), entry))
		return 0;

	if (encoding == kEseUnicode)
		st->offset = XtUt::AlignUpper(st->offset, 2u);

	entry->text		= string;
	entry->offset	= st->offset;
	entry->encoding	= encoding;

	if (encoding == kEseUnicode)
	{
		entry->uniText = MbToWc(string);

		st->offset += (entry->uniText.Length() + 1) * 2;
	}
	else
		st->offset += string.Length() + 1;

	return entry;
}

bool EmitStrings(ExportStringTable* strings, XtUt::Block* data, XtPackBuildContext* pb)
{
	if (strings->offset <= 0)
	{
		Free(data);

		return true;
	}

	if (!Allocate(data, XtUt::AlignUpper(strings->offset, kXtFlaskAlign)))
		return false;

	XtUt::Clear(data);

	for(XtUt::Dictionary<ExportStringKey, ExportString>::Iterator it(&strings->entries); it; ++it)
	{
		ExportString* string = it;

		if (string->encoding == kEseUnicode)
		{
			size_t length	= (string->uniText.Length() + 1) * 2;
			size_t written	= XtUt::Write(data, string->offset, string->uniText.Text(), length);

			if (XtGetEndian() != pb->targetProfile->endian)
			{
				XtU16* ptr = (XtU16*)(data->start + string->offset);
				XtU16* end = (XtU16*)(data->start + string->offset + written);

				for(; ptr < end; ptr++)
					*ptr = Swizzle(*ptr);
			}
		}
		else
			Write(data, string->offset, (const char*)string->text, string->text.Length() + 1);
	}

	XtFlaskEncodeStrings(data->start, Capacity(data));

	return true;
}

// ExportFieldPrep

ExportFieldPrep::~ExportFieldPrep()
{
}

XtString ExportFieldPrep::Eval(Xt::DomElement* dom)
{
	return AsText(dom);
}

// EfpMul

EfpMul::~EfpMul()
{
}

XtString EfpMul::Eval(Xt::DomElement* dom)
{
	double val = atof(AsText(dom));
	double mul = atof(AsText(At(AsBlock(dom ? dom->parent : 0), fieldName)));

	return Format("%f", val * mul);
}

// EfpRecordName

EfpRecordName::~EfpRecordName()
{
}

XtString EfpRecordName::Eval(Xt::DomElement* dom)
{
	XtId id = Xt::ParseId(AsText(At(AsBlock(dom ? dom->parent : 0), fieldName)));

	if (Xt::DomDb* db = DbOf(dom))
	{
		if (Xt::DomRecord* record = FindRecordById(db, id))
			return record->name;
	}

	return kXtNullString;
}

//

ExportType* NewType(ExportSchema* schema)
{
	return XtUt::AppendEx(&schema->types, new ExportType(schema->types.count));
}

ExportBase* NewBase(ExportType* type)
{
	return XtUt::AppendEx(&type->bases, new ExportBase(type->bases.count));
}

ExportField* NewField(ExportType* type)
{
	return XtUt::AppendEx(&type->fields, new ExportField(type->fields.count));
}

ExportType* FindTypeByName(ExportSchema* schema, const char* name)
{
	if (!*name)
		return 0;

	ExportType** type = schema->lookup[name];

	return type ? *type : 0;
}

ExportType* FindTypeByToken(ExportSchema* schema, const Grimoire::Token* token)
{
	if ((token->end - token->start) <= 0)
		return 0;

	for(size_t i = 0; i < schema->types.count; i++)
		if (*token == (const char*)schema->types[i]->name)
			return schema->types[i];

	return 0;
}

ExportField* FindFieldByName(ExportType* type, const char* name)
{
	if (!*name)
		return 0;

	for(size_t i = 0; i < type->fields.count; i++)
		if (type->fields[i]->name == name)
			return type->fields[i];

	return 0;
}

XtU32 SizeOf(ExportField* field)
{
	if (field->array)
		return sizeof(FlaskArray);
	else
	{
		switch(field->behaviour)
		{
			case kXtFbPtr:
			case kXtFbImplWeak:
			return sizeof(FlaskPtr);

			case kXtFbHandle:
			return sizeof(FlaskHandle);

			default:
			return field->type->size;
		}
	}
}

XtU32 SizeOf(ExportComponent* component)
{
	switch(component->behaviour)
	{
		case kXtCbObject:	return component->type->size;
		case kXtCbPtr:		return sizeof(FlaskPtr);
		case kXtCbHandle:	return sizeof(FlaskHandle);
		default:			return 0;
	}
}

XtU32 SizeOf(CompileComponent* component)
{
	switch(component->behaviour)
	{
		case kXtCbObject:	return component->type->size;
		case kXtCbPtr:		return sizeof(FlaskPtr);
		case kXtCbHandle:	return sizeof(FlaskHandle);
		default:			return 0;
	}
}

XtU32 HandleCountOf(CompileComponent* component)
{
	switch(component->behaviour)
	{
		case kXtCbObject:	return component->type->totalHandleCount;
		case kXtCbPtr:		return 0;
		case kXtCbHandle:	return 1;
		default:			return 0;
	}
}

bool FinaliseExportSchema(ExportSchema* schema)
{
	// Type list

	XtUt::Array<ExportType*> types;

	if (!Allocate(&types, 0, schema->types.count))
		return false;

	for(size_t i = 0; i < schema->types.count; i++)
		Append(&types, static_cast<ExportType*>(schema->types[i]));

	// Sorted type list

	XtUt::Array<ExportType*> sortedTypes;

	if (!Allocate(&sortedTypes, 0, schema->types.count))
		return false;

	while(sortedTypes.count < types.count)
	{
		int count = 0;

		for(size_t i = 0; i < types.count; i++)
		{
			ExportType* type = types[i];

			if (!type)
				continue;

			bool dependenciesMet = true;

			for(size_t j = 0; j < type->bases.count; j++)
			{
				if (types[type->bases[j]->type->index] != 0)
				{
					dependenciesMet = false;
					break;
				}
			}

			for(size_t j = 0; j < type->fields.count; j++)
			{
				ExportField* field = type->fields[j];

				if (
						(field->behaviour == kXtFbPtr) ||
						(field->behaviour == kXtFbHandle) ||
						(field->behaviour == kXtFbLabel) ||
						(field->behaviour == kXtFbImplWeak)
					)
				{
					continue;
				}

				if (types[field->type->index] != 0)
				{
					dependenciesMet = false;
					break;
				}
			}

			if (dependenciesMet)
			{
				Append(&sortedTypes, types[i]);
				types[i] = 0;
				count++;
			}
		}

		if (count == 0)
			return false;
	}

	// Set field offsets and type sizes

	for(size_t i = 0; i < sortedTypes.count; i++)
	{
		ExportType* type = sortedTypes[i];

		switch(type->behaviour)
		{
			case kXtTbStruct:
			{
				XtU32 offset			= 0;
				XtU32 totalFieldCount	= 0;
				XtU32 totalHandleCount	= 0;

				for(size_t j = 0; j < type->bases.count; j++)
				{
					ExportBase* base = type->bases[j];

					base->offset		= offset;
					base->fieldOffset	= totalFieldCount;

					offset				+= base->type->size;
					totalFieldCount		+= base->type->totalFieldCount;
					totalHandleCount	+= base->type->totalHandleCount;
				}

				for(size_t j = 0; j < type->fields.count; j++)
				{
					ExportField* field = type->fields[j];

					field->offset		= offset;
					field->fieldIndex	= totalFieldCount;

					XtU32 fieldCount	= 0;
					XtU32 handleCount	= 0;

					if (field->array)
						fieldCount = 1;
					else
					{
						switch(field->behaviour)
						{
							case kXtFbPtr:
							case kXtFbImplWeak:
								fieldCount = 1;
							break;

							case kXtFbHandle:
								fieldCount	= 1;
								handleCount	= 1;
							break;

							default:
								fieldCount	= field->type->totalFieldCount;
								handleCount	= field->type->totalHandleCount;
							break;
						}
					}

					offset				+= SizeOf(field);
					totalFieldCount		+= fieldCount;
					totalHandleCount	+= handleCount;
				}

				type->size				= offset;
				type->totalFieldCount	= totalFieldCount;
				type->totalHandleCount	= totalHandleCount;
			}
			break;

			case kXtTbAtom:
				switch(type->atom)
				{
					case kXtBool:	type->size = sizeof(XtU32); break;
					case kXtI8:		type->size = sizeof(XtI32); break;
					case kXtI16:	type->size = sizeof(XtI32); break;
					case kXtI32:	type->size = sizeof(XtI32); break;
					case kXtI64:	type->size = sizeof(XtI64); break;
					case kXtU8:		type->size = sizeof(XtU32); break;
					case kXtU16:	type->size = sizeof(XtU32); break;
					case kXtU32:	type->size = sizeof(XtU32); break;
					case kXtU64:	type->size = sizeof(XtU64); break;
					case kXtF32:	type->size = sizeof(XtF32); break;
					case kXtF64:	type->size = sizeof(XtF64); break;
					case kXtSz8:	type->size = sizeof(XtU32); break;
					case kXtSz16:	type->size = sizeof(XtU32); break;
					case kXtLocId:	type->size = sizeof(XtU32); break;

					default:
						assert(0);
					return false;
				}
			break;

			case kXtTbEnum:
				type->size = sizeof(XtU32);
			break;

			case kXtTbFlags:
				type->size = sizeof(XtU32);
			break;

			default:
				assert(0);
			return false;
		}
	}

	for(size_t i = 0; i < sortedTypes.count; i++)
	{
		if (!XtUt::Insert(&schema->lookup, sortedTypes[i]->name, new ExportType*(sortedTypes[i])))
			return false;
	}

	return true;
}

bool CreateExportSchema(Grimoire::Dom* grim, ExportSchema* schema, Xt::StatusCallback* status)
{
	// Select types to export

	XtUt::Array<Grimoire::Type*> selectedTypes;

	if (!Allocate(&selectedTypes, 0, grim->types.count))
		return false;

	for(size_t i = 0; i < grim->types.count; i++)
	{
		Grimoire::Type*	gt = grim->types[i];

		// Skip tool only types

		if (FindAttributeByName(&gt->attributes, "Tool"))
			continue;

		// Check for duplicates

		for(size_t j = 0; j < selectedTypes.count; j++)
		{
			if (gt->name == selectedTypes[j]->name)
			{
				Xt::StatusCallback::Error(status, "CreateExportSchema: Duplicate type name '%.*s'.", gt->name.end - gt->name.start, gt->name.start);
				return false; // TODO: Maybe this should be a warning, rather than an error
			}
		}

		// Confirmed as exportable type

		Append(&selectedTypes, gt);
	}

	// Create types

	if (!Allocate(&schema->types, 0, selectedTypes.count))
		return false;

	for(size_t i = 0; i < selectedTypes.count; i++)
	{
		Grimoire::Type*	gt = selectedTypes[i];
		ExportType*		et = NewType(schema);

		if (!et)
			return false;

		switch(gt->behaviour)
		{
			case Grimoire::kTbStruct:	et->behaviour = kXtTbStruct;	break;
			case Grimoire::kTbAtom:		et->behaviour = kXtTbAtom;		break;
			case Grimoire::kTbEnum:		et->behaviour = kXtTbEnum;		break;
			case Grimoire::kTbFlags:	et->behaviour = kXtTbFlags;		break;

			default:
			return false;
		}

		if (!TokenToString(&et->name, &gt->name))
			return false;

		if (gt->behaviour == Grimoire::kTbAtom)
		{
			Grimoire::Attribute* ga = FindAttributeByName(&gt->attributes, "Atom");

			if (!ga)
				return false;

			if (ga->parameters.count < 1)
				return false;

			if (!ga->parameters[0]->expression)
				return false;

			if (ga->parameters[0]->expression->token.symbol != Grimoire::kSymIdentifier)
				return false;
			
			if (ga->parameters[0]->expression->token == "bool")
				et->atom = kXtBool;
			else if (ga->parameters[0]->expression->token == "i8")
				et->atom = kXtI8;
			else if (ga->parameters[0]->expression->token == "i16")
				et->atom = kXtI16;
			else if (ga->parameters[0]->expression->token == "i32")
				et->atom = kXtI32;
			else if (ga->parameters[0]->expression->token == "i64")
				et->atom = kXtI64;
			else if (ga->parameters[0]->expression->token == "u8")
				et->atom = kXtU8;
			else if (ga->parameters[0]->expression->token == "u16")
				et->atom = kXtU16;
			else if (ga->parameters[0]->expression->token == "u32")
				et->atom = kXtU32;
			else if (ga->parameters[0]->expression->token == "u64")
				et->atom = kXtU64;
			else if (ga->parameters[0]->expression->token == "f32")
				et->atom = kXtF32;
			else if (ga->parameters[0]->expression->token == "f64")
				et->atom = kXtF64;
			else if (ga->parameters[0]->expression->token == "sz8")
				et->atom = kXtSz8;
			else if (ga->parameters[0]->expression->token == "sz16")
				et->atom = kXtSz16;
			else if (ga->parameters[0]->expression->token == "locid")
				et->atom = kXtLocId;
			else
			{
				return false;
			}
		}
	}

	for(size_t i = 0; i < selectedTypes.count; i++)
	{
		Grimoire::Type*	gt = selectedTypes[i];
		ExportType*		et = schema->types[i];

		// Create fields

		if (!Reserve(&et->fields, gt->members.count))
			return false;

		for(size_t j = 0; j < gt->members.count; j++)
		{
			Grimoire::Member* gm = gt->members[j];

			if ((gm->behaviour != Grimoire::kMbField) && (gm->behaviour != Grimoire::kMbLabel))
				continue;

			if (FindAttributeByName(&gm->attributes, "Tool"))
				continue;

			ExportField* ef = NewField(et);

			if (!ef)
				return false;

			if (!TokenToString(&ef->name, &gm->name))
				return false;

			for(size_t k = 0; k < (et->fields.count - 1); k++)
			{
				ExportField* dup = et->fields[k];

				if (ef->name == dup->name)
				{
					Xt::StatusCallback::Error(status, "CreateExportSchema: Duplicate field name '%s'.", (const char*)ef->name);
					return false;
				}
			}

			if (gm->behaviour == Grimoire::kMbField)
			{
				if ((ef->type = FindTypeByToken(schema, &gm->type)) == 0)
				{
					Xt::StatusCallback::Error(status, "CreateExportSchema: Unknown field type '%.*s'.", gm->type.end - gm->type.start, gm->type.start);
					return false;
				}

				if (FindAttributeByName(&gm->attributes, "Radix"))
				{
					ef->radix = atoi(GetParameterString(&gm->attributes, "Radix", 0));

					if ((ef->radix < 2) || (ef->radix > 36))
						ef->radix = 10;
				}

				if (Grimoire::Attribute* attr = FindAttributeByName(&gm->attributes, "Prep"))
				{
					for(XtUt::List<Grimoire::Parameter>::Iterator iParam(&attr->parameters); iParam; ++iParam)
					{
						Grimoire::Expression*	expr = iParam->expression;
						XtString				key;

						if (expr && (expr->token.symbol == Grimoire::kSymColon))
						{
							Grimoire::Atom a = EvaluateWithoutResolve(expr->left).AsAtom();

							if (!a)
							{
								Xt::StatusCallback::Warning(status, "Bad named parameter while parsing prep attributes");
								continue;
							}

							key		= a.name;
							expr	= expr->right;
						}

						if (!expr)
						{
							Xt::StatusCallback::Error(status, "Bad parameter value while parsing prep attributes");
							return false;
						}

						Grimoire::Variant val = Evaluate(expr);

						if (key == "mul")
						{
							EfpMul* mul;

							if ((ef->prep = mul = new EfpMul) == 0)
							{
								Xt::StatusCallback::Error(status, "CreateExportSchema: Out of memory allocating EfpMul");
								return false;
							}

							mul->fieldName = ToSz8(val);

							break;
						}
						else if (key == "recordName")
						{
							EfpRecordName* recordName;

							if ((ef->prep = recordName = new EfpRecordName) == 0)
							{
								Xt::StatusCallback::Error(status, "CreateExportSchema: Out of memory allocating EfpRecordName");
								return false;
							}

							recordName->fieldName = ToSz8(val);

							break;
						}
					}
				}

				if (FindAttributeByName(&gm->attributes, "Array"))
					ef->array = true;

				bool weak	= false;
				bool ptr	= false;
				bool handle	= false;

				if (FindAttributeByName(&gm->attributes, "Weak"))
					weak = true;

				if (FindAttributeByName(&gm->attributes, "Ptr"))
					ptr = true;

				if (FindAttributeByName(&gm->attributes, "WeakPtr"))
					weak = ptr = true;

				if (FindAttributeByName(&gm->attributes, "Handle"))
					handle = true;

				if (handle)
				{
					if (weak || ptr)
						return false;

					ef->behaviour = kXtFbHandle;
				}
				else if (weak)
				{
					if (!ptr)
						return false;

					ef->behaviour = kXtFbImplWeak;
				}
				else if (ptr)
					ef->behaviour = kXtFbPtr;
			}
			else
				ef->behaviour = kXtFbLabel;
		}

		// Create bases

		if (Grimoire::BaseType* gb = gt->base)
		{
			ExportBase* base = NewBase(et);

			if (!base)
				return false;

			if ((base->type = FindTypeByToken(schema, &gb->type)) == 0)
			{
				Xt::StatusCallback::Error(status, "CreateExportSchema: Unknown base type '%.*s'.", gb->type.end - gb->type.start, gb->type.start);
				return false;
			}
		}
	}

	for(size_t i = 0; i < schema->types.count; i++)
	{
		ExportType* et = schema->types[i];

		for(size_t j = 0; j < et->fields.count; j++)
		{
			ExportField* ef = et->fields[j];

			if (ef->behaviour == kXtFbImplInfer)
			{
				if ((et->behaviour == kXtTbEnum) || (et->behaviour == kXtTbFlags))
				{
					ef->behaviour = kXtFbLabel;
				}
				else
				{
					switch(ef->type->behaviour)
					{
						case kXtTbAtom:		ef->behaviour = kXtFbAtom;		break;
						case kXtTbEnum:		ef->behaviour = kXtFbEnum;		break;
						case kXtTbFlags:	ef->behaviour = kXtFbFlags;		break;
						case kXtTbStruct:	ef->behaviour = kXtFbStruct;	break;
					}
				}
			}

			switch(ef->behaviour)
			{
				case kXtFbPtr:
				case kXtFbHandle:
				case kXtFbImplWeak:
					if (ef->type->behaviour != kXtTbStruct)
						return false;
				break;
			}
		}
	}

	return FinaliseExportSchema(schema);
}