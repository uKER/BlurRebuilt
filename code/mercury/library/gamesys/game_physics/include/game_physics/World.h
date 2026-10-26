#ifndef GAMEPHYSICS_WORLD_H
#define GAMEPHYSICS_WORLD_H

#ifdef _MSC_VER
#pragma once
#endif


#include <game_physics/GamePhysicsDeclare.h>


namespace Gravity
{
	class C_Scene;
}


namespace GamePhysics
{

class C_WorldImpl;

class C_World
{
public:
	explicit C_World(Gravity::C_Scene* pGravityScene);
	~C_World();

	void SetCollisionResultAdjuster(const U32 typeUid, F_CollisionResultAdjuster pFn);
	void SetCollisionResultAdjuster(const U32 typeUidA, const U32 typeUidB, F_CollisionResultAdjuster pFn);

	void SetCollisionEventHandler(const U32 typeUid, F_CollisionEventHandler pFn);
	void SetCollisionEventHandler(const U32 typeUidA, const U32 typeUidB, F_CollisionEventHandler pFn);

	void SetFluidEventHandler(const U32 typeUid, F_FluidEventHandler pFn);

private:
	C_WorldImpl* m_pImpl;

	C_World(const C_World& copy); // can not copy
	C_World& operator = (const C_World& copy); // can not copy

};

} // namespace GamePhysics


#endif // GAMEPHYSICS_WORLD_H
