#ifndef DYNAMIC_OBJECT_MESSAGES_H
#define DYNAMIC_OBJECT_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/uid/FourCC.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/SharedMessageValues.h>
#include <shared_actors/XtActorMessages.h>
#include <GameLogic/ActorStudioActors/ResourceTypes.h>
#include <Vehicle/Damage/CollisionInfo.h>
#include <actor_system/actor.h>
#include <SpecialFX/SpecialFX.h>
#include <GameLogic/ActorStudioActors/EffectGeneratorActor.h>
#include <GameLogic/ActorStudioActors/DynamicCameraActor.h>
#include <GameLogic/ActorStudioActors/AnimatedCamera.h>
#include <Game/GameWorld/EnvironmentSettings.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>


namespace ActorMsg
{


enum E_DynamicObjMsg
{
	E_DynamicObjMsg_HiBits = FOURCC_HI20('MGDO'), 

	E_DynamicObjMsg_CameraUpdate,						// Message to update camera after everything else has been updated

	E_DynamicObjMsg_VehicleEnteredTrigger,				// Message to a vehicle that it has entered a trigger region
	E_DynamicObjMsg_VehicleExitedTrigger,				// Message to a vehicle that it has exited a trigger region
	E_DynamicObjMsg_VehicleActivatedTrigger,			// Message to a vehicle that it has activated a trigger region

	E_DynamicObjMsg_VehicleHandOfGodEntryArea,
	E_DynamicObjMsg_VehicleHandOfGodExitArea,
	E_DynamicObjMsg_VehicleHandOfGodTwoWayArea,
	E_DynamicObjMsg_VehicleHandOfGodCameraForceArea,
	E_DynamicObjMsg_VehicleHandOfGodCameraEnteredArea,
	E_DynamicObjMsg_VehicleHandOfGodCameraExitedArea,
	E_DynamicObjMsg_VehicleHandOfGodForceResetArea,

	E_DynamicObjMsg_VehicleAchieveAmboyOilfields,		// Messages for achievements that require trigger regions
	E_DynamicObjMsg_VehicleAchieveGasStation,

	E_DynamicObjMsg_ImpactEvent,						// Message to indicate the dynamic object is flying about the world, hitting stuff

	E_DynamicObjMsg_RenderDebug,						// Display dynamic object name and other info
	E_DynamicObjMsg_SorceryUpdate,						// Sorcery data has been modified on the fly
	E_DynamicObjMsg_AnimationUpdate,					// 

	E_DynamicObjMsg_VehicleSloMoEntry,					// Start slow mo mode
	E_DynamicObjMsg_VehicleSloMoExit,					// Exit Slow mo mode
	E_DynamicObjMsg_ChangeSpecialEffect,				// Modifies an Effect Generator actors behaviour
	E_DynamicObjMsg_SetAlphaValue,						// Sets an effect alpha value

	E_DynamicObjMsg_SelectLightingRig,					// Use this actor for setting backlight parameters
	E_DynamicObjMsg_StartDynamicObjectAnimation,		// Triggers the start of a dynamic object animation
	E_DynamicObjMsg_DynamicObjectApplyImpulse,			// Apply shock or explosion force
	E_DynamicObjMsg_DynamicObjectDisableElectricalEffect,	// Disable shock/shunt effects
	E_DynamicObjMsg_DynamicObjectSetPhysicsType,		// Change dynamic objects physics type

	E_DynamicObjMsg_ChangeCameraBehaviour,				// Changes a dynamic camera's behaviour
	E_DynamicObjMsg_SimpleCameraBehaviour,				// Changes a dynamic camera's behaviour to match Dave's ingame camera
	E_DynamicObjMsg_FreezeCameraState,					// Stops the camera blending and freezes it's current position
	E_DynamicObjMsg_FadeDynamicCamera,					// Fades a dynamic camera out then back in
	E_DynamicObjMsg_ResetDynamicCamera,					// Stops a dynamic camera interrogating animation data etc
	E_DynamicObjMsg_CameraCut,							// Sent when an animation driven camera cuts
	E_DynamicObjMsg_StartAnimatedCamera,				// Begins an intro/rival/321 camera sequence
	E_DynamicObjMsg_FinishedAnimatedCamera,				// Camera animation finished
	E_DynamicObjMsg_BlendAnimatedCamera,				// Blend animated camera into race camera
	E_DynamicObjMsg_AutoSelectAnimatedCamera,			// Move animated camera to best position for clear shot of a vehicle
	E_DynamicObjMsg_UICameraControl,					// Sent to a UI Camera controller for moving associated dynamic camera

	E_DynamicObjMsg_ChangeEnvironmentSettings,			// Sent to alter post processing effects

	E_DynamicObjMsg_SetAIRoute,							// AI needs to enable or disable a route
	E_DynamicObjMsg_SetVehicleLogic,					// Enable/Disable pickups, damage, drafting etc
	E_DynamicObjMsg_EnableLevelRoute,					// Enable render and collision geometry from within a level
	E_DynamicObjMsg_DisableLevelRoute,					// Disable render and collision geometry from within a level

	E_DynamicObjMsg_SplineCtrl_StopAndWait,				// Spline Control
	E_DynamicObjMsg_SplineCtrl_SetSpeed,
	E_DynamicObjMsg_SplineCtrl_ReverseDirection,
	E_DynamicObjMsg_SplineCtrl_SetBankAngle,

