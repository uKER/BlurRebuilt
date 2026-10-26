#include "stdafx.h"
#include "Game/GameWorld/PhysicsBodyUserDataTypes.h"

#include <game_physics/BodyUserDataTypes.h>




namespace PhysicsBodyUserDataTypes_Impl
{

template <class T_BodyUserData>
U32 CalcActorPhysicsBodyUserDataBitMask()
{
	// T_BodyUserData must inherit from  Actors::C_PhysicsBodyUserData!
	const T_BodyUserData* pDummyDerived = NULL;
	const Actors::C_PhysicsBodyUserData* pDummyBase = pDummyDerived;

	return (1U << T_BodyUserData::ClassTypeUid());
}

} // PhysicsBodyUserDataTypes_Impl




namespace Game
{

bool C_PhysicsBodyUserDataTypes::s_bInitialised = false;
U32 C_PhysicsBodyUserDataTypes::s_ActorUserTypesBitfield = 0;

void C_PhysicsBodyUserDataTypes::Init()
{
	if (!s_bInitialised)
	{
		using namespace PhysicsBodyUserDataTypes_Impl;

		// assemble a bitfield that indicates if a GamePhysics::C_BodyUserData of a given type can be downcast to an Actors::C_PhysicsBodyUserData

		BIZ_ASSERT(GamePhysics::C_BodyUserDataTypes::NumTypes() <= (sizeof(s_ActorUserTypesBitfield) * 8)); // need to make bitfield larger

		s_ActorUserTypesBitfield = 0
			| CalcActorPhysicsBodyUserDataBitMask<::Actors::C_SimplePhysicsBodyUserData>()
			| CalcActorPhysicsBodyUserDataBitMask<::Actors::C_PBCUserData>()
			;

		// done
		s_bInitialised = true;
	}
}

void C_PhysicsBodyUserDataTypes::Shutdown()
{
	if (s_bInitialised)
	{
		s_bInitialised = false;
		s_ActorUserTypesBitfield = 0;
	}
}

} // namespace Game
