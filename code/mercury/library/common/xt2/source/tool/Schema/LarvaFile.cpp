#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "Xt/Impl/Grimoire.h"

namespace
{
	bool ParseAllAttributes(XtUt::List<Grimoire::Attribute>* attrs, LarvaAttrList* al, Xt::StatusCallback* status);

	bool ParseAttribute(LarvaAttr* attr, Grimoire::Attribute* iAttr, Xt::StatusCallback* status)
	{
		if (!attr)
			return false;

		if (!TokenToString(&attr->name, &iAttr->name))
			return false;

		for(XtUt::List<Grimoire::Parameter>::Iterator iParam(&iAttr->parameters); iParam; ++iParam)
		{
			Grimoire::Expression*	expr = iParam->expression;
			XtString				key;

			if (expr && (expr->token.symbol == Grimoire::kSymColon))
			{
				Grimoire::Atom a = EvaluateWithoutResolve(expr->left).AsAtom();

				if (!a)
				{
					Xt::StatusCallback::Error(status, "Bad named parameter while parsing control attributes:");
					Xt::StatusCallback::Trace(status, "%s", attr->name.Text());
					return false;
				}

				key		= a.name;
				expr	= expr->right;
			}

			if (!expr)
			{
				Xt::StatusCallback::Error(status, "Bad parameter value while parsing control attributes:");
				Xt::StatusCallback::Trace(status, "%s", attr->name.Text());
				return false;
			}

			Grimoire::Variant val = Evaluate(expr);

			if (val.Type() == Grimoire::kVariantAttr)
			{
				LarvaParamAttr* param = new LarvaParamAttr;

				if (!XtUt::AppendEx(&attr->params, (LarvaParam*)param))
					return false;

				param->key = key;

				if (Grimoire::ExpAttributes* attr = val.AsAttr())
				{
					if (!ParseAllAttributes(&attr->attributes, &param->attrs, status))
					{
						Xt::StatusCallback::Error(status, "Failed parsing child control attributes:");
						Xt::StatusCallback::Trace(status, "%s", key);
						return false;
					}
				}
			}
			else
			{
				LarvaParamString* param = new LarvaParamString;

				if (!XtUt::AppendEx(&attr->params, (LarvaParam*)param))
					return false;

				param->key		= key;
				param->value	= ToSz8(val);
			}
		}

		return true;
	}

	bool ParseAllAttributes(XtUt::List<Grimoire::Attribute>* attrs, LarvaAttrList* al, Xt::StatusCallback* status)
	{
		for(XtUt::List<Grimoire::Attribute>::Iterator iAttr(attrs); iAttr; ++iAttr)
		{
			if (!ParseAttribute(AppendEx(&al->attrs, new LarvaAttr), iAttr, status))
				return false;
		}

		return true;
	}

	bool SetupFieldControl(Grimoire::Member* member, LarvaField* field, Xt::StatusCallback* status)
	{
		if (!member || !field)
			return false;

		if (!ParseAllAttributes(&member->attributes, &field->attrs, status))
			return false;

		field->sourceLine = member->name.line;

		if (field->attrs.FindAttr("Hide"))
			field->hidden = true;

		if (field->attrs.FindAttr("NoLabelColon"))
			field->noLabelColon = true;

		if (LarvaAttr* attr = field->attrs.FindAttr("Tab"))
			field->tab = attr->GetString("0");

		if (LarvaAttr* attr = field->attrs.FindAttr("Group"))
			field->group = attr->GetString("0");

		return true;
	}

	bool SetupFieldMember(Grimoire::Member* member, LarvaField* field, Xt::StatusCallback* status)
	{
		if (!SetupFieldControl(member, field, status))
			return false;

		if (field->attrs.FindAttr("Vertical"))
			field->layout = kLtlVertical;
		else if (field->attrs.FindAttr("Horizontal"))
			field->layout = kLtlHorizontal;
		else if (field->attrs.FindAttr("Inline"))
			field->layout = kLtlInline;

		if (field->attrs.FindAttr("NoBorder"))
			field->border = kLtbNone;
		else if (field->attrs.FindAttr("Border"))
			field->border = kLtbNormal;

		return true;
	}
}

