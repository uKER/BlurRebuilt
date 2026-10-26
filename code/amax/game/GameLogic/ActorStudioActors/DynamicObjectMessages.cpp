#include "stdafx.h"


#include <actor_system/actor.h>
#include <shared_actors/XtSharedLevelActorParams.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <core\containers\algorithm.h>

//#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>


XT_TYPE(ActorMsg::C_XtVehicleEnteredTriggerMsg, "VehicleEnteredTrigger")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_TriggerType,	"Type")
	XT_FIELD(m_RefNumber,	"RefNumber")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleExitedTriggerMsg, "VehicleExitedTrigger")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_TriggerType,	"Type")
	XT_FIELD(m_RefNumber,	"RefNumber")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleActivatedTriggerMsg, "VehicleActivatedTrigger")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_TriggerType,	"Type")
	XT_FIELD(m_RefNumber,	"RefNumber")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleHandOfGodResetCameraForceArea, "VehicleHandOfGodCameraForce")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mCameraFollow,	"CameraFollowVehicle")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleHandOfGodResetCameraEnteredArea, "VehicleHandOfGodCameraEntered")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mCameraFollow,	"CameraFollowVehicle")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleHandOfGodResetCameraExitedArea, "VehicleHandOfGodCameraExited")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleHandOfGodForceResetArea, "VehicleHandOfGodResetForce")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mResetPositionActors,	"ResetPositionActors")
	XT_FIELD(mDelay,				"Delay")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleHandOfGodResetTwoWayArea, "VehicleHandOfGodTwoWay")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mResetPositionActor,	"ResetPositionActor")
	XT_FIELD(mTimer,				"Timer")
	XT_FIELD(mZoneId,				"HandofGodZoneID")
	XT_FIELD(mIndivId,				"IndividualId")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleHandOfGodResetEntryArea, "VehicleHandOfGodEntry")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mResetPositionActor,	"ResetPositionActor")
	XT_FIELD(mTimer,				"Timer")
	XT_FIELD(mZoneId,				"HandofGodZoneID")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleHandOfGodResetExitArea, "VehicleHandOfGodExit")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mZoneId,				"HandofGodZoneID")
XT_END_TYPE


XT_TYPE(ActorMsg::C_XtVehicleSloMoEntryMsg, "VehicleSloMoEntry")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Disable,					"Disable")
	XT_FIELD(m_TimeScale,				"TimeScale")
	XT_FIELD(m_Duration,				"Duration")
	XT_FIELD(m_TrackCar,				"TrackCar")
	XT_FIELD(m_MinSpeed,				"MinSpeed")
	XT_FIELD(m_MaxSpeed,				"MaxSpeed")
	XT_FIELD(m_Nitro,					"Nitro")
	XT_FIELD(m_CameraPosition,			"CameraPosition")
	XT_FIELD(m_CameraLookAt,			"CameraLookAt")
	XT_FIELD(m_CameraFOV,				"CameraFOV")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleSloMoExitMsg, "VehicleSloMoExit")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Disable,					"Disable")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleAchieveAmboyOilfieldsMsg, "VehicleAmboyOilfields")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtVehicleAchieveGasStationMsg, "VehicleGasStation")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtChangeSpecialEffectMsg, "ChangeSpecialEffect")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_SpecialEffect,					"SpecialEffect")
	XT_FIELD(m_T1LoopStart,						"T1LoopStart")
	XT_FIELD(m_T2LoopEnd,						"T2LoopEnd")
	XT_FIELD(m_StartTime,						"StartTime")
	XT_FIELD(m_LoopType,						"LoopType")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSetAlphaValueMsg,			"SetAlphaValue")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_alpha,							"alpha")
XT_END_TYPE


