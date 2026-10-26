#ifndef SCHEDULER_CONFIG_H
#define SCHEDULER_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/scheduler.h>

namespace Game
{

enum E_SchedulerGroup
{
	// [Jan] TODO: these are unnecessary and need to be removed. Each subsystem (e.g. physics, etc.) should create their own group
	// don't add any more
	E_SchedulerGroup_PhysicsInternal = 0,
	E_SchedulerGroup_PhysicsRaytrace,
	E_SchedulerGroup_SceneGraph0,
	E_SchedulerGroup_SceneGraph1,

	E_SchedulerGroup_Count
};

//! C_SchedulerConfig
class C_SchedulerConfig
{
public:
	C_SchedulerConfig();
	~C_SchedulerConfig();

	static C_SchedulerConfig* GetInstance();

	HRESULT Initialise();
	void Shutdown();

	Scheduler::GroupHandle& GetGroup( const S32 groupIndex );

	Scheduler::PolicyHandle& GetCollisionPolicy();
	Scheduler::PolicyHandle& GetStreamerPolicy();
	Scheduler::PolicyHandle& GetSceneGraphPolicy();
	Scheduler::PolicyHandle& GetSingleSPUPolicy();
	Scheduler::PolicyHandle& GetAnimationPolicy();

private:
	static C_SchedulerConfig*	s_pInstance;

	Scheduler::GroupHandle m_Groups[E_SchedulerGroup_Count];

	Scheduler::PolicyHandle m_hPolicy;
	Scheduler::PolicyHandle m_CollisionPolicy;
	Scheduler::PolicyHandle m_SceneGraphPolicy;
	Scheduler::PolicyHandle m_SingleSPUPolicy;
	Scheduler::PolicyHandle m_StreamerPolicy;
	Scheduler::PolicyHandle m_AnimationPolicy;
};

} // namespace Game

#endif // SCHEDULER_CONFIG_H
