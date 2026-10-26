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
	BIZ_ASSERT(s_pInstance == NULL);
	s_pInstance = this;
}

C_SchedulerConfig::~C_SchedulerConfig()
{
	BIZ_ASSERT(s_pInstance == this);
	s_pInstance = NULL;
}

C_SchedulerConfig* C_SchedulerConfig::GetInstance()
{
	BIZ_ASSERT(s_pInstance);
	return s_pInstance;
}

S32 C_SchedulerConfig::Initialise()
{
#ifndef PLATFORM_PC

#ifdef PLATFORM_PS3
	Scheduler::Policy piMed[]				= { 7, 7, 7, 7, 7, 7, 7, 7 };
	Scheduler::Policy piHigh[]				= {10,10,10,10,10,10,10, 10};
	Scheduler::Policy piStreamer[]			= { 0, 0, 5, 5, 5, 5, 5, 5 };
	Scheduler::Policy piDeferredRaytrace[]	= { 1, 1, 5, 5, 5, 5, 5, 5 };
	Scheduler::Policy piSingleThread[]		= { 0, 0, 7, 0, 0, 0, 0, 0 };
	Scheduler::Policy piAudio[]				= { 0, 0, 1, 1, 1, 1, 1, 1 };
	Scheduler::Policy piCarDynamic[]		= { 7, 7, 7, 7, 7, 7, 7, 7 };
	Scheduler::Policy piCarUpdateLight[]	= { 0, 0, 7, 7, 7, 7, 7, 7 };
	Scheduler::Policy piLighting[]			= { 0, 0, 0,10, 0,10,10,10 };
	Scheduler::Policy piPak[]				= { 0, 0, 7, 7, 7, 7, 7, 7 };
#else
	Scheduler::Policy piMed[] = { 7, 7, 7, 7, 0, 7, 7, 7 };
	Scheduler::Policy piHigh[] = {10, 10, 10, 10, 0, 10, 10, 10 };
	Scheduler::Policy piStreamer[] = {0, 0, 5, 5, 0, 5, 5, 5 };
	Scheduler::Policy piDeferredRaytrace[] ={1, 1, 5, 5, 0, 5, 5, 5 };
	Scheduler::Policy piSingleThread[] = { 0, 0, 7, 0, 0, 0, 0, 0 };
	Scheduler::Policy piAudio[] = {0, 0, 0, 0, 10, 6, 0, 0};
	Scheduler::Policy piCarDynamic[] = {0, 0, 7, 7, 0, 7, 7, 7 };
	Scheduler::Policy piCarUpdateLight[] = {0, 0, 7, 7, 0, 7, 7, 7 };
	Scheduler::Policy piLighting[] = {0, 0, 0, 10, 0, 10, 10, 10 };
	Scheduler::Policy piPak[] = { 7, 7, 0, 7, 0, 7, 7, 7 };
#endif

	static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);
	m_hPolicy = _hPolicy;

	static Scheduler::PolicyHandle _highPolicy = Scheduler::CreatePolicy(piHigh);
	m_CollisionPolicy = _highPolicy;

	m_SceneGraphPolicy = _hPolicy;
	m_AIPolicy = _hPolicy;
	m_AnimationPolicy = _hPolicy;

	m_StreamerPolicy = Scheduler::CreatePolicy(piStreamer);
	m_DeferredRaytracePolicy = Scheduler::CreatePolicy(piDeferredRaytrace);

	m_SingleThreadPolicy = Scheduler::CreatePolicy(piSingleThread);

	m_AudioPolicy = Scheduler::CreatePolicy(piAudio);

	//Create car dynamic effects policy
	
	m_CarDynamicEffectsPolicy = Scheduler::CreatePolicy(piCarDynamic);
	m_CarUpdateLightCubeMapPolicy = Scheduler::CreatePolicy(piCarUpdateLight);

	m_LightingPolicy = Scheduler::CreatePolicy(piLighting);
	m_PakPolicy = Scheduler::CreatePolicy( piPak );
#else
	Scheduler::Policy piMed[] = { 7, 7 };
	static Scheduler::PolicyHandle _hPolicy = Scheduler::CreatePolicy(piMed);
	m_hPolicy = _hPolicy;

	Scheduler::Policy piHigh[] = {10, 10};
	static Scheduler::PolicyHandle _highPolicy = Scheduler::CreatePolicy(piHigh);
	m_CollisionPolicy = _highPolicy;

	m_SceneGraphPolicy = _hPolicy;
	m_AIPolicy = _hPolicy;
	m_AnimationPolicy = _hPolicy;

	Scheduler::Policy piStreamer[] = {5, 5};
	m_StreamerPolicy = Scheduler::CreatePolicy(piStreamer);
	m_DeferredRaytracePolicy = m_StreamerPolicy;


	Scheduler::Policy piAudio[] = {1, 1};

	m_AudioPolicy = Scheduler::CreatePolicy(piAudio);

	//Create car dynamic effects policy

	Scheduler::Policy piCarDynamic[] = {7, 7 };
	Scheduler::Policy piCarUpdateLight[] = {7, 7 };
	
	m_CarDynamicEffectsPolicy = Scheduler::CreatePolicy(piCarDynamic);
	m_CarUpdateLightCubeMapPolicy = Scheduler::CreatePolicy(piCarUpdateLight);

	Scheduler::Policy piLighting[] = {10, 10 };
	m_LightingPolicy = Scheduler::CreatePolicy(piLighting);


	Scheduler::Policy piPak[] = { 7, 7, 7, 7 };
	m_PakPolicy = Scheduler::CreatePolicy( piPak );
#endif

#ifdef PLATFORM_PC
	Scheduler::jqEnableWorkers(1|2);
#endif

	Scheduler::Init();
	Scheduler::Start();

	Scheduler::SetGroupPoolSize(E_SchedulerGroup_Count + 270);
	for (int i = 0; i < E_SchedulerGroup_Count; i++)
		m_Groups[i] = Scheduler::CreateGroup();

	return 0;
}

void C_SchedulerConfig::Shutdown()
{
	for (int i = 0; i < E_SchedulerGroup_Count; i++)
		Scheduler::DestroyGroup(m_Groups[i]);

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

Scheduler::PolicyHandle& C_SchedulerConfig::GetStreamerPolicy()
{
	return m_StreamerPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetDeferredRaytracePolicy()
{
	return m_DeferredRaytracePolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetAudioPolicy()
{
	return m_AudioPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetAIPolicy()
{
	return m_AIPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetAnimationPolicy()
{
	return m_AnimationPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetCarDynamicEffectsPolicy()
{
	return m_CarDynamicEffectsPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetCarUpdateLightCubeMapPolicy()
{
	return m_CarUpdateLightCubeMapPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetLightingPolicy()
{
	return m_LightingPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetSingleThreadPolicy()
{
	return m_SingleThreadPolicy;
}

Scheduler::PolicyHandle& C_SchedulerConfig::GetPakPolicy()
{
	return m_PakPolicy;
}

Scheduler::GroupHandle& C_SchedulerConfig::GetGroup( const S32 groupIndex )
{
	return m_Groups[ groupIndex ];
}

} // namespace Game
