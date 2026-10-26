#ifndef CUT_SCENE_MESSAGES_H
#define CUT_SCENE_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/uid/FourCC.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/SharedMessageValues.h>
#include <shared_actors/XtActorMessages.h>

#include <GameLogic/ActorStudioActors/ResourceTypes.h>
#include <GameLogic/ActorStudioActors/CutSceneVehicleSplineActor.h>
#include <GameLogic/ActorStudioActors/CutSceneVehiclePositionActor.h>

#include <GameLogic\GameData\AmaxDialogue.h>

namespace ActorMsg
{

enum E_CutSceneMsg
{
	E_CutSceneMsg_HiBits = FOURCC_HI20('MGCS'),

	E_CutSceneMsg_Start_Cutscene,
	E_CutSceneMsg_Start_ScriptedCamera,
	E_CutSceneMsg_Start_RestartScriptedCamera,
	E_CutSceneMsg_Start_PassedCutscene,
	E_CutSceneMsg_Start_PassedScriptedCamera,
	E_CutSceneMsg_Start_FailedScriptedCamera,
	E_CutSceneMsg_Finished,
	E_CutSceneMsg_StartFatalDamageCam,
	E_CutSceneMsg_Start_GarageCamera,
	E_CutSceneMsg_Start_GarageCameraReverse,

	E_CutSceneMsg_StartTheIntro,
	E_CutSceneMsg_InitialiseProxy,
	E_CutSceneMsg_EnableDisableProxy,
	E_CutSceneMsg_PlaceCarOnPoint,
	E_CutSceneMsg_SetAIOnSpline,
	E_CutSceneMsg_SetAISpeed,
	E_CutSceneMsg_SetAIRelativeSpeed,
	E_CutSceneMsg_ApplyImpulse,
	E_CutSceneMsg_RepairAllDamage,
	E_CutSceneMsg_StartTheRace,
	E_CutSceneMsg_RaceEnded,
	E_CutSceneMsg_SetDriverAnim,
	E_CutSceneMsg_SetCarBehaviour,
	E_CutSceneMsg_PlayConversationAndSubtitle,
	E_CutSceneMsg_FullScreenEffect,

	E_CutSceneMsg_Replay_SetSaveFilename,
	E_CutSceneMsg_Replay_StartRecordingCs,
	E_CutSceneMsg_Replay_Load,
	E_CutSceneMsg_Replay_Play,
	E_CutSceneMsg_Replay_Stop,
	E_CutSceneMsg_Replay_Pause,
	E_CutSceneMsg_Replay_Resume,
	E_CutSceneMsg_Damage_EnableDisable,
	E_CutSceneMsg_Damage_Apply,
	E_CutSceneMsg_Damage_Repair,
	E_CutSceneMsg_DriveRelative,
	E_CutSceneMsg_DisableAI,
	E_CutSceneMsg_SetVehicleControl,
	E_CutSceneMsg_SetVehicleHandbrake,
//	E_CutSceneMsg_SetVehicleSpeed,
	E_CutSceneMsg_SetVehicleRevs,
	E_CutSceneMsg_SetVehicleBrake,
	E_CutSceneMsg_SetVehicleSteering,
	E_CutSceneMsg_SetDoorPosition,
	E_CutSceneMsg_SetWindowPosition,
	E_CutSceneMsg_SetBonnetPosition,
	E_CutSceneMsg_SetLightState,
	E_CutSceneMsg_DisplayVehicle,
	E_CutSceneMsg_DisplayDriver,
	E_CutSceneMsg_CreateVehicleEffects,
	E_CutSceneMsg_LoadResources,
	E_CutSceneMsg_DumpResources,
	E_CutSceneMsg_SwapVehicleModel,
	E_CutSceneMsg_Start321,
	E_CutSceneMsg_Start321_2_Player,
	E_CutSceneMsg_Start321_3_Player,
	E_CutSceneMsg_Start321_4_Player,
	E_CutSceneMsg_WreckCam,
	E_CutSceneMsg_EnableRaceAI,
	E_CutSceneMsg_DisableRaceAI,
	E_CutSceneMsg_SetVehiclesToRaceSpeed,
	E_CutSceneMsg_Dialogue,
	E_CutSceneMsg_DialogueFinished,
	E_CutSceneMsg_Skipped,
	E_CutSceneMsg_DisplayCredit,

	E_CutSceneMsg_End, 
	E_CutSceneMsg_Begin = E_CutSceneMsg_HiBits + 1, 
	E_CutSceneMsg_Count = E_CutSceneMsg_End - E_CutSceneMsg_Begin
};


enum E_CutSceneDriverAnim
{
	E_CutSceneDriverAnim_PunchAir	 = 0,			// Out of driver side window.
	E_CutSceneDriverAnim_PumpFist,					// Out of driver side window.
	E_CutSceneDriverAnim_ShakeFist,					// Outof driver side window.
	E_CutSceneDriverAnim_LookBehindLeft,			
	E_CutSceneDriverAnim_LookBehindRight,			
	E_CutSceneDriverAnim_HitSteeringWheelOneHand1,	// Frustration.
	E_CutSceneDriverAnim_HitSteeringWheelOneHand2,	// Frustration.
	E_CutSceneDriverAnim_HitSteeringWheelTwoHands,	// Frustration.
	E_CutSceneDriverAnim_ShakeHead,					// Frustration.
	E_CutSceneDriverAnim_JoltSmall,					// Light collision.
	E_CutSceneDriverAnim_JoltLarge,					// Heavy collision.
	E_CutSceneDriverAnim_GlanceLeft,				// Quick turn of head to the left.
	E_CutSceneDriverAnim_GlanceRight,				// Quick turn of head to the right.

