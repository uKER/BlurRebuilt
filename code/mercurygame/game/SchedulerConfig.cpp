#include "StdAfx.h"

#include <framework\application\ApplicationTypes.h>
#include <system/scheduler.h>
#include <SchedulerConfig.h>

namespace Game
{

// C_SchedulerConfig

C_SchedulerConfig* C_SchedulerConfig::s_pInstance = NULL;

C_SchedulerConfig::C_SchedulerConfig()
{
	assert(s_pInstance == NULL);
	s_pInstance = this;
}

C_SchedulerConfig::~C_SchedulerConfig()
{
	assert(s_pInstance == this);
	s_pInstance = NULL;
}

C_SchedulerConfig* C_SchedulerConfig::GetInstance()
{
	assert(s_pInstance);
	return s_pInstance;
}

HRESULT C_SchedulerConfig::Initialise()
{
	Scheduler::Policy piMed[] = { 7, 7, 7, 7, 7, 7, 7, 7 };
	static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);
	m_hPolicy = _hPolicy;
	m_SceneGraphPolicy = _hPolicy;
	m_AnimationPolicy = _hPolicy;

	Scheduler::Policy piHigh[] = {10, 10, 10, 10, 10, 10, 10, 10 };
	static Scheduler::PolicyHandle _highPolicy = Scheduler::CreatePolicy(piHigh);
	m_CollisionPolicy = _highPolicy;

	Scheduler::Policy piOneSPU[] = { 7, 7, 7, 0, 0, 0, 0, 0 };
	static Scheduler::PolicyHandle _oneSPUPolicy = Scheduler::CreatePolicy(piOneSPU);
	m_SingleSPUPolicy = _oneSPUPolicy;
	

	Scheduler::Policy piStreamer[] = {0, 0, 7, 7, 7, 7, 7, 7 };
	m_StreamerPolicy = Scheduler::CreatePolicy(piStreamer);

#if PLATFORM_PS3
	Scheduler::jqEnableWorkers(2|4|8|16|32|64|128);
#endif

	Scheduler::Init();
	Scheduler::Start();

	// set this to a reasonable number so systems which need a variable amount of groups have some headroom
	Scheduler::SetGroupPoolSize(E_SchedulerGroup_Count + 128);
	for (int i = 0; i < E_SchedulerGroup_Count; i++)
		m_Groups[i] = Scheduler::CreateGroup();

	return S_OK;
}

void C_SchedulerConfig::Shutdown()
{
	Scheduler::Shutdown();
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetCollisionPolicy()
{
	return m_CollisionPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetSceneGraphPolicy()
{
	return m_SceneGraphPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetSingleSPUPolicy()
{
	return m_SingleSPUPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetStreamerPolicy()
{
	return m_StreamerPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetAnimationPolicy()
{
	return m_StreamerPolicy;
}

Scheduler::GroupHandle& C_SchedulerConfig::GetGroup( const S32 groupIndex )
{
	return m_Groups[ groupIndex ];
}

} // namespace Game
