#include "stdafx.h"


#include <actor_system/actor.h>
#include <shared_actors/XtSharedLevelActorParams.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <Shared_Actors/ActorImpl/SplineActor.h>
#include <core\containers\algorithm.h>

//#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <GameLogic/ActorStudioActors/CutSceneMessages.h>

XT_TYPE(ActorMsg::C_XtCutScene_Start_CutsceneMsg,				"CutScene_Start_Cutscene")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start_ScriptedCameraMsg,			"CutScene_Start_ScriptedCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start_RestartScriptedCameraMsg,	"CutScene_Start_RestartScriptedCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start_PassedCutsceneMsg,			"CutScene_Start_PassedCutscene")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start_PassedScriptedCameraMsg,	"CutScene_Start_PassedScriptedCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start_FailedScriptedCameraMsg,	"CutScene_Start_FailedScriptedCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_FinishedMsg,						"CutScene_Finished")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtStartFatalDamageCamMsg,					"StartFatalDamageCam")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start_GarageCameraMsg,			"CutScene_Start_GarageCamera")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start_GarageCameraReverseMsg,	"CutScene_Start_GarageCameraReverse")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

//=========================================================================


XT_TYPE(ActorMsg::E_CutSceneDriverAnim,"DriverAnim")
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_PunchAir, "PunchAir")
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_PumpFist, "PumpFist")		
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_ShakeFist, "ShakeFist")			
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_LookBehindLeft, "LookBehindLeft")		
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_LookBehindRight, "LookBehindRight")		
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_HitSteeringWheelOneHand1,	"HitSteeringWheelOneHand1")
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_HitSteeringWheelOneHand2,	"HitSteeringWheelOneHand2")
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_HitSteeringWheelTwoHands,	"HitSteeringWheelTwoHands")
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_ShakeHead,	"ShakeHead")				
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_JoltSmall,	"JoltSmall")
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_JoltLarge, "JoltLarge")
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_GlanceLeft, "GlanceLeft")		
	XT_OPTION(ActorMsg::E_CutSceneDriverAnim_GlanceRight, "GlanceRight")	
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCarModelModifier,"CarModelModifier")
	XT_FIELD(mDoIt,"DoIt")
	XT_FIELD(mValue,"Value")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCarModelBehaviour,"CarModelBehaviour")
	XT_FIELD(mRevs,"Revs")
	XT_FIELD(mThrottle,"Throttle")
	XT_FIELD(mBrake,"Brake")
	XT_FIELD(mSteering,"Steering")

	XT_FIELD(mHandbrake,"Handbrake")
	XT_FIELD(mHeadlights,"Headlights")
	XT_FIELD(mHeadlightCones,"HeadlightCones")
	XT_FIELD(mRearlights,"Rearlights")
	XT_FIELD(mFoglights,"Foglights")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtInitialiseProxyMsg, "CutScene_InitialiseProxy")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtEnableDisableProxyMsg, "CutScene_EnableDisableProxy")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mEnable,"Enable")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSetAIOnSplineMsg, "CutScene_SetAIOnSpline")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mSplineToFollow, "SplineToFollow")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSetAISpeedMsg, "CutScene_SetAISpeed")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Speed, "Speed")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSetAIRelativeSpeedMsg, "CutScene_SetAIRelativeSpeed")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_RelativeActor, "RelativeActor")
	XT_FIELD(m_Aggression, "Aggression")
XT_END_TYPE


XT_TYPE(ActorMsg::C_XtCSApplyImpulseMsg, "CutScene_ApplyImpulse")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_ApplyLinearSpeed, "ApplyLinearSpeed")
	XT_FIELD(m_LinearSpeed, "LinearSpeed")
	XT_FIELD(m_ApplyAngularSpeed, "ApplyAngularSpeed")
	XT_FIELD(m_AngularSpeed, "AngularSpeed")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCSRepairAllDamageMsg, "CutScene_RepairAllDamage")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE



XT_TYPE(ActorMsg::C_XtStartTheRaceMsg, "CutScene_StartTheRace")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtRaceEndedMsg, "CutScene_RaceEnded")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtStartTheIntroMsg, "CutScene_StartTheIntro")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtPlaceCarOnPointMsg, "CutScene_PlaceCarOnPoint")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mUseStartPosition, "UseStartPosition")
	XT_FIELD(mPoint, "Point")
	XT_FIELD(m_Speed, "Speed")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSetDriverAnimMsg, "CutScene_SetDriverAnim")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mPlayDriverAnim, "PlayDriverAnim")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSetCarBehaviourMsg, "CutScene_SetCarBehaviour")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mDoCarModelBehaviour, "DoCarModelBehaviour")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtPlayConversationAndSubtitleMsg, "CutScene_PlayConversationAndSubtitle")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mDialogue, "Dialogue")
	XT_FIELD(mSpokenLine, "SpokenLine")
