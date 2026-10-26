#include "SystemPch.h"

#ifdef PLATFORM_PC
#include "d3d9.h"
#endif

#include <core/maths/mathsutil.h>

namespace Perf
{

#ifdef PLATFORM_PC
void D3DBeginEvent(const char *pName)
{
	WCHAR wName[256];
	memset(wName, 0, sizeof(wName));
	mbstowcs(wName, pName, Maths::Min(strlen(pName), sizeof(wName)/sizeof(wName[0])));
	D3DPERF_BeginEvent(0, wName);
}

void D3DEndEvent()
{
	D3DPERF_EndEvent();
}
#endif

}
