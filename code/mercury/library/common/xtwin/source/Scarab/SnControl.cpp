#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnControl::SnControl(LfControl* field) :
	_field(field)
{
}

SnControl::~SnControl()
{
}

LarvaAttr* SnControl::FindAttr(const char* name)
{
	return _field->attrs.FindAttr(name);
}

LarvaParam* SnControl::FindParam(LarvaAttr* attr, const char* key)
{
	return attr->FindParam(key);
}

LarvaParam* SnControl::FindParam(const char* attr, const char* param)
{
	return _field->attrs.FindParam(attr, param);
}

XtString SnControl::GetString(const char* attrName, const char* paramName, const XtString& defaultValue)
{
	return _field->attrs.GetString(attrName, paramName, defaultValue);
}

double SnControl::GetFloat(const char* attrName, const char* paramName, double defaultValue)
{
	return _field->attrs.GetFloat(attrName, paramName, defaultValue);
}

XtI64 SnControl::GetInt(const char* attrName, const char* paramName, int defaultValue)
{
	return _field->attrs.GetInt(attrName, paramName, defaultValue);
}

bool SnControl::GetBool(const char* attrName, const char* paramName, bool defaultValue)
{
	return _field->attrs.GetBool(attrName, paramName, defaultValue);
}

XtU32 SnControl::GetColour(const char* attrName, const char* paramName, XtU32 defaultValue)
{
	return _field->attrs.GetColour(attrName, paramName, defaultValue);
}