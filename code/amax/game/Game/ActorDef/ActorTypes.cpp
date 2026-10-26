#include "stdafx.h"

#include <actor_system/ActorSys.h>
#include <actor_system/ActorFactory.h>
#include <actor_system/GameComponentFactory.h>

#include <shared_actors/SharedActorTypes.h>

#include <crowd_system/CrowdSystemActor.h>
#include <crowd_system/CrowdMessaging.h>

//#include <Game/GameActors/DynamicObjectActor.h>
#include <Game/GameActors/SystemActors.h>
#include <Game/GameActors/TestActors.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Effects/EffectTestActor.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/SharedActorMessages.h>

#include <GameLogic/ActorStudioActors/ActorStudioActors.h>

#include "../Messaging/GameMessages.h"

#include <Game/GameActors/AudioActors.h>
#include "Game/GameActors/Components/GameComponents.h"
#include <GameLogic/ActorStudioActors/AdditionalComponents.h>
#include <GameLogic/ActorStudioActors/PickupPositionActorDecoy.h>
#include <GameLogic/ActorStudioActors/UICameraController.h>

#include "Game/AmaxCam/CameraActor.h"
#include "Game/AmaxCam/DirectorActor.h"
#include "GameLogic/ActorStudioActors/MovementDualSpline.h"
#include "GameLogic/ActorStudioActors/UiLocatorActor.h"

#if ACTOR_SYSTEM_MEMORY_TRACKING
namespace Actor
{
template <> inline const char* GetMemoryTrackerNameForActorType<Game::Actors::C_VehicleActor>() {return "Vehicle";}
template <> inline const char* GetMemoryTrackerNameForActorType<Game::Actors::C_ModelTestActor>() {return "ModelTest";}
template <> inline const char* GetMemoryTrackerNameForActorType<Actors::DynamicObjectActor>() {return "DynamicObject";}
template <> inline const char* GetMemoryTrackerNameForActorType<AmaxCam::C_CameraActor>() {return "Camera";}
template <> inline const char* GetMemoryTrackerNameForActorType<Game::Actors::C_EffectTestActor>() {return "EffectTest";}
template <> inline const char* GetMemoryTrackerNameForActorType<Game::Actors::C_NodeActor>() {return "Node";}
template <> inline const char* GetMemoryTrackerNameForActorType<Game::Actors::RaceLogicCore>() {return "RaceLogicCore";}
template <> inline const char* GetMemoryTrackerNameForActorType<Game::Actors::C_AudioActor>() {return "Audio";}
template <> inline const char* GetMemoryTrackerNameForActorType<Game::Actors::C_MusicActor>() {return "Music";}
template <> inline const char* GetMemoryTrackerNameForActorType<Actors::C_PhysicalObject>() {return "PhysicalObject";}

template <> inline const char* GetMemoryTrackerNameForActorType<Game::Actors::C_VehicleLogicActor>() {return "LogicVehicle";}

template <> inline const char* GetMemoryTrackerNameForActorType<Game::Actors::C_GameActor>() {return "Game";}
template <> inline const char* GetMemoryTrackerNameForActorType<Actors::PickupPositionActor>() {return "PickupPosition";}
template <> inline const char* GetMemoryTrackerNameForActorType<Actors::SplitLineActor>() {return "SplitLine";}
template <> inline const char* GetMemoryTrackerNameForActorType<Actors::StartPositionActor>() {return "StartPosition";}
template <> inline const char* GetMemoryTrackerNameForActorType<Components::C_MovementDualSpline>() {return "MovementDualSpline";}

template <> inline const char* GetMemoryTrackerNameForActorType<Actors::FanDemandPositionActor>() {return "FanDemandPosition";}

} // namespace Actor
#endif // ACTOR_SYSTEM_MEMORY_TRACKING


//#include <GameLogic/ActorStudioActors/MessageSenderActor.h>			// Include Elsewhere





