#include "Common/XtPch.h"

#ifdef XT_TOOL
#include <Xt/XtTool.h>
#else
#include <Xt/XtGame.h>
#endif

#include "Common/XtFlask.h"

void XtFlaskDecodeStrings(XtU8* __restrict dst, const XtU8* __restrict src, int count)
{
	if (count <= 0)
		return;

	count--;

	XtU8 prev = 0xCC;

	for(int i = 0; i < count; i++, dst++, src++)
	{
		*dst = prev - (*src ^ (XtU8)0xB3);

		prev += *dst - i;
	}

	*dst = '\0';
}

void XtFlaskEncodeStrings(XtU8* __restrict data, int count)
{
	XtU8 prev = 0xCC;

	for(int i = 0; i < count; i++, data++)
	{
		XtU8 old = *data;

		*data = (prev - *data) ^ (XtU8)0xB3;

		prev += old - i;
	}
}
