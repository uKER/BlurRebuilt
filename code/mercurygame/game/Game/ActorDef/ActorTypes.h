#ifndef GAME_ACTOR_TYPES_H
#define GAME_ACTOR_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>

namespace Actor
{
	class C_ActorFactory;
}

namespace Game
{

// actor category enum

// actor category bit positions
enum E_ActorCategoryBit
{
	ActorCategoryBit_Player = 0,
	ActorCategoryBit_Vehicle,
	ActorCategoryBit_Character,
	ActorCategoryBit_DynamicObject,
	ActorCategoryBit_Platform,
	ActorCategoryBit_Effect,

	ActorCategoryNoBits
};

// category bit mask
enum E_ActorCategory
{
	ActorCategory_Player		= (1 << ActorCategoryBit_Player),
	ActorCategory_Vehicle		= (1 << ActorCategoryBit_Vehicle),
	ActorCategory_Character		= (1 << ActorCategoryBit_Character),
	ActorCategory_DynamicObject	= (1 << ActorCategoryBit_DynamicObject),
	ActorCategory_Platform		= (1 << ActorCategoryBit_Platform),
	ActorCategory_Effect		= (1 << ActorCategoryBit_Effect),

	ActorCategory_All	= 0xffffffff
};

// game actor types
enum E_ActorTypeId
{
	E_ActorTypeId_HiBits = FOURCC_HI20('MGAT'), 

	E_ActorTypeId_GameActor,
	E_ActorTypeId_Node,
	E_ActorTypeId_ModelTest,
	E_ActorTypeId_CollideModelTest,
	E_ActorTypeId_Platform,
	E_ActorTypeId_VehicleTest,
	E_ActorTypeId_CharacterTest,
	E_ActorTypeId_Vehicle,
	E_ActorTypeId_AnimationTest,
	E_ActorTypeId_RagdollTest,
	E_ActorTypeId_EffectTest,
	E_ActorTypeId_CameraActor,
	E_ActorTypeId_DirectorActor,
	E_ActorTypeId_PathActor,
	E_ActorTypeId_Spline,
	E_ActorTypeId_AnimationClip,
	E_ActorTypeId_Ragdoll,

	E_ActorTypeId_End, 
	E_ActorTypeId_Begin = E_ActorTypeId_HiBits + 1, 
	E_ActorTypeId_Count = E_ActorTypeId_End - E_ActorTypeId_Begin
};

void RegisterActorTypes( Actor::C_ActorFactory& actorFactory );

} // namespace Game

#endif //  GAME_ACTOR_TYPES_H
