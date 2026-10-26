#ifndef __SYSTEM_SPU_OVERLAYS
#define __SYSTEM_SPU_OVERLAYS

#ifdef PLATFORM_SPU

#include "system/scheduler/jobqueue_mem.h"

struct OverlayTableEntry
{
	void *iDest;
	int iSize;
	int iSource;
	int iPad;
};

extern "C" OverlayTableEntry _ovly_table;

namespace Scheduler
{
// TODO: add functionality for asynchronous DMA transfer of overlay code
inline void SelectOverlay(int i)
{
	OverlayTableEntry *p = &_ovly_table + i;

	Spu::Fetch(p->iDest, (void*)(p->iSource+(U8*)_jqThisWorker->PPUModuleAddress), p->iSize);
}
}

#endif

#endif
