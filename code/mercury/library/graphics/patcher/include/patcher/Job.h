#ifndef PATCHER_JOB_H
#define PATCHER_JOB_H

#include <System/Scheduler/JobQueue.h>

namespace Patcher
{

class ALIGN16 C_SPUJob
{
public:
	MathsStorage::Float4	m_ParameterData[64];
	U32						m_ParameterOffsets[64];
	U32						m_ParameterCount;
	U32						m_InputAddress;
	U32						m_OutputAddress;
	U32						m_Size;
	U32						m_BlockerAddress;
};

void Job(Scheduler::Task* pTask);

}; // namespace Patcher

#endif // PATCHER_JOB_H