	E_DynamicObjMsg_OutputDebug,						// Outputs a string to debug
	E_DynamicObjMsg_EnableTVCameras,					// Switch TV Cameras on
	E_DynamicObjMsg_DisableTVCameras,					// Switch TV Cameras off

//==========

	E_ActorLogicMsg_AttachToPuller,						// Notify intelligent spline follower that this actor will be following it
	E_ActorLogicMsg_PullerUpdate,						// Notify a pulled actor where on a spline it should be (replaces animate)

//==========

	E_DynamicObjMsg_VehicleAudioSloMoEntry,					// Start slow mo mode
	E_DynamicObjMsg_VehicleAudioSloMoExit,					// Exit Slow mo mode

	E_DynamicObjMsg_End, 
	E_DynamicObjMsg_Begin = E_DynamicObjMsg_HiBits + 1, 
	E_DynamicObjMsg_Count = E_DynamicObjMsg_End - E_DynamicObjMsg_Begin
};




typedef Messaging::C_MessageValue<F32, E_DynamicObjMsg_CameraUpdate> C_CameraUpdateMsg;



//=========================================================================
//						COLLISION MSG FOR AUDIO
//=========================================================================

class C_DynamicObjImpactMsg : public Messaging::C_MessageDerived<C_DynamicObjImpactMsg, E_DynamicObjMsg_ImpactEvent>
{
public:

	C_DynamicObjImpactMsg()/* : m_DynamicObjIndex(-1), m_OtherDynamicObjIndex(-1)*/ {}

public:

	Vehicle::CollisionInfo						m_CollisionInfo;
	//S32											m_DynamicObjIndex;
	//S32											m_OtherDynamicObjIndex;
	Maths::HMat43								m_Matrix;
	Actor::C_ActorHandle						m_ActorHandle;
	U32											m_HashValue;
	U32											m_CollidesWith;
};




//=========================================================================
//						TRIGGER SYSTEM MESSAGES
//=========================================================================

class C_VehicleEnteredTriggerMsg : public Messaging::C_MessageDerived<C_VehicleEnteredTriggerMsg, E_DynamicObjMsg_VehicleEnteredTrigger>
{
public:
	C_VehicleEnteredTriggerMsg()		{};
	~C_VehicleEnteredTriggerMsg()		{};

	Game::E_TRIGGER_TYPE				m_TriggerType;
	S32									m_RefNumber;
};


class C_XtVehicleEnteredTriggerMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleEnteredTriggerMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	Game::E_TRIGGER_TYPE				m_TriggerType;
	S32									m_RefNumber;

	C_VehicleEnteredTriggerMsg			m_Msg;
};


//=========================================================================


class C_VehicleExitedTriggerMsg : public Messaging::C_MessageDerived<C_VehicleExitedTriggerMsg, E_DynamicObjMsg_VehicleExitedTrigger>
{
public:
	C_VehicleExitedTriggerMsg()		{};
	~C_VehicleExitedTriggerMsg()		{};

	Game::E_TRIGGER_TYPE				m_TriggerType;
	S32									m_RefNumber;
};


class C_XtVehicleExitedTriggerMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleExitedTriggerMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	Game::E_TRIGGER_TYPE				m_TriggerType;
	S32									m_RefNumber;

	C_VehicleExitedTriggerMsg			m_Msg;
};


//=========================================================================

class C_VehicleActivatedTriggerMsg : public Messaging::C_MessageDerived<C_VehicleActivatedTriggerMsg, E_DynamicObjMsg_VehicleActivatedTrigger>
{
public:
	C_VehicleActivatedTriggerMsg()		{};
	~C_VehicleActivatedTriggerMsg()		{};

	Game::E_TRIGGER_TYPE				m_TriggerType;
	S32									m_RefNumber;
};


class C_XtVehicleActivatedTriggerMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleActivatedTriggerMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	Game::E_TRIGGER_TYPE				m_TriggerType;
	S32									m_RefNumber;

	C_VehicleActivatedTriggerMsg		m_Msg;
};

//=========================================================================

class C_VehicleHandOfGodResetCameraForceArea : public Messaging::C_MessageDerived<C_VehicleHandOfGodResetCameraForceArea, E_DynamicObjMsg_VehicleHandOfGodCameraForceArea>
{
public:
	C_VehicleHandOfGodResetCameraForceArea()		{};
	~C_VehicleHandOfGodResetCameraForceArea()	{};

	bool mCameraFollow;
};

class C_XtVehicleHandOfGodResetCameraForceArea : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleHandOfGodResetCameraForceArea(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	bool mCameraFollow;

	C_VehicleHandOfGodResetCameraForceArea	m_Msg;
};

//=========================================================================

class C_VehicleHandOfGodResetCameraEnteredArea : public Messaging::C_MessageDerived<C_VehicleHandOfGodResetCameraEnteredArea, E_DynamicObjMsg_VehicleHandOfGodCameraEnteredArea>
{
public:
	C_VehicleHandOfGodResetCameraEnteredArea()		{};
	~C_VehicleHandOfGodResetCameraEnteredArea()	{};

	bool mCameraFollow;
};

class C_XtVehicleHandOfGodResetCameraEnteredArea : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleHandOfGodResetCameraEnteredArea(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	bool mCameraFollow;

	C_VehicleHandOfGodResetCameraEnteredArea	m_Msg;
};

