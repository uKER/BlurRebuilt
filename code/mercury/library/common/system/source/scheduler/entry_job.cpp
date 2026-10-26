#include <cell/spurs/job_context.h>
#include <spu_intrinsics.h>
#include <core/Core.h>
#include "system/scheduler/jobqueue.h"

extern "C" { 
	extern char __bss_start[];
	extern char _end[];
	extern char __stack_limit[];

	typedef void (*ctorp)();
	extern ctorp __CTOR_LIST__[];

	int jq_main();
	void entry(); 
}

//void entry() __attribute__((section(".init")));
void entry() 
{
	int bss_size = (unsigned)_end-(unsigned)__bss_start;
	__builtin_memset(__bss_start, 0, bss_size);
	Spu::Alloc((bss_size+15)&~15);;

	unsigned long nptrs = (unsigned long) __CTOR_LIST__[0];
	unsigned i;

	if (nptrs == (unsigned long)-1)
		for (nptrs = 0; __CTOR_LIST__[nptrs + 1] != 0; nptrs++);
	for (i = nptrs; i >= 1; i--)
		__CTOR_LIST__[i] ();

	// Override stack size for this module based on the command line args.
	Scheduler::_jqThisWorker->MemLimit = (int)__stack_limit;

	jq_main();
}
