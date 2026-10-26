#include "Common/XtPch.h"
#include "XtGameImpl.h"

XtGlobals gXt2;

XtType		gXtNullType		= { "__null", 0, 0, 0, 0, false, 0, kXtTbStruct, kXtNotAtom, 1, &Xt::Construct<XtU8>, 0, &Xt::Destruct<XtU8> };
XtRecord	gXtNullRecord;

void XtRegisterType(Xt::AnnotationType& type)
{
	if (gXt2.typeCount >= kXtMaxTypes)
	{
		DebugLn("%s(%i): Maximum XT_TYPE count exceeded, too many types declared. (Increase kXtMaxTypes?)", type.debugFile, type.debugLine);
		assert(0);
		return;
	}

	gXt2.types[gXt2.typeCount++] = &type;
}

bool XtInitialise()
{
	if (gXt2.references <= 0)
	{
		if ((gXt2.application = XtGenerateApplicationSchema()) == 0)
			return false;
	}

	gXt2.references++;

	return true;
}

void XtShutdown()
{
	if (--gXt2.references <= 0)
	{
		XtDestroySchema(gXt2.application);
		gXt2.application = 0;
	}
}

XtSchema* XtGetApplicationSchema()
{
	return gXt2.application;
}
