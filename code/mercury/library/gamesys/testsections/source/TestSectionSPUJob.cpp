#include "stdafx.h"
#include "TestSections/TestSectionSPU.h"
#ifdef PLATFORM_SPU
//#include <spu_intrinsics>
#endif

// SPU task entry point.
void SpuJob(Scheduler::Task* pTask)
{
	snPause();

	return;
}


JQ_DECLARE_SPU_MODULE(SpuTest, SpuJob, sputest, "Sputest");
