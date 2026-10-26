#include <core/core.h>
#include "system/scheduler/jobqueue.h"

#ifdef PLATFORM_SPU

#include <spu_printf.h>
#include <core/Core.h>
#include "system/Scheduler/jobqueue.h"

void spuStaticLog(int iLevel, char *string, int iVar)
{
	*((int*)&Scheduler::_jqThisWorker->pLogger) = iLevel;
	Scheduler::_jqThisWorker->ppustringdata = (U64)(string+(int)Scheduler::_jqThisWorker->PPUModuleAddress) + (((U64)iVar)<<32);

	spu_printf("\xff");
}

#ifndef JOBQUEUE_KERNEL

bool spuAssert()
{
	if (*((int*)&Scheduler::_jqThisWorker->bIsDebuggerAttached))
	{
		// break directly into the debugger
		DebugBreak();
	}
	else
	{
		// send the assert to the PPU
		*((int*)&Scheduler::_jqThisWorker->pLogger) = 6;
		static const char szAssertString[] PPUDATA = "SPU program %s";
		Scheduler::_jqThisWorker->ppustringdata = (U64)PPUDATA_STRING(szAssertString) + (((U64)PPUDATA_STRING(gsz_jobmodulename))<<32);
		spu_printf("\xff");
	}
	return false;
}

#endif

#endif