	E_CutSceneDriverAnim_Max,
};

struct C_XtCarModelModifier
{
	bool mDoIt;
	float mValue;
};

struct C_XtCarModelBehaviour
{
	C_XtCarModelModifier mRevs;
	C_XtCarModelModifier mThrottle;
	C_XtCarModelModifier mBrake;
	C_XtCarModelModifier mSteering;
						 
	C_XtCarModelModifier mHandbrake;
	C_XtCarModelModifier mHeadlights;
	C_XtCarModelModifier mHeadlightCones;
	C_XtCarModelModifier mRearlights;
	C_XtCarModelModifier mFoglights;
};

//=========================================================================
//					Start sequence messages
//=========================================================================

class C_CutScene_Start_CutsceneMsg : public Messaging::C_MessageDerived<C_CutScene_Start_CutsceneMsg, E_CutSceneMsg_Start_Cutscene>
{ 
public:
	C_CutScene_Start_CutsceneMsg()		{};
	~C_CutScene_Start_CutsceneMsg()		{};
};

class C_XtCutScene_Start_CutsceneMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start_CutsceneMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_CutScene_Start_CutsceneMsg				m_Msg;
};

//=========================================================================

class C_CutScene_Start_ScriptedCameraMsg : public Messaging::C_MessageDerived<C_CutScene_Start_ScriptedCameraMsg, E_CutSceneMsg_Start_ScriptedCamera>
{ 
public:
	C_CutScene_Start_ScriptedCameraMsg()		{};
	~C_CutScene_Start_ScriptedCameraMsg()		{};
};

class C_XtCutScene_Start_ScriptedCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start_ScriptedCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_CutScene_Start_ScriptedCameraMsg				m_Msg;
};

//=========================================================================

class C_CutScene_Start_RestartScriptedCameraMsg : public Messaging::C_MessageDerived<C_CutScene_Start_RestartScriptedCameraMsg, E_CutSceneMsg_Start_RestartScriptedCamera>
{ 
public:
	C_CutScene_Start_RestartScriptedCameraMsg()		{};
	~C_CutScene_Start_RestartScriptedCameraMsg()		{};
};

class C_XtCutScene_Start_RestartScriptedCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start_RestartScriptedCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_CutScene_Start_RestartScriptedCameraMsg				m_Msg;
};

//=========================================================================

class C_CutScene_Start_PassedCutsceneMsg : public Messaging::C_MessageDerived<C_CutScene_Start_PassedCutsceneMsg, E_CutSceneMsg_Start_PassedCutscene>
{ 
public:
	C_CutScene_Start_PassedCutsceneMsg()		{};
	~C_CutScene_Start_PassedCutsceneMsg()		{};
};

class C_XtCutScene_Start_PassedCutsceneMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start_PassedCutsceneMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_CutScene_Start_PassedCutsceneMsg				m_Msg;
};

//=========================================================================

class C_CutScene_Start_PassedScriptedCameraMsg : public Messaging::C_MessageDerived<C_CutScene_Start_PassedScriptedCameraMsg, E_CutSceneMsg_Start_PassedScriptedCamera>
{ 
public:
	C_CutScene_Start_PassedScriptedCameraMsg()		{};
	~C_CutScene_Start_PassedScriptedCameraMsg()		{};
};

class C_XtCutScene_Start_PassedScriptedCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start_PassedScriptedCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_CutScene_Start_PassedScriptedCameraMsg				m_Msg;
};

//=========================================================================

class C_CutScene_Start_FailedScriptedCameraMsg : public Messaging::C_MessageDerived<C_CutScene_Start_FailedScriptedCameraMsg, E_CutSceneMsg_Start_FailedScriptedCamera>
{ 
public:
	C_CutScene_Start_FailedScriptedCameraMsg()		{};
	~C_CutScene_Start_FailedScriptedCameraMsg()		{};
};

class C_XtCutScene_Start_FailedScriptedCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start_FailedScriptedCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_CutScene_Start_FailedScriptedCameraMsg				m_Msg;
};

//=========================================================================

class C_CutScene_FinishedMsg : public Messaging::C_MessageDerived<C_CutScene_FinishedMsg, E_CutSceneMsg_Finished>
{ 
public:
	C_CutScene_FinishedMsg()		{};
	~C_CutScene_FinishedMsg()		{};
};

class C_XtCutScene_FinishedMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_FinishedMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_CutScene_FinishedMsg				m_Msg;
};

//=========================================================================