XT_END_TYPE


//=========================================================================

XT_TYPE(ActorMsg::C_XtReplay_RecordMsg, "Replay_Record")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtReplay_LoadMsg, "Replay_Load")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mReplayFile, "ReplayFile")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtReplay_PlayMsg, "Replay_Play")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mStartTime, "StartTime")
//	XT_FIELD(mEndTime, "EndTime")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtReplay_StopMsg, "Replay_Stop")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtReplay_PauseMsg, "Replay_Pause")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtReplay_ResumeMsg, "Replay_Resume")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDamage_EnableDisableMsg, "Damage_EnableDisable")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mRecieveDamage, "RecieveDamage")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDamage_ApplyMsg, "Damage_Apply")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mDamageState, "DamageState")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDamage_RepairMsg, "Damage_Repair")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_DriveRelativeMsg, "CutScene_DriveRelative")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mParentActor, "ParentActor")
	XT_FIELD(mParentOffset, "ParentOffset")
	XT_FIELD(mAggression, "Aggression")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_DisableAIMsg, "CutScene_DisableAI")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetVehicleControlMsg, "CutScene_SetVehicleControl")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mDirectControl, "DirectControl")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetVehicleHandbrakeMsg, "CutScene_SetVehicleHandbrake")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mApplyHandbrake, "ApplyHandbrake")
XT_END_TYPE

//XT_TYPE(ActorMsg::C_XtCutScene_SetVehicleSpeedMsg, "CutScene_SetVehicleSpeed")
//	XT_BASE(ActorMsg::C_XtActorMessageBase)
//	XT_FIELD(mSpeed, "Speed")
//XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetVehicleRevsMsg, "CutScene_SetVehicleRevs")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mRevs, "Revs")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetVehicleBrakeMsg, "CutScene_SetVehicleBrake")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mBrake, "Brake")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetVehicleSteeringMsg, "CutScene_SetVehicleSteering")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mSteer, "Steer")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetDoorPositionMsg, "CutScene_SetDoorPosition")
	XT_BASE(ActorMsg::C_XtActorMessageBase)

	XT_FIELD(mLeftPosition, "LeftPosition")
	XT_FIELD(mLeftPositionAnimTime, "LeftPositionAnimTime")

	XT_FIELD(mRightPosition, "RightPosition")
	XT_FIELD(mRightPositionAnimTime, "RightPositionAnimTime")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetWindowPositionMsg, "CutScene_SetWindowPosition")
	XT_BASE(ActorMsg::C_XtActorMessageBase)

	XT_FIELD(mLeftPosition, "LeftPosition")
	XT_FIELD(mLeftPositionAnimTime, "LeftPositionAnimTime")

	XT_FIELD(mRightPosition, "RightPosition")
	XT_FIELD(mRightPositionAnimTime, "RightPositionAnimTime")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetBonnetPositionMsg, "CutScene_SetBonnetPosition")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mBonnetPosition, "BonnetPosition")
	XT_FIELD(mBonnetPositionAnimTime, "BonnetPositionAnimTime")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetLightStateMsg, "CutScene_SetLightState")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mLeftIndicatorOn, "LeftIndicatorOn")
	XT_FIELD(mRightIndicatorOn, "RightIndicatorOn")
	XT_FIELD(mHeadlightsOn, "HeadlightsOn")
	XT_FIELD(mBrakeLightsOn, "BrakeLightsOn")
	XT_FIELD(mReverseLightsOn, "ReverseLightsOn")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_DisplayVehicleMsg, "CutScene_DisplayVehicle")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mDisplayVehicle, "DisplayVehicle")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_DisplayDriverMsg, "CutScene_DisplayDriver")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mDisplayDriver, "DisplayDriver")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_CreateVehicleEffectsMsg, "CutScene_CreateVehicleEffects")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_LoadResourcesMsg, "CutScene_LoadResources")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_DumpResourcesMsg, "CutScene_DumpResources")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SwapVehicleModelMsg, "CutScene_SwapVehicleModel")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start321Msg, "CutScene_Start321")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start321_2_Player_Msg, "CutScene_Start321_2_Player")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start321_3_Player_Msg, "CutScene_Start321_3_Player")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_Start321_4_Player_Msg, "CutScene_Start321_4_Player")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_WreckCamMsg, "CutScene_WreckCam")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_EnableRaceAIMsg, "CutScene_EnableRaceAI")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_DisableRaceAIMsg, "CutScene_DisableRaceAI")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SetVehiclesToRaceSpeedMsg, "CutScene_SetVehiclesToRaceSpeed")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mPercentageRaceSpeed, "PercentageRaceSpeed")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_DialogueMsg, "CutScene_Dialogue")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(mSoundBank, "SoundBank")
	XT_FIELD(mSoundIndex, "SoundIndex")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_DialogueFinishedMsg, "CutScene_DialogueFinished")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_SkippedMsg, "CutScene_Skipped")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCutScene_DisplayCreditMsg, "CutScene_DisplayCredit")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE


