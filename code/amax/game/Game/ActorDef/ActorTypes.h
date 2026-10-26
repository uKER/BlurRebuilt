#ifndef GAME_ACTOR_TYPES_H
#define GAME_ACTOR_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>

#include <actor_system/ActorDeclare.h>

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
	E_ActorTypeId_HiBits = FOURCC_HI20('AMAT'), 

	E_ActorTypeId_GameActor,
	E_ActorTypeId_Node,
	E_ActorTypeId_ModelTest,
	E_ActorTypeId_Vehicle,
	
	E_ActorTypeId_RaceLogicCore,
	E_ActorTypeId_StartPosition,
	E_ActorTypeId_SplitLine,
	E_ActorTypeId_PickupPosition,
	E_ActorTypeId_PickupPositionDecoy,
	E_ActorTypeId_DynamicObject,
	E_ActorTypeId_Spawner,
	E_ActorTypeId_EffectGenerator,
	E_ActorTypeId_LightingRig,
	E_ActorTypeId_DynamicLight,
	E_ActorTypeId_Logic,
	E_ActorTypeId_TriggerRegion,
	E_ActorTypeId_PhysicsModifier,
	E_ActorTypeId_DynamicCameraActor,
	E_ActorTypeId_CameraPosition,
	E_ActorTypeId_CameraTarget,
	E_ActorTypeId_TVCameraControl,
	E_ActorTypeId_ShowRoomCamera,
	E_ActorTypeId_AnimatedCamera,
	E_ActorTypeId_UICameraController,
//	E_ActorTypeId_SplinePath,
	E_ActorTypeId_MessageSender,
	E_ActorTypeId_ReplayManager,
	E_ActorTypeId_CameraActor,
	E_ActorTypeId_PhysicalObject,
	E_ActorTypeId_TrafficActor,
	E_ActorTypeId_CycleActor,
	E_ActorTypeId_Missile, 
	E_ActorTypeId_Mine,
	E_ActorTypeId_EffectTest,

#ifndef NOT_USING_AUDIO_COMPONENETS
	E_ActorTypeId_AudioEntity,
	E_ActorTypeId_MusicActor,
	E_ActorTypeId_UIAudioActor,
	E_ActorTypeId_ListenerAudioActor,
#endif

	E_ActorTypeId_DirectorActor,

	//Cut Scene Actors
	E_ActorTypeId_VehicleProxy,
	E_ActorTypeId_VehicleSplineActor,
	E_ActorTypeId_VehiclePositionActor,
	E_ActorTypeId_CutSceneProxy,

	//Vehicle Logic Actors
	E_ActorTypeId_LogicVehicle,

	E_ActorTypeId_StartLineInfo,
	E_ActorTypeId_FanDemandPosition,

	// Ui
	E_ActorTypeId_UiLocator,

	E_ActorTypeId_End, 
	E_ActorTypeId_Begin = E_ActorTypeId_HiBits + 1, 
	E_ActorTypeId_Count = E_ActorTypeId_End - E_ActorTypeId_Begin
};


void RegisterActorTypes( Actor::C_ActorFactory& actorFactory );

} // namespace Game

#endif //  GAME_ACTOR_TYPES_H
