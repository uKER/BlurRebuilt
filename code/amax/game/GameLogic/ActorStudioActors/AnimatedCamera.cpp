#include "stdafx.h"

#include <Game\Game.h>
#include <Game/AmaxCam/CameraSystem.h>
#include <camera2/camera.h>
#include <Game/AmaxCam/VehicleFeed.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameApp.h>

#include <Game/Messaging/VehicleActorMessages.h>

#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Vehicle\VehicleModel.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>


#include <Game/AmaxCam/DirectorActor.h>
#include <Game/GameUtil/ResourceBag.h>
#include <resource_utils/CycleResourceDef.h>
#include <resource_utils/ResourceBagUtils.h>

#include <animation/cycle/Cycle.h>
#include <animation/pose/PoseFormat.h>

#include "AnimatedCamera.h"
#include "ui/Game/Gadgets/ButtonMessages.h"


static void PlayVistaFunc(void *)
{
	ActorMsg::C_StartAnimatedCameraMsg		msg;
	msg.m_Sequence = ::Actors::E_ANIMCAMERASEQ_ROUTEVISTA;	// [Blur2PC] qualified (Game::Actors is visible too)
	msg.m_Viewport = 0;
	msg.m_VehicleIndex = -1;
	msg.m_StartPaused = false;
//	Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );
}
static Debug::Function PlayVista("Camera2.Animated.PlayVista", PlayVistaFunc, 0);

static void PlayPackFunc(void *)
{
	ActorMsg::C_StartAnimatedCameraMsg		msg;
	msg.m_Sequence = ::Actors::E_ANIMCAMERASEQ_ROUTEPACK;
	msg.m_Viewport = 0;
	msg.m_VehicleIndex = -1;
	msg.m_StartPaused = false;
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );
}
static Debug::Function PlayPack("Camera2.Animated.PlayPack", PlayPackFunc, 0);

Debug::Int g_DebugAnimatedCameraGenericIndex("Camera2.Animated.GenericIndex", 0, Debug::Range(0, 14));
static void PlayGenericFunc(void *)
{
	ActorMsg::C_StartAnimatedCameraMsg		msg;
	msg.m_Sequence = (::Actors::E_ANIMCAMERASEQ_TYPE)(::Actors::E_ANIMCAMERASEQ_RACESTART4 + g_DebugAnimatedCameraGenericIndex);
	msg.m_Viewport = 0;
	msg.m_VehicleIndex = 0;
	msg.m_StartPaused = false;
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );
}
static Debug::Function PlayGeneric("Camera2.Animated.PlayGeneric", PlayGenericFunc, 0);

Debug::Float g_DebugAnimatedCameraBlendTime("Camera2.Animated.BlendTime", 0, Debug::Range(0, 5));
static void BlendToRaceCamFunc(void *)
{
	ActorMsg::C_BlendAnimatedCameraMsg		msg;
	msg.m_Duration = g_DebugAnimatedCameraBlendTime;
	msg.m_Viewport = 0;
	msg.m_VehicleIndex = 0;
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );
}
static Debug::Function BlendToRace("Camera2.Animated.BlendToRaceCam", BlendToRaceCamFunc, 0);


static void AutoSelectAnimatedCamFunc(void *)
{
	ActorMsg::C_AutoSelectAnimatedCameraMsg		msg;
	msg.m_Viewport = 0;
	msg.m_VehicleIndex = 0;
	msg.m_Offsets = RLDatabase::Instance().mRLSystems[0]->mDefaultCameraOffsets;		//.Object();
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );
}
static Debug::Function AutoSelectCam("Camera2.Animated.AutoSelectCam", AutoSelectAnimatedCamFunc, 0);


Debug::Float g_debugBlendToBumperGhostTime("Vehicle.BlendToBumperGhostTime", 0.25f);
Debug::Float g_debugVehicleCamBlendMaxGhostPos("Vehicle.BlendToBumperGhostDistance", 3.0f);
Debug::Bool	g_debugAnimatedCameraUseMPPackShot("Camera2.Animated.UseMPPackShot",false);



//OPTIMISE_OFF
//#pragma optimize("", off)



//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)



