#ifndef ACTORLIST_H
#define ACTORLIST_H

#include <system/scheduler/jobqueue.h>
#include <animation/task/EvaluationTaskList.h>
#include "Actor.h"

struct ActorUpdateParams
{
	C_Actor* m_actor;
	F32 m_deltaTime;
	U32 m_numActors;
};


void ActorUpdateTask( struct Scheduler::Task* task );

#ifndef PLATFORM_SPU

class C_ActorList
{
public:

	C_ActorList();
	~C_ActorList();
	
	void Clear();

	C_Actor* AddActor();

	
	U32 GetNumActors() const;
	
	void StartUpdate( F32 deltaTime );
	void FinishFocusActorUpdate();
	void FinishUpdate();
	void AddUpdateDependencies( Scheduler::GroupHandle group );
	

	C_Actor& GetFocusActor();
	
	const C_Actor& GetActor( U32 index ) const
	{
		assert( !m_updatePending );
		return m_actors[ index ];
	}

	C_Actor& GetActor( U32 index )
	{
		assert( !m_updatePending );
		return m_actors[ index ];
	}

	C_Actor* GetActorPtr( U32 index )
	{
		return &m_actors[ index ];
	}


	
	void ReloadResources();
	
	U32 GetNumJointsPerActor() const;
	

private:	
	Scheduler::GroupHandle	m_mainUpdateGroup;
	Scheduler::GroupHandle	m_focusActorGroup;
	Scheduler::Policy m_policy[8];
	Scheduler::PolicyHandle m_allCpuPolicyHandle;
	Scheduler::PolicyHandle m_noPpuPolicyHandle;

	static const U32 MAX_ACTORS = 600;
	Btl::Vector< C_Actor, Btl::VectorParameters_Static< MAX_ACTORS > > m_actors;
	Animation::C_EvaluationTaskList	m_evaluationTaskList;
	bool m_updatePending;
};

#endif

#endif
