#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Scheduler.h>

struct C_Scheduler
{
	static bool Initialise();
	static void Shutdown();

	static const S32 					m_GroupCount = 7;
	static const S32 					m_TaskCount = 256;
	static Scheduler::GroupHandle		m_hGroups[m_GroupCount];
	static Scheduler::PolicyHandle		m_hPolicy;
};

#endif // SCHEDULER_H