//XT_TYPE(ActorMsg::C_XtSelectLightingRigMsg, "SelectBacklightActor")
XT_TYPE(ActorMsg::C_XtSelectLightingRigMsg, "SelectLightingRig")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
//	XT_FIELD(m_LightingRigActor,				"LightingRigActor")
	XT_FIELD(m_LightingDefinition,				"LightingDefinition")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtStartDynamicObjectAnimationMsg, "StartDynamicObjectAnimation")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_AnimationEvent,					"AnimationEvent")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDynamicObjectApplyImpulseMsg, "DynamicObjectApplyImpulse")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Force,							"Force")
	XT_FIELD(m_DamageScale,						"DamageScale")
	XT_FIELD(m_ElectricalEffect,				"ElectricalEffect")
	XT_FIELD(m_ElectricalEffectColour,			"ElectricalEffectColour")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDynamicObjectDisableElectricalEffectMsg, "DynamicObjectDisableElectricalEffect")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDynamicObjectSetPhysicsTypeMsg, "DynamicObjectSetPhysicsType")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_PhysType,						"PhysType")
XT_END_TYPE


XT_TYPE(ActorMsg::C_XtChangeCameraBehaviourMsg, "ChangeCameraBehaviour")
	XT_BASE(ActorMsg::C_XtActorMessageBase)

	XT_FIELD(m_BlendTime,						"BlendTime")
	XT_FIELD(m_Blending,						"Blending")

	XT_FIELD(m_CameraProperties,				"CameraProperties")
	XT_FIELD(m_CameraShake,						"CameraShake")

	XT_FIELD(m_MovementBehaviour,				"MovementBehaviour")
	XT_FIELD(m_ParentActor,						"ParentActor")
	XT_FIELD(m_ParentOffset,					"ParentOffset")
	XT_FIELD(m_ParentElementName,				"ParentElementName")
//	XT_FIELD(m_PreDefParentOffset,				"PreDefParentOffset")
	XT_FIELD(m_LocalParentSpline,				"LocalParentSpline")
	XT_FIELD(m_MovementSpeed,					"MovementSpeed")

	XT_FIELD(m_TargetBehaviour,					"TargetBehaviour")
	XT_FIELD(m_TargetActor,						"TargetActor")
	XT_FIELD(m_TargetOffset,					"TargetOffset")
	XT_FIELD(m_TargetElementName,				"TargetElementName")
//	XT_FIELD(m_PreDefTargetOffset,				"PreDefTargetOffset")
	XT_FIELD(m_LocalTargetSpline,				"LocalTargetSpline")
	XT_FIELD(m_TrackingSpeed,					"TrackingSpeed")

	XT_FIELD(m_PitchAngle,						"PitchAngle")
	XT_FIELD(m_RollAngle,						"RollAngle")
	XT_FIELD(m_FOVAttributeName,				"FOVAttributeName")
	XT_FIELD(m_DOFAttributeName,				"DOFAttributeName")
	XT_FIELD(m_ApertureAttributeName,			"ApertureAttributeName")
	XT_FIELD(m_CutflagAttributeName,			"CutflagAttributeName")

	XT_FIELD(m_DOFNearBlurBeginName,			"DOFNearBlurBeginName")
	XT_FIELD(m_DOFNearBlurEndName,				"DOFNearBlurEndName")
	XT_FIELD(m_DOFFarBlurBeginName,				"DOFFarBlurBeginName")
	XT_FIELD(m_DOFFarBlurEndName,				"DOFFarBlurEndName")
	XT_FIELD(m_DOFFarBlurScaleName,				"DOFFarBlurScaleName")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSimpleCameraBehaviourMsg, "SimpleCameraBehaviour")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_BlendTime,						"BlendTime")
	XT_FIELD(m_Blending,						"Blending")
	XT_FIELD(m_CameraProperties,				"CameraProperties")
	XT_FIELD(m_CameraShake,						"CameraShake")
	XT_FIELD(m_ParentActor,						"ParentActor")
	XT_FIELD(m_Position,						"Position")
	XT_FIELD(m_PitchHeadingRole,				"PitchHeadingRole")
	XT_FIELD(m_FOV,								"FOV")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtFreezeCameraStateMsg, "FreezeCameraState")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE


