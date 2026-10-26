#ifndef MISC_SHARED_DEFLATE_H
#define MISC_SHARED_DEFLATE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <System/Scheduler/JobQueue.h>
//#include "../../../../Vendor/Zlib/Current_Mercury/Zlib.h"

namespace Compression
{

struct C_DeflateParameters
{
	U8* pInData;
	U32 nInSize;
	U8* pOutData;
	U32 nOutSize;
	U32 totalOut;
};

class C_Deflate
{
public:

	C_Deflate();
	~C_Deflate();

	bool Create();
	void Destroy();

	void Start(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize);
	void Finish(U32& nOutSize);

	void SetPolicy(const Scheduler::PolicyHandle& hPolicy)	{ m_hPolicy = hPolicy; }

private:
	
	Scheduler::GroupHandle		m_hJob;
	Scheduler::PolicyHandle		m_hPolicy;
	
	C_DeflateParameters			m_Parameters;

	bool						m_bIsRunning;

};

void DeflateTask(Scheduler::Task* pTask);

}; // namespace Compression

#endif // MISC_SHARED_DEFLATE_H
