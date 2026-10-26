#ifndef CYCLEACTOR_H
#define CYCLEACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>

#include <GameLogic/ActorStudioActors/ResourceTypes.h>



//=========================================================================
//						CYCLE ACTOR CLASSES
//=========================================================================



namespace Actors
{

struct CycleElementCreationData
{
	const ::Actors::C_PosActorCreate*	m_ElementActor;
	bool								m_InitialState;
};


class CycleActorCreationData : public C_GameActorCreate
{
public:
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_CycleActor;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_CycleActor;}

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);

	XtHandle<Game::GlobalTime>				m_TimeStep;
	XtArray<CycleElementCreationData>		m_Elements;	
	ActorMsg::C_XtActorMessageBase			*m_ActivateMessage;
	ActorMsg::C_XtActorMessageBase			*m_DeactivateMessage;
};


//=========================================================================


class CycleActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	CycleActor();
	virtual ~CycleActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);


	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_CycleActor; }
	static const Btl::String32			TypeName()		{ return "Cycle Actor"; }
	const CycleActorCreationData*		GetCreationData() const {return static_cast<const CycleActorCreationData*>(C_GameActor::GetCreationData().GetData());}
	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:

	S32									m_NumElements;
	F32									m_NextChangeTime;
	S32									m_CurrectStateOffset;
};

}// End namespace Actors


#endif // CYCLEACTOR_H