XT_TYPE(::Actors::AnimatedCameraSequence,		"AnimatedCameraSequence")
	XT_FIELD(m_Animation,					"Animation")
	XT_FIELD(m_UiStageTime,					"StageTime")
	XT_FIELD(m_UiTextPosition0,				"TextPosition")
	XT_FIELD(m_UiTextPosition1,				"TextPosition2")
	XT_FIELD(m_UiLbPosition,				"LbPosition")
	XT_FIELD(m_UiLbSize,					"LbSize")
	XT_FIELD(m_UiLbAnimName,				"LbAnimName")
	XT_FIELD(m_UiLbAnimForwards,			"LbAnimForwards")
	XT_FIELD(m_HideFocusVehicle,			"HideFocusVehicle")
	XT_FIELD(m_HideOpponentVehicles,		"HideOpponentVehicles")
XT_END_TYPE



XT_TYPE(::Actors::AnimatedCameraCreationData,		"AnimatedCamera")
	XT_BASE(::Actors::C_PosActorCreate)
	XT_FIELD(m_Director,						"Director")
	XT_FIELD(m_Viewport,						"Viewport")
	XT_FIELD(m_FutureFeed,						"FutureFeed")
XT_END_TYPE


XT_TYPE(::Actors::E_ANIMCAMERASEQ_TYPE,			"AnimatedCameraSequenceType")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_ROUTEVISTA,	"RouteVista")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_ROUTEPACK,	"RoutePack")

	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RACESTART4,	"RaceStart4")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RACESTART3,	"RaceStart3")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RACESTART2,	"RaceStart2")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RACESTART1,	"RaceStart1")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RACESTARTGO,	"RaceStartGO")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_MPWINNER,		"MpWinner")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_MMWINNER,		"MmWinner")

	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ01,	"RivalSeq01")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ02,	"RivalSeq02")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ03,	"RivalSeq03")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ04,	"RivalSeq04")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ05,	"RivalSeq05")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ06,	"RivalSeq06")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ07,	"RivalSeq07")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ08,	"RivalSeq08")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ09,	"RivalSeq09")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_RIVALSEQ10,	"RivalSeq10")

	XT_OPTION(::Actors::E_ANIMCAMERASEQ_MPSPECTATE1,  "MpSpectate1")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_MPSPECTATE2,  "MpSpectate2")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_MPSPECTATE3,  "MpSpectate3")
	XT_OPTION(::Actors::E_ANIMCAMERASEQ_MPSPECTATE4,  "MpSpectate4")

XT_END_TYPE




//=========================================================================
//					Animated Camera Core Routines
//=========================================================================

namespace Actors
{

using namespace Maths;


AnimatedCamera::AnimatedCamera()
{
	m_Camera = NULL;

	m_Active = false;
	m_AnimTime = 0.0f;
	m_CameraFOV = 1.0f;
	m_LocalFOV = 1.0f;

	m_pRoute = NULL;
	m_pGenericSequences = NULL;

	m_pVistaAnimation = NULL;
	m_pPackAnimation = NULL;
	for(S32 a=0;a<E_NUM_ANIMCAMERASEQS;a++)
		m_pGenericAnimation[a] = NULL;

	m_Looped = true;
	m_Paused = false;

	m_CameraMode = E_ANIMCAMERA_MODE_NORMAL;

//	m_BlendToRaceCam = false;
	m_BlendDuration = 0.0f;
	m_BlendTime = 0.0f;

//	m_AutoSelection = 0;

	m_ViewportIndex = -1;
	m_FutureFeed = false;
	m_AnimFinishedSent = false;
}

//=========================================================================

AnimatedCamera::~AnimatedCamera()
{
	AmaxCam::CameraSystem().GameFeedRemove(m_Camera);
	SAFE_DELETE_PTR(m_Camera)
}

//=========================================================================

bool AnimatedCamera::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	m_WorldMatrix = GetCreationData()->GetCreationMatrix();
	m_pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)C_GameApp::GetInstance()->GetGame()->GetLevelState().mRoute];
	m_pGenericSequences	= RLDatabase::Instance().mRLSystems[0]->mAnimatedCameraSequences.Object();

	m_ViewportIndex = GetCreationData()->m_Viewport;
	m_FutureFeed = GetCreationData()->m_FutureFeed;
	return(true);
}