struct LvParseField : public LvType
{
	LarvaType*				_fieldType;
	Grimoire::Member*		_grim;
	XtUt::Ptr<LarvaField>	_field;
	LtStruct*				_containerType;
	Xt::StatusCallback*		_status;
	bool					_containedInArray;

public:
	inline LvParseField(LarvaType* fieldType, Grimoire::Member* grim, LtStruct* containerType, Xt::StatusCallback* status, bool containedInArray) :
		_fieldType(fieldType),
		_grim(grim),
		_field(0),
		_containerType(containerType),
		_status(status),
		_containedInArray(containedInArray)
	{
	}

public:
	LarvaField* SetField(LarvaField* field)
	{
		if ((_field = field) != 0)
		{
			if (!SetupFieldMember(_grim, field, _status))
				_field = 0;
		}

		return field;
	}

	LarvaField* PopField()
	{
		return XtUt::Pop(&_field);
	}

	template<typename Type>
	Type* SetField(Type* field)
	{
		return (Type*)SetField((LarvaField*)field);
	}

public:
	virtual bool Visit(LtStruct* type)
	{
		if (_containedInArray)
		{
			if (!SetField(new LfStrictPtr(_containerType, type)))
				return false;
		}
		else
		{
			if (!SetField(new LfStruct(_containerType, type)))
				return false;
		}

		return true;
	}

	virtual bool Visit(LtAtom* type)
	{
		switch(type->atomType)
		{
			case kXtBool:
			{
				LfBool* field = SetField(new LfBool(_containerType, type));

				if (!field)
					return false;

				field->defValue	= GetParameterString(&_grim->attributes, "Default", 0);

				return true;
			}
			return true;

			case kXtI8:
			case kXtI16:
			case kXtI32:
			case kXtI64:
			case kXtU8:
			case kXtU16:
			case kXtU32:
			case kXtU64:
			{
				LfInt* field = SetField(new LfInt(_containerType, type));

				if (!field)
					return false;

				field->defValue	= GetParameterString(&_grim->attributes, "Default", 0);
				field->radix	= GetParameterInt(&_grim->attributes, "Radix", 0);

				if ((field->radix < 2) || (field->radix > 36))
					field->radix = 10;
			}
			return true;

			case kXtF32:
			case kXtF64:
			{
				LfFloat* field = SetField(new LfFloat(_containerType, type));

				if (!field)
					return false;

				field->defValue	= GetParameterString(&_grim->attributes, "Default", 0);
			}
			return true;

			case kXtSz8:
			{
				LfString* field = SetField(new LfString(_containerType, type));

				if (!field)
					return false;

				field->lines	= (int)GetParameterInt(&_grim->attributes, "Lines", 0);
				field->defValue	= GetParameterString(&_grim->attributes, "Default", 0);
			}
			return true;

			case kXtSz16:
			{
				LfWideString* field = SetField(new LfWideString(_containerType, type));

				if (!field)
					return false;

				field->lines	= (int)GetParameterInt(&_grim->attributes, "Lines", 0);
				field->defValue	= GetParameterString(&_grim->attributes, "Default", 0);
			}
			return true;

			case kXtLocId:
			{
				LfLocString* field = SetField(new LfLocString(_containerType, type));

				if (!field)
					return false;
			}
			return true;
		}

		return false;
	}

	virtual bool Visit(LtEnum* type)
	{
		LfEnum* field = SetField(new LfEnum(_containerType, type));

		if (!field)
			return false;

		field->defValue	= GetParameterString(&_grim->attributes, "Default", 0);

		return true;
	}

	virtual bool Visit(LtFlags* type)
	{
		return SetField(new LfFlags(_containerType, type)) != 0;
	}
};

struct LvParseType : public LvType
{
	LarvaSchema*		_schema;
	Grimoire::Type*		_grimType;
	Xt::StatusCallback*	_status;

public:
	inline LvParseType(LarvaSchema* schema, Grimoire::Type* grimType, Xt::StatusCallback* status) :
		_schema(schema),
		_grimType(grimType),
		_status(status)
	{
	}

public:
	virtual bool Visit(LtStruct* type)
	{
		if (FindAttributeByName(&_grimType->attributes, "Record"))
			type->isRecord = true;

		return ParseBase(type) && ParseFields(type, &type->fields);
	}

