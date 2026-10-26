#ifndef MISC_SHARED_INFLATE_H
#define MISC_SHARED_INFLATE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <System/Scheduler/JobQueue.h>
#include <Misc_Shared/Compression/InflateTask.h>

namespace Compression
{

class C_Inflate
{
public:

	C_Inflate();
	~C_Inflate();

	bool Create(Scheduler::GroupHandle group);
	void Start(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize);
	void Finish();

	void SetPolicy(const Scheduler::PolicyHandle& hPolicy)	{ m_hPolicy = hPolicy; }

private:
	
	Scheduler::GroupHandle		m_hJob;
	Scheduler::PolicyHandle		m_hPolicy;
	bool						m_bIsRunning;
};

}; // namespace Compression

#endif // MISC_SHARED_INFLATE_H