//=========================================================================

bool AnimatedCamera::PrimeImpl()
{
	m_pResourceBag = C_ResourceBag::Create();
	
	m_pResourceBag->AddListener(this);
	m_pResourceBag->LockListeners();

	RequestAnimation(m_pRoute->VistaCameraSequence);
//	RequestAnimation(m_pRoute->PackCameraSequence);

	Game::C_LevelState*	levstate = &Game::C_Game::GetInstance()->GetLevelState();		// Select the pack shot needed for this race mode
	Game::E_RaceMode	racemode = levstate->mRaceMode;
	if (((racemode >= Game::E_RaceMode_Multiplayer_Racing) && (racemode <= Game::E_RaceMode_Multiplayer_Destruction)) || g_debugAnimatedCameraUseMPPackShot)
		RequestAnimation(m_pRoute->PackCameraSequenceMP);
	else if (racemode == Game::E_RaceMode_BossBattle || racemode == Game::E_RaceMode_FriendDemand_BossBattle)
		RequestAnimation(m_pRoute->PackCameraSequenceBoss);
	else if (racemode == Game::E_RaceMode_Checkpoint || racemode == Game::E_RaceMode_FriendDemand_Checkpoint || racemode == Game::E_RaceMode_Destruction || racemode == Game::E_RaceMode_FriendDemand_Destruction)
		RequestAnimation(m_pRoute->PackCameraSequenceSingle);
	else
		RequestAnimation(m_pRoute->PackCameraSequence);



	if (m_pGenericSequences)
		for(S32 a=0;a<m_pGenericSequences->mSequenceArray.count;a++)
			RequestAnimation(m_pGenericSequences->mSequenceArray[a]);

	m_pResourceBag->UnlockListeners();
	
	return GetLifetimeState() == ActorSys::E_LifetimeState_Primed;
}

//=========================================================================

bool AnimatedCamera::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	m_Camera = new Cam2::C_Camera;
	if (!m_Camera)
		return false;
			
	m_Camera->SetName("Animated");						// Initialise the camera
	m_Camera->SetPosition(m_WorldMatrix.AW());
	m_Camera->SetOrient((Maths::C_Quaternion)m_WorldMatrix.Rotation());
	m_Camera->GetPrevPosition();
	

	Animation::C_PoseFormat poseFormat;					// Get C_CycleResource pointers for each camera animation from resource bag (m_pVistaAnimation etc)

	bool	success = true;
	success &= AddAnimation(&poseFormat, &m_pVistaAnimation, m_pRoute->VistaCameraSequence);
	if (!success)										// Don't initialise if any animations are missing
	{
		LOGWARN( Resource::g_Log, "Vista camera animation is missing for this route");
		return(false);
	}


	Game::C_LevelState*	levstate = &Game::C_Game::GetInstance()->GetLevelState();		// Select the pack shot needed for this race mode
	Game::E_RaceMode	racemode = levstate->mRaceMode;

	if (((racemode >= Game::E_RaceMode_Multiplayer_Racing) && (racemode <= Game::E_RaceMode_Multiplayer_Destruction)) || g_debugAnimatedCameraUseMPPackShot)
		success &= AddAnimation(&poseFormat, &m_pPackAnimation, m_pRoute->PackCameraSequenceMP);
	else if (racemode == Game::E_RaceMode_BossBattle || racemode == Game::E_RaceMode_FriendDemand_BossBattle)
		success &= AddAnimation(&poseFormat, &m_pPackAnimation, m_pRoute->PackCameraSequenceBoss);
	else if (racemode == Game::E_RaceMode_Checkpoint || racemode == Game::E_RaceMode_FriendDemand_Checkpoint || racemode == Game::E_RaceMode_Destruction || racemode == Game::E_RaceMode_FriendDemand_Destruction)
		success &= AddAnimation(&poseFormat, &m_pPackAnimation, m_pRoute->PackCameraSequenceSingle);
	else
		success &= AddAnimation(&poseFormat, &m_pPackAnimation, m_pRoute->PackCameraSequence);

	if (!success)										// Don't initialise if any animations are missing
	{
		LOGWARN( Resource::g_Log, "Pack camera animation is missing for this route");
		return(false);
	}

	if (!m_pGenericSequences)
		return(false);


	for(S32 a=0;a<m_pGenericSequences->mSequenceArray.count;a++)
	{
		success &= AddAnimation(&poseFormat, &m_pGenericAnimation[a], m_pGenericSequences->mSequenceArray[a]);
		if (!success)										// Don't initialise if any animations are missing
		{
			LOGWARN( Resource::g_Log, "Generic camera animation #%d is missing", a);
			return(false);
		}

		m_GenericAnimationLooped[a] = m_pGenericSequences->mSequenceArray[a]->m_Animation->m_Loopable;
	}


	m_Pose.Create(poseFormat);


	m_CameraJointIndex = m_Pose.FindJointTransformByName("camera");				// Get indexes for camera info within animations
	m_CameraFovIndex = m_Pose.FindCustomScalarByName("camera.fov");

	if ((m_CameraJointIndex == -1) || (m_CameraFovIndex == -1))
	{
		LOGWARN( Resource::g_Log, "Animated camera can't find the label 'camera'");
		return(false);
	}

	AddSubscriber( ActorMsg::E_DynamicObjMsg_CameraUpdate );					// Update cameras after everything else
	AddSubscriber( ActorMsg::E_DynamicObjMsg_StartAnimatedCamera );				// Request to start an animation
	AddSubscriber( ActorMsg::E_DynamicObjMsg_BlendAnimatedCamera );				// Blend into race cam
	AddSubscriber( ActorMsg::E_DynamicObjMsg_AutoSelectAnimatedCamera );		// Move into clear view of vehicle

	return(true);
}