	virtual bool Visit(LtEnum* type)	{ return ParseOptions(&type->options);	}
	virtual bool Visit(LtFlags* type)	{ return ParseOptions(&type->options);	}

public:
	bool ParseFields(LtStruct* containerType, XtUt::List<LarvaField>* fields)
	{
		for(XtUt::List<Grimoire::Member>::Iterator iMember(&_grimType->members); iMember; ++iMember)
		{
			XtString memberName;

			if (!TokenToString(&memberName, &iMember->name))
				return false;

			if (iMember->behaviour == Grimoire::kMbField)
			{
				XtString memberTypeName;

				if (!TokenToString(&memberTypeName, &iMember->type))
					return false;

				LarvaType* memberType = _schema->types[memberTypeName];

				if (!memberType)
				{
					Xt::StatusCallback::Error(_status, "Field uses undefined type");
					Xt::StatusCallback::Trace(_status, "Type: %s", memberTypeName.Text());
					Xt::StatusCallback::Trace(_status, "Field: %s::%s", containerType->name.Text(), memberName.Text());
					Xt::StatusCallback::Trace(_status, "Location: %s(%i)", containerType->sourceFile.Text(), iMember->name.line);
					return false;
				}

				LarvaField* member = CreateField(iMember, memberType, containerType);

				if (!member)
					continue;

				if (!AppendEx(fields, member))
					return false;

				member->name = memberName;

				if (FindAttributeByName(&iMember->attributes, "Label"))
					member->label = GetParameterString(&iMember->attributes, "Label", 0);
				else
					member->label = CleanUpLabel(memberName);

				if (FindAttributeByName(&iMember->attributes, "Export"))
					containerType->hasExport = true;
			}
			else if (iMember->behaviour == Grimoire::kMbControl)
			{
				LfControl* control = new LfControl(containerType);

				if (!control)
					continue;

				if (!AppendEx(fields, (LarvaField*)control))
					return false;

				if (!TokenToString(&control->type, &iMember->type))
					return false;

				control->label = GetParameterString(&iMember->attributes, "Label", 0);

				if (!SetupFieldControl(iMember, control, _status))
					return false;
			}
		}

		return true;
	}

	LarvaField* CreateField(Grimoire::Member* member, LarvaType* type, LtStruct* containerType)
	{
		if (FindAttributeByName(member, "Array"))
		{
			LfArray* lf = new LfArray(containerType);

			if (!lf)
				return 0;

			lf->field = CreateField_Ptr(member, type, containerType, true);

			if (!SetupFieldMember(member, lf, _status))
				return false;

			return lf;
		}

		return CreateField_Ptr(member, type, containerType, false);
	}

	LarvaField* CreateField_Ptr(Grimoire::Member* member, LarvaType* type, LtStruct* containerType, bool containedInArray)
	{
		bool weak	= false;
		bool ptr	= false;
		bool handle	= false;

		if (FindAttributeByName(&member->attributes, "Weak"))
			weak = true;

		if (FindAttributeByName(&member->attributes, "Ptr"))
			ptr = true;

		if (FindAttributeByName(&member->attributes, "WeakPtr"))
			weak = ptr = true;

		if (FindAttributeByName(&member->attributes, "Handle"))
			handle = true;

		if (handle)
		{
			if (weak || ptr)
				return 0;

			if (type->kind != kLtStruct)
			{
				Xt::StatusCallback::Error(_status, "Only structures may be used as a handle");
				return 0;
			}

			LfHandle* lf = new LfHandle(containerType, (LtStruct*)type);

			if (!lf)
				return 0;

			if (!SetupFieldMember(member, lf, _status))
			{
				delete lf;
				return 0;
			}

			return lf;
		}
		else if (weak)
		{
			if (!ptr)
				return 0;

			if (type->kind != kLtStruct)
			{
				Xt::StatusCallback::Error(_status, "Only structures may be used as a pointer");
				return 0;
			}

			LfWeakPtr* lf = new LfWeakPtr(containerType, (LtStruct*)type);

			if (!lf)
				return 0;

			if (!SetupFieldMember(member, lf, _status))
			{
				delete lf;
				return 0;
			}

			return lf;
		}
		else if (ptr)
		{
			if (type->kind != kLtStruct)
			{
				Xt::StatusCallback::Error(_status, "Only structures may be used as a weak pointer");
				return 0;
			}

			LfPtr* lf = new LfPtr(containerType, (LtStruct*)type);

			if (!lf)
				return 0;

			if (!SetupFieldMember(member, lf, _status))
			{
				delete lf;
				return 0;
			}

			lf->defValue = GetParameterString(&member->attributes, "Default", 0);

			return lf;
		}

		return CreateField_Base(member, type, containerType, containedInArray);
	}

