#include "StdAfx.h"

#include <system/scheduler.h>
#include "tools_shared/SchedulerConfig.h"

void C_SchedulerConfig::Initialise()
{
	Scheduler::Policy piMed[] = { 7, 7, 7, 7, 7, 7, 7, 7 };
	static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);
	m_hPolicy = _hPolicy;

#ifndef PLATFORM_PC
#ifdef PLATFORM_PS3
	Scheduler::Policy piPak[]	= { 0,  0,  5,  5,  5,  5,  5, 5 };
#else
	Scheduler::Policy piPak[]	= { 5,  5,  5,  5,  5,  5,  5,  5 };
#endif
	m_PakPolicy = Scheduler::CreatePolicy(piPak);

#else
	Scheduler::Policy piPak[] = { 10, 10, 10, 10 };
	m_PakPolicy = Scheduler::CreatePolicy(piPak);
#endif

#ifdef PLATFORM_PC
	Scheduler::jqEnableWorkers(2|4|8);
#endif

	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(64);
}

void C_SchedulerConfig::Shutdown()
{
	Scheduler::Shutdown();
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetPolicy()
{
	return m_hPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetPakPolicy()
{
	return m_PakPolicy;
}