//=========================================================================

class C_VehicleHandOfGodResetCameraExitedArea : public Messaging::C_MessageDerived<C_VehicleHandOfGodResetCameraExitedArea, E_DynamicObjMsg_VehicleHandOfGodCameraExitedArea>
{
public:
	C_VehicleHandOfGodResetCameraExitedArea()	{};
	~C_VehicleHandOfGodResetCameraExitedArea()	{};
};

class C_XtVehicleHandOfGodResetCameraExitedArea : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleHandOfGodResetCameraExitedArea(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_VehicleHandOfGodResetCameraExitedArea	m_Msg;
};

//=========================================================================

class C_VehicleHandOfGodForceResetArea : public Messaging::C_MessageDerived<C_VehicleHandOfGodForceResetArea, E_DynamicObjMsg_VehicleHandOfGodForceResetArea>
{
public:
	C_VehicleHandOfGodForceResetArea()
	{
		mResetPositions=0;
		mDelay=0.f;
	};
	~C_VehicleHandOfGodForceResetArea()	{};

	XtArray< ::Actors::C_GameActorCreate*>* mResetPositions;
	float mDelay;
};

class C_XtVehicleHandOfGodForceResetArea : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleHandOfGodForceResetArea()
	{
		mDelay=0.f;
	}

	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	XtArray<Actors::C_GameActorCreate*>	mResetPositionActors;
	float mDelay;
	C_VehicleHandOfGodForceResetArea	m_Msg;
};

//=========================================================================

class C_VehicleHandOfGodResetTwoWayArea : public Messaging::C_MessageDerived<C_VehicleHandOfGodResetTwoWayArea, E_DynamicObjMsg_VehicleHandOfGodTwoWayArea>
{
public:
	C_VehicleHandOfGodResetTwoWayArea()		{};
	~C_VehicleHandOfGodResetTwoWayArea()	{};

	F32							mTimer;
	Actors::C_GameActorCreate*	mResetPositionActor;
	S32							mZoneId;
	S32							mIndivId;
};

class C_XtVehicleHandOfGodResetTwoWayArea : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleHandOfGodResetTwoWayArea(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	F32									mTimer;
	Actors::C_GameActorCreate*			mResetPositionActor;
	S32									mZoneId;
	S32									mIndivId;

	C_VehicleHandOfGodResetTwoWayArea	m_Msg;
};

//=========================================================================

class C_VehicleHandOfGodResetEntryArea : public Messaging::C_MessageDerived<C_VehicleHandOfGodResetEntryArea, E_DynamicObjMsg_VehicleHandOfGodEntryArea>
{
public:
	C_VehicleHandOfGodResetEntryArea()		{};
	~C_VehicleHandOfGodResetEntryArea()		{};

	F32							mTimer;
	Actors::C_GameActorCreate*	mResetPositionActor;
	S32							mZoneId;
};


class C_XtVehicleHandOfGodResetEntryArea : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleHandOfGodResetEntryArea(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	F32									mTimer;
	Actors::C_GameActorCreate*			mResetPositionActor;
	S32									mZoneId;

	C_VehicleHandOfGodResetEntryArea	m_Msg;
};

//=========================================================================

class C_VehicleHandOfGodResetExitArea : public Messaging::C_MessageDerived<C_VehicleHandOfGodResetExitArea, E_DynamicObjMsg_VehicleHandOfGodExitArea>
{
public:
	C_VehicleHandOfGodResetExitArea()		{};
	~C_VehicleHandOfGodResetExitArea()		{};

	S32							mZoneId;
};


class C_XtVehicleHandOfGodResetExitArea : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleHandOfGodResetExitArea(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	S32									mZoneId;

	C_VehicleHandOfGodResetExitArea	m_Msg;
};


//=========================================================================


class C_VehicleSloMoEntryMsg : public Messaging::C_MessageDerived<C_VehicleSloMoEntryMsg, E_DynamicObjMsg_VehicleSloMoEntry>
{
public:
	C_VehicleSloMoEntryMsg()	{m_Disable=false; m_TimeScale=1.0f; m_Duration=0.0f; m_TrackCar=true; m_CameraFOV=1.0f; m_Manual=false;};
	~C_VehicleSloMoEntryMsg()	{};

	bool						m_Disable;
	F32							m_TimeScale;
	F32							m_Duration;
	bool						m_TrackCar;
	F32							m_MinSpeed;
	F32							m_MaxSpeed;
	bool						m_Nitro;
	Maths::HVec3				m_CameraPosition;
	Maths::HVec3				m_CameraLookAt;
	F32							m_CameraFOV;
	bool						m_Manual;
};

class C_XtVehicleSloMoEntryMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleSloMoEntryMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_VehicleSloMoEntryMsg		m_Msg;

	bool						m_Disable;
	F32							m_TimeScale;
	F32							m_Duration;
	bool						m_TrackCar;
	F32							m_MinSpeed;
	F32							m_MaxSpeed;
	bool						m_Nitro;
	XtVec3						m_CameraPosition;
	XtVec3						m_CameraLookAt;
	F32							m_CameraFOV;
};

//=========================================================================


class C_VehicleSloMoExitMsg : public Messaging::C_MessageDerived<C_VehicleSloMoExitMsg, E_DynamicObjMsg_VehicleSloMoExit>
{
public:
	C_VehicleSloMoExitMsg()	{m_Disable=false;};
	~C_VehicleSloMoExitMsg()	{};