class C_StartFatalDamageCamMsg : public Messaging::C_MessageDerived<C_StartFatalDamageCamMsg, E_CutSceneMsg_StartFatalDamageCam>
{ 
public:
	C_StartFatalDamageCamMsg()		{};
	~C_StartFatalDamageCamMsg()		{};
};

class C_XtStartFatalDamageCamMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtStartFatalDamageCamMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_StartFatalDamageCamMsg				m_Msg;
};

//=========================================================================

class C_CutScene_Start_GarageCameraMsg : public Messaging::C_MessageDerived<C_CutScene_Start_GarageCameraMsg, E_CutSceneMsg_Start_GarageCamera>
{ 
public:
	C_CutScene_Start_GarageCameraMsg()		{};
	~C_CutScene_Start_GarageCameraMsg()		{};
};

class C_XtCutScene_Start_GarageCameraMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start_GarageCameraMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_CutScene_Start_GarageCameraMsg				m_Msg;
};

//=========================================================================

class C_CutScene_Start_GarageCameraReverseMsg : public Messaging::C_MessageDerived<C_CutScene_Start_GarageCameraReverseMsg, E_CutSceneMsg_Start_GarageCameraReverse>
{ 
public:
	C_CutScene_Start_GarageCameraReverseMsg()		{};
	~C_CutScene_Start_GarageCameraReverseMsg()		{};
};

class C_XtCutScene_Start_GarageCameraReverseMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start_GarageCameraReverseMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_CutScene_Start_GarageCameraReverseMsg				m_Msg;
};


//=========================================================================
//						
//=========================================================================


class C_InitialiseProxyMsg : public Messaging::C_MessageDerived<C_InitialiseProxyMsg, E_CutSceneMsg_InitialiseProxy>
{ 
public:
	C_InitialiseProxyMsg()		{};
	~C_InitialiseProxyMsg()		{};
};

class C_XtInitialiseProxyMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtInitialiseProxyMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_InitialiseProxyMsg				m_Msg;
};

//=========================================================================

class C_EnableDisableProxyMsg : public Messaging::C_MessageDerived<C_EnableDisableProxyMsg, E_CutSceneMsg_EnableDisableProxy>
{ 
public:
	C_EnableDisableProxyMsg()		{};
	~C_EnableDisableProxyMsg()		{};

	bool							mEnable;
};

class C_XtEnableDisableProxyMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtEnableDisableProxyMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_EnableDisableProxyMsg				m_Msg;

	bool								mEnable;
};

//=========================================================================


class C_SetAIOnSplineMsg : public Messaging::C_MessageDerived<C_SetAIOnSplineMsg, E_CutSceneMsg_SetAIOnSpline>
{ 
public:
	C_SetAIOnSplineMsg()		{};
	~C_SetAIOnSplineMsg()		{};

	::Actors::C_CutSceneVehicleSplineActorCreate*	mSplineToFollow;
};

class C_XtSetAIOnSplineMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSetAIOnSplineMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	::Actors::C_CutSceneVehicleSplineActorCreate*	mSplineToFollow;

	C_SetAIOnSplineMsg					m_Msg;
};

//=========================================================================

class C_SetAISpeedMsg : public Messaging::C_MessageDerived<C_SetAISpeedMsg, E_CutSceneMsg_SetAISpeed>
{
public:
	C_SetAISpeedMsg()		{};
	~C_SetAISpeedMsg()		{};

	F32								m_Speed;
};

class C_XtSetAISpeedMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSetAISpeedMsg()					{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_SetAISpeedMsg						m_Msg;

	F32								m_Speed;
};

//=========================================================================

class C_SetAIRelativeSpeedMsg : public Messaging::C_MessageDerived<C_SetAIRelativeSpeedMsg, E_CutSceneMsg_SetAIRelativeSpeed>
{
public:
	C_SetAIRelativeSpeedMsg()		{};
	~C_SetAIRelativeSpeedMsg()		{};

	::Actors::C_GameActorCreate*	m_RelativeActor;
	F32								m_Aggression;
};

class C_XtSetAIRelativeSpeedMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSetAIRelativeSpeedMsg()			{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_SetAIRelativeSpeedMsg				m_Msg;

	::Actors::C_GameActorCreate*	m_RelativeActor;
	F32								m_Aggression;
};

//=========================================================================

class C_CSApplyImpulseMsg : public Messaging::C_MessageDerived<C_SetAISpeedMsg, E_CutSceneMsg_ApplyImpulse>
{
public:
	C_CSApplyImpulseMsg()		{};
	~C_CSApplyImpulseMsg()		{};

	bool								m_ApplyLinearSpeed;
	XtVec3								m_LinearSpeed;
	bool								m_ApplyAngularSpeed;
	XtVec3								m_AngularSpeed;
};

class C_XtCSApplyImpulseMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCSApplyImpulseMsg()				{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CSApplyImpulseMsg					m_Msg;

	bool								m_ApplyLinearSpeed;
	XtVec3								m_LinearSpeed;
	bool								m_ApplyAngularSpeed;
	XtVec3								m_AngularSpeed;
};

//=========================================================================

