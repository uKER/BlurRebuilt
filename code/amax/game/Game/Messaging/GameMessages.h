#ifndef VEHICLE_MESSAGES_H
#define VEHICLE_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>

#include <misc_shared/Messaging/GameMessaging.h>
#include <misc_shared/Messaging/MessagingDeclare.h>
#include <misc_shared/Messaging/MessageBase.h>

#include <GameLogic\Replays\ReplayMessages.h>


namespace Game
{

extern void RegisterMessageDebugInfo(Debug::Logger* pLogger = NULL);

enum E_GameMsg
{
	E_GameMsg_HiBits = FOURCC_HI20('AMGM'),	///< NULL identifier, should never be returned from C_MessageBase::Type().
	E_GameMsg_ActivePhysicsBodyUpdate,
	E_GameMsg_PhysicalObjectImpact,
	E_GameMsg_UpdateLifeTime,

	E_GameMsg_PostCreateInit,	///< Should be broadcast after creating all entities in the game world.

	E_GameMsg_KillObjectsForCutScene,

	E_GameMsg_End, 
	E_GameMsg_Begin = E_GameMsg_HiBits + 1, 
	E_GameMsg_Count = E_GameMsg_End - E_GameMsg_Begin
};

enum E_AIMessage
{
	E_AIMessage_HiBits = FOURCC_HI20('AIMX'),
	
	E_AIMessage_Simulate,
	E_AIMessage_StartSimulate,
	E_AIMessage_StopSimulate,
	E_AIMessage_Start,
	E_AIMessage_Stop,
	E_AIMessage_StartSetSpeed,
	E_AIMessage_EndSetSpeed,
	E_AIMessage_AllLoaded,
	E_AIMessage_LoadFailed,
	E_AIMessage_TrackLoaded,
	E_AIMessage_DriverLoaded,
	E_AIMessage_LoadAITrackResources,
	E_AIMessage_LoadAIDriverResources,
	E_AIMessage_ReLoadAIDriverResources,
	E_AIMessage_SetRoute,
	E_AIMessage_InitialiseIntro,
	E_AIMessage_CreateIntroSpline,
	E_AIMessage_SetIntroOutroSpeed,
	E_AIMessage_SetIntroOutroSpline,
	E_AIMessage_DisableAI,
	E_AIMessage_DriveTowardsPoint,
	E_AIMessage_AtEndIntroOutroSpline,
	E_AIMessage_AttemptedSideSwipe,
	E_AIMessage_CreatePerkLink,

	E_AIMessage_End, 
	E_AIMessage_Begin = E_AIMessage_HiBits + 1, 
	E_AIMessage_Count = E_AIMessage_End - E_AIMessage_Begin
};

enum E_VehicleMessage	//CA - Sorry renamed to E_VehicleMessages as we already have a GameMsg...
{
	/****************************************************************************\
	|						Vehicle Messages									 |
	\****************************************************************************/

	E_VehicleMessage_HiBits = FOURCC_HI20('AGMS'),	

	//Vehicle resources all loaded and we're ready to go
	E_VehicleMessage_Loaded,

	//Update vehicle controls
	E_VehicleMessage_UpdatePlayerControls,	
	E_VehicleMessage_UpdateAIControls,
	E_VehicleMessage_UpdateNetworkControls,
	E_VehicleMessage_PostUpdate,

	E_VehicleMessage_UpdatePerformanceModifiers,

	E_VehicleMessage_UpdateDriver,
	E_VehicleMessage_ResetDriver,

	E_VehicleMessage_ForceAIControl,				//Sets the vehicle to be
	E_VehicleMessage_ForceHumanControl, 
	
	E_VehicleMessage_ResetAtSpawn,					//Reset to the starting position
	E_VehicleMessage_ResetAtWorldPosition,			//Put the vehicle where we tell it to be, damnit!
	E_VehicleMessage_ResetOnTrack,					//Reset the vehicle according to the spline
	E_VehicleMessage_ResetAtVehiclePosition,
	E_VehicleMessage_ResetAtCameraPosition,			//Reset the vehicle from where the camera is

	E_VehicleMessage_ResetEnteredCameraZone,
	E_VehicleMessage_ResetExitedCameraZone,
	E_VehicleMessage_ResetForceCameraZone,
	E_VehicleMessage_ResetEntryZone,
	E_VehicleMessage_ResetExitZone,
	E_VehicleMessage_ResetTwoWayZone,
	E_VehicleMessage_ResetForceZone,
	
	E_VehicleMessage_ResetAfterWrecked,				//Sent from the damage component to the HoG component to tell it to cause a HoG reset after wrecked
	E_VehicleMessage_CancelWreckedTimer,						//If the vehicle has been force reseted then cause a HoG reset after wrecked immediately rather than do two

	E_VehicleMessage_EnableDisableAutoHOGPlacement,

	E_VehicleMessage_VehicleWorldPosition,			//The world position of the vehicle
	E_VehicleMessage_VehicleFocusPosition,

	E_VehicleMessage_Enable,
	E_VehicleMessage_Disable,
	
	E_VehicleMessage_DebugRender,

	E_VehicleMessage_RaceStarted,
	E_VehicleMessage_ImpactEvent,
	E_VehicleMessage_ShieldedImpactEvent,

	E_VehicleMessage_ControlsDisable,				//Stop any form of control getting to the vehicle, essentially leave it as a dynamic object (with its brakes and handbrake on)
	E_VehicleMessage_ControlsEnable,						

