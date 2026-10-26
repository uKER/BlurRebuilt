#ifndef CROWD_SYSTEM_ACTOR_H
#define CROWD_SYSTEM_ACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>
#include <crowd_system/XtCrowdPopulationListener.h>
#include <crowd_system/CrowdSystem.h>

#include <shared_actors/SharedActorBase.h>

#include <system/debug/I_DebugRender.h>

#include <shared_actors/XtActorBase.h>

namespace AnimatedCrowd
{

//! C_CrowdSystemCreate

class C_CrowdSystemCreate : public Actors::C_GameActorCreate
{
public:

	static Actor::C_ActorTypeId			ClassType()			{ return E_CROWD_ACTOR_TYPE_SYSTEM; }
	const Actor::C_ActorTypeId			GetType()	const	{ return E_CROWD_ACTOR_TYPE_SYSTEM; }

	C_CrowdSystem*						m_pCrowdSystem;
};

//! C_CrowdSystemActor

class C_CrowdSystemActor : public Actors::C_SharedActorBase, public C_XtCrowdPopulationListener, public C_XtCrowdTextureListListener, public C_CrowdSystemListener
{
public:

	C_CrowdSystemActor();
	~C_CrowdSystemActor();

	static const Actor::C_ActorTypeId	TypeId()	{ return E_CROWD_ACTOR_TYPE_SYSTEM; }
	static const Btl::String32			TypeName()	{ return "CrowdSystem"; }

	const bool							IsPaused() const;
	void								SetPaused(const bool paused);

#ifndef CFG_GOLD
	virtual void						DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

	bool								OnMessage(const Messaging::C_MessageBase *pMessage);

	virtual void						OnXtCrowdPopulationCreated(XtRecord* pRecord, const C_XtCrowdPopulation* pPopulation);
	virtual void						OnXtCrowdPopulationDestroyed(XtRecord* pRecord, const C_XtCrowdPopulation* pPopulation);

	virtual void						OnXtCrowdCharacterCreated(XtRecord* pRecord, const C_XtCrowdCharacter* pCharacter);
	virtual void						OnXtCrowdCharacterDestroyed(XtRecord* pRecord, const C_XtCrowdCharacter* pCharacter);

	virtual void						OnXtCrowdCycleGroupCreated(XtRecord* pRecord, const C_XtCrowdCycleGroup* pCycleGroup);
	virtual void						OnXtCrowdCycleGroupDestroyed(XtRecord* pRecord, const C_XtCrowdCycleGroup* pCycleGroup);

	virtual void						OnXtCrowdCycleGroupItemCreated(XtRecord* pRecord, const C_XtCrowdCycleGroupItem* pCycleGroupItem);
	virtual void						OnXtCrowdCycleGroupItemDestroyed(XtRecord* pRecord, const C_XtCrowdCycleGroupItem* pCycleGroupItem);

	virtual void						OnXtCrowdEffectSystemCreated(XtRecord* pRecord, const C_XtCrowdEffectSystem* pEffectSystem);
	virtual void						OnXtCrowdEffectSystemDestroyed(XtRecord* pRecord, const C_XtCrowdEffectSystem* pEffectSystem);

	virtual void						OnXtCrowdAnimationEventEffectCreated(XtRecord* pRecord, const C_XtCrowdAnimationEventEffect* pAnimationEventEffect);
	virtual void						OnXtCrowdAnimationEventEffectDestroyed(XtRecord* pRecord, const C_XtCrowdAnimationEventEffect* pAnimationEventEffect);

	virtual void						OnXtCrowdTextureListCreated(XtRecord* pRecord, const C_XtCrowdTextureList* pTextureList);
	virtual void						OnXtCrowdTextureListDestroyed(XtRecord* pRecord, const C_XtCrowdTextureList* pTextureList);

	virtual void						OnCrowdSystemRouteChanged();

protected:

	bool								InitialiseImpl();
	bool								CreateImpl();

	void								Initialise();
	void								Destroy();

private:

	void								SetState(const enum CrowdSystemActorState state);
	const CrowdSystemActorState			GetState() const;

	void								Process(const F32 deltaTime);

	void								LoadCrowdResources();

	bool								BuildPopulation(const C_XtCrowdPopulation* pPopulation);

private:
	C_CrowdSystem*						m_pCrowdSystem;
	
	CrowdSystemActorState				m_State;
};

}; // namespace AnimatedCrowd

#endif // CROWD_SYSTEM_ACTOR_H