	bool						m_Disable;
};

class C_XtVehicleSloMoExitMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleSloMoExitMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_VehicleSloMoExitMsg		m_Msg;

	bool						m_Disable;
};

//=========================================================================

class C_VehicleAchieveAmboyOilfieldsMsg : public Messaging::C_MessageDerived<C_VehicleAchieveAmboyOilfieldsMsg, E_DynamicObjMsg_VehicleAchieveAmboyOilfields>
{
public:
	C_VehicleAchieveAmboyOilfieldsMsg()	{};
	~C_VehicleAchieveAmboyOilfieldsMsg()	{};
};

class C_XtVehicleAchieveAmboyOilfieldsMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleAchieveAmboyOilfieldsMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_VehicleAchieveAmboyOilfieldsMsg	m_Msg;
};

//========================================================================

class C_VehicleAchieveGasStationMsg : public Messaging::C_MessageDerived<C_VehicleAchieveGasStationMsg, E_DynamicObjMsg_VehicleAchieveGasStation>
{
public:
	C_VehicleAchieveGasStationMsg()	{};
	~C_VehicleAchieveGasStationMsg()	{};
};

class C_XtVehicleAchieveGasStationMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtVehicleAchieveGasStationMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_VehicleAchieveGasStationMsg	m_Msg;
};

//=========================================================================
//						EFFECT GENERATOR MESSAGES
//=========================================================================


class C_ChangeSpecialEffectMsg : public Messaging::C_MessageDerived<C_ChangeSpecialEffectMsg, E_DynamicObjMsg_ChangeSpecialEffect>
{
public:
	C_ChangeSpecialEffectMsg()			{};
	~C_ChangeSpecialEffectMsg()			{};

	XtHandle<SpecialFX::C_SpecialFX>	m_SpecialEffect;
	F32									m_T1LoopStart;
	F32									m_T2LoopEnd;
	::Actors::E_EFFECT_LOOP_TYPE		m_LoopType;
	F32									m_StartTime;
};


class C_XtChangeSpecialEffectMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtChangeSpecialEffectMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_ChangeSpecialEffectMsg			m_Msg;


	XtHandle<SpecialFX::C_SpecialFX>	m_SpecialEffect;
	F32									m_T1LoopStart;
	F32									m_T2LoopEnd;
	::Actors::E_EFFECT_LOOP_TYPE		m_LoopType;
	F32									m_StartTime;
};

//=========================================================================


class C_SetAlphaValueMsg : public Messaging::C_MessageDerived<C_SetAlphaValueMsg, E_DynamicObjMsg_SetAlphaValue>
{
public:
	C_SetAlphaValueMsg()			{};
	~C_SetAlphaValueMsg()			{};

	F32									m_alpha;
};


class C_XtSetAlphaValueMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSetAlphaValueMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_SetAlphaValueMsg			m_Msg;

	F32									m_alpha;
};






//=========================================================================
//						DYNAMIC OBJECT MESSAGES
//=========================================================================


class C_SelectLightingRigMsg : public Messaging::C_MessageDerived<C_SelectLightingRigMsg, E_DynamicObjMsg_SelectLightingRig>
{
public:
	C_SelectLightingRigMsg()	{};
	~C_SelectLightingRigMsg()	{};

//	Actors::C_GameActorCreate*			m_LightingRigActor;
	XtHandle<Game::CutSceneLightDef>	m_LightingDefinition;
};


class C_XtSelectLightingRigMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSelectLightingRigMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_SelectLightingRigMsg		m_Msg;

//	Actors::C_GameActorCreate*			m_LightingRigActor;
	XtHandle<Game::CutSceneLightDef>	m_LightingDefinition;
};

//=========================================================================

class C_StartDynamicObjectAnimationMsg : public Messaging::C_MessageDerived<C_StartDynamicObjectAnimationMsg, E_DynamicObjMsg_StartDynamicObjectAnimation>
{
public:
	C_StartDynamicObjectAnimationMsg()	{};
	~C_StartDynamicObjectAnimationMsg()	{};

	Game::E_ANIM_EVENT_TYPE				m_AnimationEvent;
};


class C_XtStartDynamicObjectAnimationMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtStartDynamicObjectAnimationMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_StartDynamicObjectAnimationMsg	m_Msg;

	Game::E_ANIM_EVENT_TYPE				m_AnimationEvent;
};

//=========================================================================


class C_DynamicObjectApplyImpulseMsg : public Messaging::C_MessageDerived<C_DynamicObjectApplyImpulseMsg, E_DynamicObjMsg_DynamicObjectApplyImpulse>
{
public:
	C_DynamicObjectApplyImpulseMsg() : m_Force(Maths::UNDEFINED)	{};
	~C_DynamicObjectApplyImpulseMsg()	{};

//	XtVec3								m_Force;
	Maths::HVec3						m_Force;
	F32									m_DamageScale;
	bool								m_ElectricalEffect;
	XtRgb								m_ElectricalEffectColour;
};


class C_XtDynamicObjectApplyImpulseMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtDynamicObjectApplyImpulseMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_DynamicObjectApplyImpulseMsg	m_Msg;

	XtVec3								m_Force;
	F32									m_DamageScale;
	bool								m_ElectricalEffect;
	XtRgb								m_ElectricalEffectColour;
};