class C_CSRepairAllDamageMsg : public Messaging::C_MessageDerived<C_SetAISpeedMsg, E_CutSceneMsg_RepairAllDamage>
{
public:
	C_CSRepairAllDamageMsg()		{};
	~C_CSRepairAllDamageMsg()		{};
};

class C_XtCSRepairAllDamageMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCSRepairAllDamageMsg()			{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CSRepairAllDamageMsg				m_Msg;
};

//=========================================================================

class C_StartTheRaceMsg : public Messaging::C_MessageDerived<C_StartTheRaceMsg, E_CutSceneMsg_StartTheRace>
{
public:
	C_StartTheRaceMsg()		{};
	~C_StartTheRaceMsg()		{};
};
class C_XtStartTheRaceMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtStartTheRaceMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_StartTheRaceMsg					m_Msg;
};

//=========================================================================

class C_RaceEndedMsg : public Messaging::C_MessageDerived<C_RaceEndedMsg, E_CutSceneMsg_RaceEnded>
{
public:
	C_RaceEndedMsg()		{};
	~C_RaceEndedMsg()		{};
};
class C_XtRaceEndedMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtRaceEndedMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_RaceEndedMsg					m_Msg;
};

//=========================================================================

class C_StartTheIntroMsg : public Messaging::C_MessageDerived<C_StartTheIntroMsg, E_CutSceneMsg_StartTheIntro>
{
public:
	C_StartTheIntroMsg()		{};
	~C_StartTheIntroMsg()		{};
};
class C_XtStartTheIntroMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtStartTheIntroMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_StartTheIntroMsg					m_Msg;
};

//=========================================================================

class C_PlaceCarOnPointMsg : public Messaging::C_MessageDerived<C_PlaceCarOnPointMsg, E_CutSceneMsg_PlaceCarOnPoint>
{
public:
	C_PlaceCarOnPointMsg()		{};
	~C_PlaceCarOnPointMsg()		{};

	bool											mUseStartPosition;
	::Actors::C_CutSceneVehiclePositionActorCreate*	mPoint;
	F32												m_Speed;
};


class C_XtPlaceCarOnPointMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtPlaceCarOnPointMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	bool											mUseStartPosition;
	::Actors::C_CutSceneVehiclePositionActorCreate*	mPoint;
	F32												m_Speed;

	C_PlaceCarOnPointMsg				m_Msg;
};


//=========================================================================

class C_SetDriverAnimMsg : public Messaging::C_MessageDerived<C_SetDriverAnimMsg, E_CutSceneMsg_SetDriverAnim>
{
public:
	C_SetDriverAnimMsg()		{};
	~C_SetDriverAnimMsg()		{};

	E_CutSceneDriverAnim	mPlayDriverAnim;
};
class C_XtSetDriverAnimMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSetDriverAnimMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	E_CutSceneDriverAnim	mPlayDriverAnim;

	C_SetDriverAnimMsg					m_Msg;
};


//=========================================================================

class C_SetCarBehaviourMsg : public Messaging::C_MessageDerived<C_SetCarBehaviourMsg, E_CutSceneMsg_SetCarBehaviour>
{
public:
	C_SetCarBehaviourMsg()		{};
	~C_SetCarBehaviourMsg()		{};

	C_XtCarModelBehaviour	mDoCarModelBehaviour;
};
class C_XtSetCarBehaviourMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtSetCarBehaviourMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	C_XtCarModelBehaviour	mDoCarModelBehaviour;

	C_SetCarBehaviourMsg					m_Msg;
};


//=========================================================================

class C_PlayConversationAndSubtitleMsg : public Messaging::C_MessageDerived<C_PlayConversationAndSubtitleMsg, E_CutSceneMsg_PlayConversationAndSubtitle>
{
public:
	C_PlayConversationAndSubtitleMsg()		{};
	~C_PlayConversationAndSubtitleMsg()		{};

	XtLocId				mDialogue;
	AmaxDialogueAudio	mSpokenLine;
};
class C_XtPlayConversationAndSubtitleMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtPlayConversationAndSubtitleMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	XtLocId				mDialogue;
	AmaxDialogueAudio	mSpokenLine;

	C_PlayConversationAndSubtitleMsg					m_Msg;
};







//=========================================================================
//						NEW MESSAGES
//=========================================================================



class C_Replay_RecordMsg : public Messaging::C_MessageDerived<C_Replay_RecordMsg, E_CutSceneMsg_Replay_StartRecordingCs>
{
public:
	C_Replay_RecordMsg()		{};
	~C_Replay_RecordMsg()		{};

	Btl::String32			mReplayFile;
};

class C_XtReplay_RecordMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtReplay_RecordMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_Replay_RecordMsg		m_Msg;
};

//=========================================================================

class C_Replay_LoadMsg : public Messaging::C_MessageDerived<C_Replay_LoadMsg, E_CutSceneMsg_Replay_Load>
{
public:
	C_Replay_LoadMsg()		{};
	~C_Replay_LoadMsg()		{};

	Btl::String32			mReplayFile;
};

class C_XtReplay_LoadMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtReplay_LoadMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_Replay_LoadMsg		m_Msg;

	XtSz8					mReplayFile;
};