	LarvaField* CreateField_Base(Grimoire::Member* member, LarvaType* type, LtStruct* containerType, bool containedInArray)
	{
		LvParseField visitor(type, member, containerType, _status, containedInArray);

		if (!type->Accept(&visitor))
			return 0;

		return visitor.PopField();
	}

private:
	bool ParseOptions(XtUt::List<LarvaOption>* options)
	{
		for(XtUt::List<Grimoire::Member>::Iterator iMember(&_grimType->members); iMember; ++iMember)
		{
			XtString memberName;

			if (!TokenToString(&memberName, &iMember->name))
				return false;

			LarvaOption* option = AppendEx(options,  new LarvaOption);

			if (!option || !ParseAllAttributes(&iMember->attributes, &option->attrs, _status))
				return false;

			option->name		= memberName;
			option->label		= GetParameterString(&iMember->attributes, "Label", 0);
			option->defValue	= GetParameterString(&iMember->attributes, "Default", 0);
			option->sourceLine	= iMember->name.line;

			if (option->label.Length() == 0)
				option->label = CleanUpLabel(memberName);

			if (Grimoire::Attribute* iAttr = FindAttributeByName(&iMember->attributes, "Summary"))
				option->summary = GetParameterString(&iMember->attributes, "Summary", 0);
			else
				option->summary = option->label;
		}

		return true;
	}

private:
	bool ParseBase(LtStruct* parentType)
	{
		if (_grimType->base)
		{
			XtString baseTypeName;

			if (!TokenToString(&baseTypeName, &_grimType->base->type))
				return false;

			LarvaType* lt = _schema->types[baseTypeName];

			if (!lt)
			{
				Xt::StatusCallback::Error(_status, "Undefined base type");
				Xt::StatusCallback::Trace(_status, "Type: %s", parentType->name.Text());
				Xt::StatusCallback::Trace(_status, "Base type: %s", baseTypeName.Text());
				Xt::StatusCallback::Trace(_status, "Location: %s(%i)", parentType->sourceFile.Text(), _grimType->base->type.line);

				return false;
			}

			LtStruct* baseTypeType = AsStruct(lt);

			if (!baseTypeType)
			{
				Xt::StatusCallback::Error(_status, "Illegal base type");
				Xt::StatusCallback::Trace(_status, "Type: %s", parentType->name.Text());
				Xt::StatusCallback::Trace(_status, "Base type: %s", baseTypeName.Text());
				Xt::StatusCallback::Trace(_status, "Location: %s(%i)", parentType->sourceFile.Text(), _grimType->base->type.line);

				return false;
			}

			LarvaBaseType* baseType = new LarvaBaseType;

			if (!baseType)
				return false;

			parentType->base	= baseType;
			baseType->type		= baseTypeType;

			if (!ParseAllAttributes(&_grimType->base->attributes, &baseType->attrs, _status))
				return false;

			baseType->hidden = baseType->attrs.FindAttr("Hide") != 0;
		}

		return true;
	}
};

bool PropagateHasExport(LtStruct* type)
{
	if (!type)
		return false;

	if (type->hasExport)
		return true;

	if (type->base)
	{
		if (PropagateHasExport(type->base->type))
			return true;
	}

	return false;
}

LarvaField* MlsFindFieldByNameIgnoring(LtStruct* type, const char* fieldName, LarvaField* ignoring)
{
	if (!type)
		return 0;

	for(LtStruct::FieldIterator it(&type->fields); it; ++it)
	{
		if (it == ignoring)
			continue;

		if (strcmp(it->name, fieldName) == 0)
			return it;
	}

	if (type->base)
	{
		if (LarvaField* field = MlsFindFieldByNameIgnoring(type->base->type, fieldName, ignoring))
			return field;
	}

	return 0;
}

