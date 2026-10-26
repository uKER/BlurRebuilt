#include <cell/spurs/policy_module.h>
#include <spu_intrinsics.h>
#include <core/Core.h>
#include "system/scheduler/jobqueue.h"

extern "C" { 
void jq_main(uintptr_t pKernelContext, uint64_t argp);
}

void cellSpursModuleEntry(uintptr_t _pKernelContext, uint64_t _argp)
{
	static uintptr_t pKernelContext;
	static uint64_t  argp;

	pKernelContext = _pKernelContext;
	argp           = _argp;
	// Set stack pointer
	register qword* sp __asm__("$sp") = (qword*)(256*1024 - 16*3);
	sp[0] = si_from_ptr(&sp[2]);
	sp[1] = (qword)(0);
	sp[2] = (qword)(0);

	jq_main(pKernelContext, argp);

	cellSpursModuleExit(pKernelContext);
}