//=========================================================================

class C_Replay_PlayMsg : public Messaging::C_MessageDerived<C_Replay_PlayMsg, E_CutSceneMsg_Replay_Play>
{
public:
	C_Replay_PlayMsg()		{};
	~C_Replay_PlayMsg()		{};

	F32						mStartTime;
//	F32						mEndTime;
};

class C_XtReplay_PlayMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtReplay_PlayMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_Replay_PlayMsg		m_Msg;

	F32						mStartTime;
//	F32						mEndTime;
};

//=========================================================================

class C_Replay_StopMsg : public Messaging::C_MessageDerived<C_Replay_StopMsg, E_CutSceneMsg_Replay_Stop>
{
public:
	C_Replay_StopMsg()		{};
	~C_Replay_StopMsg()	{};

};

class C_XtReplay_StopMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtReplay_StopMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_Replay_StopMsg		m_Msg;

};

//=========================================================================

class C_Replay_PauseMsg : public Messaging::C_MessageDerived<C_Replay_PauseMsg, E_CutSceneMsg_Replay_Pause>
{
public:
	C_Replay_PauseMsg()		{};
	~C_Replay_PauseMsg()	{};

};

class C_XtReplay_PauseMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtReplay_PauseMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_Replay_PauseMsg		m_Msg;

};

//=========================================================================

class C_Replay_ResumeMsg : public Messaging::C_MessageDerived<C_Replay_ResumeMsg, E_CutSceneMsg_Replay_Resume>
{
public:
	C_Replay_ResumeMsg()	{};
	~C_Replay_ResumeMsg()	{};

};

class C_XtReplay_ResumeMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtReplay_ResumeMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_Replay_ResumeMsg		m_Msg;

};

//=========================================================================

class C_Damage_EnableDisableMsg : public Messaging::C_MessageDerived<C_Damage_EnableDisableMsg, E_CutSceneMsg_Damage_EnableDisable>
{
public:
	C_Damage_EnableDisableMsg()		{};
	~C_Damage_EnableDisableMsg()	{};

	bool							mRecieveDamage;
};

class C_XtDamage_EnableDisableMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtDamage_EnableDisableMsg()	{}
	const Messaging::C_MessageBase * GetMessage(const Actor::C_Actor *pActorContext);   
	C_Damage_EnableDisableMsg		m_Msg;

	bool							mRecieveDamage;
};

//=========================================================================

class C_Damage_ApplyMsg : public Messaging::C_MessageDerived<C_Damage_ApplyMsg, E_CutSceneMsg_Damage_Apply>
{
public:
	C_Damage_ApplyMsg()			{};
	~C_Damage_ApplyMsg()		{};

	S32							mDamageState;
};

class C_XtDamage_ApplyMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtDamage_ApplyMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_Damage_ApplyMsg			m_Msg;

	S32							mDamageState;
};

//=========================================================================

class C_Damage_RepairMsg : public Messaging::C_MessageDerived<C_Damage_RepairMsg, E_CutSceneMsg_Damage_Repair>
{
public:
	C_Damage_RepairMsg()		{};
	~C_Damage_RepairMsg()		{};

};

class C_XtDamage_RepairMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtDamage_RepairMsg()		{}
	const Messaging::C_MessageBase * GetMessage(const Actor::C_Actor *pActorContext);   
	C_Damage_RepairMsg			m_Msg;

};

//=========================================================================

class C_CutScene_DriveRelativeMsg : public Messaging::C_MessageDerived<C_CutScene_DriveRelativeMsg, E_CutSceneMsg_DriveRelative>
{
public:
	C_CutScene_DriveRelativeMsg()		{};
	~C_CutScene_DriveRelativeMsg()		{};

	::Actors::C_GameActorCreate*		mParentActor;
	XtVec3								mParentOffset;
	F32									mAggression;
};

class C_XtCutScene_DriveRelativeMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_DriveRelativeMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_DriveRelativeMsg			m_Msg;

	::Actors::C_GameActorCreate*		mParentActor;
	XtVec3								mParentOffset;
	F32									mAggression;
};

//=========================================================================

class C_CutScene_DisableAIMsg : public Messaging::C_MessageDerived<C_CutScene_DisableAIMsg, E_CutSceneMsg_DisableAI>
{
public:
	C_CutScene_DisableAIMsg()		{};
	~C_CutScene_DisableAIMsg()		{};

};

class C_XtCutScene_DisableAIMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_DisableAIMsg()	{}
	const Messaging::C_MessageBase *		GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_DisableAIMsg		m_Msg;
};


//=========================================================================


class C_CutScene_SetVehicleControlMsg : public Messaging::C_MessageDerived<C_CutScene_SetVehicleControlMsg, E_CutSceneMsg_SetVehicleControl>
{
public:
	C_CutScene_SetVehicleControlMsg()		{};
	~C_CutScene_SetVehicleControlMsg()		{};

	bool									mDirectControl;
};

class C_XtCutScene_SetVehicleControlMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetVehicleControlMsg()		{}
	const Messaging::C_MessageBase *		GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetVehicleControlMsg			m_Msg;

	bool									mDirectControl;
};

