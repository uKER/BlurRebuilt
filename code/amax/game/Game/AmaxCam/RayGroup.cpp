
#include "stdafx.h"
#include "RayGroup.h"

#include <gravity/narrowphase/Ray.h>
#include <System/Scheduler/jobqueue.h>

// This is a group of deferred raytraces which is combined into a Scehduler task

namespace AmaxCam
{

C_RayGroup g_RayGroup;

C_RayGroup::C_RayGroup()
{
	m_SchedulerGroup = -1;
}

void C_RayGroup::Init()
{
	m_SchedulerGroup = Scheduler::CreateGroup();
	
	for (S32 i = 0; i < 8; i++)
	{
		Collision::C_Ray* ray = new Collision::C_Ray;
		if (!ray)
			break;
		m_Rays.PushBack(ray);
	}
}

void C_RayGroup::Exit()
{
	for (S32 i = 0; i < m_Rays.Size(); i++)
	{
		SAFE_DELETE_PTR(m_Rays[i])
	}
	m_Rays.Clear();

	if (m_SchedulerGroup >= 0)
		Scheduler::DestroyGroup(m_SchedulerGroup);
	
	m_SchedulerGroup = -1;
}

void C_RayGroup::StartFrame()
{
	m_NextRay = 0;
}

S32 C_RayGroup::NextRay()
{
	return m_NextRay++;
}

Collision::C_Ray* C_RayGroup::GetRay(S32 index)
{
	if (index < 0 || index >= m_Rays.Size())
		return 0;
	
	return m_Rays[index];
}

void C_RayGroup::Flush()
{
	Scheduler::Flush(m_SchedulerGroup);
}

} // End of namespace