//=========================================================================

class C_DynamicObjectDisableElectricalEffectMsg : public Messaging::C_MessageDerived<C_DynamicObjectDisableElectricalEffectMsg, E_DynamicObjMsg_DynamicObjectDisableElectricalEffect>
{
public:
	C_DynamicObjectDisableElectricalEffectMsg()	{};
	~C_DynamicObjectDisableElectricalEffectMsg()	{};
};


class C_XtDynamicObjectDisableElectricalEffectMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtDynamicObjectDisableElectricalEffectMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_DynamicObjectDisableElectricalEffectMsg	m_Msg;
};

//=========================================================================

class C_DynamicObjectSetPhysicsTypeMsg : public Messaging::C_MessageDerived<C_DynamicObjectSetPhysicsTypeMsg, E_DynamicObjMsg_DynamicObjectSetPhysicsType>
{
public:
	C_DynamicObjectSetPhysicsTypeMsg()	{};
	~C_DynamicObjectSetPhysicsTypeMsg()	{};

	Game::E_PHYSICS_BEHAVIOUR			m_PhysType;
};


class C_XtDynamicObjectSetPhysicsTypeMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtDynamicObjectSetPhysicsTypeMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_DynamicObjectSetPhysicsTypeMsg	m_Msg;

	Game::E_PHYSICS_BEHAVIOUR			m_PhysType;
};




//=========================================================================
//						CAMERA MESSAGES
//=========================================================================


class C_ChangeCameraBehaviourMsg : public Messaging::C_MessageDerived<C_ChangeCameraBehaviourMsg, E_DynamicObjMsg_ChangeCameraBehaviour>
{
public:
	C_ChangeCameraBehaviourMsg()		{};
	~C_ChangeCameraBehaviourMsg()		{};

	F32									m_BlendTime;
	Actors::E_BLEND_TYPE				m_Blending;

	XtHandle<Game::DyCamPropertyDef>	m_CameraProperties;
	XtHandle<Game::DyCamShakeDef>		m_CameraShake;

	XtHandle<Game::DyCamMovementDef>	m_MovementBehaviour;
	Actors::C_GameActorCreate*			m_ParentActor;
	XtVec3								m_ParentOffset;
	Btl::String32						m_ParentElementName;
	Actors::C_GameActorCreate*			m_LocalParentSpline;
	F32									m_MovementSpeed;

	XtHandle<Game::DyCamTargetDef>		m_TargetBehaviour;
	Actors::C_GameActorCreate*			m_TargetActor;
	XtVec3								m_TargetOffset;
	Btl::String32						m_TargetElementName;
	Actors::C_GameActorCreate*			m_LocalTargetSpline;
	F32									m_TrackingSpeed;

	F32									m_PitchAngle;
	F32									m_RollAngle;
	Btl::String32						m_FOVAttributeName;
	Btl::String32						m_DOFAttributeName;
	Btl::String32						m_ApertureAttributeName;
	Btl::String32						m_CutflagAttributeName;

	Btl::String32						m_DOFNearBlurBeginName;
	Btl::String32						m_DOFNearBlurEndName;
	Btl::String32						m_DOFFarBlurBeginName;
	Btl::String32						m_DOFFarBlurEndName;
	Btl::String32						m_DOFFarBlurScaleName;
};


class C_XtChangeCameraBehaviourMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtChangeCameraBehaviourMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_ChangeCameraBehaviourMsg			m_Msg;

	F32									m_BlendTime;
	Actors::E_BLEND_TYPE				m_Blending;

	XtHandle<Game::DyCamPropertyDef>	m_CameraProperties;
	XtHandle<Game::DyCamShakeDef>		m_CameraShake;

	XtHandle<Game::DyCamMovementDef>	m_MovementBehaviour;
	Actors::C_GameActorCreate*			m_ParentActor;
	XtVec3								m_ParentOffset;
	XtSz8								m_ParentElementName;
//	Actors::E_PREDEFINEDOFFSET			m_PreDefParentOffset;			// NEW
	Actors::C_GameActorCreate*			m_LocalParentSpline;
	F32									m_MovementSpeed;

	XtHandle<Game::DyCamTargetDef>		m_TargetBehaviour;
	Actors::C_GameActorCreate*			m_TargetActor;
	XtVec3								m_TargetOffset;
	XtSz8								m_TargetElementName;
//	Actors::E_PREDEFINEDOFFSET			m_PreDefTargetOffset;			// NEW
	Actors::C_GameActorCreate*			m_LocalTargetSpline;
	F32									m_TrackingSpeed;

	F32									m_PitchAngle;
	F32									m_RollAngle;
	XtSz8								m_FOVAttributeName;
	XtSz8								m_DOFAttributeName;
	XtSz8								m_ApertureAttributeName;
	XtSz8								m_CutflagAttributeName;

	XtSz8								m_DOFNearBlurBeginName;
	XtSz8								m_DOFNearBlurEndName;
	XtSz8								m_DOFFarBlurBeginName;
	XtSz8								m_DOFFarBlurEndName;
	XtSz8								m_DOFFarBlurScaleName;
};

//=========================================================================