//=========================================================================

class C_CutScene_SetVehicleHandbrakeMsg : public Messaging::C_MessageDerived<C_CutScene_SetVehicleHandbrakeMsg, E_CutSceneMsg_SetVehicleHandbrake>
{
public:
	C_CutScene_SetVehicleHandbrakeMsg()		{};
	~C_CutScene_SetVehicleHandbrakeMsg()	{};

	bool									mApplyHandbrake;
};

class C_XtCutScene_SetVehicleHandbrakeMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetVehicleHandbrakeMsg()	{}
	const Messaging::C_MessageBase *		GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetVehicleHandbrakeMsg		m_Msg;

	bool									mApplyHandbrake;
};

//=========================================================================
/*
class C_CutScene_SetVehicleSpeedMsg : public Messaging::C_MessageDerived<C_CutScene_SetVehicleSpeedMsg, E_CutSceneMsg_SetVehicleSpeed>
{
public:
	C_CutScene_SetVehicleSpeedMsg()			{};
	~C_CutScene_SetVehicleSpeedMsg()		{};

	F32										mSpeed;
};

class C_XtCutScene_SetVehicleSpeedMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetVehicleSpeedMsg()		{}
	const Messaging::C_MessageBase *		GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetVehicleSpeedMsg			m_Msg;

	F32										mSpeed;
};
*/
//=========================================================================

class C_CutScene_SetVehicleRevsMsg : public Messaging::C_MessageDerived<C_CutScene_SetVehicleRevsMsg, E_CutSceneMsg_SetVehicleRevs>
{
public:
	C_CutScene_SetVehicleRevsMsg()		{};
	~C_CutScene_SetVehicleRevsMsg()		{};

	F32									mRevs;
};

class C_XtCutScene_SetVehicleRevsMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetVehicleRevsMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetVehicleRevsMsg		m_Msg;

	F32									mRevs;
};

//=========================================================================

class C_CutScene_SetVehicleBrakeMsg : public Messaging::C_MessageDerived<C_CutScene_SetVehicleBrakeMsg, E_CutSceneMsg_SetVehicleBrake>
{
public:
	C_CutScene_SetVehicleBrakeMsg()		{};
	~C_CutScene_SetVehicleBrakeMsg()	{};

	F32									mBrake;
};

class C_XtCutScene_SetVehicleBrakeMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetVehicleBrakeMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetVehicleBrakeMsg		m_Msg;

	F32									mBrake;
};

//=========================================================================

class C_CutScene_SetVehicleSteeringMsg : public Messaging::C_MessageDerived<C_CutScene_SetVehicleSteeringMsg, E_CutSceneMsg_SetVehicleSteering>
{
public:
	C_CutScene_SetVehicleSteeringMsg()	{};
	~C_CutScene_SetVehicleSteeringMsg()	{};

	F32									mSteer;
};

class C_XtCutScene_SetVehicleSteeringMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetVehicleSteeringMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetVehicleSteeringMsg	m_Msg;

	F32									mSteer;
};

//=========================================================================

class C_CutScene_SetDoorPositionMsg : public Messaging::C_MessageDerived<C_CutScene_SetDoorPositionMsg, E_CutSceneMsg_SetDoorPosition>
{
public:
	C_CutScene_SetDoorPositionMsg()		{};
	~C_CutScene_SetDoorPositionMsg()	{};

	F32									mLeftPosition;
	F32									mLeftPositionAnimTime;
	F32									mRightPosition;
	F32									mRightPositionAnimTime;
};

class C_XtCutScene_SetDoorPositionMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetDoorPositionMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetDoorPositionMsg		m_Msg;

	F32									mLeftPosition;
	F32									mLeftPositionAnimTime;
	F32									mRightPosition;
	F32									mRightPositionAnimTime;
};

//=========================================================================

class C_CutScene_SetWindowPositionMsg : public Messaging::C_MessageDerived<C_CutScene_SetWindowPositionMsg, E_CutSceneMsg_SetWindowPosition>
{
public:
	C_CutScene_SetWindowPositionMsg()	{};
	~C_CutScene_SetWindowPositionMsg()	{};

	F32									mLeftPosition;
	F32									mLeftPositionAnimTime;
	F32									mRightPosition;
	F32									mRightPositionAnimTime;
};

class C_XtCutScene_SetWindowPositionMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetWindowPositionMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetWindowPositionMsg		m_Msg;

	F32									mLeftPosition;
	F32									mLeftPositionAnimTime;
	F32									mRightPosition;
	F32									mRightPositionAnimTime;
};

//=========================================================================

class C_CutScene_SetBonnetPositionMsg : public Messaging::C_MessageDerived<C_CutScene_SetBonnetPositionMsg, E_CutSceneMsg_SetBonnetPosition>
{
public:
	C_CutScene_SetBonnetPositionMsg()	{};
	~C_CutScene_SetBonnetPositionMsg()	{};

	F32									mBonnetPosition;
	F32									mBonnetPositionAnimTime;
};