XT_TYPE(ActorMsg::C_XtFadeDynamicCameraMsg, "FadeDynamicCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_FadeType, "Fade")
	XT_FIELD(m_TotalDuration, "TotalDuration")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtResetDynamicCameraMsg, "ResetDynamicCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCameraCutMsg, "CameraCut")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtFinishedAnimatedCameraMsg, "FinishedAnimatedCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Viewport,		"Viewport")
	XT_FIELD(m_FutureFeed,		"FutureFeed")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtStartAnimatedCameraMsg, "StartAnimatedCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Viewport,		"Viewport")
	XT_FIELD(m_FutureFeed,		"FutureFeed")
	XT_FIELD(m_VehicleIndex,	"VehicleIndex")
	XT_FIELD(m_Sequence,		"Sequence")
	XT_FIELD(m_StartPaused,		"StartPaused")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtBlendAnimatedCameraMsg, "BlendAnimatedCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Viewport,		"Viewport")
	XT_FIELD(m_FutureFeed,		"FutureFeed")
	XT_FIELD(m_VehicleIndex,	"VehicleIndex")
	XT_FIELD(m_Duration,		"Duration")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtAutoSelectAnimatedCameraMsg, "AutoSelectAnimatedCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Viewport,		"Viewport")
	XT_FIELD(m_FutureFeed,		"FutureFeed")
	XT_FIELD(m_VehicleIndex,	"VehicleIndex")
	XT_FIELD(m_Offsets,			"Offsets")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtUICameraControlMsg, "UICameraControl")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Viewport,			"Viewport")
	XT_FIELD(m_WorldMat,			"WorldMat")
//	XT_FIELD(m_FOV,					"FOV")
	XT_FIELD(m_BlendDuration,		"BlendDuration")
	XT_FIELD(m_MovementType,		"MovementType")
XT_END_TYPE


XT_TYPE(ActorMsg::C_XtChangeEnvironmentSettingsMsg, "ChangeEnvironmentSettings")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_NewSettings,		"NewSettings")
XT_END_TYPE



XT_TYPE(ActorMsg::C_XtSetAIRouteMsg, "SetAIRouteMessage")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_RouteName, "RouteName")
	XT_FIELD(m_Active, "Active")
XT_END_TYPE


XT_TYPE(ActorMsg::C_XtSetVehicleLogicMsg, "SetVehicleLogicMessage")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Pickups,		"Pickups")
	XT_FIELD(m_Damage,		"Damage")
	XT_FIELD(m_HandOfGod,	"HandOfGod")
	XT_FIELD(m_Drafting,	"Drafting")
	XT_FIELD(m_Ghosting,	"Ghosting")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtEnableLevelRouteMsg, "EnableLevelRouteMessage")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_RouteNumber,	"RouteNumber")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDisableLevelRouteMsg, "DisableLevelRouteMessage")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_RouteNumber,	"RouteNumber")
XT_END_TYPE



XT_TYPE(ActorMsg::C_XtSplineCtrl_StopAndWaitMsg, "SplineCtrl_StopAndWait_Message")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_WaitTime, "WaitTime")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSplineCtrl_SetSpeedMsg, "SplineCtrl_SetSpeed_Message")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Speed, "Speed")
	XT_FIELD(m_ReturnSpeed, "ReturnSpeed")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSplineCtrl_ReverseDirectionMsg, "SplineCtrl_ReverseDirection_Message")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSplineCtrl_SetBankAngleMsg, "SplineCtrl_SetBankAngle_Message")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Angle, "Angle")
	XT_FIELD(m_ReturnAngle, "ReturnAngle")
XT_END_TYPE




XT_TYPE(ActorMsg::C_XtOutputDebugMsg, "OutputDebugMessage")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_String, "String")
XT_END_TYPE


XT_TYPE(ActorMsg::C_XtEnableTVCamerasMsg, "EnableTVCameras")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDisableTVCamerasMsg, "DisableTVCameras")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE




