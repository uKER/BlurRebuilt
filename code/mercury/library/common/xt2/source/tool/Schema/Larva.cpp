#include "Common/XtPch.h"
#include "Xt/XtTool.h"

// Xt::LarvaParam

LarvaParam::~LarvaParam()
{
}

XtString LarvaParam::AsString()
{
	return kXtNullString;
}

double LarvaParam::AsFloat()
{
	return 0.0f;
}

XtI64 LarvaParam::AsInt()
{
	return 0;
}

bool LarvaParam::AsBool(bool defaultValue)
{
	return defaultValue;
}

XtU32 LarvaParam::AsColour(XtU32 defaultValue)
{
	return defaultValue;
}

LarvaAttrList* LarvaParam::AsAttr()
{
	return 0;
}

// Xt::LarvaParamString

XtString LarvaParamString::AsString()
{
	return value;
}

double LarvaParamString::AsFloat()
{
	return atof(value);
}

XtI64 LarvaParamString::AsInt()
{
	if (_strnicmp(value, "0x", 2) == 0)
		return strtol(value, 0, 16);

	return atoi(value);
}

bool LarvaParamString::AsBool(bool defaultValue)
{
	switch(_tolower_l(value[0], 0))
	{
		case 'f': return ((value.Length() == 1) || (_stricmp(value, "false") == 0)) ? false : defaultValue;
		case 't': return ((value.Length() == 1) || (_stricmp(value, "true") == 0)) ? true : defaultValue;

		case 'n': return ((value.Length() == 1) || (_stricmp(value, "no") == 0)) ? false : defaultValue;
		case 'y': return ((value.Length() == 1) || (_stricmp(value, "yes") == 0)) ? true : defaultValue;

		case '0': return (value.Length() == 1) ? false : defaultValue;
		case '1': return (value.Length() == 1) ? true : defaultValue;
	}

	return defaultValue;
}

XtU32 LarvaParamString::AsColour(XtU32 defaultValue)
{
	if (_strnicmp(value, "0x", 2) == 0)
	{
		return (strtol(value + 2, 0, 16)) & 0xFFFFFF;
	}
	else
	{
		if (_stricmp(value, "red") == 0)
			return 0xFF4020;
		else if (_stricmp(value, "green") == 0)
			return 0x40FF20;
		else if (_stricmp(value, "blue") == 0)
			return 0x2040FF;
		else if (_stricmp(value, "yellow") == 0)
			return 0xFFD020;
		else if (_stricmp(value, "magenta") == 0)
			return 0xFF20D0;
		else if (_stricmp(value, "cyan") == 0)
			return 0x20D0FF;
		else if (_stricmp(value, "white") == 0)
			return 0xD0D0D0;
		else if (_stricmp(value, "black") == 0)
			return 0x202020;
		else if (_stricmp(value, "lightred") == 0)
			return 0xFF9050;
		else if (_stricmp(value, "lightgreen") == 0)
			return 0x90FF50;
		else if (_stricmp(value, "lightblue") == 0)
			return 0x5090FF;
		else if (_stricmp(value, "lightyellow") == 0)
			return 0xFFD050;
		else if (_stricmp(value, "lightmagenta") == 0)
			return 0xFF50D0;
		else if (_stricmp(value, "lightcyan") == 0)
			return 0x50D0FF;
	}

	return defaultValue;
}

// Xt::LarvaParamAttr

LarvaAttrList* LarvaParamAttr::AsAttr()
{
	return &attrs;
}

// Xt::LarvaAttr

LarvaParam* LarvaAttr::FindParam(const char* paramName)
{
	if (isdigit(*paramName))
		return At(&params, atoi(paramName));

	for(XtUt::List<LarvaParam>::Iterator it(&params); it; ++it)
		if (it->key == paramName)
			return it;

	return 0;
}

XtString LarvaAttr::GetString(const char* paramName, const XtString& defaultValue)
{
	if (LarvaParam* param = FindParam(paramName))
		return param->AsString();

	return defaultValue;
}

double LarvaAttr::GetFloat(const char* paramName, double defaultValue)
{
	if (LarvaParam* param = FindParam(paramName))
		return param->AsFloat();

	return defaultValue;
}

XtI64 LarvaAttr::GetInt(const char* paramName, XtI64 defaultValue)
{
	if (LarvaParam* param = FindParam(paramName))
		return param->AsInt();

	return defaultValue;
}

