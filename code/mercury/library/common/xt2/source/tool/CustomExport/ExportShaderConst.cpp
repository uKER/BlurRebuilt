#include "Common/XtPch.h"
#include <Xt/XtTool.h>

bool XtCustomExport_ShaderConst(XtCustomExportContext* ctx)
{
	LfStruct* srcField = AsStruct(FindFieldByName(ctx->containerType, ctx->attr->GetString("field")));

	if (!srcField)
		return true;

	Xt::DomBlock* srcDom = AcquireBlock(ctx->containerDom, srcField);

	if (!srcDom)
		return true;

	LtStruct* srcType = srcField->type;

	Xt::DomArray* dstArray = AcquireArray(ctx->containerDom, ctx->field);

	if (!dstArray)
		return true;

	Clear(dstArray);

	int constCount = (int)srcType->GetInt("ShaderConstCount", "0");

	for(int i = 0; i < constCount; i++)
	{
		if (!Xt::Append(dstArray, new Xt::DomField))
			return false;
	}

	for(XtUt::List<LarvaAttr>::Iterator it(&srcType->attrs.attrs); it; ++it)
	{
		if (it->name != "ShaderConst")
			continue;

		XtI64		constIdx	= it->GetInt("0", -1);
		XtString	path		= it->GetString("1");

		if ((constIdx < 0) || (constIdx >= constCount))
			continue;

		if (!*path)
			continue;

		XtString value = AsText(Xt::FindElementByPath(srcDom, path));

		if (!*value)
			value = "1.401e-045";

		Xt::DomField* dstField = AsField(Xt::At(dstArray, (int)constIdx));

		if (!dstField)
			continue;

		if (!Set(dstField, value))
			return false;
	}

	return true;
}