class C_SimpleCameraBehaviourMsg : public Messaging::C_MessageDerived<C_SimpleCameraBehaviourMsg, E_DynamicObjMsg_SimpleCameraBehaviour>
{
public:
	C_SimpleCameraBehaviourMsg()		{};
	~C_SimpleCameraBehaviourMsg()		{};

	F32									m_BlendTime;
	Actors::E_BLEND_TYPE				m_Blending;
	XtHandle<Game::DyCamPropertyDef>	m_CameraProperties;
	XtHandle<Game::DyCamShakeDef>		m_CameraShake;
	Actors::C_GameActorCreate*			m_ParentActor;
	XtVec3								m_Position;
	XtVec3								m_PitchHeadingRole;
	F32									m_FOV;
};


class C_XtSimpleCameraBehaviourMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSimpleCameraBehaviourMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_SimpleCameraBehaviourMsg				m_Msg;

	F32									m_BlendTime;
	Actors::E_BLEND_TYPE				m_Blending;
	XtHandle<Game::DyCamPropertyDef>	m_CameraProperties;
	XtHandle<Game::DyCamShakeDef>		m_CameraShake;
	Actors::C_GameActorCreate*			m_ParentActor;
	XtVec3								m_Position;
	XtVec3								m_PitchHeadingRole;
	F32									m_FOV;
};

//=========================================================================

class C_FreezeCameraStateMsg : public Messaging::C_MessageDerived<C_FreezeCameraStateMsg, E_DynamicObjMsg_FreezeCameraState>
{
public:
	C_FreezeCameraStateMsg()		{};
	~C_FreezeCameraStateMsg()		{};
};


class C_XtFreezeCameraStateMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtFreezeCameraStateMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_FreezeCameraStateMsg				m_Msg;
};

//=========================================================================

class C_FadeDynamicCameraMsg : public Messaging::C_MessageDerived<C_FadeDynamicCameraMsg, E_DynamicObjMsg_FadeDynamicCamera>
{
public:
	C_FadeDynamicCameraMsg()		{};
	~C_FadeDynamicCameraMsg()		{};

	Actors::E_FADE_TYPE				m_FadeType;
	F32								m_TotalDuration;
};


class C_XtFadeDynamicCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtFadeDynamicCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_FadeDynamicCameraMsg				m_Msg;

	Actors::E_FADE_TYPE				m_FadeType;
	F32								m_TotalDuration;
};

//=========================================================================

class C_ResetDynamicCameraMsg : public Messaging::C_MessageDerived<C_ResetDynamicCameraMsg, E_DynamicObjMsg_ResetDynamicCamera>
{
public:
	C_ResetDynamicCameraMsg()		{};
	~C_ResetDynamicCameraMsg()		{};
};


class C_XtResetDynamicCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtResetDynamicCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_ResetDynamicCameraMsg					m_Msg;
};

//=========================================================================

class C_CameraCutMsg : public Messaging::C_MessageDerived<C_CameraCutMsg, E_DynamicObjMsg_CameraCut>
{
public:
	C_CameraCutMsg()		{};
	~C_CameraCutMsg()		{};
};


class C_XtCameraCutMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCameraCutMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CameraCutMsg					m_Msg;
};

//=========================================================================

class C_FinishedAnimatedCameraMsg : public Messaging::C_MessageDerived<C_FinishedAnimatedCameraMsg, E_DynamicObjMsg_FinishedAnimatedCamera>
{
public:
	C_FinishedAnimatedCameraMsg()		{};
	~C_FinishedAnimatedCameraMsg()		{};

	S32								m_Viewport;
	bool							m_FutureFeed;
};

class C_XtFinishedAnimatedCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtFinishedAnimatedCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_FinishedAnimatedCameraMsg					m_Msg;

	S32								m_Viewport;
	bool							m_FutureFeed;
};


//=========================================================================

class C_StartAnimatedCameraMsg : public Messaging::C_MessageDerived<C_StartAnimatedCameraMsg, E_DynamicObjMsg_StartAnimatedCamera>
{
public:
	C_StartAnimatedCameraMsg()		{};
	~C_StartAnimatedCameraMsg()		{};

	S32								m_Viewport;
	bool							m_FutureFeed;
	S32								m_VehicleIndex;
	Actors::E_ANIMCAMERASEQ_TYPE	m_Sequence;
	bool							m_StartPaused;
};

class C_XtStartAnimatedCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtStartAnimatedCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_StartAnimatedCameraMsg			m_Msg;

	S32								m_Viewport;
	bool							m_FutureFeed;
	S32								m_VehicleIndex;
	Actors::E_ANIMCAMERASEQ_TYPE	m_Sequence;
	bool							m_StartPaused;
};

//=========================================================================

class C_BlendAnimatedCameraMsg : public Messaging::C_MessageDerived<C_BlendAnimatedCameraMsg, E_DynamicObjMsg_BlendAnimatedCamera>
{
public:
	C_BlendAnimatedCameraMsg()		{};
	~C_BlendAnimatedCameraMsg()		{};

	S32								m_Viewport;
	bool							m_FutureFeed;
	S32								m_VehicleIndex;
	F32								m_Duration;
};


class C_XtBlendAnimatedCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtBlendAnimatedCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_BlendAnimatedCameraMsg			m_Msg;

	S32								m_Viewport;
	bool							m_FutureFeed;
	S32								m_VehicleIndex;
	F32								m_Duration;
};

//=========================================================================