	E_VehicleMessage_ChangedHealthLevel,
	E_VehicleMessage_FatallyDamaged,
	E_VehicleMessage_ResetFatalDamage,
	E_VehicleMessage_RepairDamage,

	E_VehicleMessage_Eliminated,

	E_VehicleMessage_Overtake,
	E_VehicleMessage_OvertakeWithNitro,
	E_VehicleMessage_CloseProximity,
	E_VehicleMessage_ExhaustPopEvent,
	
	E_VehicleMessage_FirePickup,
	E_VehicleMessage_ApplyShockedEffect,

	E_VehicleMessage_LeftRace,

	E_VehicleMessage_NearlyFinishedRace,			//In the vinegar strokes of the race as it were
	E_VehicleMessage_TakeOut,		

	E_VehicleMessage_AudioImpactEvent,

	E_VehicleMessage_CheckTriggerRegion,
	E_VehicleMessage_TriggerActorEvent,

	E_VehicleMessage_Freeze,

	E_VehicleMessage_LoseGrip,
	E_VehicleMessage_DontLoseGrip,
	E_VehicleMessage_StopWreckedEffect,
	E_VehicleMessage_KillNitro,

	E_VehicleMessage_AudioVehiclePartSlamming,

	E_VehicleMessage_ApplyDebugDamage,

	E_VehicleMessage_AudioWaterSplash,

	E_VehicleMessage_Respray,
	
	E_VehicleMessage_SetVisibility,

	//Allows us to iterate through these bad boys funkily
	E_VehicleMessage_End, 
	E_VehicleMessage_Begin = E_VehicleMessage_HiBits + 1, 
	E_VehicleMessage_Count = E_VehicleMessage_End - E_VehicleMessage_Begin
};

enum E_PickupMessage	
{
	/****************************************************************************\
	|						Pickup Messages										 |
	\****************************************************************************/

	E_PickupMessage_HiBits = FOURCC_HI20('KCUF'),	

	E_PickupMessage_PickupPickedUp,					//The pickup has been picked up
	E_PickupMessage_GrantPickup,					//Pickup has been gifted
	E_PickupMessage_MissileExploded,				//Missile has blown up
	E_PickupMessage_MissileHitWorld,				//Missile has hit the world and is just about to blow
	E_PickupMessage_MineExploded,					//Mine has blown up

	E_PickupMessage_PickupInitiated,				//Someone's fired off a pickup
	E_PickupMessage_PickupFinished,					//Someone's finished using their pickup
	E_PickupMessage_HitByPickup,					//This vehicle's been hit by a pickup fired by someone else
	E_PickupMessage_HitDecoy,						//This vehicle has hit someone else's decoy pickup

	E_PickupMessage_EnablePickupType,				//Allow a type of pickup being given
	E_PickupMessage_DisablePickupType,				//Disable a type of pickup

	E_PickupMessage_EnableAllPickupTypes,			//Allow all types of pickup to be given
	E_PickupMessage_DisableAllPickupTypes,			//Disable all types of pickup to be given

	E_PickupMessage_SetPickupPositionId,			//Set the pickup position's id

	E_PickupMessage_SetPickup,
	E_PickupMessage_RemovePower,
	E_PickupMessage_ClearHitUpdaters,
	E_PickupMessage_Shuffle,

	E_PickupMessage_EnteredPickup,
	E_PickupMessage_ExitedPickup,
	E_PickupMessage_UpdateFanDemandState,

	E_PickupMessage_ActivatePickup,
	E_PickupMessage_ResetPickup,
	E_PickupMessage_ShockedAPickup,

	E_PickupMessage_WakeAudio,
	E_PickupMessage_SleepAudio,

	E_PickupMessage_ActivatePickups,
	E_PickupMessage_DebugForcePickupRespawn,		//Allow debugging of the random pickup system

	E_PickupMessage_CollectStopwatch,

	E_PickupMessage_KillAllPickups,

	//Allows us to iterate through these bad boys funkily
	E_PickupMessage_End, 
	E_PickupMessage_Begin = E_PickupMessage_HiBits + 1, 
	E_PickupMessage_Count = E_PickupMessage_End - E_PickupMessage_Begin

};

typedef Messaging::C_MessageValue<F32, E_GameMsg_ActivePhysicsBodyUpdate> C_ActivePhysicsBodyUpdateMsg;

enum E_PhysicalObjectImpactType
{
	E_PhysicalObjectImpactType_Unknown = -1,

	E_PhysicalObjectImpactType_Hit,
	E_PhysicalObjectImpactType_Break,

	E_PhysicalObjectImpactType_Num,
};

class C_PhysicalObjectImpactMsg : public Messaging::C_MessageDerived<C_PhysicalObjectImpactMsg, Game::E_GameMsg_PhysicalObjectImpact>
{
public:
	C_PhysicalObjectImpactMsg(E_PhysicalObjectImpactType impactType, float impactSize) : m_ImpactType(impactType), m_ImpactSize(impactSize) {}
	E_PhysicalObjectImpactType	m_ImpactType;
	float						m_ImpactSize;
};

class C_KillForOutroMsg : public Messaging::C_MessageDerived<C_KillForOutroMsg, Game::E_GameMsg_KillObjectsForCutScene>
{
public:
	C_KillForOutroMsg(Maths::HVec3 p_Position, float p_DistanceSquared) : m_Position(p_Position), m_DistanceSquared(p_DistanceSquared) {}
	Maths::HVec3				m_Position;
	float						m_DistanceSquared;
};

} // namespace Game

#endif // VEHICLE_MESSAGES_H
