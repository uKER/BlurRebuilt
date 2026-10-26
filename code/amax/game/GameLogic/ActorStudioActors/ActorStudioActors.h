#ifndef ACTORSTUDIOACTORS_H
#define ACTORSTUDIOACTORS_H

///////////////////////////////////////////////////////////////////////////////
//
//	                   Creating a new ActorStudioActor
//                     -------------------------------
//
// 1. Create a new cpp and h file in the ActorStudioData folder
// 2. Add both a Create structure/class and the Actor itself
// 3. At the bottom of the new actors cpp file register the XT_TYPE with 
//    sorcery/xt or whatever its called!
// 4. Register the actor in the RegisterActorStudioActorTypes() function in 
//    ActorTypes.cpp
// 5. Create a new value in a new enum value for your actor in ActorTypes.h
// 6. Register the Create structure at the bottom of ActorTypes.cpp into the
//    XT_LIBRARY(AmaxActorStudioActors)
// 7. Add the include to the h file here...
// 8. Add the source file to the jamfile.jam (look for the StartPositionActor.cpp)
// 7. Cross your fingers and compile!!
//
///////////////////////////////////////////////////////////////////////////////


#include "StartPositionActor.h"
#include "SplitLineActor.h"
#include "PickupPositionActor.h"
#include "FanDemandPositionActor.h"

//#include "DynamicObjectActor.h"

#include <GameLogic/ActorStudioActors/DynamicObjectActor.h>
#include <GameLogic/ActorStudioActors/MessageSenderActor.h>
#include <GameLogic/ActorStudioActors/ReplayManagerActor.h>
#include <GameLogic/ActorStudioActors/DynamicCameraActor.h>
#include <GameLogic/ActorStudioActors/CameraPositionActor.h>
#include <GameLogic/ActorStudioActors/CameraTargetActor.h>
#include <GameLogic/ActorStudioActors/EffectGeneratorActor.h>
#include <GameLogic/ActorStudioActors/StartLineInfoActor.h>
#include <GameLogic/ActorStudioActors/DynamicLightActor.h>
#include <GameLogic/ActorStudioActors/LogicActor.h>
#include <GameLogic/ActorStudioActors/PhysicsModifierActor.h>
#include <GameLogic/ActorStudioActors/SpawnerActor.h>
#include <GameLogic/ActorStudioActors/TriggerRegionActor.h>
//#include <GameLogic/ActorStudioActors/SplinePathActor.h>
#include <GameLogic/ActorStudioActors/PhysicalObjectActor.h>
#include <GameLogic/ActorStudioActors/TrafficActor.h>
#include <GameLogic/ActorStudioActors/CycleActor.h>
#include <GameLogic/ActorStudioActors/TVCameraControl.h>
#include <GameLogic/ActorStudioActors/ShowRoomCamera.h>
#include <GameLogic/ActorStudioActors/AnimatedCamera.h>


//Cut Scene Actors
#include <GameLogic/ActorStudioActors/VehicleProxyActor.h>
#include <GameLogic/ActorStudioActors/CutSceneVehicleSplineActor.h>
#include <GameLogic/ActorStudioActors/CutSceneVehiclePositionActor.h>
#include <GameLogic/ActorStudioActors/CutSceneProxyActor.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/AdditionalComponents.h>


#endif //ACTORSTUDIOACTORS_H