bool MakeLarvaSchemaSub(Grimoire::Dom* grim, LarvaSchema* schema, Xt::StatusCallback* status)
{
	// Create types

	for(XtUt::List<Grimoire::Type>::Iterator iType(&grim->types); iType; ++iType)
	{
		XtString typeName;

		if (!TokenToString(&typeName, &iType->name))
			return false;

		if (LarvaType* dupType = At(&schema->types, typeName))
		{
			Xt::StatusCallback::Error(status, "Type redefinition");
			Xt::StatusCallback::Trace(status, "Type: %s", typeName.Text());
			Xt::StatusCallback::Trace(status, "Original location: %s(%i)", dupType->sourceFile.Text(), dupType->sourceLine);
			Xt::StatusCallback::Trace(status, "Duplicate location: %s(%i)", iType->document->filename.Text(), iType->name.line);
			return false;
		}

		LarvaType* type = 0;

		switch(iType->behaviour)
		{
			case Grimoire::kTbStruct:
				type = new LtStruct;
			break;

			case Grimoire::kTbAtom:
			{
				Grimoire::Attribute* ga = FindAttributeByName(&iType->attributes, "Atom");

				if (!ga || !ga->parameters[0] || !ga->parameters[0]->expression)
					return false;

				if (ga->parameters[0]->expression->token.symbol != Grimoire::kSymIdentifier)
					return false;

				if (ga->parameters[0]->expression->token == "bool")
					type = new LtAtom(kXtBool);
				else if (ga->parameters[0]->expression->token == "i8")
					type = new LtAtom(kXtI8);
				else if (ga->parameters[0]->expression->token == "i16")
					type = new LtAtom(kXtI16);
				else if (ga->parameters[0]->expression->token == "i32")
					type = new LtAtom(kXtI32);
				else if (ga->parameters[0]->expression->token == "i64")
					type = new LtAtom(kXtI64);
				else if (ga->parameters[0]->expression->token == "u8")
					type = new LtAtom(kXtU8);
				else if (ga->parameters[0]->expression->token == "u16")
					type = new LtAtom(kXtU16);
				else if (ga->parameters[0]->expression->token == "u32")
					type = new LtAtom(kXtU32);
				else if (ga->parameters[0]->expression->token == "u64")
					type = new LtAtom(kXtU64);
				else if (ga->parameters[0]->expression->token == "f32")
					type = new LtAtom(kXtF32);
				else if (ga->parameters[0]->expression->token == "f64")
					type = new LtAtom(kXtF64);
				else if (ga->parameters[0]->expression->token == "sz8")
					type = new LtAtom(kXtSz8);
				else if (ga->parameters[0]->expression->token == "sz16")
					type = new LtAtom(kXtSz16);
				else if (ga->parameters[0]->expression->token == "locid")
					type = new LtAtom(kXtLocId);
			}
			break;

			case Grimoire::kTbEnum:
				type = new LtEnum;
			break;

			case Grimoire::kTbFlags:
				type = new LtFlags;
			break;
		}

		if (!Insert(&schema->types, typeName, type))
			return false;

		if (!ParseAllAttributes(&iType->attributes, &type->attrs, status))
			return false;

		type->name			= typeName;
		type->label			= GetParameterString(&iType->attributes, "Label", 0);
		type->sourceFile	= iType->document->filename;
		type->sourceLine	= iType->name.line;

		if (_strnicmp(type->name, "Xt", 2) == 0)
		{
			if (!FindAttributeByName(&iType->attributes, "Internal"))
			{
				Xt::StatusCallback::Error(status, "User type using restricted prefix:");
				Xt::StatusCallback::Trace(status, "%s", type->name.Text());
				return false;
			}
		}

		if (type->label.Length() == 0)
			type->label = CleanUpLabel(typeName);

		if (FindAttributeByName(&iType->attributes, "Vertical"))
			type->layout = kLtlVertical;
		else if (FindAttributeByName(&iType->attributes, "Horizontal"))
			type->layout = kLtlHorizontal;
		else if (FindAttributeByName(&iType->attributes, "Inline"))
			type->layout = kLtlInline;

		if (FindAttributeByName(&iType->attributes, "NoBorder"))
			type->border = kLtbNone;
		else if (FindAttributeByName(&iType->attributes, "Border"))
			type->border = kLtbNormal;

		if (FindAttributeByName(&iType->attributes, "Hide"))
			type->hidden = true;

		if (FindAttributeByName(&iType->attributes, "Category"))
			type->category = GetParameterString(&iType->attributes, "Category", 0);

		if (Grimoire::Attribute* iAttr = FindAttributeByName(&iType->attributes, "Icon"))
			type->icon = iType->document->path + GetParameterString(&iType->attributes, "Icon", 0);
	}

	// Parse types

	for(XtUt::List<Grimoire::Type>::Iterator iType(&grim->types); iType; ++iType)
	{
		XtString typeName;

		if (!TokenToString(&typeName, &iType->name))
			return false;

		LarvaType* ltType = schema->types[typeName];

		if (!ltType)
			return false;

		if (!ltType->Accept(&LvParseType(schema, iType, status)))
			return false;
	}

	// Check for duplicate members

	for(LarvaSchema::TypeIterator itType(&schema->types); itType; ++itType)
	{
		if (LtStruct* type = AsStruct(itType))
		{
			for(LtStruct::FieldIterator itField(&type->fields); itField; ++itField)
			{
				if (itField->kind == kLfControl)
					continue;

				LarvaField* dupField = MlsFindFieldByNameIgnoring(type, itField->name, itField);

				if (dupField)
				{
					Xt::StatusCallback::Error(status, "Field redefinition");
					Xt::StatusCallback::Trace(status, "Original field: %s::%s", itType->name.Text(), itField->name.Text());
					Xt::StatusCallback::Trace(status, "Original location: %s(%i)", itType->sourceFile.Text(), itField->sourceLine);
					Xt::StatusCallback::Trace(status, "Duplicate field: %s::%s", dupField->container->name.Text(), dupField->name.Text());
					Xt::StatusCallback::Trace(status, "Duplicate location: %s(%i)", dupField->container->sourceFile.Text(), dupField->sourceLine);
					return false;
				}
			}
		}
		else if (LtEnum* type = AsEnum(itType))
		{
			for(LtEnum::OptionIterator itOption(&type->options); itOption; ++itOption)
			{
				LarvaOption* dupOption = FindOptionByName(type, itOption->name);

				if (dupOption != itOption)
				{
					Xt::StatusCallback::Error(status, "Label redefinition");
					Xt::StatusCallback::Trace(status, "Label: %s", itOption->name.Text());
					Xt::StatusCallback::Trace(status, "Original location: %s(%i)", itType->sourceFile.Text(), dupOption->sourceLine);
					Xt::StatusCallback::Trace(status, "Duplicate location: %s(%i)", itType->sourceFile.Text(), itOption->sourceLine);
					return false;
				}
			}
		}
		else if (LtFlags* type = AsFlags(itType))
		{
			for(LtFlags::OptionIterator itOption(&type->options); itOption; ++itOption)
			{
				LarvaOption* dupOption = FindOptionByName(type, itOption->name);

				if (dupOption != itOption)
				{
					Xt::StatusCallback::Error(status, "Label redefinition");
					Xt::StatusCallback::Trace(status, "Label: %s", itOption->name.Text());
					Xt::StatusCallback::Trace(status, "Original location: %s(%i)", itType->sourceFile.Text(), dupOption->sourceLine);
					Xt::StatusCallback::Trace(status, "Duplicate location: %s(%i)", itType->sourceFile.Text(), itOption->sourceLine);
					return false;
				}
			}
		}
	}

	// Propagate hasExport flag

	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&schema->types); it; ++it)
	{
		if (LtStruct* type = AsStruct(it))
			type->hasExport = PropagateHasExport(type);
	}

	return true;
}

LarvaSchema* CreateLarvaSchema(Grimoire::Dom* grim, Xt::StatusCallback* status)
{
	LarvaSchema* schema = new LarvaSchema;

	if (!schema)
		return 0;

	if (!MakeLarvaSchemaSub(grim, schema, status))
	{
		delete schema;
		return 0;
	}

	return schema;
}