// Register Messages
XT_LIBRARY(DynamicObjectMessages)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleEnteredTriggerMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleExitedTriggerMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleActivatedTriggerMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleHandOfGodResetCameraForceArea)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleHandOfGodResetCameraEnteredArea)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleHandOfGodResetCameraExitedArea)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleHandOfGodForceResetArea)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleHandOfGodResetEntryArea)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleHandOfGodResetExitArea)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleHandOfGodResetTwoWayArea)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleSloMoEntryMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleSloMoExitMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleAchieveAmboyOilfieldsMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtVehicleAchieveGasStationMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtChangeSpecialEffectMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtSetAlphaValueMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtSelectLightingRigMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtStartDynamicObjectAnimationMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtDynamicObjectApplyImpulseMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtDynamicObjectDisableElectricalEffectMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtDynamicObjectSetPhysicsTypeMsg)

XT_REGISTER_TYPE(ActorMsg::C_XtChangeCameraBehaviourMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtSimpleCameraBehaviourMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtFreezeCameraStateMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtFadeDynamicCameraMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtResetDynamicCameraMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtCameraCutMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtStartAnimatedCameraMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtBlendAnimatedCameraMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtAutoSelectAnimatedCameraMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtUICameraControlMsg)

XT_REGISTER_TYPE(ActorMsg::C_XtChangeEnvironmentSettingsMsg)

XT_REGISTER_TYPE(ActorMsg::C_XtSetAIRouteMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtSetVehicleLogicMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtEnableLevelRouteMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtDisableLevelRouteMsg)

XT_REGISTER_TYPE(ActorMsg::C_XtSplineCtrl_StopAndWaitMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtSplineCtrl_SetSpeedMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtSplineCtrl_ReverseDirectionMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtSplineCtrl_SetBankAngleMsg)

XT_REGISTER_TYPE(ActorMsg::C_XtOutputDebugMsg)

XT_REGISTER_TYPE(ActorMsg::C_XtEnableTVCamerasMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtDisableTVCamerasMsg)

XT_END_LIBRARY






//=========================================================================
//						THE MESSAGES
//=========================================================================


namespace ActorMsg
{

const Messaging::C_MessageBase * C_XtVehicleEnteredTriggerMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_TriggerType = m_TriggerType;
	m_Msg.m_RefNumber = m_RefNumber;
	
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleExitedTriggerMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_TriggerType = m_TriggerType;
	m_Msg.m_RefNumber = m_RefNumber;
	
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleActivatedTriggerMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_TriggerType = m_TriggerType;
	m_Msg.m_RefNumber = m_RefNumber;

	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleHandOfGodResetCameraForceArea::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mCameraFollow	= mCameraFollow;

	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtVehicleHandOfGodResetCameraEnteredArea::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mCameraFollow	= mCameraFollow;

	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtVehicleHandOfGodResetCameraExitedArea::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleHandOfGodForceResetArea::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mResetPositions=&mResetPositionActors;
	m_Msg.mDelay=mDelay;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleHandOfGodResetTwoWayArea::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mResetPositionActor	= mResetPositionActor;
	m_Msg.mTimer				= mTimer;
	m_Msg.mZoneId				= mZoneId;
	m_Msg.mIndivId				= mIndivId;

	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleHandOfGodResetEntryArea::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mResetPositionActor	= mResetPositionActor;
	m_Msg.mTimer				= mTimer;
	m_Msg.mZoneId				= mZoneId;
	
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleHandOfGodResetExitArea::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mZoneId				= mZoneId;

	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleSloMoEntryMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Disable			= m_Disable;
	m_Msg.m_TimeScale		= m_TimeScale;
	m_Msg.m_Duration		= m_Duration;
	m_Msg.m_TrackCar		= m_TrackCar;
	m_Msg.m_MinSpeed		= m_MinSpeed;
	m_Msg.m_MaxSpeed		= m_MaxSpeed;
	m_Msg.m_Nitro			= m_Nitro;
	m_Msg.m_CameraPosition	= Load(m_CameraPosition);
	m_Msg.m_CameraLookAt	= Load(m_CameraLookAt);
	m_Msg.m_CameraFOV		= m_CameraFOV;

	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleSloMoExitMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Disable			= m_Disable;
	
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleAchieveAmboyOilfieldsMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtVehicleAchieveGasStationMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtChangeSpecialEffectMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_SpecialEffect	= m_SpecialEffect;
	m_Msg.m_T1LoopStart		= m_T1LoopStart;
	m_Msg.m_T2LoopEnd		= m_T2LoopEnd;
	m_Msg.m_StartTime		= m_StartTime;
	m_Msg.m_LoopType		= m_LoopType;

	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtSetAlphaValueMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_alpha			= m_alpha;

