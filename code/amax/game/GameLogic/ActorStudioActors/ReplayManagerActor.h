#ifndef REPLAYMANAGERACTOR_H
#define REPLAYMANAGERACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>


//=========================================================================
//						REPLAY MANAGER ACTOR CLASSES
//=========================================================================



namespace Actors
{
	
class ReplayManagerCreationData : public C_GameActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_ReplayManager;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_ReplayManager;}

};


//=========================================================================


class ReplayManagerActor : public Game::Actors::C_GameActor
{
public:
	ReplayManagerActor();
	virtual ~ReplayManagerActor();

	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
	

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_ReplayManager; }

protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();



private:
};

}// End namespace Actors


#endif // REPLAYMANAGERACTOR_H