class C_XtCutScene_SetBonnetPositionMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetBonnetPositionMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetBonnetPositionMsg		m_Msg;

	F32									mBonnetPosition;
	F32									mBonnetPositionAnimTime;
};

//=========================================================================

class C_CutScene_SetLightStateMsg : public Messaging::C_MessageDerived<C_CutScene_SetLightStateMsg, E_CutSceneMsg_SetLightState>
{
public:
	C_CutScene_SetLightStateMsg()		{};
	~C_CutScene_SetLightStateMsg()		{};

	bool								mLeftIndicatorOn;
	bool								mRightIndicatorOn;
	bool								mHeadlightsOn;
	bool								mBrakeLightsOn;
	bool								mReverseLightsOn;
};

class C_XtCutScene_SetLightStateMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetLightStateMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetLightStateMsg			m_Msg;

	bool								mLeftIndicatorOn;
	bool								mRightIndicatorOn;
	bool								mHeadlightsOn;
	bool								mBrakeLightsOn;
	bool								mReverseLightsOn;
};

//=========================================================================

class C_CutScene_DisplayVehicleMsg : public Messaging::C_MessageDerived<C_CutScene_DisplayVehicleMsg, E_CutSceneMsg_DisplayVehicle>
{
public:
	C_CutScene_DisplayVehicleMsg()		{};
	~C_CutScene_DisplayVehicleMsg()		{};

	bool								mDisplayVehicle;
};

class C_XtCutScene_DisplayVehicleMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_DisplayVehicleMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_DisplayVehicleMsg		m_Msg;

	bool								mDisplayVehicle;
};

//=========================================================================

class C_CutScene_DisplayDriverMsg : public Messaging::C_MessageDerived<C_CutScene_DisplayDriverMsg, E_CutSceneMsg_DisplayDriver>
{
public:
	C_CutScene_DisplayDriverMsg()		{};
	~C_CutScene_DisplayDriverMsg()		{};

	bool								mDisplayDriver;
};

class C_XtCutScene_DisplayDriverMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_DisplayDriverMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_DisplayDriverMsg			m_Msg;

	bool								mDisplayDriver;
};

//=========================================================================

class C_CutScene_CreateVehicleEffectsMsg : public Messaging::C_MessageDerived<C_CutScene_CreateVehicleEffectsMsg, E_CutSceneMsg_CreateVehicleEffects>
{
public:
	C_CutScene_CreateVehicleEffectsMsg()	{};
	~C_CutScene_CreateVehicleEffectsMsg()	{};

};

class C_XtCutScene_CreateVehicleEffectsMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_CreateVehicleEffectsMsg()	{}
	const Messaging::C_MessageBase *		GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_CreateVehicleEffectsMsg		m_Msg;

};

//=========================================================================

class C_CutScene_LoadResourcesMsg : public Messaging::C_MessageDerived<C_CutScene_LoadResourcesMsg, E_CutSceneMsg_LoadResources>
{
public:
	C_CutScene_LoadResourcesMsg()		{};
	~C_CutScene_LoadResourcesMsg()		{};

};

class C_XtCutScene_LoadResourcesMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_LoadResourcesMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_LoadResourcesMsg			m_Msg;

};

//=========================================================================

class C_CutScene_DumpResourcesMsg : public Messaging::C_MessageDerived<C_CutScene_DumpResourcesMsg, E_CutSceneMsg_DumpResources>
{
public:
	C_CutScene_DumpResourcesMsg()		{};
	~C_CutScene_DumpResourcesMsg()		{};

};

class C_XtCutScene_DumpResourcesMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_DumpResourcesMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_DumpResourcesMsg			m_Msg;

};

//=========================================================================

class C_CutScene_SwapVehicleModelMsg : public Messaging::C_MessageDerived<C_CutScene_SwapVehicleModelMsg, E_CutSceneMsg_SwapVehicleModel>
{
public:
	C_CutScene_SwapVehicleModelMsg()	{};
	~C_CutScene_SwapVehicleModelMsg()	{};

};

class C_XtCutScene_SwapVehicleModelMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SwapVehicleModelMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SwapVehicleModelMsg		m_Msg;

};


//=========================================================================
//					GENERIC MESSAGE STUFF
//=========================================================================

class C_CutScene_Start321Msg : public Messaging::C_MessageDerived<C_CutScene_Start321Msg, E_CutSceneMsg_Start321>
{
public:
	C_CutScene_Start321Msg()	{};
	~C_CutScene_Start321Msg()	{};

};

class C_XtCutScene_Start321Msg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start321Msg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_Start321Msg		m_Msg;
};

class C_CutScene_Start321_2_Player_Msg : public Messaging::C_MessageDerived<C_CutScene_Start321_2_Player_Msg, E_CutSceneMsg_Start321_2_Player>
{
public:
	C_CutScene_Start321_2_Player_Msg()	{};
	~C_CutScene_Start321_2_Player_Msg()	{};

};

class C_XtCutScene_Start321_2_Player_Msg: public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start321_2_Player_Msg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_Start321_2_Player_Msg		m_Msg;
};