namespace Game
{

void RegisterActorTypes( Actor::C_ActorFactory& actorFactory )
{
// Note: Don't need to register creation data with ActorCreationDataFactory unless spawning actors from code
// however we will do just in case


//Register any types that are used in Mercury
	::Actors::RegisterSharedActorTypes(actorFactory );

	actorFactory.Register<Actor::C_Actor>();


// Race logic actor
	actorFactory.Register<Actors::RaceLogicCore>();
	Actor::C_ActorCreationDataFactory::Register<Actors::RaceLogicCoreActorCreate>(E_ActorTypeId_RaceLogicCore);

	actorFactory.Register<Actors::C_NodeActor>();
	Actor::C_ActorCreationDataFactory::Register<Actors::C_NodeActorCreate>(E_ActorTypeId_Node);

//	actorFactory.Register<Actors::C_ModelTestActor>();
//	Actor::C_ActorCreationDataFactory::Register<Actors::C_ModelTestActorCreate>(E_ActorTypeId_ModelTest);


// Vehicle actors
	actorFactory.Register<Actors::C_VehicleActor>();
	Actor::C_ActorCreationDataFactory::Register<Actors::C_VehicleActorCreate>(E_ActorTypeId_Vehicle);


// Cameras
	actorFactory.Register<AmaxCam::C_CameraActor>();
	Actor::C_ActorCreationDataFactory::Register<AmaxCam::C_CameraActorCreate>(E_ActorTypeId_CameraActor);

	actorFactory.Register<AmaxCam::C_DirectorActor>();
	Actor::C_ActorCreationDataFactory::Register<AmaxCam::C_DirectorActorCreate>(E_ActorTypeId_DirectorActor);


// Race Logic stuff
	actorFactory.Register< ::Actors::StartPositionActor>();			// Should register creation data for consistancy
	actorFactory.Register< ::Actors::SplitLineActor>();
	actorFactory.Register< ::Actors::PickupPositionActor>();
	actorFactory.Register< ::Actors::PickupPositionActorDecoy>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::PickupPositionCreate>(E_ActorTypeId_PickupPosition);
	Actor::C_ActorCreationDataFactory::Register< ::Actors::PickupPositionDecoyCreate>(E_ActorTypeId_PickupPositionDecoy);

	actorFactory.Register< ::Actors::FanDemandPositionActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::FanDemandPositionCreationData>(E_ActorTypeId_FanDemandPosition);

// Perk stuff
	actorFactory.Register<Actors::C_EffectTestActor>();
	Actor::C_ActorCreationDataFactory::Register<Actors::C_EffectTestActorCreate>(E_ActorTypeId_EffectTest);


// Dynamic objects
	actorFactory.Register< ::Actors::DynamicObjectActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::DynamicObjectCreationData>(E_ActorTypeId_DynamicObject);

	actorFactory.Register< ::Actors::SpawnerActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::SpawnerCreationData>(E_ActorTypeId_Spawner);

	actorFactory.Register< ::Actors::EffectGeneratorActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::EffectGeneratorCreationData>(E_ActorTypeId_EffectGenerator);

	actorFactory.Register< ::Actors::StartLineInfoActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::StartLineInfoCreationData>(E_ActorTypeId_StartLineInfo);

	actorFactory.Register< ::Actors::LightingRigActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::LightingRigCreationData>(E_ActorTypeId_LightingRig);

	actorFactory.Register< ::Actors::DynamicLightActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::DynamicLightCreationData>(E_ActorTypeId_DynamicLight);

	actorFactory.Register< ::Actors::LogicActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::LogicCreationData>(E_ActorTypeId_Logic);

	actorFactory.Register< ::Actors::TriggerRegionActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::TriggerRegionCreationData>(E_ActorTypeId_TriggerRegion);

	actorFactory.Register< ::Actors::PhysicsModifierActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::PhysicsModifierCreationData>(E_ActorTypeId_PhysicsModifier);

	actorFactory.Register< ::Actors::TrafficActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::TrafficActorCreationData>(E_ActorTypeId_TrafficActor);

	actorFactory.Register< ::Actors::CycleActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::CycleActorCreationData>(E_ActorTypeId_CycleActor);

	actorFactory.Register< ::Actors::DynamicCameraActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::DynamicCameraActorCreationData>(E_ActorTypeId_DynamicCameraActor);

	actorFactory.Register< ::Actors::CameraPositionActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::CameraPositionCreationData>(E_ActorTypeId_CameraPosition);

	actorFactory.Register< ::Actors::CameraTargetActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::CameraTargetCreationData>(E_ActorTypeId_CameraTarget);

	actorFactory.Register< ::Actors::TVCameraControlActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::TVCameraControlCreationData>(E_ActorTypeId_TVCameraControl);

	actorFactory.Register< ::Actors::ShowRoomCamera>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::ShowRoomCameraCreationData>(E_ActorTypeId_ShowRoomCamera);

	actorFactory.Register< ::Actors::AnimatedCamera>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::AnimatedCameraCreationData>(E_ActorTypeId_AnimatedCamera);

	actorFactory.Register< ::Actors::UICameraController>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::UICameraControllerCreationData>(E_ActorTypeId_UICameraController);


	actorFactory.Register< ::Actors::C_PhysicalObject>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::C_PhysicalObjectCreate>(E_ActorTypeId_PhysicalObject);

//	actorFactory.Register< ::Actors::SplinePathActor>();
//	Actor::C_ActorCreationDataFactory::Register< ::Actors::SplinePathCreationData>(E_ActorTypeId_SplinePath);

	actorFactory.Register< ::Actors::MessageSenderActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::MessageSenderCreationData>(E_ActorTypeId_MessageSender);

	actorFactory.Register< ::Actors::ReplayManagerActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::ReplayManagerCreationData>(E_ActorTypeId_ReplayManager);


//Cut Scene Actors
	actorFactory.Register< ::Actors::VehicleProxyActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::VehicleProxyCreationData>(E_ActorTypeId_VehicleProxy);

	actorFactory.Register< ::Actors::C_CutSceneVehicleSplineActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::C_CutSceneVehicleSplineActorCreate>(E_ActorTypeId_VehicleSplineActor);

	actorFactory.Register< ::Actors::C_CutSceneVehiclePositionActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::C_CutSceneVehiclePositionActorCreate>(E_ActorTypeId_VehiclePositionActor);

	actorFactory.Register< ::Actors::CutSceneProxyActor>();
	Actor::C_ActorCreationDataFactory::Register< ::Actors::CutSceneProxyCreationData>(E_ActorTypeId_CutSceneProxy);

	//audio actors
	actorFactory.Register< Actors::C_AudioActor>();
	Actor::C_ActorCreationDataFactory::Register< Actors::C_AudioActorCreate>(E_ActorTypeId_AudioEntity);

	actorFactory.Register< Actors::C_UIAudioActor>();
	Actor::C_ActorCreationDataFactory::Register< Actors::C_MusicActorCreate>(E_ActorTypeId_UIAudioActor);

	actorFactory.Register< Actors::C_ListenerAudioActor>();
	Actor::C_ActorCreationDataFactory::Register< Actors::C_ListenerAudioActorCreate>(E_ActorTypeId_ListenerAudioActor);


//Vehicle Logic Actor
	actorFactory.Register< Actors::C_VehicleLogicActor>();
	Actor::C_ActorCreationDataFactory::Register< Actors::C_VehicleLogicActorCreate>(E_ActorTypeId_LogicVehicle);

//Crowd actors
	actorFactory.Register< AnimatedCrowd::C_CrowdSystemActor >();
	Actor::C_ActorCreationDataFactory::Register< AnimatedCrowd::C_CrowdSystemCreate >(AnimatedCrowd::E_CROWD_ACTOR_TYPE_SYSTEM);

	// Note: namespace Component & Components !!!!!!
	Component::C_GameComponentFactory *cfactory = Component::C_GameComponentFactory::GetInstance();
	assert(cfactory);

	cfactory->Register<Components::C_MovementDualSpline>();

//Ui
	actorFactory.Register< ::Actors::UiLocatorActor >();

// register actor components... TODO: [Oscar] move this somewhere more sensible?
	Components::RegisterSharedComponents();
	Components::RegisterGameComponents();


#ifndef CFG_GOLD
	// Debug messages
	GameMessaging::RegisterMessageDebugInfo();
	Actor::RegisterMessageDebugInfo();
	ActorMsg::RegisterSharedMessageDebugInfo();
	Game::RegisterMessageDebugInfo();
	AnimatedCrowd::RegisterMessageDebugInfo();
#endif		
}

} // namespace Game



