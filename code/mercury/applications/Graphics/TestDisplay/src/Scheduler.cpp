#include "StdAfx.h"
#include "Scheduler.h"

Scheduler::GroupHandle		C_Scheduler::m_hGroups[C_Scheduler::m_GroupCount];
Scheduler::PolicyHandle 	C_Scheduler::m_hPolicy;

bool C_Scheduler::Initialise()
{
	Scheduler::Policy policy[] = { 7, 7, 7, 7, 7, 7, 7, 7 };

	m_hPolicy = Scheduler::CreatePolicy(policy);

	Scheduler::Init();
	Scheduler::Start();

	Scheduler::SetGroupPoolSize(m_GroupCount);

	for(int i=0; i<m_GroupCount; i++)
		m_hGroups[i] = Scheduler::CreateGroup();

	// 0-1 Single dependency.
	Scheduler::AddGroupDependency(m_hGroups[1], m_hGroups[0]);

	// 2-5 Multiple dependencies.
	Scheduler::AddGroupDependency(m_hGroups[6], m_hGroups[2]);
	Scheduler::AddGroupDependency(m_hGroups[6], m_hGroups[3]);
	Scheduler::AddGroupDependency(m_hGroups[6], m_hGroups[4]);
	Scheduler::AddGroupDependency(m_hGroups[6], m_hGroups[5]);

	return true;
}

void C_Scheduler::Shutdown()
{
	Scheduler::Shutdown();
}