bool LarvaAttr::GetBool(const char* paramName, bool defaultValue)
{
	if (LarvaParam* param = FindParam(paramName))
		return param->AsBool(defaultValue);

	return defaultValue;
}

XtU32 LarvaAttr::GetColour(const char* paramName, XtU32 defaultValue)
{
	if (LarvaParam* param = FindParam(paramName))
		return param->AsColour(defaultValue);

	return defaultValue;
}

// Xt::LarvaAttrList

LarvaAttr* LarvaAttrList::FindAttr(const char* attrName)
{
	for(XtUt::List<LarvaAttr>::Iterator it(&attrs); it; ++it)
		if (it->name == attrName)
			return it;

	return 0;
}

LarvaParam* LarvaAttrList::FindParam(const char* attrName, const char* paramName)
{
	if (LarvaAttr* attr = FindAttr(attrName))
		return attr->FindParam(paramName);

	return 0;
}

XtString LarvaAttrList::GetString(const char* attrName, const char* paramName, const XtString& defaultValue)
{
	if (LarvaAttr* attr = FindAttr(attrName))
		return attr->GetString(paramName, defaultValue);

	return defaultValue;
}

double LarvaAttrList::GetFloat(const char* attrName, const char* paramName, double defaultValue)
{
	if (LarvaAttr* attr = FindAttr(attrName))
		return attr->GetFloat(paramName, defaultValue);

	return defaultValue;
}

XtI64 LarvaAttrList::GetInt(const char* attrName, const char* paramName, XtI64 defaultValue)
{
	if (LarvaAttr* attr = FindAttr(attrName))
		return attr->GetInt(paramName, defaultValue);

	return defaultValue;
}

bool LarvaAttrList::GetBool(const char* attrName, const char* paramName, bool defaultValue)
{
	if (LarvaAttr* attr = FindAttr(attrName))
		return attr->GetBool(paramName, defaultValue);

	return defaultValue;
}

XtU32 LarvaAttrList::GetColour(const char* attrName, const char* paramName, XtU32 defaultValue)
{
	if (LarvaAttr* attr = FindAttr(attrName))
		return attr->GetColour(paramName, defaultValue);

	return defaultValue;
}

// Xt::SchemaElement

SchemaElement::SchemaElement(SchemaElementKind kind_) :
	kind(kind_),
	sourceLine(0)
{
}

SchemaElement::~SchemaElement()
{
}

// Xt::LarvaField

LarvaField::LarvaField(SchemaElementKind kind_, LtStruct* container_) :
	SchemaElement(kind_),
	hidden(false),
	noLabelColon(false),
	layout(kLtlDefault),
	border(kLtbDefault),
	container(container_)
{
}

LarvaField::~LarvaField()
{
}

// Xt::LarvaOption

LarvaOption::LarvaOption() :
	SchemaElement(kSchemaOption)
{
}

LarvaOption::~LarvaOption()
{
}

// Xt::LarvaBaseType

LarvaBaseType::LarvaBaseType() :
	SchemaElement(kSchemaBaseType),
	type(0)
{
}

LarvaBaseType::~LarvaBaseType()
{
}

// Xt::LarvaType

LarvaType::LarvaType(SchemaElementKind kind_) :
	SchemaElement(kind_),
	layout(kLtlDefault),
	border(kLtbDefault),
	hidden(false)
{
}

LarvaType::~LarvaType()
{
}

LarvaAttr* LarvaType::FindAttr(const char* attrName)
{
	return attrs.FindAttr(attrName);
}

LarvaParam* LarvaType::FindParam(const char* attrName, const char* paramName)
{
	return attrs.FindParam(attrName, paramName);
}

XtString LarvaType::GetString(const char* attrName, const char* paramName, const XtString& defaultValue)
{
	if (LarvaParam* param = FindParam(attrName, paramName))
		return param->AsString();

	return defaultValue;
}

double LarvaType::GetFloat(const char* attrName, const char* paramName, double defaultValue)
{
	if (LarvaParam* param = FindParam(attrName, paramName))
		return param->AsFloat();

	return defaultValue;
}

XtI64 LarvaType::GetInt(const char* attrName, const char* paramName, XtI64 defaultValue)
{
	if (LarvaParam* param = FindParam(attrName, paramName))
		return param->AsInt();

	return defaultValue;
}

