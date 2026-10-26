#include "StdAfx.h"
#include "Display2/Utils/Utils.h"

namespace Display2
{

Util::C_String g_VertexShaderFileExt = "p3vsh";

Util::C_String g_PixelShaderFileExt	= "p3psh";

const U32 g_NextTimingIndex_Start = 1024;

const U32 g_NextTimingIndex_End	= 2048;

U32 g_NextTimingIndex = g_NextTimingIndex_Start;

U32 GetNextTimingIndex(const U32 inc)
{
	if(g_NextTimingIndex+inc >= g_NextTimingIndex_End)
	{
		g_NextTimingIndex = g_NextTimingIndex_Start;
	}

	U32 nextTimingIndex = g_NextTimingIndex;

	g_NextTimingIndex += inc;

	return nextTimingIndex;
}

} // namespace Display2
