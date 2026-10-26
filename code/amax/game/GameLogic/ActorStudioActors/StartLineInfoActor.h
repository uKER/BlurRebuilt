#ifndef STARTLINEINFOACTOR_H
#define STARTLINEINFOACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>

#include <Dynamic/Node.h>
#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>

#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <SpecialFX/SpecialFX.h>

#include <shared_actors/Components/Creation/MovementComponentsCreate.h>
#include <GameLogic/ActorStudioActors/EffectGeneratorActor.h>


//=========================================================================
//						SPAWNER ACTOR CLASSES
//=========================================================================


namespace Actors
{

enum StartInfoType
{
	StartInfoType_Start,
	StartInfoType_Checkpoint,
	StartInfoType_Final,
	StartInfoType_Finish,
	StartInfoType_Checkpoint_StartFinish,
};

//=========================================================================



class StartLineInfoCreationData : public EffectGeneratorCreationData
{
public:
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_StartLineInfo;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_StartLineInfo;}

	StartInfoType mType;
};


//=========================================================================


class StartLineInfoActor : public EffectGeneratorActor
{
public:
	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_StartLineInfo; }
	static const Btl::String32			TypeName()		{ return "Start Info Line"; }


	StartLineInfoActor();
	~StartLineInfoActor();

	const StartLineInfoCreationData		*pCreationData;
	const StartLineInfoCreationData		*GetCreationData() const {return static_cast<const StartLineInfoCreationData*>(C_GameActor::GetCreationData().GetData());}

protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
	StartInfoType						mType;
};


}// End namespace Actors

#endif // STARTLINEINFOACTOR_H
