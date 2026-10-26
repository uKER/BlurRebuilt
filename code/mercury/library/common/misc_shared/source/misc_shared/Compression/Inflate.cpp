#include "MiscSharedPch.h"
#include "Misc_Shared/Compression/Inflate.h"
#include <System/DebugLog.h>
#include <System/Scheduler.h>

extern Scheduler::jqModule InflateModule;

namespace Compression
{

C_Inflate::C_Inflate()
{
	m_bIsRunning = false;
	m_hJob = -1;
}

C_Inflate::~C_Inflate()
{
}

bool C_Inflate::Create(Scheduler::GroupHandle group)
{
	m_hJob = group;

	return true;
}

void C_Inflate::Start(U8* pInData, U32 nInSize, U8* pOutData, U32 nOutSize)
{
	if (m_bIsRunning)
		return;

	assert(pInData && nInSize && pOutData && nOutSize);

	C_InflateParameters	parameters;

	parameters.pInData = pInData;
	parameters.nInSize = nInSize;
	parameters.pOutData = pOutData;
	parameters.nOutSize = nOutSize;
	
	Scheduler::AddTask(InflateModule, 0, 0, m_hJob, m_hPolicy, &parameters, sizeof(parameters));

	m_bIsRunning = true;
}

void C_Inflate::Finish()
{
	if (m_bIsRunning)
	{
		Scheduler::Flush(m_hJob);
		m_bIsRunning = false;
	}
}

}; // namespace Compression