//=========================================================================

void AnimatedCamera::OnResourceBagLoaded(const C_ResourceBag& bag)
{
	SetPrimed();
}

//=========================================================================

void AnimatedCamera::RequestAnimation(XtHandle<::Actors::AnimatedCameraSequence> anim)
{
	if (!anim)
		return;
	AddToResourceBag(*m_pResourceBag, MakeResourceBagTag(anim->m_Animation), anim->m_Animation);
}

//=========================================================================


bool AnimatedCamera::AddAnimation(Animation::C_PoseFormat *poseFormat, Animation::C_CycleResource** ptr, XtHandle<::Actors::AnimatedCameraSequence> anim)
{
	if (!anim)
		return(false);

	*ptr = m_pResourceBag->GetResource<Animation::C_CycleResource>(MakeResourceBagTag(anim->m_Animation));
	if (!*ptr)
		return(false);
	poseFormat->AddCycleFields((*ptr)->GetCycle());
	return(true);
}


void AnimatedCamera::HandleVehicleVisibility(S32 p_Sequence, S32 p_FocusVehicleIndex)
{
	if (m_FutureFeed)					// Future feed cameras don't affect anything else
		return;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	bool isSplitScreen = false;
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr())
		isSplitScreen = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetID() == Game::E_GameMode_SplitScreen;

	if (pRLC)
	{
		bool showFocus = true;
		bool showOpponents = true;
		if (m_pGenericSequences && p_Sequence >= 0 && p_Sequence < m_pGenericSequences->mSequenceArray.count)
		{
			if (m_pGenericSequences->mSequenceArray[p_Sequence])
			{
				showFocus		= !m_pGenericSequences->mSequenceArray[p_Sequence]->m_HideFocusVehicle;
				showOpponents	= !m_pGenericSequences->mSequenceArray[p_Sequence]->m_HideOpponentVehicles;
			}
		}

		for (S32 iVehicle = 0; iVehicle < (S32)pRLC->GetNumVehicles(); iVehicle++)
		{
			const Game::Actors::C_VehicleActor *pActor = pRLC->GetVehicleActor(iVehicle);
			if (pActor)
			{
				bool show = (iVehicle == p_FocusVehicleIndex) ? showFocus : showOpponents;
				
				if (isSplitScreen && pRLC->IsVehiclePlayerControlled(iVehicle))
					show = true;

				if(iVehicle == p_FocusVehicleIndex)
				{
					if(IsSilentRunningActiveAndRunningOnThisVehicle(iVehicle))
						show = false;
				}

				Game::Actors::C_SetVehicleVisibility visMsg(show, m_ViewportIndex);
				SendMessageToActor(pActor->GetHandle(), &visMsg);
			}
		}
	}
}

