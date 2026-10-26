#include "stdafx.h"
#include "Display/Device/GCM/GCM.h"
#include "Display/Device/GCM/GCMDeviceLabels.h"

namespace Display
{
namespace GCM
{

sys_ppu_thread_t g_MainThreadID = 0;
sys_ppu_thread_t g_RenderThreadID = 0;

bool IsMainThread()
{
	sys_ppu_thread_t threadID;
	int error = sys_ppu_thread_get_id(&threadID);
	BIZ_ASSERT(error == CELL_OK);
	return threadID == g_MainThreadID ? true : false;
}

void SetMainThread( sys_ppu_thread_t threadID )
{
	if( threadID == UINT_MAX )
	{
		int error = sys_ppu_thread_get_id(&g_MainThreadID);
		BIZ_ASSERT(error == CELL_OK);
	}
	else
	{
		g_MainThreadID = threadID;
	}
}

const U32 	g_NextReportIndex1_Start	= g_ReportIndexStart;
const U32 	g_NextReportIndex1_End		= g_ReportIndexStart+g_ReportIndexCount;
U32			g_NextReportIndex1			= g_NextReportIndex1_Start;

const U32 GCMGetNextReportIndex1()
{
	if(g_NextReportIndex1 >= g_NextReportIndex1_End)
	{
		//LOGINFO(g_Log, "GCMGetNextReportIndex1 - Looping report indexes\n");
		g_NextReportIndex1 = g_NextReportIndex1_Start;
	}
	const U32 nextReportIndex = g_NextReportIndex1;
	g_NextReportIndex1 += 1;
	return nextReportIndex;
}

const U32 	g_NextReportIndex2_Start	= g_TimingIndexStart;
const U32 	g_NextReportIndex2_End		= g_TimingIndexStart+g_TimingIndexCount;
U32			g_NextReportIndex2			= g_NextReportIndex2_Start;

const U32 GCMGetNextReportIndex2()
{
	if(g_NextReportIndex2 >= g_NextReportIndex2_End)
	{
		//LOGINFO(g_Log, "GCMGetNextReportIndex2 - Looping report indexes\n");
		g_NextReportIndex2 = g_NextReportIndex2_Start;
	}
	const U32 nextReportIndex = g_NextReportIndex2;
	g_NextReportIndex2 += 2;
	return nextReportIndex;
}

#ifdef CFG_DEBUG

#define CheckRange(t, min, max) assert(t >= min && t <= max)

void CheckGCMTexture(const CellGcmTexture* p)
{
	assert(p);
	CheckRange(p->mipmap, 1, 13);
	CheckRange(p->dimension, 1, 3);
	assert(p->cubemap < 2);
	CheckRange(p->width, 1, 4096);
	CheckRange(p->height, 1, 4096);
	CheckRange(p->depth, 1, 512);
}

void CheckGCMVertexTexture(const CellGcmTexture* p)
{
	CheckGCMTexture(p);
	if (p->format == (CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR | CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT))
		assert(p->pitch % 16 == 0);
	else if (p->format == (CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR | CELL_GCM_TEXTURE_X32_FLOAT))
		assert(p->pitch % 4 == 0);
	else
		assert(0);
}

#endif

}; // namespace GCM
}; // namespace Display
