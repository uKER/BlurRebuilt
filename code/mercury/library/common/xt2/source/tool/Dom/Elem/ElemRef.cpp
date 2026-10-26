#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace Xt
{

	// Xt::DomRef

	DomRef::DomRef() :
		DomElement(kDomRef),
		value(0)
	{
	}

	DomRef::~DomRef()
	{
		Notify(this, &DomRemoved(this));
	}

	// Xt::DomRef::Set

	bool Set(DomRef* ref, XtId value)
	{
		if (!ref)
			return false;

		ref->value = value;

		Notify(ref, &DomRefChanged(ref));

		return true;
	}

}