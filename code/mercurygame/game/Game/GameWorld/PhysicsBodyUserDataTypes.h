#ifndef GAMEWORLD_PHYSICSBODYUSERDATATYPES_H
#define GAMEWORLD_PHYSICSBODYUSERDATATYPES_H

#ifdef _MSC_VER
#pragma once
#endif


#include <game_physics/BodyUserData.h>
#include <shared_actors/Utility/PhysicsBodyUserData.h>


namespace Game
{

class C_PhysicsBodyUserDataTypes
{
public:
	static void Init();
	static void Shutdown();

	static const Actor::C_Actor* GetActorFromUserData(const void* pUserData); ///< gets the actor from the user data

private:
	static bool s_bInitialised; ///< has the static data been initialised
	static U32	s_ActorUserTypesBitfield; ///< bit set if body user data type is for actors

	C_PhysicsBodyUserDataTypes(); // can not instantiate
	~C_PhysicsBodyUserDataTypes(); // can not instantiate

	C_PhysicsBodyUserDataTypes(const C_PhysicsBodyUserDataTypes& copy); // can not copy
	C_PhysicsBodyUserDataTypes& operator = (const C_PhysicsBodyUserDataTypes& copy); // can not copy

};

/**
If the user data object is derived from Actors::C_PhysicsBodyUserData then we
can get the actor from there.
@param pUserData NULL or holds the address of a GamePhysics::C_BodyUserData object.
@returns NULL if this is not actor user data.
*/
inline const Actor::C_Actor* C_PhysicsBodyUserDataTypes::GetActorFromUserData(const void* pUserData)
{
	BIZ_ASSERT(s_bInitialised);

	const Actor::C_Actor* pActor = NULL;

	if (pUserData != NULL)
	{
		const GamePhysics::C_BodyUserData* pBodyUserData = reinterpret_cast<const GamePhysics::C_BodyUserData*>(pUserData);

		if ((s_ActorUserTypesBitfield & (1U << pBodyUserData->GetTypeUid())) != 0)
		{
			pActor = static_cast<const Actors::C_PhysicsBodyUserData*>(pBodyUserData)->GetOwner();
		}
	}

	return pActor;
}

} // namespace Game


#endif // GAMEWORLD_PHYSICSBODYUSERDATATYPES_H
