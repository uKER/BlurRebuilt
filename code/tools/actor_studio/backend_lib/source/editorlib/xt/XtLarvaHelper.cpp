#include "stdafx.h"

#include <EditorLib/Xt/XtLarvaHelper.h>

namespace EditorLib
{
namespace XtLarvaHelpers
{

LtStruct* FindLarvaType(LarvaSchema* larvaSchema, const Btl::String& typeName, E_XtSchemaType filter)
{
	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&larvaSchema->types); it; ++it)
	{
		if (it->kind != kLtStruct)
			continue;

		LtStruct* pType = (LtStruct*)(LarvaType*)it;

		if (filter == E_XtSchemaType_Record && (!pType->isRecord))
			continue;
		else if (filter == E_XtSchemaType_Other && pType->isRecord)
			continue;

		if (typeName == (const char*)(pType->name))
			return pType;
	}

	return 0;
}

}
}