#include "GamePhysicsPch.h"
#include <game_physics/BodyUserData.h>

#include <game_physics/BodyUserDataTypes.h>


namespace GamePhysics
{

U32 C_BodyUserDataTypes::s_NumTypes = 0;

U32 C_BodyUserData::AllocateTypeUid()
{
	return C_BodyUserDataTypes::s_NumTypes++;
}

} // namespace GamePhysics