//=========================================================================



// Register Messages
XT_LIBRARY(CutSceneMessages)

	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start_CutsceneMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start_ScriptedCameraMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start_RestartScriptedCameraMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start_PassedCutsceneMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start_PassedScriptedCameraMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start_FailedScriptedCameraMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_FinishedMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtStartFatalDamageCamMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start_GarageCameraMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start_GarageCameraReverseMsg)

	XT_REGISTER_TYPE(ActorMsg::E_CutSceneDriverAnim)

	XT_REGISTER_TYPE(ActorMsg::C_XtCarModelModifier)
	XT_REGISTER_TYPE(ActorMsg::C_XtCarModelBehaviour)

	XT_REGISTER_TYPE(ActorMsg::C_XtInitialiseProxyMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtEnableDisableProxyMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtSetAIOnSplineMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtSetAISpeedMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtSetAIRelativeSpeedMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCSApplyImpulseMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCSRepairAllDamageMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtStartTheRaceMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtRaceEndedMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtStartTheIntroMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtPlaceCarOnPointMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtSetDriverAnimMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtSetCarBehaviourMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtPlayConversationAndSubtitleMsg)

	XT_REGISTER_TYPE(ActorMsg::C_XtReplay_RecordMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtReplay_LoadMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtReplay_PlayMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtReplay_StopMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtReplay_PauseMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtReplay_ResumeMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtDamage_EnableDisableMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtDamage_ApplyMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtDamage_RepairMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_DriveRelativeMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_DisableAIMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetVehicleControlMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetVehicleHandbrakeMsg)
//	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetVehicleSpeedMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetVehicleRevsMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetVehicleBrakeMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetVehicleSteeringMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetDoorPositionMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetWindowPositionMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetBonnetPositionMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetLightStateMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_DisplayVehicleMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_DisplayDriverMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_CreateVehicleEffectsMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_LoadResourcesMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_DumpResourcesMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SwapVehicleModelMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start321Msg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start321_2_Player_Msg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start321_3_Player_Msg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_Start321_4_Player_Msg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_WreckCamMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_EnableRaceAIMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_DisableRaceAIMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SetVehiclesToRaceSpeedMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_DialogueMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_DialogueFinishedMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_SkippedMsg)
	XT_REGISTER_TYPE(ActorMsg::C_XtCutScene_DisplayCreditMsg)
XT_END_LIBRARY


//=========================================================================
//						THE MESSAGES
//=========================================================================


namespace ActorMsg
{

const Messaging::C_MessageBase * C_XtCutScene_Start_CutsceneMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}
const Messaging::C_MessageBase * C_XtCutScene_Start_ScriptedCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}
const Messaging::C_MessageBase * C_XtCutScene_Start_RestartScriptedCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}
const Messaging::C_MessageBase * C_XtCutScene_Start_PassedCutsceneMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}
const Messaging::C_MessageBase * C_XtCutScene_Start_PassedScriptedCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}
const Messaging::C_MessageBase * C_XtCutScene_Start_FailedScriptedCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}
const Messaging::C_MessageBase * C_XtCutScene_FinishedMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtStartFatalDamageCamMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}
const Messaging::C_MessageBase * C_XtCutScene_Start_GarageCameraMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}
const Messaging::C_MessageBase * C_XtCutScene_Start_GarageCameraReverseMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

//=================================================================

