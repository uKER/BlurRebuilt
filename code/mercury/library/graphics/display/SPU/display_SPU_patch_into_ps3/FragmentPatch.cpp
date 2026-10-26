#include <stdint.h>
#include <stdio.h>
#include <spu_intrinsics.h>
#include <sys/spu_thread.h>
#include <cell/spurs/types.h>
#include <cell/spurs/task.h>
#include <spu_printf.h>
#include <cell/dma.h>
#include <cell/gcm_spu.h>

#include <Core/Core.h>
#include <Core/Maths.h>
#include <Core/Containers.h>
#include <Core/Threads.h>

#include <System/debuglog.h>
#include <System/debug/variables.h>
#include <System/SPU/Queue.h>
#include <System/SPU/Scheduler_SPU.h>
#include <System/SPU/AutoPointer.h>

#include "Display/SPU/FPIntoDefines.h"
#include "Display/SPU/FPIntoParams.h"



namespace Display
{

void FragmentPatchInto(void* pVoidParams)
{
	Spu::AutoPointer<SpuFPIntoParams> pParams = (SpuFPIntoParams *) pVoidParams;

	U32 fragmentProgramSize = (pParams->m_FPSize + 0xF) & ~0xF;

	// load
	static U8 * fpBuffer = (U8 *) memalign(128, 16 * 1024);

	U32	fpInAlignment = pParams->m_FPInAddress & 0x7F;
	U8 * fragmentProgram = fpBuffer + fpInAlignment;

	cellDmaGet(fragmentProgram, pParams->m_FPInAddress, fragmentProgramSize, 0, 0, 0);
	cellDmaWaitTagStatusAll(1 << 0);

	// patch
	U32 * pOffset = pParams->m_Offsets;
	for (U32 paramNo = 0; paramNo < pParams->m_NumberToPatch; ++paramNo)
	{
		qword patch = si_roti(si_lqd(si_from_ptr(pParams->m_Data + 4 * paramNo), 0x0), 16);
		si_stqx(patch, si_from_ptr(fragmentProgram), si_from_uint(*pOffset++));
	}

	// save
	cellDmaPut(fragmentProgram, pParams->m_FPOutAddress, fragmentProgramSize, 0, 0, 0);
	cellDmaWaitTagStatusAll(1 << 0);

	// if we are outputting into local memory, then read four bytes (with fence) from local memory
	// to ensure that the DMA is completed and the data has been committed to video memory
	if (pParams->m_FPOutLocation == CELL_GCM_LOCATION_LOCAL)
	{
		U32 outData = 0;
		U32 outAddress = (U32) &outData;
		U32 outAlignment = outAddress & 0xF;
		U32 inAddress = pParams->m_FPOutAddress;
		while ( (inAddress & 0xF) != outAlignment )
			inAddress += 4;

		cellDmaSmallGetf((void *) outAddress, inAddress, 4, 1, 0, 0);
		cellDmaWaitTagStatusAll(1 << 1);
	}

	// overwrite jump to self with a nop
	U32 j2sAlignment = pParams->m_J2SAddress & 0x7F;
	U8 nops[128] __attribute__((__aligned__(128))) = { 0 };

	cellDmaSmallPut(&nops[j2sAlignment], pParams->m_J2SAddress, 4, 0, 0, 0);
	cellDmaWaitTagStatusAll(1 << 0);
}

}; // namespace Display

void cellSpursMain(qword argTask, U64 argTaskSet)
{
	//DebugBreak();
	Spu::Scheduler::WorkerThread("FragmentPatchInto", argTask, argTaskSet, Display::FragmentPatchInto);
}