//=========================================================================


XT_LIBRARY(AmaxActorStudioActors)
	XT_DEPENDS_ON(XtActorBase)
	XT_DEPENDS_ON(XtActorCreation)
	XT_DEPENDS_ON(XtSharedActorMessages)
	XT_DEPENDS_ON(XtSharedLevelActorParams)
	XT_DEPENDS_ON(SharedComponentCreation)
	XT_DEPENDS_ON(SharedActorCreate)
	XT_DEPENDS_ON(DynamicObjectMessages)
	XT_DEPENDS_ON(CutSceneMessages)
	XT_DEPENDS_ON(AmaxActorComponents)
	
	XT_REGISTER_TYPE(Actors::StartPositionCreate)
	XT_REGISTER_TYPE(Actors::SplitLineCreate)
	XT_REGISTER_TYPE(Actors::PickupPositionCreate)

	XT_REGISTER_TYPE(Actors::DynamicObjectCreationData)
	XT_REGISTER_TYPE(Actors::SpawnerCreationData)
	XT_REGISTER_TYPE(Actors::EffectGeneratorCreationData)
	XT_REGISTER_TYPE(Actors::StartLineInfoCreationData)
	XT_REGISTER_TYPE(Actors::LightingRigCreationData)
	XT_REGISTER_TYPE(Actors::DynamicLightCreationData)
	XT_REGISTER_TYPE(Actors::LogicCreationData)
	XT_REGISTER_TYPE(Actors::TriggerRegionCreationData)
	XT_REGISTER_TYPE(Actors::PhysicsModifierCreationData)
	XT_REGISTER_TYPE(Actors::TrafficActorCreationData)
	XT_REGISTER_TYPE(Actors::CycleElementCreationData)
	XT_REGISTER_TYPE(Actors::CycleActorCreationData)
	XT_REGISTER_TYPE(Actors::DynamicCameraActorCreationData)
	XT_REGISTER_TYPE(Actors::CameraPositionCreationData)
	XT_REGISTER_TYPE(Actors::CameraTargetCreationData)
	XT_REGISTER_TYPE(Actors::TVCameraControlCreationData)
	XT_REGISTER_TYPE(Actors::ShowRoomCameraCreationData)
	XT_REGISTER_TYPE(Actors::AnimatedCameraCreationData)
	XT_REGISTER_TYPE(Actors::UICameraControllerCreationData)
