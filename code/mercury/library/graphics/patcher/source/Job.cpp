#include "stdafx.h"
#include "Patcher/Job.h"

namespace Patcher
{

void Job(Scheduler::Task* pTask)
{
	Spu::MemBaseMarker marker;

	C_SPUJob* pJob = (C_SPUJob*)Spu::FetchAlloc(pTask->Output, sizeof(C_SPUJob));
	
	U32* pMicrocode = (U32*)Spu::FetchAlloc((void*)pJob->m_InputAddress, pJob->m_Size);

	for (U32 paramNo = 0; paramNo < pJob->m_ParameterCount; ++paramNo)
	{
		#if PLATFORM_SPU
		qword data = si_roti(si_lqd(si_from_ptr(&pJob->m_ParameterData[paramNo]), 0x0), 16);
		si_stqx(data, si_from_ptr(pMicrocode), si_from_uint(pJob->m_ParameterOffsets[paramNo]));
		#endif
	}

	Spu::Store((void*)pJob->m_OutputAddress, pMicrocode, pJob->m_Size);

	#if PLATFORM_SPU
	// read back from video memory with a fence to ensure shader has transferred
	U32 vramSync ALIGN16 = 0;
	cellDmaSmallGetf(&vramSync, pJob->m_OutputAddress, 4, 0, 0, 0);

	// flag job as complete (overwrite jump to self)
	const U8	pNopArray[16] ALIGN16	= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	const U32	iNopAlignment			= pJob->m_BlockerAddress & 0xF;
	const void*	pNopAddress				= &pNopArray[iNopAlignment];
	cellDmaSmallPutf(pNopAddress, pJob->m_BlockerAddress, 4, 0, 0, 0);
	cellDmaWaitTagStatusAll(1 << 0);
	#endif
}

}; // namespace Patcher

JQ_DECLARE_SPU_MODULE(PatcherModule, Patcher::Job, PatcherJob, "patcher");