	return &m_Msg;
}


//=========================================================================

const Messaging::C_MessageBase * C_XtSelectLightingRigMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
//	m_Msg.m_LightingRigActor	= m_LightingRigActor;
	m_Msg.m_LightingDefinition	= m_LightingDefinition;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtStartDynamicObjectAnimationMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_AnimationEvent	= m_AnimationEvent;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtDynamicObjectApplyImpulseMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Force					= Load(m_Force);//(m_Force.vx, m_Force.vy, m_Force.vz);
	m_Msg.m_DamageScale				= m_DamageScale;
	m_Msg.m_ElectricalEffect		= m_ElectricalEffect;
	m_Msg.m_ElectricalEffectColour	= m_ElectricalEffectColour;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtDynamicObjectDisableElectricalEffectMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtDynamicObjectSetPhysicsTypeMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_PhysType				= m_PhysType;
	return &m_Msg;
}


//=========================================================================

const Messaging::C_MessageBase * C_XtChangeCameraBehaviourMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_BlendTime				= m_BlendTime;
	m_Msg.m_Blending				= m_Blending;

	m_Msg.m_CameraProperties		= m_CameraProperties;
	m_Msg.m_CameraShake				= m_CameraShake;

	m_Msg.m_MovementBehaviour		= m_MovementBehaviour;
	m_Msg.m_ParentActor				= m_ParentActor;
	m_Msg.m_ParentOffset			= m_ParentOffset;
	m_Msg.m_ParentElementName		= m_ParentElementName.Text();
//	m_Msg.m_PreDefParentOffset		= m_PreDefParentOffset;
	m_Msg.m_LocalParentSpline		= m_LocalParentSpline;
	m_Msg.m_MovementSpeed			= m_MovementSpeed;

	m_Msg.m_TargetBehaviour			= m_TargetBehaviour;
	m_Msg.m_TargetActor				= m_TargetActor;
	m_Msg.m_TargetOffset			= m_TargetOffset;
	m_Msg.m_TargetElementName		= m_TargetElementName.Text();
//	m_Msg.m_PreDefTargetOffset		= m_PreDefTargetOffset;
	m_Msg.m_LocalTargetSpline		= m_LocalTargetSpline;
	m_Msg.m_TrackingSpeed			= m_TrackingSpeed;

	m_Msg.m_PitchAngle				= m_PitchAngle;
	m_Msg.m_RollAngle				= m_RollAngle;
	m_Msg.m_FOVAttributeName		= m_FOVAttributeName.Text();
	m_Msg.m_DOFAttributeName		= m_DOFAttributeName.Text();
	m_Msg.m_ApertureAttributeName	= m_ApertureAttributeName.Text();
	m_Msg.m_CutflagAttributeName	= m_CutflagAttributeName.Text();

	m_Msg.m_DOFNearBlurBeginName	= m_DOFNearBlurBeginName.Text();
	m_Msg.m_DOFNearBlurEndName		= m_DOFNearBlurEndName.Text();
	m_Msg.m_DOFFarBlurBeginName		= m_DOFFarBlurBeginName.Text();
	m_Msg.m_DOFFarBlurEndName		= m_DOFFarBlurEndName.Text();
	m_Msg.m_DOFFarBlurScaleName		= m_DOFFarBlurScaleName.Text();

	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtSimpleCameraBehaviourMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_BlendTime				= m_BlendTime;
	m_Msg.m_Blending				= m_Blending;
	m_Msg.m_CameraProperties		= m_CameraProperties;
	m_Msg.m_CameraShake				= m_CameraShake;
	m_Msg.m_ParentActor				= m_ParentActor;
	m_Msg.m_Position				= m_Position;
	m_Msg.m_PitchHeadingRole		= m_PitchHeadingRole;
	m_Msg.m_FOV						= m_FOV;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtFreezeCameraStateMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