class C_AutoSelectAnimatedCameraMsg : public Messaging::C_MessageDerived<C_AutoSelectAnimatedCameraMsg, E_DynamicObjMsg_AutoSelectAnimatedCamera>
{
public:
	C_AutoSelectAnimatedCameraMsg()		{};
	~C_AutoSelectAnimatedCameraMsg()		{};

	S32								m_Viewport;
	bool							m_FutureFeed;
	S32								m_VehicleIndex;
	XtHandle<GenericCameraOffsets>	m_Offsets;
};


class C_XtAutoSelectAnimatedCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtAutoSelectAnimatedCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_AutoSelectAnimatedCameraMsg			m_Msg;

	S32								m_Viewport;
	bool							m_FutureFeed;
	S32								m_VehicleIndex;
	XtHandle<GenericCameraOffsets>	m_Offsets;
};

//=========================================================================

class C_UICameraControlMsg : public Messaging::C_MessageDerived<C_UICameraControlMsg, E_DynamicObjMsg_UICameraControl>
{
public:
	C_UICameraControlMsg()		{};
	~C_UICameraControlMsg()		{};

	S32								m_Viewport;
	XtMat43							m_WorldMat;
	F32								m_BlendDuration;
	Actors::E_BLEND_TYPE			m_MovementType;
};


class C_XtUICameraControlMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtUICameraControlMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_UICameraControlMsg			m_Msg;

	S32								m_Viewport;
	XtMat43							m_WorldMat;
	F32								m_BlendDuration;
	Actors::E_BLEND_TYPE			m_MovementType;
};

//=========================================================================


class C_ChangeEnvironmentSettingsMsg : public Messaging::C_MessageDerived<C_ChangeEnvironmentSettingsMsg, E_DynamicObjMsg_ChangeEnvironmentSettings>
{
public:
	C_ChangeEnvironmentSettingsMsg()		{};
	~C_ChangeEnvironmentSettingsMsg()		{};

	XtHandle<Game::EnvironmentSettings>		m_NewSettings;
};


class C_XtChangeEnvironmentSettingsMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtChangeEnvironmentSettingsMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_ChangeEnvironmentSettingsMsg					m_Msg;

	XtHandle<Game::EnvironmentSettings>		m_NewSettings;
};



//=========================================================================
//					MESSAGES TO AI SYSTEM AND OTHER GAME LOGIC
//=========================================================================


class C_SetAIRouteMsg : public Messaging::C_MessageDerived<C_SetAIRouteMsg, E_DynamicObjMsg_SetAIRoute>
{
public:
	C_SetAIRouteMsg()		{};
	~C_SetAIRouteMsg()		{};

	Btl::String32			m_RouteName;
	bool					m_Active;
};


class C_XtSetAIRouteMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSetAIRouteMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	XtSz8							m_RouteName;
	bool							m_Active;

	C_SetAIRouteMsg					m_Msg;
};


//=========================================================================

class C_SetVehicleLogicMsg : public Messaging::C_MessageDerived<C_SetVehicleLogicMsg, E_DynamicObjMsg_SetVehicleLogic>
{
public:
	C_SetVehicleLogicMsg() : m_Pickups(false), m_Damage(false), m_HandOfGod(false), m_Drafting(false), m_Ghosting(false) {};
	~C_SetVehicleLogicMsg()		{};

	bool							m_Pickups;
	bool							m_Damage;
	bool							m_HandOfGod;
	bool							m_Drafting;
	bool							m_Ghosting;
};


class C_XtSetVehicleLogicMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSetVehicleLogicMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	bool							m_Pickups;
	bool							m_Damage;
	bool							m_HandOfGod;
	bool							m_Drafting;
	bool							m_Ghosting;

	C_SetVehicleLogicMsg			m_Msg;
};

//=========================================================================

class C_EnableLevelRouteMsg : public Messaging::C_MessageDerived<C_EnableLevelRouteMsg, E_DynamicObjMsg_EnableLevelRoute>
{
public:
	C_EnableLevelRouteMsg()			{};
	~C_EnableLevelRouteMsg()		{};

	S32								m_RouteNumber;
};


class C_XtEnableLevelRouteMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtEnableLevelRouteMsg()		{}
	const Messaging::C_MessageBase *GetMessage(const Actor::C_Actor *pActorContext);   
	C_EnableLevelRouteMsg			m_Msg;

	S32								m_RouteNumber;

};

//=========================================================================

class C_DisableLevelRouteMsg : public Messaging::C_MessageDerived<C_DisableLevelRouteMsg, E_DynamicObjMsg_DisableLevelRoute>
{
public:
	C_DisableLevelRouteMsg()		{};
	~C_DisableLevelRouteMsg()		{};

	S32								m_RouteNumber;
};


class C_XtDisableLevelRouteMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtDisableLevelRouteMsg()		{}
	const Messaging::C_MessageBase *GetMessage(const Actor::C_Actor *pActorContext);   
	C_DisableLevelRouteMsg			m_Msg;

	S32								m_RouteNumber;

};


//=========================================================================
//						SPLINE CONTROL MESSAGES
//=========================================================================


class C_SplineCtrl_StopAndWaitMsg : public Messaging::C_MessageDerived<C_SplineCtrl_StopAndWaitMsg, E_DynamicObjMsg_SplineCtrl_StopAndWait>
{
public:
	C_SplineCtrl_StopAndWaitMsg()		{};
	~C_SplineCtrl_StopAndWaitMsg()		{};