const bool AnimatedCamera::IsSilentRunningActiveAndRunningOnThisVehicle(const S32 vehicleIndex)
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	//Return if Silent Running's active
	return levelState.mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities()->GetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Silent_Running);
}

//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool AnimatedCamera::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_DynamicObjMsg_CameraUpdate:						// ======= Camera Update =======
		{
			if (m_Active)
				UpdateCamera(Messaging::MessageCast<ActorMsg::C_CameraUpdateMsg>(pMessage)->GetValue());
			break;
		}

		case ActorMsg::E_DynamicObjMsg_StartAnimatedCamera:
		{
			const ActorMsg::C_StartAnimatedCameraMsg* msg = Messaging::MessageCast<ActorMsg::C_StartAnimatedCameraMsg>(pMessage);

			if (msg->m_Viewport != m_ViewportIndex)				// Ignore this message if it applies to a different viewport than ours
				break;
			if (msg->m_FutureFeed != m_FutureFeed)				// Ignore if the future feed setting is different
				break;

			m_CameraMode = E_ANIMCAMERA_MODE_NORMAL;

			if (msg->m_VehicleIndex == -2)										// Use -2 to specify using old camera position as origin
			{
				AmaxCam::C_CameraSystem& cameraSystem = AmaxCam::CameraSystem();	// Get current chase cam position
				m_LocalOrigin = cameraSystem.VehicleFeed(m_ViewportIndex)->Mat43();
				m_LocalFOV = cameraSystem.VehicleFeed(m_ViewportIndex)->Fov();
			}

			switch(msg->m_Sequence)
			{
				case ::Actors::E_ANIMCAMERASEQ_ROUTEVISTA:
					if (m_pVistaAnimation)
						m_CurrentAnimation.Create(m_pVistaAnimation->GetCycle(), &m_Pose);
					m_Looped = false;
					break;

				case ::Actors::E_ANIMCAMERASEQ_ROUTEPACK:
					if (m_pPackAnimation)
						m_CurrentAnimation.Create(m_pPackAnimation->GetCycle(), &m_Pose);
					m_Looped = false;
					break;

				default:
					if (m_pGenericAnimation[msg->m_Sequence - ::Actors::E_ANIMCAMERASEQ_RACESTART4])			// Index into array of animations
					{
						m_CurrentAnimation.Create(m_pGenericAnimation[msg->m_Sequence - ::Actors::E_ANIMCAMERASEQ_RACESTART4]->GetCycle(), &m_Pose);
						m_Looped = m_GenericAnimationLooped[msg->m_Sequence - ::Actors::E_ANIMCAMERASEQ_RACESTART4];
						HandleVehicleVisibility(msg->m_Sequence - ::Actors::E_ANIMCAMERASEQ_RACESTART4, msg->m_VehicleIndex);
					}
					break;
			}

			m_Paused = msg->m_StartPaused;
			StartAnimation(msg->m_VehicleIndex);
			break;
		}


		case ActorMsg::E_DynamicObjMsg_BlendAnimatedCamera:
		{
			const ActorMsg::C_BlendAnimatedCameraMsg* msg = Messaging::MessageCast<ActorMsg::C_BlendAnimatedCameraMsg>(pMessage);

			if (msg->m_Viewport != m_ViewportIndex)				// Ignore this message if it applies to a different viewport than ours
				break;
			if (msg->m_FutureFeed != m_FutureFeed)				// Ignore if the future feed setting is different
				break;

			m_CameraMode = E_ANIMCAMERA_MODE_BLENDTORACECAM;

			Game::Actors::C_SetVehicleVisibility visMsg(true, msg->m_Viewport);
			::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage(&visMsg);			

			m_BlendDuration = msg->m_Duration;
			m_BlendTime = 0.0f;

			UpdateCamera(0.0f);													// Initialise first frame
			break;
		}

		case ActorMsg::E_DynamicObjMsg_AutoSelectAnimatedCamera:
		{
			const ActorMsg::C_AutoSelectAnimatedCameraMsg* msg = Messaging::MessageCast<ActorMsg::C_AutoSelectAnimatedCameraMsg>(pMessage);

			if (msg->m_Viewport != m_ViewportIndex)				// Ignore this message if it applies to a different viewport than ours
				break;
			if (msg->m_FutureFeed != m_FutureFeed)				// Ignore if the future feed setting is different
				break;

			if ((msg->m_VehicleIndex < 0) || (msg->m_VehicleIndex > 19))		// Need a valid vehicle
				break;

			m_CameraMode = E_ANIMCAMERA_MODE_AUTOSELECT;

//			const GenericCameraOffsets*	offsets = msg->m_Offsets;
//			const GenericCameraOffset*	offset = offsets->mOffsetList[0];

			for(S32 a=0;a<msg->m_Offsets->mOffsetList.count;a++)
			{
				GenericCameraOffset offset = msg->m_Offsets->mOffsetList[a];
				if (ValidCameraPosition(m_CarIndex, Maths::HVec3(offset.mVehicleOffset.vx, offset.mVehicleOffset.vy, offset.mVehicleOffset.vz) ))
				{
					m_VehicleOffset = Maths::HVec3(offset.mVehicleOffset.vx, offset.mVehicleOffset.vy, offset.mVehicleOffset.vz);
					m_CurrentAnimation.Create(m_pGenericAnimation[offset.mSequence - ::Actors::E_ANIMCAMERASEQ_RACESTART4]->GetCycle(), &m_Pose);
					m_Looped = m_GenericAnimationLooped[offset.mSequence - ::Actors::E_ANIMCAMERASEQ_RACESTART4];
					StartAnimation(msg->m_VehicleIndex);
					UpdateCamera(0.0f);													// Initialise first frame
					break;
				}
			}
			break;
		}



		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}