class C_CutScene_Start321_3_Player_Msg : public Messaging::C_MessageDerived<C_CutScene_Start321_3_Player_Msg, E_CutSceneMsg_Start321_3_Player>
{
public:
	C_CutScene_Start321_3_Player_Msg()	{};
	~C_CutScene_Start321_3_Player_Msg()	{};

};

class C_XtCutScene_Start321_3_Player_Msg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start321_3_Player_Msg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_Start321_3_Player_Msg		m_Msg;
};

class C_CutScene_Start321_4_Player_Msg : public Messaging::C_MessageDerived<C_CutScene_Start321_4_Player_Msg, E_CutSceneMsg_Start321_4_Player>
{
public:
	C_CutScene_Start321_4_Player_Msg()	{};
	~C_CutScene_Start321_4_Player_Msg()	{};

};

class C_XtCutScene_Start321_4_Player_Msg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_Start321_4_Player_Msg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_Start321_4_Player_Msg		m_Msg;
};

//=========================================================================

class C_CutScene_WreckCamMsg : public Messaging::C_MessageDerived<C_CutScene_WreckCamMsg, E_CutSceneMsg_WreckCam>
{
public:
	C_CutScene_WreckCamMsg()	{};
	~C_CutScene_WreckCamMsg()	{};

};

class C_XtCutScene_WreckCamMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_WreckCamMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_WreckCamMsg		m_Msg;
};


//=========================================================================


class C_CutScene_EnableRaceAIMsg : public Messaging::C_MessageDerived<C_CutScene_EnableRaceAIMsg, E_CutSceneMsg_EnableRaceAI>
{
public:
	C_CutScene_EnableRaceAIMsg()		{};
	~C_CutScene_EnableRaceAIMsg()	{};

};

class C_XtCutScene_EnableRaceAIMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_EnableRaceAIMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_EnableRaceAIMsg		m_Msg;
};


//=========================================================================

class C_CutScene_DisableRaceAIMsg : public Messaging::C_MessageDerived<C_CutScene_DisableRaceAIMsg, E_CutSceneMsg_DisableRaceAI>
{
public:
	C_CutScene_DisableRaceAIMsg()		{};
	~C_CutScene_DisableRaceAIMsg()	{};

};

class C_XtCutScene_DisableRaceAIMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_DisableRaceAIMsg()	{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_DisableRaceAIMsg		m_Msg;
};

//=========================================================================

class C_CutScene_SetVehiclesToRaceSpeedMsg : public Messaging::C_MessageDerived<C_CutScene_SetVehiclesToRaceSpeedMsg, E_CutSceneMsg_SetVehiclesToRaceSpeed>
{
public:
	C_CutScene_SetVehiclesToRaceSpeedMsg()		{};
	~C_CutScene_SetVehiclesToRaceSpeedMsg()		{};

	F32								mPercentageRaceSpeed;
};

class C_XtCutScene_SetVehiclesToRaceSpeedMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SetVehiclesToRaceSpeedMsg()	{}
	const Messaging::C_MessageBase *			GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SetVehiclesToRaceSpeedMsg		m_Msg;

	F32								mPercentageRaceSpeed;
};


//=========================================================================

class C_CutScene_DialogueMsg : public Messaging::C_MessageDerived<C_CutScene_DialogueMsg, E_CutSceneMsg_Dialogue>
{
public:
	C_CutScene_DialogueMsg()		{};
	~C_CutScene_DialogueMsg()		{};

	S32								mSoundBank;
	S32								mSoundIndex;
};

class C_XtCutScene_DialogueMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_DialogueMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_DialogueMsg			m_Msg;

	S32								mSoundBank;
	S32								mSoundIndex;
};

//=========================================================================

class C_CutScene_DialogueFinishedMsg : public Messaging::C_MessageDerived<C_CutScene_DialogueFinishedMsg, E_CutSceneMsg_DialogueFinished>
{
public:
	C_CutScene_DialogueFinishedMsg()		{};
	~C_CutScene_DialogueFinishedMsg()		{};
};

class C_XtCutScene_DialogueFinishedMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_DialogueFinishedMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_DialogueFinishedMsg			m_Msg;
};


//=========================================================================

class C_CutScene_SkippedMsg : public Messaging::C_MessageDerived<C_CutScene_SkippedMsg, E_CutSceneMsg_Skipped>
{
public:
	C_CutScene_SkippedMsg()		{};
	~C_CutScene_SkippedMsg()		{};
};

class C_XtCutScene_SkippedMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_SkippedMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_SkippedMsg			m_Msg;
};

//=========================================================================

class C_CutScene_DisplayCreditMsg : public Messaging::C_MessageDerived<C_CutScene_DisplayCreditMsg, E_CutSceneMsg_DisplayCredit>
{
public:
	C_CutScene_DisplayCreditMsg()		{};
	~C_CutScene_DisplayCreditMsg()		{};
};

class C_XtCutScene_DisplayCreditMsg : public C_XtActorMessageBase			// Xt Version
{
public:
	C_XtCutScene_DisplayCreditMsg()		{}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   
	C_CutScene_DisplayCreditMsg			m_Msg;
};



} //namespace ActorMsg


#endif		// CUT_SCENE_MESSAGES_H

