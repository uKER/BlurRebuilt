#include "stdafx.h"
#include "ActorList.h"
#include <animation/skeleton/Skeleton.h>

JQ_DECLARE_MODULE(ActorUpdateModule, ActorUpdateTask, actorupdatetask);

void ActorUpdateTask( Scheduler::Task* task )
{
	PERF_EVENT( "ActorUpdateTask" );
	ActorUpdateParams* actorUpdateParams = reinterpret_cast<ActorUpdateParams*>( &task->ParamData );
	
	C_Actor* currentActorEa = actorUpdateParams->m_actor;
	
	for ( U32 i = 0; i < actorUpdateParams->m_numActors; ++i )
	{
		Spu::MemBaseMarker actorMemBaseMarker;
		
		C_Actor* actor = ( C_Actor* ) Spu::FetchAlloc( currentActorEa, sizeof( C_Actor ) );
		
		actor->SpuFetch();
		
		actor->Update( actorUpdateParams->m_deltaTime );
		
		actor->SpuStore();
		
		Spu::Store( currentActorEa, actor, sizeof( C_Actor ) );
		
		++currentActorEa;
	}
}

#ifndef PLATFORM_SPU

C_ActorList::C_ActorList()
{
	if ( Scheduler::Active() )
	{
		m_focusActorGroup = Scheduler::CreateGroup();
		m_mainUpdateGroup = Scheduler::CreateGroup();
		
		m_allCpuPolicyHandle = 0;
		m_noPpuPolicyHandle = 1;
	}
	
	//m_animResourceManager.SetAssetPath( "Animations/renwick_test/" );
	
	// Create anim set.
	LOGINFO( Debug::GetLogger("mercury.animation"), "Loading Resources...\n" );
	
	gAnimationStore.LoadResources();
	
	LOGINFO( Debug::GetLogger("mercury.animation"), "num joints: %d", gAnimationStore.GetSkeleton().GetNumJoints() );
	
	m_updatePending = false;
}

C_ActorList::~C_ActorList()
{
	if ( Scheduler::Active() )
	{
		Scheduler::jqDestroyBatchGroup( m_focusActorGroup );
		Scheduler::jqDestroyBatchGroup( m_mainUpdateGroup );
	}
}

void C_ActorList::Clear()
{
	m_actors.Clear();
}
	
C_Actor* C_ActorList::AddActor()
{
	if ( m_actors.Size() >= MAX_ACTORS )
	{
		return 0;
	}
	
	m_actors.PushBack();
	m_actors.Back().Create( m_actors.Size() - 1 );
	
	return &m_actors.Back();
}

U32 C_ActorList::GetNumActors() const
{
	return m_actors.Size();
}

Debug::Bool gForceSingleThreaded( "Animation.ForceSingleThreaded", false );
Debug::Int gNumActorsPerTask( "Animation.NumActorsPerTask", 4, Debug::Range( 1, 32 ) );
Debug::Bool gEnablePpuTasks( "Animation.EnablePpuTasks", true );

void C_ActorList::StartUpdate( F32 deltaTime )
{
	PERF_EVENT( "ActorList.Update ");
	
	FinishUpdate();

	if ( Scheduler::Active() && !gForceSingleThreaded )
	{
		ActorUpdateParams params;
		params.m_deltaTime = deltaTime;
		
		U32 numActorsPerTask = (U32) gNumActorsPerTask;
		
		numActorsPerTask = Maths::Clamped( numActorsPerTask, 1U, 32U );
		
		U32 focusActorIndex = 0;
		U32 halfNumActors = GetNumActors() / 2;
		
		for ( U32 i = 0, numActors = GetNumActors(); i < numActors; i += numActorsPerTask )
		{
			params.m_actor = &m_actors[ i ];
			params.m_numActors = Maths::Min( i + numActorsPerTask, GetNumActors() ) - i;
			
			Scheduler::GroupHandle group;
			
			if ( focusActorIndex >= i && focusActorIndex < (i + params.m_numActors) )
			{
				group = m_focusActorGroup;
			}
			else
			{
				group = m_mainUpdateGroup;
			}

			// Kick off an update task.
			Scheduler::AddTask(
				ActorUpdateModule,
				0, // input ptr
				0, // output ptr
				group,	// group ID
				gEnablePpuTasks ? m_allCpuPolicyHandle : m_noPpuPolicyHandle, // policy
				&params,
				sizeof( ActorUpdateParams )
			);
		}
		
		m_updatePending = true;
	}
	else
	{
		for ( U32 i = 0, numActors = GetNumActors(); i < numActors; ++i )
		{
			m_actors[ i ].Update( deltaTime );
		}
		m_updatePending = false;
	}
}

void C_ActorList::FinishFocusActorUpdate()
{
	if ( m_updatePending && Scheduler::Active() && !gForceSingleThreaded )
	{
		Scheduler::Flush( m_focusActorGroup );
	}
}

void C_ActorList::FinishUpdate()
{
	if ( m_updatePending && Scheduler::Active() && !gForceSingleThreaded )
	{
		// Wait for all tasks to complete.
		Scheduler::Flush( m_focusActorGroup );
		Scheduler::Flush( m_mainUpdateGroup );
		
		m_updatePending = false;
	}
}

void C_ActorList::AddUpdateDependencies( Scheduler::GroupHandle group )
{
	Scheduler::AddGroupDependency( group, m_focusActorGroup );
	Scheduler::AddGroupDependency( group, m_mainUpdateGroup );
}


C_Actor& C_ActorList::GetFocusActor()
{
	FinishFocusActorUpdate();
	return m_actors[0];
}

void C_ActorList::ReloadResources()
{
	// TODO:
//	m_animResourceManager.ReloadChangedResources();
	
	//for ( U32 i = 0; i < GetNumActors(); ++i )
	//{
	//	m_actors[ i ].m_animSet.Load( m_animResourceManager );
	//}
	
}

U32 C_ActorList::GetNumJointsPerActor() const
{
	return gAnimationStore.GetSkeleton().GetNumJoints();
}

#endif