	F32									m_WaitTime;
};

class C_XtSplineCtrl_StopAndWaitMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSplineCtrl_StopAndWaitMsg()		{}
	const Messaging::C_MessageBase		*GetMessage(const Actor::C_Actor *pActorContext);   
	C_SplineCtrl_StopAndWaitMsg			m_Msg;

	F32									m_WaitTime;
};

//=========================================================================

class C_SplineCtrl_SetSpeedMsg : public Messaging::C_MessageDerived<C_SplineCtrl_SetSpeedMsg, E_DynamicObjMsg_SplineCtrl_SetSpeed>
{
public:
	C_SplineCtrl_SetSpeedMsg()			{};
	~C_SplineCtrl_SetSpeedMsg()			{};

	F32									m_Speed;
	F32									m_ReturnSpeed;
};

class C_XtSplineCtrl_SetSpeedMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSplineCtrl_SetSpeedMsg()		{}
	const Messaging::C_MessageBase		*GetMessage(const Actor::C_Actor *pActorContext);   
	C_SplineCtrl_SetSpeedMsg			m_Msg;

	F32									m_Speed;
	F32									m_ReturnSpeed;
};

//=========================================================================

class C_SplineCtrl_ReverseDirectionMsg : public Messaging::C_MessageDerived<C_SplineCtrl_ReverseDirectionMsg, E_DynamicObjMsg_SplineCtrl_ReverseDirection>
{
public:
	C_SplineCtrl_ReverseDirectionMsg()			{};
	~C_SplineCtrl_ReverseDirectionMsg()			{};
};


class C_XtSplineCtrl_ReverseDirectionMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSplineCtrl_ReverseDirectionMsg()		{}
	const Messaging::C_MessageBase				*GetMessage(const Actor::C_Actor *pActorContext);   
	C_SplineCtrl_ReverseDirectionMsg			m_Msg;
};


//=========================================================================

class C_SplineCtrl_SetBankAngleMsg : public Messaging::C_MessageDerived<C_SplineCtrl_SetBankAngleMsg, E_DynamicObjMsg_SplineCtrl_SetBankAngle>
{
public:
	C_SplineCtrl_SetBankAngleMsg()		{};
	~C_SplineCtrl_SetBankAngleMsg()		{};

	F32									m_Angle;
	F32									m_ReturnAngle;
};

class C_XtSplineCtrl_SetBankAngleMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSplineCtrl_SetBankAngleMsg()	{}
	const Messaging::C_MessageBase		*GetMessage(const Actor::C_Actor *pActorContext);   
	C_SplineCtrl_SetBankAngleMsg		m_Msg;

	F32									m_Angle;
	F32									m_ReturnAngle;
};




//=========================================================================
//						MESSAGES TO DEBUG OUTPUT
//=========================================================================


class C_OutputDebugMsg : public Messaging::C_MessageDerived<C_OutputDebugMsg, E_DynamicObjMsg_OutputDebug>
{
public:
	C_OutputDebugMsg()		{};
	~C_OutputDebugMsg()		{};

	Btl::String64			m_String;
};


class C_XtOutputDebugMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtOutputDebugMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	XtSz8							m_String;

	C_OutputDebugMsg					m_Msg;
};



//=========================================================================
//							TV CAMERA CONTROL
//=========================================================================


class C_EnableTVCamerasMsg : public Messaging::C_MessageDerived<C_EnableTVCamerasMsg, E_DynamicObjMsg_EnableTVCameras>
{
public:
	C_EnableTVCamerasMsg()		{};
	~C_EnableTVCamerasMsg()		{};
};


class C_XtEnableTVCamerasMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtEnableTVCamerasMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_EnableTVCamerasMsg					m_Msg;
};

//=========================================================================

class C_DisableTVCamerasMsg : public Messaging::C_MessageDerived<C_DisableTVCamerasMsg, E_DynamicObjMsg_DisableTVCameras>
{
public:
	C_DisableTVCamerasMsg()		{};
	~C_DisableTVCamerasMsg()		{};
};


class C_XtDisableTVCamerasMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtDisableTVCamerasMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_DisableTVCamerasMsg					m_Msg;
};





//=========================================================================
//						NON ANNOTATED ACTOR LOGIC MESSAGES
//=========================================================================


class C_ActorLogicMsg_AttachToPuller : public Messaging::C_MessageDerived<C_ActorLogicMsg_AttachToPuller, E_ActorLogicMsg_AttachToPuller>
{
public:
	C_ActorLogicMsg_AttachToPuller()	{};
	~C_ActorLogicMsg_AttachToPuller()	{};
	
	Actor::C_ActorHandle				m_handle;					// Handle to this components parent actor
	Maths::HVec3						m_worldpos;					// Where this object was created
};


//=============================================================

class C_ActorLogicMsg_PullerUpdate : public Messaging::C_MessageDerived<C_ActorLogicMsg_AttachToPuller, E_ActorLogicMsg_PullerUpdate>
{
public:
	C_ActorLogicMsg_PullerUpdate()		{};
	~C_ActorLogicMsg_PullerUpdate()		{};

	F32									m_splinedistance;
	Maths::HMat43						m_worldmat;					// Where this object was created
};



} //namespace ActorMsg


#endif		// DYNAMIC_OBJECT_MESSAGES_H

