#ifndef MISC_SHARED_INFLATE_TASK_H
#define MISC_SHARED_INFLATE_TASK_H

#if _MSC_VER > 1000
#pragma once
#endif

//#include "../../../../Vendor/Zlib/Current_Mercury/Zlib.h"

namespace Scheduler { struct Task; }

namespace Compression
{

struct ALIGN16 C_InflateParameters
{
	//z_stream m_CompressionStream;
	U8* pInData;
	U32 nInSize;
	U8* pOutData;
	U32 nOutSize;
};

void InflateTask(Scheduler::Task* pTask);

};

#endif // MISC_SHARED_INFLATE_TASK_H