//=========================================================================

const Messaging::C_MessageBase * C_XtFadeDynamicCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_FadeType = m_FadeType;
	m_Msg.m_TotalDuration = m_TotalDuration;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtResetDynamicCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtCameraCutMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtStartAnimatedCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Viewport = m_Viewport;
	m_Msg.m_FutureFeed = m_FutureFeed;
	m_Msg.m_VehicleIndex = m_VehicleIndex;
	m_Msg.m_Sequence = m_Sequence;
	m_Msg.m_StartPaused = m_StartPaused;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtFinishedAnimatedCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Viewport = m_Viewport;
	m_Msg.m_FutureFeed = m_FutureFeed;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtBlendAnimatedCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Viewport = m_Viewport;
	m_Msg.m_FutureFeed = m_FutureFeed;
	m_Msg.m_VehicleIndex = m_VehicleIndex;
	m_Msg.m_Duration = m_Duration;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtAutoSelectAnimatedCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Viewport = m_Viewport;
	m_Msg.m_FutureFeed = m_FutureFeed;
	m_Msg.m_VehicleIndex = m_VehicleIndex;
	m_Msg.m_Offsets = m_Offsets;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtUICameraControlMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Viewport		= m_Viewport;
	m_Msg.m_WorldMat		= m_WorldMat;
	m_Msg.m_BlendDuration	= m_BlendDuration;
	m_Msg.m_MovementType	= m_MovementType;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtChangeEnvironmentSettingsMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_NewSettings = m_NewSettings;
	return &m_Msg;
}


//=========================================================================

const Messaging::C_MessageBase * C_XtSetAIRouteMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_RouteName = m_RouteName.Text();
	m_Msg.m_Active = m_Active;
	
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtSetVehicleLogicMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Pickups = m_Pickups;
	m_Msg.m_Damage = m_Damage;
	m_Msg.m_HandOfGod = m_HandOfGod;
	m_Msg.m_Drafting = m_Drafting;
	m_Msg.m_Ghosting = m_Ghosting;
	
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtEnableLevelRouteMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_RouteNumber = m_RouteNumber;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtDisableLevelRouteMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_RouteNumber = m_RouteNumber;
	return &m_Msg;
}


//=========================================================================

const Messaging::C_MessageBase * C_XtSplineCtrl_StopAndWaitMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_WaitTime = m_WaitTime;
	
	return &m_Msg;
}
//=========================================================================

const Messaging::C_MessageBase * C_XtSplineCtrl_SetSpeedMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Speed = m_Speed;
	m_Msg.m_ReturnSpeed = m_ReturnSpeed;
	
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtSplineCtrl_ReverseDirectionMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtSplineCtrl_SetBankAngleMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Angle = m_Angle;
	m_Msg.m_ReturnAngle = m_ReturnAngle;
	
	return &m_Msg;
}
//=========================================================================

const Messaging::C_MessageBase * C_XtOutputDebugMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_String = m_String.Text();
	
	return &m_Msg;
}


//=========================================================================

const Messaging::C_MessageBase * C_XtEnableTVCamerasMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtDisableTVCamerasMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


} //namespace ActorMsg




