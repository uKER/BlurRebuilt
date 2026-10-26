#ifndef JOBQUEUE_XENON_H
#define JOBQUEUE_XENON_H

#include "system/scheduler/core/tl_defs.h"
//#include "system/scheduler/core/tl_system.h"

#if TL_PLAT_PC
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


namespace Scheduler
{

enum jqProcessor
{
	JQ_CORE_0 = 1 << 0,
	JQ_CORE_1 = 1 << 1,
	JQ_CORE_2 = 1 << 2,
	JQ_CORE_3 = 1 << 3,
	JQ_CORE_4 = 1 << 4,
	JQ_CORE_5 = 1 << 5,
	JQ_CORE_6 = 1 << 6,
	JQ_CORE_7 = 1 << 7,
	JQ_CORE_ALL = JQ_CORE_0 | JQ_CORE_1 | JQ_CORE_2 | JQ_CORE_3 | JQ_CORE_4 | JQ_CORE_5 | JQ_CORE_6 | JQ_CORE_7
};

//! Pointer to the function that is responsible for actually processing a batch of data.
typedef void (*jqFunctor)( struct Task* Batch );

struct jqModule
{
	jqModule() {}
	jqModule(jqFunctor Code, const char* Name) : Code(Code), Name(Name)
	{
	}
	jqFunctor Code;
	const char* Name;
};

typedef int TaskHandle;

struct _Task
{
};

struct _jqWorker
{
	int Processor;
	HANDLE Thread;
	int WorkerID;
	bool Idle;
};

#define jqPrintf tlPrintf

#define JQ_ASSERT(x, desc) TL_ASSERT(x, desc)

#define JQ_ALIGNED
#define JQ_DMA_ALIGNED

#define JQ_NO_PPU ERROR_JobQueue_Function_required_on_Xenon
#define JQ_NO_SPU 

#define JQ_DECLARE_MODULE(name,fn,elfname) \
	Scheduler::jqModule name(fn,#name);

#define JQ_DECLARE_PPU_MODULE(name,fn) \
	Scheduler::jqModule name(fn,#name);

#define JQ_DECLARE_SPU_MODULE(name,fn,elfname,logger) \
	Scheduler::jqModule name(fn,#name);

#define JQ_DECLARE_DUAL_MODULE(name,fn,elfname,logger) \
	Scheduler::jqModule name(fn,#name);

} // namespace Scheduler

#endif