//=========================================================================

void AnimatedCamera::StartAnimation(S32 vehicleindex)
{
	m_Active = true;
	m_AnimTime = 0.0f;
	m_AnimFinishedSent=false;
	m_CarIndex = vehicleindex;


	AmaxCam::C_ShowLevelCameraMsg showmsg;			// Tell director actor to use this camera for this viewport
	showmsg.m_ViewportId = m_ViewportIndex;
	showmsg.m_CameraActor = GetHandle();
	if (m_FutureFeed)
		showmsg.m_WhichFeed = AmaxCam::E_WhichFeed_Future;
	else
		showmsg.m_WhichFeed = AmaxCam::E_WhichFeed_Current;

	if (GetCreationData()->m_Director)
		SendMessageToActor(GetCreationData()->m_Director->GetActorHandle(), &showmsg);

	UpdateCamera(0.0f);													// Initialise first frame
}

//=========================================================================

bool AnimatedCamera::ValidCameraPosition(S32 vehicleindex, Maths::HVec3 localpos)
{
	Maths::HMat43	vehicle_mat;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	if (pRLC)
	{
		const Game::Actors::C_VehicleActor *pActor = pRLC->GetVehicleActor(vehicleindex);
		if (pActor)
			vehicle_mat = pActor->GetMatrix();
	}

	Maths::HVec3 worldpos = vehicle_mat.Translation() + (vehicle_mat.AX() * localpos.X()) + (vehicle_mat.AY() * localpos.Y()) + (vehicle_mat.AZ() * localpos.Z());
	if (!CheckLineOfSight(&worldpos, &vehicle_mat, 1.0f, 0.5f, 2.0f))		// Check against each wheel(ish)
		return(false);
	if (!CheckLineOfSight(&worldpos, &vehicle_mat, -1.0f, 0.5f, 2.0f))
		return(false);
	if (!CheckLineOfSight(&worldpos, &vehicle_mat, 1.0f, 0.5f, -2.0f))
		return(false);
	if (!CheckLineOfSight(&worldpos, &vehicle_mat, -1.0f, 0.5f, -2.0f))
		return(false);

	return(true);
}

//=========================================================================