bool LarvaType::GetBool(const char* attrName, const char* paramName, bool defaultValue)
{
	if (LarvaParam* param = FindParam(attrName, paramName))
		return param->AsBool(defaultValue);

	return defaultValue;
}

XtU32 LarvaType::GetColour(const char* attrName, const char* paramName, XtU32 defaultValue)
{
	if (LarvaParam* param = FindParam(attrName, paramName))
		return param->AsColour(defaultValue);

	return defaultValue;
}

// Xt::LarvaSchema

LarvaSchema::LarvaSchema()
{
}

LarvaSchema::~LarvaSchema()
{
}

// Xt::LtStruct

LtStruct::LtStruct() :
	LarvaType(kLtStruct),
	base(0),
	isRecord(false),
	hasExport(false)
{
}

LtStruct::~LtStruct()
{
	delete base;
}

bool LtStruct::Accept(LvType* visitor)
{
	return visitor->Visit(this);
}

bool LtStruct::Accept(LvTypeDom* visitor, Xt::DomElement* element)
{
	return visitor->Visit(this, AsBlock(element));
}

LarvaAttr* LtStruct::FindAttr(const char* attrName)
{
	if (LarvaAttr* attr = attrs.FindAttr(attrName))
		return attr;

	if (base)
	{
		if (LarvaAttr* attr = base->type->FindAttr(attrName))
			return attr;
	}

	return 0;
}

LarvaParam* LtStruct::FindParam(const char* attrName, const char* paramName)
{
	if (LarvaParam* param = attrs.FindParam(attrName, paramName))
		return param;

	if (base)
	{
		if (LarvaParam* param = base->type->FindParam(attrName, paramName))
			return param;
	}

	return 0;
}

// Xt::LtAtom

LtAtom::LtAtom(XtAtomType atomType_) :
	LarvaType(kLtAtom),
	atomType(atomType_)
{
}

LtAtom::~LtAtom()
{
}

bool LtAtom::Accept(LvType* visitor)
{
	return visitor->Visit(this);
}

bool LtAtom::Accept(LvTypeDom* visitor, Xt::DomElement* element)
{
	return visitor->Visit(this, AsField(element));
}

// Xt::LtEnum

LtEnum::LtEnum() :
	LarvaType(kLtEnum)
{
}

LtEnum::~LtEnum()
{
}

bool LtEnum::Accept(LvType* visitor)
{
	return visitor->Visit(this);
}

bool LtEnum::Accept(LvTypeDom* visitor, Xt::DomElement* element)
{
	return visitor->Visit(this, AsField(element));
}

// Xt::LtFlags

LtFlags::LtFlags() :
	LarvaType(kLtFlags)
{
}

LtFlags::~LtFlags()
{
}

bool LtFlags::Accept(LvType* visitor)
{
	return visitor->Visit(this);
}

bool LtFlags::Accept(LvTypeDom* visitor, Xt::DomElement* element)
{
	return visitor->Visit(this, AsBlock(element));
}

// Xt::LfStruct

LfStruct::LfStruct(LtStruct* container_, LtStruct* type_) :
	LarvaField(kLfStruct, container_),
	type(type_)
{
}

LfStruct::~LfStruct()
{
}

bool LfStruct::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfStruct::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireBlock(element, this, method));
}

LarvaType* LfStruct::Type()
{
	return type;
}

// Xt::LfInt

LfInt::LfInt(LtStruct* container_, LtAtom* type_) :
	LarvaField(kLfInt, container_),
	type(type_),
	radix(10)
{
}

LfInt::~LfInt()
{
}

bool LfInt::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfInt::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireField(element, this, method));
}

LarvaType* LfInt::Type()
{
	return type;
}

// Xt::LfFloat

LfFloat::LfFloat(LtStruct* container_, LtAtom* type_) :
	LarvaField(kLfFloat, container_),
	type(type_)
{
}

LfFloat::~LfFloat()
{
}

bool LfFloat::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfFloat::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireField(element, this, method));
}

LarvaType* LfFloat::Type()
{
	return type;
}

// Xt::LfString

LfString::LfString(LtStruct* container_, LtAtom* type_) :
	LarvaField(kLfString, container_),
	type(type_),
	lines(0)
{
}

LfString::~LfString()
{
}

bool LfString::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfString::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireField(element, this, method));
}

LarvaType* LfString::Type()
{
	return type;
}