const Messaging::C_MessageBase * C_XtInitialiseProxyMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtEnableDisableProxyMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mEnable = mEnable;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtSetAIOnSplineMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mSplineToFollow = mSplineToFollow;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtSetAISpeedMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_Speed = m_Speed;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtSetAIRelativeSpeedMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_RelativeActor = m_RelativeActor;
	m_Msg.m_Aggression = m_Aggression;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCSApplyImpulseMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.m_ApplyLinearSpeed = m_ApplyLinearSpeed;
	m_Msg.m_LinearSpeed = m_LinearSpeed;
	m_Msg.m_ApplyAngularSpeed = m_ApplyAngularSpeed;
	m_Msg.m_AngularSpeed = m_AngularSpeed;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCSRepairAllDamageMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtStartTheRaceMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtRaceEndedMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtStartTheIntroMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtPlaceCarOnPointMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mUseStartPosition = mUseStartPosition;
	m_Msg.mPoint = mPoint;
	m_Msg.m_Speed = m_Speed;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtSetDriverAnimMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mPlayDriverAnim = mPlayDriverAnim;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtSetCarBehaviourMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	memcpy(&m_Msg.mDoCarModelBehaviour,&mDoCarModelBehaviour,sizeof(C_XtCarModelBehaviour));
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtPlayConversationAndSubtitleMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mDialogue = mDialogue;
	m_Msg.mSpokenLine = mSpokenLine;
	return &m_Msg;
}

//=========================================================================

const Messaging::C_MessageBase * C_XtReplay_RecordMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtReplay_LoadMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mReplayFile = mReplayFile.Text();
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtReplay_PlayMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mStartTime = mStartTime;
//	m_Msg.mEndTime = mEndTime;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtReplay_StopMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtReplay_PauseMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtReplay_ResumeMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtDamage_EnableDisableMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mRecieveDamage = mRecieveDamage;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtDamage_ApplyMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mDamageState = mDamageState;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtDamage_RepairMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_DriveRelativeMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mParentActor = mParentActor;
	m_Msg.mParentOffset = mParentOffset;
	m_Msg.mAggression = mAggression;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_DisableAIMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_SetVehicleControlMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mDirectControl = mDirectControl;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_SetVehicleHandbrakeMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mApplyHandbrake = mApplyHandbrake;
	return &m_Msg;
}

/*
const Messaging::C_MessageBase * C_XtCutScene_SetVehicleSpeedMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mSpeed = mSpeed;
	return &m_Msg;
}
*/

const Messaging::C_MessageBase * C_XtCutScene_SetVehicleRevsMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mRevs = mRevs;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_SetVehicleBrakeMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mBrake = mBrake;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_SetVehicleSteeringMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mSteer = mSteer;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_SetDoorPositionMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mLeftPosition = mLeftPosition;
	m_Msg.mLeftPositionAnimTime = mLeftPositionAnimTime;

	m_Msg.mRightPosition = mRightPosition;
	m_Msg.mRightPositionAnimTime = mRightPositionAnimTime;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_SetWindowPositionMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mLeftPosition = mLeftPosition;
	m_Msg.mLeftPositionAnimTime = mLeftPositionAnimTime;

	m_Msg.mRightPosition = mRightPosition;
	m_Msg.mRightPositionAnimTime = mRightPositionAnimTime;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_SetBonnetPositionMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mBonnetPosition = mBonnetPosition;
	m_Msg.mBonnetPositionAnimTime = mBonnetPositionAnimTime;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_SetLightStateMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mLeftIndicatorOn = mLeftIndicatorOn;
	m_Msg.mRightIndicatorOn = mRightIndicatorOn;
	m_Msg.mHeadlightsOn = mHeadlightsOn;
	m_Msg.mBrakeLightsOn = mBrakeLightsOn;
	m_Msg.mReverseLightsOn = mReverseLightsOn;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_DisplayVehicleMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mDisplayVehicle = mDisplayVehicle;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_DisplayDriverMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mDisplayDriver = mDisplayDriver;
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_CreateVehicleEffectsMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_LoadResourcesMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_DumpResourcesMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}


const Messaging::C_MessageBase * C_XtCutScene_SwapVehicleModelMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_Start321Msg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_Start321_2_Player_Msg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_Start321_3_Player_Msg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_Start321_4_Player_Msg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_WreckCamMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_EnableRaceAIMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_DisableRaceAIMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_SetVehiclesToRaceSpeedMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mPercentageRaceSpeed = mPercentageRaceSpeed;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_DialogueMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	m_Msg.mSoundBank = mSoundBank;
	m_Msg.mSoundIndex = mSoundIndex;
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_DialogueFinishedMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_SkippedMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}

const Messaging::C_MessageBase * C_XtCutScene_DisplayCreditMsg::GetMessage(const Actor::C_Actor *pActorContext)
{
	return &m_Msg;
}



} //namespace ActorMsg
