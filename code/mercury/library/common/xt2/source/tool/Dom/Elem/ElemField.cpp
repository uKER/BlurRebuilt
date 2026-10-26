#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace Xt
{

	// Xt::DomField

	DomField::DomField() :
		DomElement(kDomField)
	{
	}

	DomField::~DomField()
	{
		Notify(this, &DomRemoved(this));
	}

	// Xt::DomField::Set

	bool Set(DomField* field, const XtString& value)
	{
		if (!field)
			return false;

		field->value = value;

		Notify(field, &DomFieldChanged(field));

		return true;
	}

	// Xt::DomField::Set

	bool Set(DomField* field, const char* value)
	{
		if (!field)
			return false;

		if (!field->value.Set(value))
			return false;

		Notify(field, &DomFieldChanged(field));

		return true;
	}

}