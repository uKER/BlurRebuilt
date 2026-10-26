#include "stdafx.h"

#include <actor_system/ActorSys.h>
#include <actor_system/ActorFactory.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <shared_actors/SharedActorTypes.h>
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/ActorImpl/SplineActor.h>

#include <crowd_system/CrowdSystemActor.h>

#include <Game/GameActors/AnimationTestActor.h>
#include <Game/GameActors/CharacterTestActor.h>
#include <Game/GameActors/SystemActors.h>
#include <Game/GameActors/TestActors.h>
#include <Game/GameActors/RagdollActor.h>
#include <Game/GameActors/CollideModelTestActor.h>
#include <Game/GameActors/VehicleActor.h>
#include <Game/GameActors/VehicleTestActor.h>
#include <Game/GameActors/EffectTestActor.h>
#include <Game/MGCam/CameraActor.h>
#include <Game/MGCam/DirectorActor.h>
#include <Game/MGCam/MovementDualSpline.h>

XT_LIBRARY(GameActors)
	XT_DEPENDS_ON(XtActorBase)
	XT_DEPENDS_ON(XtActorCreation)

	XT_DEPENDS_ON(XtSharedActorMessages)
	XT_DEPENDS_ON(XtSharedLevelActorParams)
	XT_DEPENDS_ON(SharedComponentCreation)
	XT_DEPENDS_ON(SharedActorCreate)
	
	XT_REGISTER_TYPE(Game::Actors::C_ModelTestActorCreateXt)
	XT_REGISTER_TYPE(Game::Actors::C_CollideModelTestActorCreate)
	XT_REGISTER_TYPE(Game::Actors::C_PlatformActorCreate)
XT_END_LIBRARY


namespace Game
{

void RegisterActorTypes( Actor::C_ActorFactory& actorFactory )
{
	::Actors::RegisterSharedActorTypes(actorFactory);
	
	// register actors with factory
	actorFactory.Register<Actor::C_Actor>();
	actorFactory.Register<Actors::C_AnimationTestActor>();
	actorFactory.Register<Actors::C_CharacterTestActor>();
	actorFactory.Register<Actors::C_ModelTestActor>();
	actorFactory.Register<Actors::C_CollideModelTestActor>();
	actorFactory.Register<Actors::C_NodeActor>();
	actorFactory.Register<Actors::C_VehicleActor>();
	actorFactory.Register<Actors::C_VehicleTestActor>();
	actorFactory.Register<Actors::C_EffectTestActor>();
	actorFactory.Register<MGCam::C_CameraActor>();
	actorFactory.Register<MGCam::C_DirectorActor>();
	//actorFactory.Register< ::Actors::C_SplineActor>();
	actorFactory.Register<Actors::C_RagdollModelTestActor>();
	actorFactory.Register<AnimatedCrowd::C_CrowdSystemActor>();

	// register creation data for spawned actors
	Actor::C_ActorCreationDataFactory::Register<Actors::C_AnimationTestActorCreate>(E_ActorTypeId_AnimationTest);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_CharacterTestActorCreate>(E_ActorTypeId_CharacterTest);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_ModelTestActorCreateProcedural>(E_ActorTypeId_ModelTest);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_NodeActorCreate>(E_ActorTypeId_Node);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_VehicleActorCreate>(E_ActorTypeId_Vehicle);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_VehicleTestActorCreate>(E_ActorTypeId_VehicleTest);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_EffectTestActorCreate>(E_ActorTypeId_EffectTest);
	Actor::C_ActorCreationDataFactory::Register<MGCam::C_CameraActorCreate>(E_ActorTypeId_CameraActor);
	Actor::C_ActorCreationDataFactory::Register<MGCam::C_DirectorActorCreate>(E_ActorTypeId_DirectorActor);
	Actor::C_ActorCreationDataFactory::Register< ::Actors::C_SplineActorCreate>(E_ActorTypeId_Spline);
	Actor::C_ActorCreationDataFactory::Register<Actors::C_RagdollModelTestActorCreateProcedural>(E_ActorTypeId_Ragdoll);
	Actor::C_ActorCreationDataFactory::Register<AnimatedCrowd::C_CrowdSystemCreate>(AnimatedCrowd::E_CROWD_ACTOR_TYPE_SYSTEM);

	// register actor components... TODO: [Oscar] move this somewhere more sensible?
	Components::RegisterSharedComponents();

#ifndef CFG_GOLD
	// register messages... TODO: [Oscar] move this somewhere more sensible?
	GameMessaging::RegisterMessageDebugInfo();
	Actor::RegisterMessageDebugInfo();
	ActorMsg::RegisterSharedMessageDebugInfo();
#endif	
}

} // namespace Game
