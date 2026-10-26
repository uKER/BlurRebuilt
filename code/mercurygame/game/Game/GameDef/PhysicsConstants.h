#ifndef GAME_PHYSICS_CONSTANTS_H
#define GAME_PHYSICS_CONSTANTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/GameDef/Polyflags.h>
#include <gravity/narrowphase/CollisionFilter.h>

namespace Game
{

namespace PhysicsConstants
{

extern Collision::C_CollisionFilter	kDynamicObjectCollisionFilter;	
extern Collision::C_CollisionFilter	kVehicleBodyCollisionFilter;	
extern Collision::C_CollisionFilter	kVehicleWheelCollisionFilter;	
extern Collision::C_CollisionFilter	kVehicleOverlayCollisionFilter;	
extern Collision::C_CollisionFilter	kCharacterCollisionFilter;	
extern Collision::C_CollisionFilter	kRagdollBodyCollisionFilter;	

extern F32 kDefaultSlop;
extern F32 kDefaultVehicleSlop;
extern F32 kDefaultBaumgarte;

} // namespace PhysicsConstants
} // namespace Game

#endif // GAME_PHYSICS_CONSTANTS_H
