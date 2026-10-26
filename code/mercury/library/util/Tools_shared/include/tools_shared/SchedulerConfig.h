#ifndef MERCURY_TOOLS_SCHEDULERCONFIG_H
#define MERCURY_TOOLS_SCHEDULERCONFIG_H

#include <system/scheduler.h>
#include <core/utility/Singleton.h>

//! C_SchedulerConfig
class C_SchedulerConfig : public Ut::Singleton<C_SchedulerConfig>
{
public:
	void Initialise();
	void Shutdown();

	Scheduler::PolicyHandle& GetPolicy();
	Scheduler::PolicyHandle& GetPakPolicy();

private:
	Scheduler::PolicyHandle m_hPolicy;
	Scheduler::PolicyHandle m_PakPolicy;
};

#endif
