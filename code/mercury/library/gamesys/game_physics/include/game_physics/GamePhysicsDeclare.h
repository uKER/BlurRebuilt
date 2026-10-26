#ifndef GAMEPHYSICS_GAMEPHYSICSDECLARE_H
#define GAMEPHYSICS_GAMEPHYSICSDECLARE_H

#ifdef _MSC_VER
#pragma once
#endif


namespace GamePhysics
{

class C_BodyUserData;
class C_CollisionFilter;
class C_World;

struct C_CollisionEvent;
struct C_CollisionResult;
struct C_FluidEvent;

/**
@brief Callback for adjusting collision results.
@param collisionResult result to optionally adjust.
@returns false to ignore this result.
*/
typedef bool (*F_CollisionResultAdjuster)(const C_CollisionResult& collisionResult);

/**
@brief Callback for handling collision events.
@param collisionEvent event to handle.
*/
typedef void (*F_CollisionEventHandler)(const C_CollisionEvent& collisionEvent);

/**
@brief Callback for handling fluid events.
@param fluidEvent event to handle.
*/
typedef void (*F_FluidEventHandler)(const C_FluidEvent& fluidEvent);

} // namespace GamePhysics


#endif // GAMEPHYSICS_GAMEPHYSICSDECLARE_H
