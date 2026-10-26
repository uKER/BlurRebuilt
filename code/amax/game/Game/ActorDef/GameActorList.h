#ifndef GAMEACTORLIST_C980FBA7_DCB6_4EA3_AE10_0003BD3A7A55_H
#define GAMEACTORLIST_C980FBA7_DCB6_4EA3_AE10_0003BD3A7A55_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorList.h>
#include <xt/XtGame.h>

namespace Game
{

class C_GameActorList : public Actor::C_ActorList
{
public:
	C_GameActorList(const U32 maxNoActors, const U32 noMsgQueues, Actor::C_ActorFactory* pFactory, const Sys::I_ContextProvider* pDefaultContextProvider = NULL);
	virtual ~C_GameActorList();

	Actor::C_ActorHandle CreateActor(const DataRef::C_Ptr<Actor::C_ActorCreationData>& pCreationData);

	bool CreateActors(const Util::C_String& actorsFilename);
	bool CreateActors(Actor::I_ActorCreationList* pActorList);

	void Update(const F32 timeStep);

	XtPack* GetXtPack() 
	{
		return m_pActorListXtPack;
	}

	// returns true if there are no pending actors
	bool UpdatePendingActors();

#ifndef CFG_GOLD
	void DebugWriteBackActorHandlesToCreationData() const;
#endif

private:
	void ScheduleQueuedAnimationTasks();

	typedef Btl::List<Actor::I_ActorCreationList*> C_PendingActorCreators;

	C_PendingActorCreators	m_PendingActorCreators;	///< strong refs to actors awaiting resources
	XtPack*					m_pActorListXtPack;		///< strong ref to Xt data for level actors
};

} // namespace Game


#endif // GAMEACTORLIST_C980FBA7_DCB6_4EA3_AE10_0003BD3A7A55_H