//	XT_REGISTER_TYPE(Actors::SplinePathCreationData)
	XT_REGISTER_TYPE(Actors::MessageSenderCreationData)
	XT_REGISTER_TYPE(Actors::ReplayManagerCreationData)
	XT_REGISTER_TYPE(Actors::C_PhysicalObjectCreate)
	
	XT_REGISTER_TYPE(Actors::E_EFFECT_LOOP_TYPE)
	XT_REGISTER_TYPE(Actors::E_BLEND_TYPE)
	XT_REGISTER_TYPE(Actors::E_FADE_TYPE)
	XT_REGISTER_TYPE(Actors::E_ANIMCAMERASEQ_TYPE)
	
	XT_REGISTER_TYPE(Actors::StartInfoType)
//	XT_REGISTER_TYPE(Actors::E_PREDEFINEDOFFSET)

	XT_REGISTER_TYPE(Actors::ShowRoomCameraParams)
	XT_REGISTER_TYPE(Actors::AnimatedCameraSequence)

//	XT_REGISTER_TYPE(Components::MovementSplineFollowerCreationData)
	XT_REGISTER_TYPE(Components::C_MovementDualSplineCreate)

	//Cut Scene actors
	XT_REGISTER_TYPE(Actors::VehicleProxyCreationData)
	XT_REGISTER_TYPE(Actors::C_CutSceneVehicleSplineActorCreate)
	XT_REGISTER_TYPE(Actors::C_CutSceneVehiclePositionActorCreate)
	XT_REGISTER_TYPE(Actors::CutSceneProxyCreationData)

	//Fan Demand Actors
	XT_REGISTER_TYPE(Actors::FanDemandPositionCreationData)
	XT_REGISTER_TYPE(Actors::FanDemandMoveBase)

	XT_REGISTER_TYPE(Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE)
	XT_REGISTER_TYPE(Actors::FANDEMANDMOVE_SPEED_TYPE)
	XT_REGISTER_TYPE(Actors::FANDEMANDMOVE_DRIFTING_TYPE)
	XT_REGISTER_TYPE(Actors::FANDEMANDMOVE_PICKUPS_TYPE)
	XT_REGISTER_TYPE(Actors::FANDEMANDMOVE_DAMAGE_TYPE)
	XT_REGISTER_TYPE(Actors::FANDEMANDMOVE_MISC_TYPE)

	XT_REGISTER_TYPE(Actors::FanDemandMoveTriggerBox)
	XT_REGISTER_TYPE(Actors::FanDemandMoveScore)
	XT_REGISTER_TYPE(Actors::FanDemandMoveOvertake)
	XT_REGISTER_TYPE(Actors::FanDemandMoveJump)
	XT_REGISTER_TYPE(Actors::FanDemandMoveSpeed)
	XT_REGISTER_TYPE(Actors::FanDemandMoveDrifting)
	XT_REGISTER_TYPE(Actors::FanDemandMovePickups)
	XT_REGISTER_TYPE(Actors::FanDemandMoveCarnage)
	XT_REGISTER_TYPE(Actors::FanDemandMoveDamage)
	XT_REGISTER_TYPE(Actors::FanDemandMoveMisc)

	XT_REGISTER_TYPE(Actors::FanDemandGate)
	XT_REGISTER_TYPE(Actors::FanDemand)

	//Ui
	XT_REGISTER_TYPE(Actors::UiLocatorCreationData)

#ifndef NOT_USING_AUDIO_COMPONENETS
	XT_REGISTER_TYPE(Game::Actors::C_AudioActorCreate)
	XT_REGISTER_TYPE(Game::Actors::C_MusicActorCreate)
	XT_REGISTER_TYPE(Game::Actors::C_UIAudioActorCreate)
	XT_REGISTER_TYPE(Game::Actors::C_ListenerAudioActorCreate)
#endif 

XT_END_LIBRARY