// Xt::LfWideString

LfWideString::LfWideString(LtStruct* container_, LtAtom* type_) :
	LarvaField(kLfWideString, container_),
	type(type_),
	lines(0)
{
}

LfWideString::~LfWideString()
{
}

bool LfWideString::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfWideString::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireField(element, this, method));
}

LarvaType* LfWideString::Type()
{
	return type;
}

// Xt::LfBool

LfBool::LfBool(LtStruct* container_, LtAtom* type_) :
	LarvaField(kLfBool, container_),
	type(type_)
{
	noLabelColon = true;
}

LfBool::~LfBool()
{
}

bool LfBool::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfBool::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireField(element, this, method));
}

LarvaType* LfBool::Type()
{
	return type;
}

// Xt::LfEnum

LfEnum::LfEnum(LtStruct* container_, LtEnum* type_) :
	LarvaField(kLfEnum, container_),
	type(type_)
{
}

LfEnum::~LfEnum()
{
}

bool LfEnum::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfEnum::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireField(element, this, method));
}

LarvaType* LfEnum::Type()
{
	return type;
}

// Xt::LfFlags

LfFlags::LfFlags(LtStruct* container_, LtFlags* type_) :
	LarvaField(kLfFlags, container_),
	type(type_)
{
}

LfFlags::~LfFlags()
{
}

bool LfFlags::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfFlags::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireBlock(element, this, method));
}

LarvaType* LfFlags::Type()
{
	return type;
}

// Xt::LfStrictPtr

LfStrictPtr::LfStrictPtr(LtStruct* container_, LtStruct* type_) :
	LarvaField(kLfStrictPtr, container_),
	type(type_)
{
}

LfStrictPtr::~LfStrictPtr()
{
}

bool LfStrictPtr::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfStrictPtr::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireComponent(element, this, method));
}

LarvaType* LfStrictPtr::Type()
{
	return type;
}

// Xt::LfPtr

LfPtr::LfPtr(LtStruct* container_, LtStruct* baseType_) :
	LarvaField(kLfPtr, container_),
	baseType(baseType_)
{
}

LfPtr::~LfPtr()
{
}

bool LfPtr::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfPtr::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireComponent(element, this, method));
}

LarvaType* LfPtr::Type()
{
	return baseType;
}

// Xt::LfWeakPtr

LfWeakPtr::LfWeakPtr(LtStruct* container_, LtStruct* baseType_) :
	LarvaField(kLfWeakPtr, container_),
	baseType(baseType_)
{
}

LfWeakPtr::~LfWeakPtr()
{
}

bool LfWeakPtr::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfWeakPtr::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireRef(element, this, method));
}

LarvaType* LfWeakPtr::Type()
{
	return baseType;
}

// Xt::LfHandle

LfHandle::LfHandle(LtStruct* container_, LtStruct* baseType_) :
	LarvaField(kLfHandle, container_),
	baseType(baseType_)
{
}

LfHandle::~LfHandle()
{
}

bool LfHandle::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfHandle::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireField(element, this, method));
}

LarvaType* LfHandle::Type()
{
	return baseType;
}

// Xt::LfArray

LfArray::LfArray(LtStruct* container_) :
	LarvaField(kLfArray, container_),
	field(0)
{
}

LfArray::~LfArray()
{
	delete field;
}

bool LfArray::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfArray::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireArray(element, this, method));
}

LarvaType* LfArray::Type()
{
	return field ? field->Type() : 0;
}

// Xt::LfLocString

LfLocString::LfLocString(LtStruct* container_, LtAtom* type_) :
	LarvaField(kLfLocString, container_),
	type(type_)
{
}

LfLocString::~LfLocString()
{
}

bool LfLocString::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfLocString::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, AcquireBlock(element, this, method));
}

LarvaType* LfLocString::Type()
{
	return type;
}

// Xt::LfControl

LfControl::LfControl(LtStruct* container_) :
	LarvaField(kLfControl, container_)
{
}

LfControl::~LfControl()
{
}

bool LfControl::Accept(LvField* visitor)
{
	return visitor->Visit(this);
}

bool LfControl::Accept(LvFieldDom* visitor, Xt::DomElement* element, Xt::IndexMethod method)
{
	return visitor->Visit(this, element);
}

LarvaType* LfControl::Type()
{
	return 0;
}