bool AnimatedCamera::CheckLineOfSight(Maths::HVec3* camerapos, Maths::HMat43* vehiclemat, F32 xoffset, F32 yoffset, F32 zoffset)
{
	Maths::HVec3	collision;
	Maths::HVec3	target = vehiclemat->Translation() + (vehiclemat->AX() * xoffset) + (vehiclemat->AY() * yoffset) + (vehiclemat->AZ() * zoffset);

	if (C_ActorUtils::Instance().CheckCollision(&target, camerapos, &collision))
		return(false);

	return(true);
}

//=========================================================================

void AnimatedCamera::UpdateCamera(F32 deltatime)
{
	m_WorldMatrix.SetIdentity();							// Initialise to world origin

	if ((m_CarIndex >= 0) && (m_CarIndex < 20))			// If valid vehicle index then re-initialise to vehicles position
	{
		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		if (pRLC)
		{
			const Game::Actors::C_VehicleActor *pActor = pRLC->GetVehicleActor(m_CarIndex);
			if (pActor)
				m_WorldMatrix = pActor->GetMatrix();
		}
	}

	if (m_CarIndex == -2)									// Use -2 to specify using old camera position as origin
	{
		m_WorldMatrix = m_LocalOrigin;
		m_CameraFOV = m_LocalFOV;
	}

	if (m_CameraMode == E_ANIMCAMERA_MODE_AUTOSELECT)
	{
		Maths::HMat33	cam_mat;
		Maths::HVec3	lookat = -m_VehicleOffset;
//		Maths::HVec3	lookat = m_VehicleOffset;			// Until camera looking down Z axis correctly
		lookat.Y(lookat.Y() + 1.0f);							// Look at centre of car, 1m up
		lookat.Normalise();
		cam_mat.AZ(lookat);
		cam_mat.AY(Maths::HVec3(0.0f, 1.0f, 0.0f));
		C_ActorUtils::Instance().NormaliseMatXYZ(&cam_mat);			// About world origin

		m_WorldMatrix.Translation(m_WorldMatrix.Translation() + (m_WorldMatrix.AX() * m_VehicleOffset.X()) + (m_WorldMatrix.AY() * m_VehicleOffset.Y()) + (m_WorldMatrix.AZ() * m_VehicleOffset.Z()));
		m_WorldMatrix = cam_mat * m_WorldMatrix;
	}


	if (!m_Paused)
	{
		m_AnimTime += deltatime;							// Offset animated camera position

		if ((m_AnimTime >= m_CurrentAnimation.GetDuration()))
		{
			if ((m_Looped))
				m_AnimTime -= m_CurrentAnimation.GetDuration();
			else
			{
				if (!m_AnimFinishedSent)
				{
					m_AnimFinishedSent=true;

					ActorMsg::C_FinishedAnimatedCameraMsg finishedAnimatedCameraMsg;
					finishedAnimatedCameraMsg.m_FutureFeed=m_FutureFeed;
					finishedAnimatedCameraMsg.m_Viewport=m_ViewportIndex;
					_UI_MESSAGE_FULL( finishedAnimatedCameraMsg );
				}
			}
		}
	}

//	if (m_CameraMode != E_ANIMCAMERA_MODE_AUTOSELECT)			// TEMP TEMP TEMP
	if (m_CurrentAnimation.IsValid() && m_CameraJointIndex >= 0)		// Add in animated position (this camera ALWAYS uses an animation)
	{
		m_Pose.Clear();
		m_CurrentAnimation.GetCycle()->Evaluate(Animation::E_BlendMode_Accumulate, m_AnimTime, m_CurrentAnimation, 0, 1.f, m_Pose, Animation::E_LoopMode_Once);
		m_Pose.Normalise(NULL);
				
		Maths::HMat43 cameraMatrix = m_Pose.GetJointTransform(m_CameraJointIndex).AsMat43();
		
		if (m_CameraFovIndex >= 0)
			m_CameraFOV = m_Pose.GetCustomScalar(m_CameraFovIndex);

		// If we are in a vertical splitscreen, we need to half the FOV of these animated camera, otherwise
		// they will look crazy distorted:
		int viewConfig = AmaxCam::CameraSystem().ViewConfig();
		if (viewConfig == 2)
			m_CameraFOV *= 0.5f;
				
		m_WorldMatrix = cameraMatrix * m_WorldMatrix;
	}

	if (m_CameraMode == E_ANIMCAMERA_MODE_BLENDTORACECAM)
	{
		if (!m_Paused)
			m_BlendTime += deltatime;
		if (m_BlendTime >= m_BlendDuration)
		{
			if (!m_FutureFeed)												// Future feed cameras don't affect anything else
			{
				AmaxCam::C_ShowGameCameraMsg		showmsg;					// Tell director to flip into race camera
				showmsg.m_CameraType = AmaxCam::E_GameCam_VehicleFeed;
				showmsg.m_ViewportId = m_ViewportIndex;			// New
				if (GetCreationData()->m_Director)
					SendMessageToActor(GetCreationData()->m_Director->GetActorHandle(), &showmsg);

				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				Game::Actors::C_VehicleActor *pActor = pRLC->GetVehicleActor(m_CarIndex);
				if (pActor)
				{			
					pActor->SetVisibilityOverride(false);
					pActor->Deghost(g_debugBlendToBumperGhostTime,1.0f);
				}
			}

			m_CameraMode = E_ANIMCAMERA_MODE_NORMAL;						// In case we re-active this camera
			m_Active = false;												// Disable this camera after blending
		}

		AmaxCam::C_CameraSystem& cameraSystem = AmaxCam::CameraSystem();	// Get current chase cam position
		Maths::HMat43 race_cam_pos = cameraSystem.VehicleFeed(m_ViewportIndex)->Mat43();
		F32 target_FOV = cameraSystem.VehicleFeed(m_ViewportIndex)->Fov();

		F32	lerpval = m_BlendTime / m_BlendDuration;						// Normalised blend time
		lerpval = 0.5f - (Maths::Cos(lerpval * Maths::Pi)/2.0f);			// Correct for easing in and out

		Maths::VF32 castval = Load<VF32>(&lerpval);							// Blend the mats using Quaternions
		QuatTrans	oldqtran( MakeQuatTrans(m_WorldMatrix));
		QuatTrans	targqtran(MakeQuatTrans(race_cam_pos));
		QuatTrans	slerpqtran = NearestSlerp( oldqtran, targqtran, castval);
		m_WorldMatrix = MakeMat43( slerpqtran );

		m_CameraFOV = m_CameraFOV + ((target_FOV - m_CameraFOV) * lerpval);	// Blend the camera FOV

		bool isSplitScreen = false;
		if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr())
			isSplitScreen = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetID() == Game::E_GameMode_SplitScreen;

		if (cameraSystem.VehicleFeed(m_ViewportIndex)->CurrentType() == AmaxCam::E_VFCamera_Bumper && 
			m_CameraMode == E_ANIMCAMERA_MODE_BLENDTORACECAM)
		{
			Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
			if (pRLC)
			{
				Game::Actors::C_VehicleActor *pActor = pRLC->GetVehicleActor(m_CarIndex);
				if ((pActor) && (!m_FutureFeed))
				{
					float fDistance = (pActor->GetMatrix().AW() - m_WorldMatrix.AW()).Length();
					if (fDistance <= (float)g_debugVehicleCamBlendMaxGhostPos)
					{
						if (pActor->GetVisibility() && !pActor->Ghosted())
						{
							if (!isSplitScreen) //Problem if you see it from the other view
								pActor->Ghost(g_debugBlendToBumperGhostTime, 0.0f);
							pActor->SetVisibilityOverride(true);
						}
						else if (pActor->GetVehicleModel()->GetAlpha() == 0.0f)
						{
							pActor->Deghost();
							Game::Actors::C_SetVehicleVisibility visMsg(false, m_ViewportIndex);
							SendMessageToActor(pActor->GetHandle(), &visMsg);
						}
					}
				}
			}
		}
	}


	m_Camera->SetFov(m_CameraFOV);							// w DegToRad(m_CameraFOV)
	m_Camera->StartUpdate(deltatime);						// Update camera
	m_Camera->SetPosition(m_WorldMatrix.AW());
	m_Camera->SetOrient((Maths::C_Quaternion)m_WorldMatrix.Rotation());
	m_Camera->EndUpdate();
}



}// End namespace Actors



//#pragma optimize("", on)


