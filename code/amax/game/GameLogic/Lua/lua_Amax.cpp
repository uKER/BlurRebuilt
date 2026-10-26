#include "stdafx.h"
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>	// [Blur2PC]
#include <Game/Game.h>
#include <Game/GameControls/GameControls.h>	// [Blur2PC] retail split-screen keyboard remapping
#include <GameSystems.h>

#include <system/serialise/C_FileSystem.h>
#include <system/serialise/BinarySerialise.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Display/Device/DeviceDebug.h>	// [Blur2PC] Display::g_Resolutions (graphics options)
#include <Display/View/ViewSetup.h>			// [Blur2PC] UI refresh after a graphics options change
#include <ui/Graphics/Simple2d.h>
#include <ui/Game/Gadgets/UICamera.h>
#include <ui/UIDefines.h>
#include <Game/GameWorld/GameWorld.h>
#include <ui/Game/UILua.h>
#include <SafeString.inl>
#include <input\PadVirtual.h>

#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <GameLogic\ActorStudioActors\CutSceneMessages.h>
#include <GameLogic\ActorStudioActors\AnimatedCamera.h>
#include <GameLogic\ActorStudioActors\ActorUtils.h>
#include <GameLogic\ActorStudioActors\ShowRoomCamera.h>
#include <shared_actors\ActorImpl\SplineActor.h>
#include <Shared_Actors\TypeWrappers\SplineVec3Wrapper.h>
#include <GameLogic\ActorStudioActors\UiLocatorActor.h>

#include <RaceBook/RaceBookManager.h>

#include <ui/Game/Gadgets/ButtonController.h>
#include "ui/UIGameDefines.h"
#include "UI/Game/Multiplayer/MultiplayerCache.h"
#include <ui\Game\Profiles\ProfileInterface.h>

#include "Game/GameActors/Vehicles/VehicleActor.h"
#include "Game/GameActors\Components\Messages\AudioMessages.h"
#include "GameAudio/SystemSubsystem.h"
#include "GameAudio/GameAudioLayer.h"
#include <GameLogic/RaceIntroOutro\RaceIntroOutro.h>
#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <Ui/UiInterface.h>
#include <UI/UiMessages.h>
#include <Ui/VideoConfig.h>

#include <Game/Messaging/GameflowMessages.h>

#include "ui\Graphics\movie.h"
#include "Ui\UiLayer.h"
#include "Ui\UiRenderBuffer.h"
#include "Ui/Game/UiPhotoMode.h"
#include "Ui/Game/Gadgets/Hud/UIAiHud.h"
#include "Ui/Game/Gadgets/Hud/UIMultiplayerData.h"
#include "Ui/Game/Gadgets/UISpeakerSetup.h"
#include "Ui/Game/Gadgets/UICheckbox.h"
#include "Ui/Game/Gadgets/Vinyl/UIColourPicker.h"
#include "Ui/Game/Gadgets/UICarousel.h"
#include "Ui/Game/Gadgets/UIChicklets.h"
#include "Ui/Game/Gadgets/UIAmaxSlider.h"
#include "Ui/Game/Gadgets/UILocatorNode.h"
#include "Ui/Game/Gadgets/UIToggle.h"
#include "Ui/UiHelpers.h"
#include "Ui/UiXtConfig.h"
#include "Ui/SoundEffects.h"
#include "Ui/HudDataFinder.h"
#include <ui\Game\UILocaliser.h>


#include <vehicle\VehicleModel.h>

#include "Automation\GamePerfLogging.h"
#include <GameLogic/Structures/MpResults.h>

#include <jpeghelper/JpegDecoder.h> 
#include <jpeghelper/JpegEncoder.h>

#include "Game/AmaxCam/CameraSystem.h"
#include "Game/AmaxCam/PhotoCamera2.h"
#include "AppPhotoCapture.h"

#include "Advertising\Advertising.h"
#include <GameLogic\ScoreboardManager.h>

#include "Automation\GameDataLogging.h"

#include <Game\Achievements\AchievementSystem.h>

#include <Game\GameActors\Components\VehicleLogic\VLC_Damage.h>

#include <GameLogic\Twitter\TwitterManager.h>
#include <GameLogic\Facebook\FacebookManager.h>

#include <Gamelogic\SocialNetworking\SocialNetworkManager.h>

#include <ui/DashboardInterface.h>
#include <AI/AI_Amax.h>

#include <GameLogic/SubtitleManager.h>

#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <GameLogic\ContentServer\KeyArchiveManager.h>
#include <GameLogic\ContentServer\ContentServerManager.h>
#include <GameLogic\ContentServer\RecordEventManager.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

#include <Network/Session.h>
#include <Network/Core/NetServices.h>
#include <Network/Core/NetMatchingParameters.h>
#include <Network/Core/NetRace.h>

#include <GameAudio/GameAudioLayer.h>
#include <GameAudio/SystemSubsystem.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>
#include <GameLogic\FriendsChallenges/MessagingManager.h>

#include <Network\Core\NetScoreboards.h>

#include <GameLogic\ScoreboardManager.h>
#include <GameLogic\AchievementsManager.h>

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>

#include "bdLobby\bdLobbyCommon\bdAuthCommon.h"

#include <GameLogic\SocialNetworking\EventOwnerManager.h>
#ifndef CFG_GOLD
#include <Game\AmaxCam\Helper.h>
#include <render\postprocess\Manager.h>
#endif //CFG_GOLD

#include <ui\Game\Profiles\GamerPictureManager.h>

#ifdef PLATFORM_PS3
#include <PS3Installer/PS3InstallData.h>
#include <cell/l10n.h>
#endif

Debug::Logger &gLUAMessageLogger = Debug::GetLogger("amax.LuaLibrary.messages");

Debug::Bool		g_DebugDevMode("Game.DevMode", true, Debug::Persist());
Debug::Bool		g_DebugPOB("Game.PreviouslyOnBlurEnabled", true, Debug::Persist());

#ifndef CFG_GOLD
Debug::Bool		g_DebugCheatIgnoreEventCarClass( "Cheats.EventIgnoreCarClass", false, Debug::Persist() );
#endif

//Debug::Bool g_PlayRaceCutScene("Game.PlayRaceCutScene", false, Debug::Persist());

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

//From RaceLogicCore.cpp
extern Debug::Bool g_UseDebugHud;
extern Debug::Int g_maxPhysicsUpdates;

#ifndef CFG_GOLD
Debug::Bool g_debugDumpUIResourcesForCutScenes("CutScene.DumpUIResourcesForCutScenes", false, Debug::Persist());
Debug::Bool g_debugOKToDumpDumpResources("CutScene.OKToDumpResources", false);
Debug::Bool g_debugUiHUDShowRevs("ui.ShowRevs", false, Debug::Persist() );
Debug::Bool g_debugUiHUDShowCameraHelpers("ui.ShowCameraHelpers", false );
extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;
#endif

void CheckSinglePlayerStickers();

//From RaceLogicCore.cpp

/*********************************************************************************\
|									DEBUG										  |
\*********************************************************************************/
namespace Game
{
	extern Debug::Bool g_EnableHeadlights;
	//extern Debug::Bool g_debugEnableDropShadows;
	extern Debug::Bool g_DynamicLightEnable;

	extern Debug::Bool g_CubeMapEnable;
	extern Debug::Bool g_ReflectionEnable;
}

namespace DynamicEffect
{
	namespace Particle
	{
		extern Debug::Bool	g_debugParticleQualityOverride;
		extern Debug::Float g_debugParticleQuality;
	}
}

namespace Framework
{
	extern bool g_bShowFps;
}

namespace GameDebug
{
	extern Debug::Bool g_debugMiniMonitorEnable;
}


// Sake!
namespace HDR
{
	extern Debug::Float g_Gamma;
}


extern Debug::Bool g_DebugDisableDebugHud;

extern Debug::Int g_debugShieldsEarnedRepairSinglePlayer;
extern Debug::Int g_debugShieldsEarnedHitByPerkSinglePlayer;
extern Debug::Int g_debugShieldsEarnedFansSinglePlayer;
extern Debug::Int g_debugShieldsEarnedFansRequiredSinglePlayer;

/*********************************************************************************\
|									DEBUG										  |
\*********************************************************************************/


namespace Game
{

static int SendMessage( lua_State* pState )
{
	if( lua_type(pState,1) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump(pState,"SendMessage(msg) - incorrect arguments\n");
		return 0;
	}

	Messaging::TMsgType msg = (Messaging::TMsgType)lua_tonumber(pState,1);

	if( C_GameApp::GetInstance()->LoadingScreenEnabledState() )
	{
		UI::C_UiLayer* pUILayer = C_GameApp::GetInstance()->GetUiLayer();
		if( pUILayer )
			pUILayer->AddDeferredGameMessage( msg );
	}
	else
	{
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( msg );
		RaceBook::C_RaceBookManager::Instance().GetSubscriberSystem()->BroadcastMessage( msg );
	}

	return 0;
}

static int StartAnimatedCamera( lua_State* pState )
{
	if( lua_type(pState,1) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"StartAnimatedCamera(number, [bool], [number], [number]) - incorrect arguments\n");
		return 0;
	}

	ActorMsg::C_StartAnimatedCameraMsg msg;

	msg.m_Sequence = (::Actors::E_ANIMCAMERASEQ_TYPE)lua_tointeger( pState, 1 );
	msg.m_VehicleIndex = -1;
	msg.m_Viewport = 0;
	msg.m_StartPaused = false;
	msg.m_FutureFeed = false;

	// Overide defaults
	if( lua_type( pState, 2 ) == LUA_TBOOLEAN )
		msg.m_StartPaused = (lua_toboolean( pState, 2 ) != 0);

	if( lua_type( pState, 3 ) == LUA_TNUMBER )
		msg.m_VehicleIndex = (S32)lua_tointeger( pState, 3 );

	if( lua_type( pState, 4 ) == LUA_TNUMBER )
		msg.m_Viewport = (S32)lua_tointeger( pState, 4 );

	if( lua_type( pState, 5 ) == LUA_TBOOLEAN )
		msg.m_FutureFeed = (lua_toboolean( pState, 5 ) != 0);


	if( C_GameApp::GetInstance()->LoadingScreenEnabledState() )
	{
		LOGERROR( g_Log, DBSTR "Can't send while loading is active");
		return 0;
	}

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );

	return 0;
}

static int AutoSelectAnimatedCamera( lua_State* pState )
{
	ActorMsg::C_AutoSelectAnimatedCameraMsg	msg;

	msg.m_Offsets = RLDatabase::Instance().mRLSystems[0]->mDefaultCameraOffsets;		//.Object();
	msg.m_VehicleIndex = -1;
	msg.m_Viewport = 0;

	// Overide defaults
	if( lua_type( pState, 1 ) == LUA_TNUMBER )
		msg.m_VehicleIndex = (S32)lua_tointeger( pState, 1 );

	if( lua_type( pState, 2 ) == LUA_TNUMBER )
		msg.m_Viewport = (S32)lua_tointeger( pState, 2 );

	if( C_GameApp::GetInstance()->LoadingScreenEnabledState() )
	{
		LOGERROR( g_Log, DBSTR "Can't send while loading is active");
		return 0;
	}

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );
	return 0;
}

static int RestartCarsToStartPositions( lua_State* pState )
{
	if (::Actors::C_ActorUtils::Instance().gameworld == NULL || ::Actors::C_ActorUtils::Instance().gameworld->GetActorList() == NULL)
		return 0;

	Game::Actors::C_VehicleResetAtSpawnPositionMsg msg(-1);
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );	

	return 0;
}

static int DisableCarControls( lua_State* pState )
{
	if (::Actors::C_ActorUtils::Instance().gameworld == NULL || ::Actors::C_ActorUtils::Instance().gameworld->GetActorList() == NULL)
		return 0;

	Game::Actors::C_VehicleControlsDisableMsg msg(-1);
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );	

	return 0;
}

static int BlendAnimatedCamera( lua_State* pState )
{
	ActorMsg::C_BlendAnimatedCameraMsg msg;

	msg.m_Duration		= 1.0f;
	msg.m_VehicleIndex	= 0;
	msg.m_Viewport		= 0;
	msg.m_FutureFeed	= false;

	if( lua_type( pState, 1 ) == LUA_TNUMBER )
		msg.m_Duration = (F32)lua_tonumber( pState, 1 );

	if( lua_type( pState, 2 ) == LUA_TNUMBER )
		msg.m_VehicleIndex = (S32)lua_tointeger( pState, 2 );

	if( lua_type( pState, 3 ) == LUA_TNUMBER )
		msg.m_Viewport = (S32)lua_tointeger( pState, 3 );

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );
	return 0;
}

static void LuaPushCameraSequence( lua_State* pState, XtHandle< ::Actors::AnimatedCameraSequence > ref )
{
	if( !ref.Object() )
		return;

	Lua::AddDataMember( pState, "time", ref->m_UiStageTime );
	Lua::AddDataMember( pState, "t0px", ref->m_UiTextPosition0.vx );
	Lua::AddDataMember( pState, "t0py", ref->m_UiTextPosition0.vy );
	Lua::AddDataMember( pState, "t1px", ref->m_UiTextPosition1.vx );
	Lua::AddDataMember( pState, "t1py", ref->m_UiTextPosition1.vy );
	Lua::AddDataMember( pState, "px", ref->m_UiLbPosition.vx );
	Lua::AddDataMember( pState, "py", ref->m_UiLbPosition.vy );
	Lua::AddDataMember( pState, "sx", ref->m_UiLbSize.vx );
	Lua::AddDataMember( pState, "sy", ref->m_UiLbSize.vy );
	Lua::AddDataMember( pState, "anim", ref->m_UiLbAnimName );
	Lua::AddDataMember( pState, "forwards", ref->m_UiLbAnimForwards );
}

static int GetAnimatedCameraParams( lua_State* pState )
{
	if( lua_type(pState,1) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"GetAnimatedCameraParams(number) - incorrect arguments\n");
		return 0;
	}

	::Actors::E_ANIMCAMERASEQ_TYPE seq_type = (::Actors::E_ANIMCAMERASEQ_TYPE)lua_tointeger( pState, 1 );

	// Route specific animations
	if( seq_type == ::Actors::E_ANIMCAMERASEQ_ROUTEVISTA || seq_type == ::Actors::E_ANIMCAMERASEQ_ROUTEPACK )
	{
		// Route overide
		U32 route = UI::GetLevelState().mRoute;
		if( lua_type( pState, 2 ) == LUA_TNUMBER)
			route = (U32)lua_tointeger( pState, 2 );

		AmaxRoute*pRoute = C_GameData::Instance().mRoutes[ (GAMEDATA_ID)route ];
		if( !pRoute )
		{
			UI::g_pUILua->StackDump(pState,"GetAnimatedCameraParams(number) - invalid route\n");
			return 0;
		}

		if( seq_type == ::Actors::E_ANIMCAMERASEQ_ROUTEVISTA )
		{
			if( !pRoute->VistaCameraSequence )
				return 0;

			lua_newtable(pState);
			LuaPushCameraSequence( pState, pRoute->VistaCameraSequence );
			return 1;
		}
		else
		if( seq_type == ::Actors::E_ANIMCAMERASEQ_ROUTEPACK )
		{
			if( !pRoute->PackCameraSequence )
				return 0;

			lua_newtable(pState);
			LuaPushCameraSequence( pState, pRoute->PackCameraSequence );
			return 1;
		}

		return 0;
	}
	if( !RLDatabase::Instance().mRLSystems[0] || !RLDatabase::Instance().mRLSystems[0]->mAnimatedCameraSequences )
		return 0;

	S32 index = (S32)( seq_type - ::Actors::E_ANIMCAMERASEQ_RACESTART4 );
	if( index < 0 || index >= RLDatabase::Instance().mRLSystems[0]->mAnimatedCameraSequences->mSequenceArray.count )
		return 0;

	lua_newtable(pState);
	LuaPushCameraSequence( pState, RLDatabase::Instance().mRLSystems[0]->mAnimatedCameraSequences->mSequenceArray[ index ] );
	return 1;
}

static int ChangeUiCamera( lua_State* pState )
{
	if( (lua_type(pState,1) != LUA_TSTRING && lua_type(pState,1) != LUA_TNUMBER) || lua_type(pState,2) != LUA_TNUMBER || lua_type(pState,3) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "ChangeUiCamera( string|number, number, number, [number] ) - incorrect arguments" );
		return 0;
	}

	ActorMsg::C_UICameraControlMsg msg;

	if( lua_type(pState,1) == LUA_TSTRING )
	{
		// Find locator actor
		const char* pName = lua_tostring( pState, 1 );
		if( !pName )
			return 0;

		::Actors::UiLocatorActor *pActor = ::Actors::UiLocatorActor::FindByName( pName );
		if( !pActor )
		{
			LOGWARN( g_Log, DBSTR "Locator actor %s not found", pName );
			return 0;
		}

		msg.m_WorldMat = Store( pActor->GetMatrix() );;
	}
	else
	{
		S32 id = (S32)lua_tonumber( pState, 1 );

		// Find gadget
		UI::C_GadgetBase * pObject = UI::gButtons.Find( id );
		if( !pObject )
		{
			LOGWARN( g_Log, DBSTR "Gadget [id:%d] not found", id );
			return 0;
		}

		msg.m_WorldMat = Store( pObject->Matrix() );
	}

	msg.m_BlendDuration = (F32)lua_tonumber( pState, 2 );
	msg.m_Viewport		= (S32)lua_tointeger( pState, 3 );

	// Optional movement type
	::Actors::E_BLEND_TYPE mt = ::Actors::E_BLEND_TYPE_EASEBOTH;
	if( lua_type( pState, 4 ) == LUA_TNUMBER )
		mt = (::Actors::E_BLEND_TYPE)lua_tointeger( pState, 4 );
	msg.m_MovementType	= mt;

	
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );
	return 0;
}

static int ChangeUiSplineCamera( lua_State* pState )
{
	if( lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "ChangeUiSplineCamera( number, number, [number], [string], [number], [string], [number] ) - incorrect arguments" );
		return 0;
	}

	Game::C_GameWorld* pWorld = UI::GetGameWorld();
	if( !pWorld )
		return 0;

	// Essential params
	const F32 bt	= (F32)lua_tonumber( pState, 1 );
	const S32 vp	= (S32)lua_tointeger( pState, 2 );

	// Optional movement type
	::Actors::E_BLEND_TYPE mt = ::Actors::E_BLEND_TYPE_EASEBOTH;
	if( lua_type( pState, 3 ) == LUA_TNUMBER )
		mt = (::Actors::E_BLEND_TYPE)lua_tointeger( pState, 3 );

	// Param 4 - SplineActorCreate or ActorCreate
	::Actors::C_GameActorCreate* pCreationData = NULL;
	::Actors::UiLocatorActor* pLocatorActor = NULL;

	if( lua_type( pState, 4 ) == LUA_TSTRING )
	{
		const char* pName = lua_tostring( pState, 4 );
		if( !pName )
			return 0;

		// God! Oscar is going to kill me
		if( strcmp( pName, "_ShowRoomCamera" ) == 0 )
		{
			::Actors::ShowRoomCamera* pShowRoomCamera = ::Actors::ShowRoomCamera::GetInstance();
			if( !pShowRoomCamera )
			{
				LOGWARN( g_Log, DBSTR "Showroom actor not found" );
				return 0;
			}
			pCreationData = (::Actors::C_GameActorCreate*)pShowRoomCamera->GetCreationData();
		}
		else
		{
			pLocatorActor = ::Actors::UiLocatorActor::FindByName( pName );
			if( !pLocatorActor )
			{
				LOGWARN( g_Log, DBSTR "Actor %s not found", pName );
				return 0;
			}
		}

	}
	// Param 5 - parent move speed
	F32 ms = 0.0f;
	if( lua_type( pState, 5 ) == LUA_TNUMBER )
		ms = lua_tonumber( pState, 5 );

	// Param 6 - Tracking spline name
	::Actors::C_GameActorCreate* pTargetSpline = NULL;
	if( lua_type( pState, 6 ) == LUA_TSTRING )
	{
		const char* pName = lua_tostring( pState, 6 );
		if( !pName )
			return 0;

		const ::Actors::UiLocatorActor* pTargetActor = ::Actors::UiLocatorActor::FindByName( pName );
		if( !pTargetActor )
		{
			LOGWARN( g_Log, DBSTR "Locator actor %s not found", pName );
			return 0;
		}
		if( !pTargetActor->GetCreationData() || !pTargetActor->GetCreationData()->m_Spline )
		{
			LOGWARN( g_Log, DBSTR "Locator actor %s does not have a spline", pName );
			return 0;
		}

		pTargetSpline = (::Actors::C_GameActorCreate*)pTargetActor->GetCreationData()->m_Spline;
	}
	// Param 7 - target move speed
	F32 ts = 0.0f;
	if( lua_type( pState, 7 ) == LUA_TNUMBER )
		ts = lua_tonumber( pState, 7 );

	// Setup the message
	ActorMsg::C_ChangeCameraBehaviourMsg	msg;

	msg.m_CameraProperties	= RLDatabase::Instance().mRLSystems[0]->mUICameraParameters;
	msg.m_TargetBehaviour	= RLDatabase::Instance().mRLSystems[0]->mUiCameraTarget;
	msg.m_MovementBehaviour	= RLDatabase::Instance().mRLSystems[0]->mUiCameraMovement;
	msg.m_ParentActor		= NULL;
	msg.m_LocalParentSpline	= NULL;
	msg.m_TargetActor		= NULL;
	msg.m_LocalTargetSpline	= NULL;
	msg.m_PitchAngle		= 0;
	msg.m_RollAngle			= 0;

	// Change movement behavior
	if( lua_type( pState, 8 ) == LUA_TNUMBER ) 
		switch( (S32)lua_tointeger( pState, 8 ) )
		{
		case 0:
			msg.m_MovementBehaviour	= RLDatabase::Instance().mRLSystems[0]->mUiLastCameraMovement;
			break;
		case 1:
			msg.m_MovementBehaviour	= RLDatabase::Instance().mRLSystems[0]->mUiGameFeedCameraMovement;
			break;
		case 2:
			msg.m_MovementBehaviour	= RLDatabase::Instance().mRLSystems[0]->mUiAttachedCameraMovement;
			break;
		}

	// Movement
	::Actors::C_GameActorCreate* pParentSplineCreate = NULL;
	if( msg.m_MovementBehaviour )
	{
		switch(msg.m_MovementBehaviour->m_MovementType)
		{
		case Game::E_DYCAM_MOVEMENT_ATTACHED:
			if( pCreationData )
				msg.m_ParentActor = pCreationData;
			break;

		case Game::E_DYCAM_MOVEMENT_SPLINEFOLLOWER:
			{
				if( !pLocatorActor || !pLocatorActor->GetCreationData() || !pLocatorActor->GetCreationData()->m_Spline )
				{
					LOGWARN( g_Log, DBSTR "Locator actor %s does not have a spline", lua_tostring( pState, 6 ) );
					return 0;
				}

				msg.m_ParentActor	= (::Actors::C_GameActorCreate*)pLocatorActor->GetCreationData()->m_Spline;
				pParentSplineCreate	= msg.m_ParentActor;
			}
			break;

		case Game::E_DYCAM_MOVEMENT_LOCALSPLINEFOLLOWER:
			{
				if( !pLocatorActor || !pLocatorActor->GetCreationData() || !pLocatorActor->GetCreationData()->m_Spline )
				{
					LOGWARN( g_Log, DBSTR "Locator actor %s does not have a spline", lua_tostring( pState, 6 ) );
					return 0;
				}
				msg.m_LocalParentSpline	= (::Actors::C_GameActorCreate*)pLocatorActor->GetCreationData()->m_Spline;
				pParentSplineCreate		= msg.m_LocalParentSpline;
			}
			break;
		}
	}

	// Target
	if( msg.m_TargetBehaviour && pTargetSpline )
	{
		::Actors::C_GameActorCreate* pTargetSplineCreate = NULL;
		switch(msg.m_TargetBehaviour->m_TargetType)
		{
		case Game::E_DYCAM_TARGET_SPLINEFOLLOWER:
			msg.m_TargetActor		= pTargetSpline;
			pTargetSplineCreate		= pTargetSpline;
			break;

		case Game::E_DYCAM_TARGET_LOCALSPLINEFOLLOWER:
			msg.m_LocalTargetSpline	= pTargetSpline;
			pTargetSplineCreate		= pTargetSpline;
			break;
		}

		// Set speed same for both
		if( pParentSplineCreate && pTargetSplineCreate )
		{
			Actor::C_ActorHandle parentHandle = pParentSplineCreate->GetActorHandle();
			Actor::C_ActorHandle targetHandle = pTargetSplineCreate->GetActorHandle();

			if( parentHandle && targetHandle )
			{
				const ::Actors::C_SplineActor* pParentSplineActor = Actor::ActorCast_Safe< ::Actors::C_SplineActor>( pWorld->GetActorList()->GetActorPtr( parentHandle ) );
				const ::Actors::C_SplineActor* pTargetSplineActor = Actor::ActorCast_Safe< ::Actors::C_SplineActor>( pWorld->GetActorList()->GetActorPtr( targetHandle ) );
				if( pParentSplineActor && pTargetSplineActor )
				{
					const F32 parentLength = pParentSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();
					const F32 targetLength = pTargetSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();

					if( ms > 0.0f )
					{
						const F32 parentTime = parentLength / ms;
						ts = targetLength / parentTime;
					}
					else
					{
						ts = 0.0f;
					}
				}
			}
		}
	}

	msg.m_TrackingSpeed	= ts;
	msg.m_MovementSpeed	= ms;
	msg.m_BlendTime		= bt;
	msg.m_Blending		= mt;

	// Send the message
	pWorld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );

	return 0;
}

// Manually call the camera update fromw the UI so it can move around when paused.
static int UpdateCameraActor( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "UpdateCameraActor( mode ) - incorrect arguments" );
		return 0;
	}

	ActorMsg::C_CameraUpdateMsg cameraUpdateMsg( (F32)lua_tonumber( pState, 1 ) );
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &cameraUpdateMsg );
	return 0;
}

static int ResetViewpointVisibility( lua_State* pState )
{
 	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	for(U8 i = 0; i < levelState.mNumViewports; i++)
	{
		Actors::C_VehicleActor* pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(i)->GetGraphicalActor(i);
		pActor->SetVisibility(true, -1, true, false);
	}
	
	return 0;
}

static int StartTheRace( lua_State* pState ) 
{
	ActorMsg::C_StartTheRaceMsg msg;

	Game::RaceIntroOutro::Instance().FinishIntro();
	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );			// Pass message on to all actors (especially the vehicle proxys)
	return 0;
}

static int SetGlobalFade( lua_State* pState )
{
	if( lua_type(pState,1) != LUA_TNUMBER )
	{
		LOGINFO( gLUAMessageLogger,"SetGlobalFade( number ) - incorrect arguments\n");
		return 0;
	}

	::Actors::C_ActorUtils::Instance().gameworld->SetGlobalFade( lua_tonumber(pState,1) );
	return 0;
}

static int GetLevelData( lua_State* pState ) 
{
	lua_newtable(pState);
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	Lua::AddDataMember(pState,"City", levelState.mCity);
	Lua::AddDataMember(pState,"Route", levelState.mRoute);

	Lua::AddDataMember(pState,"RaceMode", levelState.mRaceMode);

	Lua::AddDataMember(pState,"RandomCars", levelState.mRandomCars);

	Lua::AddDataMember(pState,"Environment", levelState.mEnvironmentOveride);

	Lua::AddDataMember(pState,"NumberOfCars", levelState.mNumCars);

	Lua::AddDataMember(pState,"Laps", levelState.mLaps);

	Lua::AddDataMember(pState,"NetworkRace", levelState.mNetworkRace);

	//int partyModeValue = levelState.GetSplitScreenSettings().mSplitScreenMode ? 1 : 0;
	//Lua::AddDataMember(pState,"PartyMode", partyModeValue);
	Lua::AddDataMember(pState,"Viewports", levelState.mNumViewports);
	//Lua::AddDataMember(pState,"SplitScreenRaceMode", levelState.GetSplitScreenSettings().mSplitScreenEventXtId);
	//Lua::AddDataMember(pState,"PadPartyModeAI", levelState.GetSplitScreenSettings().meSplitScreenAIDifficulty);

	int handicapValue = levelState.GetMultiplayerSettings().mHandicap ? 1 : 0;
	Lua::AddDataMember(pState,"Handicap", handicapValue);


	for(int a=0;a<MAX_LEVELSTATE_CARS;a++)
	{
		char temp[16];
		Amax::C_SafeString::StringPrintF(temp, lengthof(temp), "Manufacturer%d",a+1);
		Lua::AddDataMember(pState,temp, levelState.mCars[a].mManufacturerID);
	
		Amax::C_SafeString::StringPrintF(temp, lengthof(temp), "Vehicle%d",a+1);
		Lua::AddDataMember(pState,temp, levelState.mCars[a].mCarLocID);

		Amax::C_SafeString::StringPrintF(temp, lengthof(temp), "VehicleType%d",a+1);
		Lua::AddDataMember(pState,temp, levelState.mCars[a].mCarStyle);

		Amax::C_SafeString::StringPrintF(temp, lengthof(temp), "Control%d",a+1);
		Lua::AddDataMember(pState,temp, levelState.mCars[a].mController);

		Amax::C_SafeString::StringPrintF(temp, lengthof(temp), "Character%d",a+1);
		Lua::AddDataMember(pState,temp, levelState.mCars[a].mCharacterID);
	}

	return 1;
}


static int IsNetworkRace( lua_State* pState )
{
	// Probally need to check that levelstate is valid incase the function is called in the startscreen?
	bool isNetworkRace = C_Game::GetInstance()->GetLevelState().mNetworkRace;

	lua_pushboolean( pState, isNetworkRace );

	return 1;
}

static int IsGamePaused( lua_State* pState )
{
	//Is the game paused?
	bool isPaused = C_GameApp::GetInstance()->IsPaused();

	lua_pushboolean( pState, isPaused );

	return 1;
}

static int SetGameMode( lua_State* pState )
{
	if(!lua_isnumber(pState,1) )
	{
		LOGINFO( gLUAMessageLogger,"Amax::SetGameMode(gm) - incorrect arguments\n");
		return 0;
	}

	int gm = (int)lua_tonumber(pState,1);
	C_GameApp::GetInstance()->GetRaceBookLayer()->SetGameMode((E_GameMode)gm);

	return 0;
}

static int GetGameMode( lua_State* pState )
{
	lua_pushnumber(pState,C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetID());
	return 1;
}

static S32 SetPlayMode( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SetPlayMode( mode ) - incorrect arguments" );
		return 0;
	}

	S32 mode = (S32)lua_tonumber( pState, 1 );

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	pLayer->GetGameModePtr()->SetPlayMode( (Game::E_PlayMode)mode );

	return 0;
}

static S32 GetPlayMode( lua_State* pState )
{
	lua_pushnumber( pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetPlayMode() );

	return 1;
}

static int StartScoreboardWrite(lua_State* pState)
{
	C_ScoreboardManager::Instance().StartScoreboardWrite();
	return 0;
}

static int ScoreboardWriteComplete(lua_State* pState)
{
	bool complete = C_ScoreboardManager::Instance().ScoreboardWriteComplete();
	lua_pushboolean(pState, complete);
	return 1;
}

static int StartSPScoreboardWrite(lua_State* pState)
{
	C_ScoreboardManager::Instance().StartSPScoreboardWrite();
	return 0;
}

static int SPScoreboardWriteComplete(lua_State* pState)
{
	bool reTry = false;

	if(lua_isboolean(pState,1))
	{
		reTry = (lua_toboolean(pState,1) != 0) ? true : false;
	}

	bool complete = C_ScoreboardManager::Instance().SPScoreboardWriteComplete( reTry );
	lua_pushboolean(pState, complete);
	return 1;
}

static int SPScooreboardWriteSuccess(lua_State* pState)
{
	int success = 0;
	success = C_ScoreboardManager::Instance().GetWriteSuccess() == true ? 1 : 0;
	lua_pushboolean(pState, success);
	return 1;
}

static int StartScoreboardRead(lua_State* pState)
{
	if(!lua_isnumber(pState,1) )
	{
		LOGINFO( gLUAMessageLogger,"Amax::StartScoreboardRead(offset) - incorrect arguments\n");
		return 0;
	}

	int offset=(int)lua_tonumber(pState,1);

	C_ScoreboardManager::Instance().SetScoreboardRankOffset(offset);
	C_ScoreboardManager::Instance().StartScoreboardRead(C_ScoreboardManager::eScoreboardTypeTop);

	return 0;
}

static int StartLobbyScoreboardRead(lua_State* pState)
{
	if(!lua_isnumber(pState,1) )
	{
		LOGINFO( gLUAMessageLogger,"Amax::StartScoreboardRead(offset) - incorrect arguments\n");
		return 0;
	}

	int offset=(int)lua_tonumber(pState,1);

	C_ScoreboardManager::Instance().SetScoreboardRankOffset(offset);
	C_ScoreboardManager::Instance().StartScoreboardRead(C_ScoreboardManager::eScoreboardTypeLobby);

	return 0;
}

static int StartFriendScoreboardRead(lua_State* pState)
{
	if(!lua_isnumber(pState,1) )
	{
		LOGINFO( gLUAMessageLogger,"Amax::StartFriendScoreboardRead(offset) - incorrect arguments\n");
		return 0;
	}

	int offset=(int)lua_tonumber(pState,1);

	C_ScoreboardManager::Instance().SetScoreboardRankOffset(offset);
	C_ScoreboardManager::Instance().StartScoreboardRead(C_ScoreboardManager::eScoreboardTypeFriends);

	return 0;
}

static int StartYouScoreboardRead(lua_State* pState)
{
	if(!lua_isnumber(pState,1) )
	{
		LOGINFO( gLUAMessageLogger,"Amax::StartFriendScoreboardRead(offset) - incorrect arguments\n");
		return 0;
	}

	int offset=(int)lua_tonumber(pState,1);

	C_ScoreboardManager::Instance().SetScoreboardRankOffset(offset);
	C_ScoreboardManager::Instance().StartScoreboardRead(C_ScoreboardManager::eScoreboardTypeYou);

	return 0;
}

static int ScoreboardReadComplete(lua_State* pState)
{
	bool complete = C_ScoreboardManager::Instance().ScoreboardReadComplete();
	int rows=C_ScoreboardManager::Instance().GetCurrentScoreboardSize();

	S32 sIndexMe = -1;

	SysProfile::FullProfile& profileMe = UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );

	for ( S32 i = 0; i < rows; i++ )
	{
#ifdef PLATFORM_PS3
		bdOnlineUserID id=bdAuthUtility::getUserID(profileMe.m_Uid.handle.data);
		if(id == Game::C_ScoreboardManager::Instance().GetCurrentRowData( i ).m_EntityId)
#else
		if ( (SysProfile::UserIdentifier)Game::C_ScoreboardManager::Instance().GetCurrentRowData( i ).m_Userdata == (SysProfile::UserIdentifier)profileMe )
#endif
		{
			sIndexMe = i;
			break;
		}
	}

	S32 sRankMe = -1;

	if ( sIndexMe != -1 )
		sRankMe = Game::C_ScoreboardManager::Instance().GetCurrentRowData( sIndexMe ).m_Rank;
	
	lua_pushboolean(pState, complete);
	lua_pushnumber(pState, rows);
	lua_pushnumber( pState, ( sIndexMe == -1 ) ? -1 : sIndexMe + 1 );
	lua_pushnumber( pState, sRankMe );
	return 4;
}

static int ScoreboardNumCurrentRows(lua_State* pState)
{
	int rows=C_ScoreboardManager::Instance().GetCurrentScoreboardSize();

	lua_pushnumber(pState, rows);
	
	return 1;
}

static int ScoreboardLookAt(lua_State* pState)
{
	if(!lua_isnumber(pState,1) )
	{
		LOGINFO( gLUAMessageLogger,"Amax::ScoreboardLookAt(sb) - incorrect arguments\n");
		return 0;
	}

	int sb = (int)lua_tonumber(pState,1);
	C_ScoreboardManager::Instance().LookAtScoreboard(sb);
	return 0;
}

const int GAMERANK = 0;
const int TOTALFANS = 1;
const int TOTALTIMEPLAYED = 2;
const int RACEPOINTS = 3;
const int TOPTHREE = 4;
const int TOTALRACES = 5;
const int RACEWINS = 6;
const int POWERUPHITS = 7;
const int POWERUPSFIRED = 8;
const int CARSWRECKED = 9;
const int LEGENDLEVEL = 10;
const int TIMETOACHIEVELEGEND = 11;

static S32 ScoreboardGetRowRank( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"ScoreboardGetRowRank(row) - incorrect arguments");
		return 0;
	}	

	U32 row = (U32)lua_tointeger( pState, 1 );

	if( row > (U32)Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
		return 0;

	lua_pushinteger( pState, Game::C_ScoreboardManager::Instance().GetCurrentRowData(row-1).m_Columns[GAMERANK].m_Data.m_S64 );
	return 1;
}

static S32 ScoreboardGetRowLegend( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"ScoreboardGetRowLegend(row) - incorrect arguments");
		return 0;
	}	

	U32 row = (U32)lua_tointeger( pState, 1 );

	if( row > (U32)Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
		return 0;

	lua_pushinteger( pState, Game::C_ScoreboardManager::Instance().GetCurrentRowData(row-1).m_Columns[LEGENDLEVEL].m_Data.m_S64 );
	return 1;
}

static int IsGameModeNetworked(lua_State* pState)
{
	lua_pushboolean(pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork());
	return 1;
}

static int IsGameModeOnline(lua_State* pState)
{
	lua_pushboolean(pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsOnline());
	return 1;
}

static int IsGameModeRanked(lua_State* pState)
{
	lua_pushboolean(pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsRanked());
	return 1;
}

static S32 IsGameModeMultiplayer( lua_State* pState )
{
	lua_pushboolean(pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer());
	return 1;
}

// IsGameModeSplitScreen
static S32 IsGameModeSplitScreen( lua_State* pState )
{
	lua_pushboolean( pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_SplitScreen );
	return 1;
}

static S32 ShouldDisablePosition( lua_State* pState )
{
	if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_SplitScreen)
	{
		bool AIOK = AIAmax::AmaxAIR::Instance().AllOK();
		lua_pushboolean( pState, (!AIOK) );
	}
	else
	{
		lua_pushboolean( pState, false);
	}

	return 1;
}

// AreAllSplitScreenHumanPlayersWrecked
static S32 AreAllSplitScreenHumanPlayersWrecked( lua_State* pState )
{
	bool allSplitScreenHumanPlayersWrecked = true; 

	// Traverse list of cars in level
	Game::C_LevelState&  levelState	= Game::C_Game::GetInstance()->GetLevelState();
	for(unsigned int i = 0; i < levelState.mNumCars; i++)
	{
		Game::E_VEHICLE_STATE currState = levelState.mCars[i].mState;

		if(levelState.mCars[i].mHuman == true)				
		{
			const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(i);
			if(!pLogicActor->IsFatallyDamaged())
			{
				// If any human car is not wrecked break out early and return result
				allSplitScreenHumanPlayersWrecked = false; 

				// Break out early
				i = levelState.mNumCars;
			}
		}
	}

	// Push onto stack
	lua_pushboolean( pState, allSplitScreenHumanPlayersWrecked );

	// 1 Argument so return 1
	return 1;
}

// AreAllSplitScreenHumanPlayersEliminated
static S32 AreAllSplitScreenHumanPlayersEliminated( lua_State* pState )
{
	bool allSplitScreenHumanPlayersEliminated = true; 

	// Traverse list of cars in level
	Game::C_LevelState&  levelState	= Game::C_Game::GetInstance()->GetLevelState();
	for(unsigned int i = 0; i < levelState.mNumCars; i++)
	{
		Game::E_VEHICLE_STATE currState = levelState.mCars[i].mState;

		if(levelState.mCars[i].mHuman == true)				
		{
			Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(i);
			if(!(pLogicActor->IsWreckedOrEliminated()))
			{
				// If any human car is not eliminated break out early and return result
				allSplitScreenHumanPlayersEliminated = false; 

				// Break out early
				i = levelState.mNumCars;
			}
		}
	}

	// Push onto stack
	lua_pushboolean( pState, allSplitScreenHumanPlayersEliminated );

	// 1 Argument so return 1
	return 1;
}

//Creates an event from the lua table passed in
static int SetupRace(lua_State *pState)
{
	//Setup the race
	C_GameApp::GetInstance()->GetRaceBookLayer()->SetupRace(pState);
	C_Game::GetInstance()->GetLevelState().mDebugEvent = false;
	return 0;
}

static int SetCurrentEvent(lua_State* pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SetCurrentEvent(number) - incorrect arguments\n");
		return 0;
	}

	const XtId eventID = lua_tointeger( pState, 1 );

	// Safety
	AmaxEvent* pEvent = C_GameEventData::Instance().mEvents[ (GAMEDATA_ID)eventID ];
	if( !pEvent )
	{
		LOGERROR( g_Log, "Event id %d not found", eventID );
		return 0;
	}

	if( C_GameProfileManager::Instance().GetPrimaryProfile() )
		C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentEventId = eventID;

	return 0;
}

static int SetCurrentCar(lua_State* pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SetCurrentCar(number) - incorrect arguments\n");
		return 0;
	}

	const int carID = lua_tointeger( pState, 1 );

	// Safety
	AmaxVehicle *pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)carID];
	if( !pVehicle )
	{
		LOGERROR( g_Log, "Vehicle id %d not found", carID );
		return 0;
	}

	// Push into the *old* RaceBook structure for now
	RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mCurrentCarID = carID;
	return 0;
}

static int SetupDebugRace(lua_State *pState)
{
	if(!lua_istable(pState,1) )
	{
		UI::g_pUILua->StackDump( pState, "SetupDebugRace(event_table) - incorrect arguments\n");
		return 0;
	}

	U32 eventID=-1;
	Lua::GetDataMember(pState,"EventID",eventID,true);
	assert(eventID != -1);
	
	C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentEventId = eventID;

	U32 carID=-1;
	Lua::GetDataMember(pState,"Vehicle",carID,true);
	assert(carID != -1);

	if(RaceBook::C_RaceBookManager::Instance().GetPlayerProfile())
		RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mCurrentCarID=carID;

	U32 diff=-1;
	Lua::GetDataMember(pState,"Difficulty",diff,true);
	assert(diff != -1);
	C_GameProfileManager::Instance().GetPrimaryProfile()->SetDifficulty(diff);
	//if(diff == GAME_DIFFICULTY_VERY_EASY)
	//{
	//	C_GameProfileManager::Instance().GetPrimaryProfile()->mOptions.mSteeringAssist=1;
	//	C_GameProfileManager::Instance().GetPrimaryProfile()->mOptions.mBrakingAssist=1;
	//}
	//else if(diff == GAME_DIFFICULTY_EASY)
	//{
	//	C_GameProfileManager::Instance().GetPrimaryProfile()->mOptions.mSteeringAssist=0;
	//	C_GameProfileManager::Instance().GetPrimaryProfile()->mOptions.mBrakingAssist=1;
	//}
	//else
	//{
	//	C_GameProfileManager::Instance().GetPrimaryProfile()->mOptions.mSteeringAssist=0;
	//	C_GameProfileManager::Instance().GetPrimaryProfile()->mOptions.mBrakingAssist=0;
	//}

	C_GameApp::GetInstance()->GetRaceBookLayer()->SetupRace(pState);

	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	U32 pickup_slots=-1;
	U32 shield_slots=-1;
	U32 health_slots=-1;

	Lua::GetDataMember(pState,"PickupSlots",pickup_slots,true);
	Lua::GetDataMember(pState,"ShieldSlots",shield_slots,true);
	Lua::GetDataMember(pState,"HealthSlots",health_slots,true);
	
	for(U32 a=0;a<levelState.mNumCars;a++)
	{
		levelState.mCars[a].mUpgrades.NumPickupSlots(pickup_slots);
		levelState.mCars[a].mUpgrades.Shield(shield_slots ? 1 : 0);
		levelState.mCars[a].mUpgrades.NumShieldSlots(shield_slots);
		levelState.mCars[a].mUpgrades.NumAdditionalHealthSlots(health_slots);
		levelState.mCars[a].mUpgrades.CollectAnyPerkType(true);
		levelState.mCars[a].mUpgrades.ManualPerkChange(true);

		levelState.mCars[a].mUpgrades.DoubleNitro(true);
		levelState.mCars[a].mUpgrades.DoubleShunt(true);
		levelState.mCars[a].mUpgrades.DoubleShock(true);
		levelState.mCars[a].mUpgrades.DoubleBarge(true);
		levelState.mCars[a].mUpgrades.DoubleBomb(true);
	}

	levelState.mDebugEvent = true;
	return 0;
}

static int SetDebugEvent(lua_State* pState)
{
	if ( lua_type( pState, 1 ) == LUA_TBOOLEAN )
		C_Game::GetInstance()->GetLevelState().mDebugEvent = lua_toboolean(pState,1) ? true : false;	

	return 0;
}

static int EnsureVehicleStateValid(lua_State *pState)
{
	((C_RaceModeMultiplayer*) C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr())->EnsureValidVehicleStatesSet();
	return 0;
}

static int SetupResults(lua_State *pState)
{
	//Setup the race
	if(lua_istable(pState,1) )
		C_GameApp::GetInstance()->GetRaceBookLayer()->SetupQuitAndWinResults(pState);
	else
		C_GameApp::GetInstance()->GetRaceBookLayer()->SetupResults();

	return 0;
}

static int SetVehicleByID( lua_State* pState )
{
	if(lua_type( pState, 1) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump( pState, "SetVehicleByID(number) - incorrect arguments\n");
		return 0;
	}

	XtId carID = (XtId)lua_tointeger(pState,1);

	RaceBook::C_RaceBookVehicle *rbv=RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)carID);
	if( rbv )
	{
		RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mCurrentCarID = carID;
	}
	else
	{
		LOGERROR( g_Log, "Car id %x not found\n", carID );
	}

	return 0;
}

static int GetVehicleByID( lua_State* pState )
{
	XtId id_rb = 0;
	XtId id_amax = 0;

	if ( RaceBook::C_RaceBookManager::Instance().GetPlayerProfile() )
	{
		id_rb = RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mCurrentCarID;

		RaceBook::C_RaceBookVehicle* pRaceBookVehicle = RaceBook::C_RaceBookManager::Instance().GetVehicle( (GAMEDATA_ID)id_rb );

		if ( pRaceBookVehicle )
			id_amax = pRaceBookVehicle->mBaseModelID;
	}

	lua_pushinteger( pState, id_rb );
	lua_pushinteger( pState, id_amax );

	return 2;
}

/*---------------------------------------------------------------------------------------
	Debug Hud helper functions.
----------------------------------------------------------------------------------------*/
static int UseDebugHud( lua_State *pState )
{
	lua_pushboolean( pState, g_UseDebugHud );

	return 1;
}

static int GetDebugHudSettings( lua_State *pState )
{
	lua_newtable( pState );

	// Push on debug hud settings.
	Lua::AddDataMember( pState, "ShowLapTime", false );
	Lua::AddDataMember( pState, "ShowSplitTime", false );
	Lua::AddDataMember( pState, "ShowDamageGraphic", false );

	return 1;
}

// GetNumRules
static S32 GetNumRules( lua_State* pState )
{
	S32 sNumPassRules = Game::C_RaceRulesManager::Instance().GetNumPassRules();
	S32 sNumFailRules = Game::C_RaceRulesManager::Instance().GetNumFailRules();

	lua_pushnumber( pState, ( sNumPassRules > 0 ) ? sNumPassRules : sNumFailRules );

	return 1;
}

static S32 HasPassedRule( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "HasPassedRule( num ) - incorrect arguments" );
		return 0;
	}

	S32 sNumPassRules = Game::C_RaceRulesManager::Instance().GetNumPassRules();
	S32 sNumFailRules = Game::C_RaceRulesManager::Instance().GetNumFailRules();

	S32 sIndex = (S32)lua_tonumber( pState, 1 ) - 1;

	if ( sIndex < 0 )
		return 0;

	if (   ( ( sNumPassRules  > 0 ) && ( sIndex >= sNumPassRules ) )
		|| ( ( sNumPassRules == 0 ) && ( sIndex >= sNumFailRules ) ) )
	{
		LOGERROR( gLUAMessageLogger, "HasPassedRule() - index out of range" );
		return 0;
	}

	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	if ( sNumPassRules > 0 )
	{
		lua_pushboolean( pState,  results.mCars[ 0 ].mPassedRule[ sIndex ] );
	}
	else
	{
		lua_pushboolean( pState, !results.mCars[ 0 ].mFailedRule[ sIndex ] );
	}	
	
	return 1;
}

/*---------------------------------------------------------------------------------------
	Quit and Win helper functions.
----------------------------------------------------------------------------------------*/
static int GetPassRuleString(lua_State *pState)
{
	if(!lua_isnumber(pState,1) )
	{
		LOGINFO( gLUAMessageLogger,"Amax::GetPassRuleString(prf) - incorrect arguments\n");
		return 0;
	}

	int rule = (int)lua_tonumber(pState,1);
	
	UI::C_WideString512 str;
	Game::C_RaceRulesManager::Instance().GetPassRuleString(0, str, rule, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);
	
	lua_pushstring(pState,str.AsString().Chars());
	
	return 1;
}

static int GetFailRuleString(lua_State *pState)
{
	if(!lua_isnumber(pState,1) )
	{
		LOGINFO( gLUAMessageLogger,"Amax::GetFailRuleString(prf) - incorrect arguments\n");
		return 0;
	}

	int rule = (int)lua_tonumber(pState,1);

	UI::C_WideString512 str;
	Game::C_RaceRulesManager::Instance().GetFailRuleString(0, str, rule, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);

	lua_pushstring(pState,str.AsString().Chars());

	return 1;
}

static int ResetSubtitle(lua_State *pState)
{
	if(!lua_isnumber(pState,1))
	{
		UI::g_pUILua->StackDump(pState,"ResetSubtitle(num) - incorrect arguments\n");
		return 0;
	}

	UI::C_GadgetBase * object = UI::gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if(!object)
	{
		LOGWARN( g_Log, DBSTR" : Button ID not found\n");
		return 0;
	}
	if( object->GetType() != UI::E_UIButtonType_StaticText )
	{
		LOGWARN( g_Log, DBSTR" : Button not text\n");
		return 0;
	}
	UI::UIStaticText *p = (UI::UIStaticText *)object;

	p->ResetTextPrint();

	return 0;
}

static int FinishIntro(lua_State *pState)
{
	RaceIntroOutro::Instance().FinishIntro();
	return 0;
}

static int FinishOutro(lua_State *pState)
{
	RaceIntroOutro::Instance().FinishOutro();
	return 0;
}

Debug::Float gUIDebugViewportScale( "UI.Viewport.scale", -0.005, Debug::Range( 0.0f, 0.1f ) );
Debug::Float gUIDebugViewportScaleClamp( "UI.Viewport.scale_clamp", 0.05f, Debug::Range( -0.5f, 0.5f ) );
Debug::Float gUIDebugViewportPosX( "UI.Viewport.pos_x_scale", -1.0f, Debug::Range( 0.0f, 8.0f ) );
Debug::Float gUIDebugViewportPosY( "UI.Viewport.pos_y_scale", 2.0f, Debug::Range( 0.0f, 8.0f ) );
Debug::Float gUIDebugViewportAngle( "UI.Viewport.rotation_scale", 0.8f, Debug::Range( 0.0f, 3.14f ) );

// GetViewportVelocities
static S32 GetViewportVelocities( lua_State* pState )
{
	if ( !C_GameApp::GetInstance() || 
		 !C_GameApp::GetInstance()->GetRaceBookLayer() || 
		 !C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() )
		return 0;

	Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	if ( !pCore )
		return 0;

	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetViewportVelocities( player_index ) - incorrect arguments" );
		return 0;
	}

	// get the vehicle
	S32 sLocalPlayerIndex = (S32)lua_tonumber( pState, 1 ) - 1;
	S32 sVehicleIndex = pCore->GetViewportFocusVehicleIndex( sLocalPlayerIndex );

	Game::Actors::C_VehicleActor* pVehicleActor = (Game::Actors::C_VehicleActor*)pCore->GetVehicleActor( sVehicleIndex );

	if ( !pVehicleActor || !pVehicleActor->GetVehiclePhysics() )
		return 0;

	RacingPhysics::Car* pVehicle = pVehicleActor->GetVehiclePhysics();

	Maths::HMat43 mat = RacingPhysics::ConvertHVec( pVehicle->GetAxis() );
	Maths::HVec3  acc = RacingPhysics::ConvertHVec( pVehicle->Acceleration() );
	Maths::HVec3  ang = RacingPhysics::ConvertHVec( pVehicle->AngularAcceleration() );

	mat.AW( Maths::HVec3( 0.0, 0.0, 0.0f ) );
	mat = mat.InverseRotTrans();

	acc = acc * mat;
	ang = ang * mat;

	F32 scale = ( ( acc.Z() - 0.5f ) * gUIDebugViewportScale );
	
	scale = Maths::Clamp<F32>( scale, -gUIDebugViewportScaleClamp, gUIDebugViewportScaleClamp );

	acc.X( Maths::Clamp<F32>( acc.X(), -16.0f, 16.0f ) );
	acc.Y( Maths::Clamp<F32>( acc.Y(), -16.0f, 16.0f ) );
	ang.Z( Maths::Clamp<F32>( ang.Z(), -8.0f, 8.0f ) );

	lua_pushnumber( pState, scale + 1.0f );
	lua_pushnumber( pState, scale + 1.0f );
	lua_pushnumber( pState, acc.X() * gUIDebugViewportPosX );
	lua_pushnumber( pState, acc.Y() * gUIDebugViewportPosY );
	lua_pushnumber( pState, ang.Z() * gUIDebugViewportAngle );

	return 5;
}

// GetRevs01
static S32 ShowRevs( lua_State* pState )
{
#ifdef CFG_GOLD
	lua_pushboolean( pState, 0 );
#else
	lua_pushboolean( pState, g_debugUiHUDShowRevs ? 1 : 0 );
#endif
	return 1;
}

static S32 ShowCameraHelpers( lua_State* pState )
{
#ifdef CFG_GOLD
	lua_pushboolean( pState, 0 );
#else
	lua_pushboolean( pState, g_debugUiHUDShowCameraHelpers ? 1 : 0 );
#endif
	return 1;
}

static S32 GetRevs01( lua_State* pState )
{
	if ( !C_GameApp::GetInstance() || 
		 !C_GameApp::GetInstance()->GetRaceBookLayer() || 
		 !C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() )
		return 0;

	Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	if ( !pCore )
		return 0;

	S32 sLocalPlayerIndex = (S32)lua_tonumber( pState, 1 ) - 1;
	S32 sVehicleIndex = pCore->GetViewportFocusVehicleIndex( sLocalPlayerIndex );

	U32 uRevs = pCore->GetVehicleRevs( sVehicleIndex );
	U32 uMaxRevs = pCore->GetVehicleRedLine( sVehicleIndex );
	F32 fRevs01 = (F32)uRevs / (U32)uMaxRevs;

	lua_pushnumber( pState, fRevs01 );

	return 1;
}

extern Debug::Bool g_debugStreamerBlockLoad;

static int FillStreamer( lua_State* pState )
{
	 AmaxPerfLogging::BeginFillStreamer();

	//C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->LevelSceneManagerUpdate();
	//if (!C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->WaitForStreamer())
	if (g_debugStreamerBlockLoad)
	{
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->LoadAllIntoStreamer();
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( Game::E_GameflowMessage_StreamerFull );		
	}
	else
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( Game::E_GameflowMessage_FillStreamer );		

	return 1;
}

static int WaitForStreamer( lua_State* pState )
{
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() && 
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld())
	{
		lua_pushboolean(pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->WaitForStreamer());
	}
	else
	{
		lua_pushboolean(pState, false);
	}

	return 1;
}

static int StreamerBlocksLoad( lua_State* pState )
{
	lua_pushboolean(pState, g_debugStreamerBlockLoad);

	return 1;
}

static int RenderUIScene( lua_State* pState )
{
	bool renderUIScene = false;
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetUIWorldLayer() &&
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetUIWorldLayer()->CanRender())
	{
		renderUIScene = true;
	}

	lua_pushboolean(pState, renderUIScene);

	return 1;
}

static int ChangeEnvironmentSettings( lua_State* pState )
{
	if( lua_type(pState,1) != LUA_TBOOLEAN )
	{
		UI::g_pUILua->StackDump(pState,"ChangeEnvironmentSettings(bool) - incorrect arguments\n");
		return 0;
	}

	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->CreateEnvironmentSettingsFromSorcery(lua_toboolean(pState,1) ? true : false);

	return 0;
}

// TS : Debug cutscene mode stuff
static int DebugCutSceneStart( lua_State* pState )
{
	// Type check stack variables here
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		LOGWARN( g_Log, "Invalid params : DebugCutSceneStart(number)\n");
		return 0;
	}

	// Pop params from lua off the stack
	F32 my_number = (F32)lua_tonumber( pState, 1 );

	return 0; // No variables pushed onto stack to return
}

static int DebugCutSceneEnd( lua_State* pState )
{
	F32 my_number = 1.0f;
	bool my_bool = false;
	
	lua_pushnumber( pState, my_number );
	lua_pushboolean( pState, my_bool ? 1 : 0 );

	return 2; // Number of variables pushed on stack to return to lua
}

/*********************************************************************************\
|									DEBUG										  |
\*********************************************************************************/
static int GetSetDebugDevMode(lua_State* pState)
{
	if ( lua_type( pState, 1 ) == LUA_TBOOLEAN )
	{
		g_DebugDevMode=lua_toboolean(pState,1) ? true : false;
	}

	lua_pushboolean(pState,g_DebugDevMode);

	if(g_DebugDevMode)
	{
		GameDebug::g_debugMiniMonitorEnable=true;
		g_DebugDisableDebugHud=false;
	}
	else
	{
		GameDebug::g_debugMiniMonitorEnable=false;
		g_DebugDisableDebugHud=true;
	}

	return 1;
}

static int ShouldPlayCutScene(lua_State* pState)
{
	lua_pushboolean(pState,false);
		
	return 1;
}

static int ParseHDDForReplays(lua_State* pState)
{
#ifndef CFG_GOLD
	C_ReplayDirector::Instance().ScanHDDForReplays();
#endif
	return 0;
}

static int GetReplayFilesFromHDD(lua_State* pState)
{
#ifndef CFG_GOLD
	lua_newtable(pState);
	int index=1;
	for(S32 i = 0; i < C_ReplayDirector::Instance().GetNumScannedReplays(); i++)
	{
		lua_newtable(pState);

		Lua::AddDataMember(pState,"name",(char *)C_ReplayDirector::Instance().GetScannedReplayStringFromIndex(i));
		Lua::AddDataMember(pState,"id",i);

		lua_rawseti(pState,-2,index);
		index++;
	}
#endif // CFG_GOLD

	return 1;
}

static int SetLoadReplayIndex(lua_State* pState)
{
#ifndef CFG_GOLD
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SetReplayIndex(num) - incorrect arguments\n");
		return 0;
	}

	int replay_index=(int)lua_tonumber(pState,1);
	bool ok = C_ReplayDirector::Instance().LoadReplayFromIndex(replay_index);
	lua_pushboolean(pState, ok);
#endif // CFG_GOLD

	return 1;
}

static int SetConvertReplayIndex(lua_State* pState)
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SetReplayIndex(num) - incorrect arguments\n");
		return 0;
	}

	int replay_index=(int)lua_tonumber(pState,1);
	bool ok = false;//C_ReplayDirector::Instance().ConvertBigEndianCutsceneReplayToLittleEndian(replay_index);
	lua_pushboolean(pState, ok);

	return 1;
}

static int SetEditCutsceneIndex(lua_State* pState)
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SetReplayIndex(num) - incorrect arguments\n");
		return 0;
	}

#ifndef CFG_GOLD
	int replay_index=(int)lua_tonumber(pState,1);
	bool ok = C_ReplayDirector::Instance().LoadCutsceneForEditing(replay_index);
	lua_pushboolean(pState, ok);
#endif // CFG_GOLD

	return 1;
}

static int DebugLoadReplay()
{
#ifndef CFG_GOLD
	if(C_ReplayDirector::Instance().Debug_LoadReplay())
		return 0;
	else
#endif // CFG_GOLD
		return 1;
}

static int CreateUiResources(lua_State* pState)
{
	_UI_MESSAGE( UI::E_UiGameMsg_UiCreateResources );
	
	return 0;
}

static int DestroyUiResources(lua_State* pState)
{
	_UI_MESSAGE( UI::E_UiGameMsg_UiDestroyResources );

	return 0;
}

static S32 GetNumVehicles( lua_State* pState )
{
	if ( !C_GameApp::GetInstance() ||
		 !C_GameApp::GetInstance()->GetRaceBookLayer() ||
		 !C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() ||
		 !C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore() )
	{
		lua_pushnumber( pState, 0 );
	}
	else
	{
		lua_pushnumber( pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetNumVehicles() );
	}

	return 1;
}

static int GetCurrentDebugEventID( lua_State* pState )
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	const char* kDebugEventFile = "debug_event.dat";
	if (!fs.FileExists(kDebugEventFile))
	{
		lua_pushnumber(pState,0);
		return 1;
	}
	Sys::I_File* file = fs.Open(kDebugEventFile);
	if (!file)
	{
		lua_pushnumber(pState,0);
		return 1;
	}

	Sys::BinaryDeserialiserAutoEndian deserial(*file);

	U32 eventid=0;
	U32 vehicleID=0;
	U32 diffID=0;
	U32 pickup_slots=0;
	U32 shield_slots=0;
	U32 health_slots=0;

	deserial >> eventid >> vehicleID >> diffID >> pickup_slots >> shield_slots >> health_slots;

	delete file;
	file = 0;

	lua_pushinteger(pState,eventid);
	lua_pushinteger(pState,vehicleID);
	lua_pushinteger(pState,diffID);
	lua_pushinteger(pState,pickup_slots);
	lua_pushinteger(pState,shield_slots);
	lua_pushinteger(pState,health_slots);

	return 6;
};

static int SetCurrentDebugEventID( lua_State* pState )
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	if(lua_type( pState, 1) != LUA_TNUMBER || lua_type( pState, 2) != LUA_TNUMBER || lua_type( pState, 3) != LUA_TNUMBER
		|| lua_type( pState, 4) != LUA_TNUMBER || lua_type( pState, 5) != LUA_TNUMBER || lua_type( pState, 6) != LUA_TNUMBER)
	{
		LOGINFO( gLUAMessageLogger,"Amax::SetCurrentDebugEventID(number,number,number,number,number,number) - incorrect arguments\n");
		return 0;
	}

	U32 eventID = (U32)lua_tointeger(pState,1);
	U32 vehicleID = (U32)lua_tointeger(pState,2);
	U32 diffID = (U32)lua_tointeger(pState,3);
	U32 pickup_slots= (U32)lua_tointeger(pState,4);
	U32 shield_slots = (U32)lua_tointeger(pState,5);
	U32 health_slots = (U32)lua_tointeger(pState,6);

	Sys::C_OpenParams params;
	params.m_IsCritical = false;
	Sys::I_File* file = fs.Open("debug_event.dat", Sys::E_FileAccess_Write, params);
	if (file == 0)
		return 0;

	Sys::BinarySerialiserAutoEndian serial(*file);

	serial << eventID << vehicleID << diffID << pickup_slots << shield_slots << health_slots;

	delete file;
	file = 0;

	return 0;
};

static int ChangeText( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || ( lua_type( pState, 2 ) != LUA_TNUMBER && lua_type( pState, 2 ) != LUA_TSTRING ) )
	{
		UI::g_pUILua->StackDump( pState, "ChangeText( ID, [IDS|string] ) - incorrect arguments" );
		return 0;
	}
	
	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if ( !pObject )
	{
		LOGWARN( UI::g_Log, DBSTR " : Button ID not found" );
		return 0;
	}

	S32 type = pObject->GetType();

	if( type == UI::E_UIButtonType_StaticText || 
		type == UI::E_UIGameButtonType_CutoutText || 
		type == UI::E_UIGameButtonType_ReinforceText || 
		type == UI::E_UIGameButtonType_Checkbox )
	{
		UI::UIStaticText* pStaticText = (UI::UIStaticText*)pObject;

		if ( lua_type( pState, 2 ) == LUA_TNUMBER )
			pStaticText->ChangeText( lua_tonumber( pState, 2 ) );
		else if ( lua_type( pState, 2 ) == LUA_TSTRING )
			pStaticText->ChangeText( lua_tostring( pState, 2 ) );
	}
	else
	if ( type == UI::E_UIGameButtonType_AmaxSlider )
	{
		if ( lua_type( pState, 2 ) == LUA_TNUMBER )
			((UI::UIAmaxSlider*)pObject)->ChangeText( lua_tonumber( pState, 2 ) );
	}
	else
	if ( type == UI::E_UIGameButtonType_Toggle )
	{
		if ( lua_type( pState, 2 ) == LUA_TNUMBER )
			((UI::UIToggle*)pObject)->ChangeTitle( lua_tonumber( pState, 2 ) );
	}
	else
	{
		UI::g_pUILua->StackDump(pState,"UIButtons.ChangeText() : You are trying to change the text of a button that doesn't have any.");
	}

	return 0;
}

// ResolveAiHud
static S32 ResolveAiHud( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "ResolveAiHud( id, id ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );
	UI::C_GadgetBase* pParentClone = UI::gButtons.Find( (U32)lua_tointeger( pState, 2 ) );

	if ( !pObject || !pParentClone )
		return 0;

	if ( pObject->GetType() != UI::E_UIGameButtonType_AiHud )
	{
		LOGDEBUG( UI::g_Log, DBSTR "Incorrect gadget type" );
		return 0;
	}

	((UI::UIAiHud*)pObject)->Resolve( pParentClone );

	return 0;
}

// SetChickletsValue
static S32 SetChickletsValue( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SetChickletsValue( id, value ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if ( !pObject || ( pObject->GetType() != UI::E_UIGameButtonType_Chicklets ) )
		return 0;

	((UI::UIChicklets*)pObject)->SetValue( (F32)lua_tonumber( pState, 2 ) );

	return 0;
}

static S32 PlayPostEventCheer( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "PlayPostEventCheer( id ) - incorrect arguments" );
		return 0;
	}

/*
	RaceBook::C_RaceBookProfile *prof = UI::GetPlayerProfile( (S32)lua_tonumber( pState, 1 ) );
	if( !prof )
		return 0;

	AmaxEvent *ev = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentEvent();
	if( !ev )
		return 0;


	int diff = prof->mNumFans - prof->mNumPreviousFans;
	int range = ev->mMaximumFansAwarded - ev->mMinimumFansAwarded;
	if( range <= 0 )
		return 0;

	UI::E_UiSoundEffect sfx = UI::E_UiSoundEffect_Unknown;

	if( diff < range / 3 )
		sfx = UI::E_UiSoundEffect_FansSmallCheerActionIn;
	else
	if( diff < ((range * 2) / 3) )
		sfx = UI::E_UiSoundEffect_FansMidCheerActionIn;
	else
		sfx = UI::E_UiSoundEffect_FansLargeCheerActionIn;

	UI::UiInterface()->PlaySound( sfx );
*/

	return 0;
}

static int PlayVoiceMailMessage( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "PlayVoiceMailMessage( number, number ) - incorrect arguments" );
		return 0;
	}

	// Get voice mail dialogue record
	UI::C_UiXtConfig* pUiConfig = UI::C_UiXtConfig::Instance();
	if ( !pUiConfig || !pUiConfig->m_VoiceMailDialogueRef )
		return 0;

	// Get loc id to match up
	XtU32 loc_id = 0;
	if ( lua_type( pState, 2 ) == LUA_TNUMBER )
	{
		// Get from message
		const U32 uId = (U32)lua_tointeger( pState, 1 );
		const U32 uIndex = (U32)lua_tointeger( pState, 2 );
	}
	else
	{
		// Just get from text id
		loc_id = (XtU32)lua_tointeger( pState, 1 );
	}

	for( int j = 0; j < pUiConfig->m_VoiceMailDialogueRef->items.count; j++ )
		if( pUiConfig->m_VoiceMailDialogueRef->items[ j ]->locId.id == loc_id ) 
		{
			U32 split = pUiConfig->m_VoiceMailDialogueRef->items[ j ]->splitIndex;

			ActorMsg::C_AudioPlayUIDialogue msg( split );
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );

			lua_pushinteger( pState, split );
			return 1;
		}

	return 0;
}

static int StopVoiceMailMessage( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "StopVoiceMailMessage( number ) - incorrect arguments" );
		return 0;
	}

	const U32 split = (U32)lua_tointeger( pState, 1 );

	ActorMsg::C_AudioStopUIDialogue msg( split );
	Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );

	return 0;
}

// GetMessagePopup
static S32 GetMessagePopup( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetMessagePopup( id, index ) - incorrect arguments" );
		return 0;
	}

	const U32 uId = (U32)lua_tointeger( pState, 1 );
	const U32 uIndex = (U32)lua_tointeger( pState, 2 );

	return 0;
}

static int GetCurrentEvent( lua_State* pState )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	AmaxEvent* pEvent = C_GameEventData::Instance().mEvents[ (GAMEDATA_ID)levelState.mEvent ];

	lua_newtable( pState );

	Lua::AddDataMember( pState, "icon", pEvent->route->city->icon_name );
	Lua::AddDataMember( pState, "name", pEvent->loc_string.id );

	return 1;
}

static S32 GetRaceRewards( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetRaceRewards( id ) - incorrect arguments" );
		return 0;
	}

	const Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor( (S32)lua_tonumber( pState, 1 ) );
	S32 count = 0;
	S32 fans = 0;

	if ( pLogicActor )
	{
		//this needs to be added to the new scoring if the whizzy score at the end of race is needed again
		//const Components::C_VehicleComponent_Scoring* pScoring = (Components::C_VehicleComponent_Scoring*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Scoring);
		//count = pScoring->AllScoresBuffer().Size();
		//fans = pScoring->GetCurrentXP();
	}

	lua_newtable( pState );
	Lua::AddDataMember( pState, "count", count );
	Lua::AddDataMember( pState, "fans", fans );

	return 1;
}


static S32 GetPlayerVehicleIndex( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetPlayerVehicleIndex( player_index ) - incorrect arguments" );
		return 0;
	}

	lua_pushnumber( pState, UI::GetPlayerVehicleIndex( (S32)lua_tonumber( pState, 1 ) ) );

	return 1;
}

// GetCurrentCarClass
static S32 GetCurrentCarClass( lua_State* pState )
{
#ifndef CFG_GOLD
	if ( g_DebugCheatIgnoreEventCarClass )
	{
		lua_pushnumber( pState, (S32)AmaxCarClass_ANY );
		return 1;
	}
#endif

	if ( !RaceBook::C_RaceBookManager::Instance().GetPlayerProfile() )
		return 0;

	RaceBook::C_RaceBookVehicle* pVehicle = RaceBook::C_RaceBookManager::Instance().GetVehicle( (GAMEDATA_ID)RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mCurrentCarID );

	if ( !pVehicle )
		return 0;

	AmaxVehicle* pVehicleAM = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)pVehicle->mBaseModelID ];

	if ( !pVehicleAM )
		return 0;

	lua_pushnumber( pState, (S32)pVehicleAM->carClass );

	return 1;
}

static S32 GetCurrentCarID( lua_State* pState )
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	lua_pushinteger( pState, levelState.mCars[0].mCarID );
	return 1;
}

static S32 CanSpectateVehicle( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "CanSpectateVehicle( index ) - incorrect arguments" );
		return 0;
	}
	
	Game::C_RaceLogicLayer& rll = *C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	Actors::C_VehicleLogicActor* pLogicActor = rll.GetLogicVehicleActor((S32)lua_tonumber( pState, 1 ));

	bool stillRacing = false;
	if(pLogicActor)
	{
		stillRacing = C_MpResults::Instance().m_LiveVehicleData[pLogicActor->GetVehicleIndex()].m_State == E_VEHICLE_STATE_RACING;
	}
	
	if(pLogicActor == NULL || (pLogicActor && pLogicActor->IsWreckedOrEliminated()) || (!stillRacing) )
		lua_pushboolean(pState, false);
	else
		lua_pushboolean(pState, true);

	return 1;
}

static S32 SpectatedVehicleIsEliminated( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "CanSpectateVehicle( index ) - incorrect arguments" );
		return 0;
	}


	bool eliminated = false;
	S32 vehicleIndex = (S32)lua_tonumber( pState, 1 );
	eliminated = C_MpResults::Instance().m_LiveVehicleData[vehicleIndex].m_State == E_VEHICLE_STATE_ELIMINATED;

	lua_pushboolean(pState, eliminated);

	return 1;
}

static S32 SpectateVehicle( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SpectateVehicle( index ) - incorrect arguments" );
		return 0;
	}

	Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	if ( pCore )
	{
		pCore->SetViewportFocusVehicleIndex( 0, (S32)lua_tonumber( pState, 1 ) );
	}
	return 0;
}


static S32 GetSpectateVehicleData( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SpectateVehicle( index ) - incorrect arguments" );
		return 0;
	}
	
	const S32 index = lua_tonumber( pState, 1 );
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	Btl::Vector<int>& loadOuts = levelState.mCars[index].mLoadOuts;

	lua_newtable( pState );

	Lua::AddDataMember( pState, "rank",		levelState.mCars[index].mMultiplayerRank );
	Lua::AddDataMember( pState, "legend",	levelState.mCars[index].mMultiplayerLegendLevel );
	Lua::AddDataMember( pState, "join_ref",	levelState.mCars[index].mNetJoinId );
	Lua::AddDataMember( pState, "team",		levelState.mCars[index].mTeam );

	// For AI drivers
	Lua::AddDataMember( pState, "ai_driver", ( levelState.mCars[index].mAiAvatarId == -1 ) ? false : true );
	Lua::AddDataMember( pState, "ai_avatar_id", levelState.mCars[index].mAiAvatarId );

	// push mods
	lua_pushstring( pState, "mods" );
	lua_newtable( pState );
		for ( S32 i = 0; i < loadOuts.Size(); i++ )
		{
			const MultiplayerAbilityData* pAbility = C_MultiplayerData::Instance().FindAbility( (UI::E_MultiplayerVehicleAbilities)loadOuts[ i ] );
			if ( !pAbility )
				continue;

			lua_newtable( pState );
				Lua::AddDataMember( pState, "type",		pAbility->mAbilityId );
				Lua::AddDataMember( pState, "category", pAbility->mAbilityCategory );
			lua_rawseti( pState, -2, i+1 );
		}
	lua_rawset( pState, -3 );

	return 1;
}

static S32 ToggleIngameScoreboard( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"ToggleIngameScoreboard(id) - incorrect arguments");
		return 0;
	}	

	UI::C_GadgetBase* pGadget = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if ( pGadget && pGadget->GetType() == UI::E_UIGameButtonType_MultiplayerData )
		((UI::UIMultiplayerData*)pGadget)->ToggleScoreboard();

	return 0;
}

static S32 GetWreckerModLoadout( lua_State* pState )
{
	//return that loadout
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetWreckerModLoadout( player_index ) - incorrect arguments" );
		return 0;
	}

	Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor( UI::GetPlayerVehicleIndex( (S32)lua_tonumber( pState, 1 ) ) );
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
		
	//find the wrecker
	S32 wreckerVehIndex = -1;
	if(pLogicActor)
	{
		Components::C_VehicleComponent_NewScoring* pScoringComponent = (Components::C_VehicleComponent_NewScoring*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_NewScoring);
		
		if(pScoringComponent && pScoringComponent->WreckedByAnotherPlayer())
		{
			wreckerVehIndex = pScoringComponent->GetIndexOfVehicleThatWreckedUs();
		}
	}
	
	//get his loadout
	if( wreckerVehIndex != -1 && wreckerVehIndex != Game::SYSTEM_FIRING_INDEX)
	{
		lua_newtable( pState );

		Lua::AddDataMember( pState, "wrecker",	wreckerVehIndex );

		Btl::Vector<int>& loadOuts = levelState.mCars[wreckerVehIndex].mLoadOuts;
		// push mods
		lua_pushstring( pState, "mods" );
		lua_newtable( pState );
			for ( S32 i = 0; i < loadOuts.Size(); i++ )
			{
				const MultiplayerAbilityData* pAbility = C_MultiplayerData::Instance().FindAbility( (UI::E_MultiplayerVehicleAbilities)loadOuts[ i ] );
				if ( !pAbility )
					continue;

				lua_newtable( pState );
					Lua::AddDataMember( pState, "type",		pAbility->mAbilityId );
					Lua::AddDataMember( pState, "category", pAbility->mAbilityCategory );
				lua_rawseti( pState, -2, i+1 );
			}
		lua_rawset( pState, -3 );

		return 1;
	}

	return 0;
}

static S32 EndWreckedEffect( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "EndWreckedEffect( player_index ) - incorrect arguments" );
		return 0;
	}

	Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor( UI::GetPlayerVehicleIndex( (S32)lua_tonumber( pState, 1 ) ) );

	pLogicActor->BroadcastMessage( E_VehicleMessage_StopWreckedEffect );

	return 0;
}

static S32 IsMpTimeoutActive( lua_State* pState )
{
	Game::C_RaceModeBase* pRaceMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();

	bool result = false;
	if ( pRaceMode->GetRaceTimeout() > 0.0f )
		result = true;

	lua_pushboolean( pState, result );

	return 1;
}

static int StartPhotoCam( lua_State* pState )
{
	AmaxCam::CameraSystem().StartPhotoCamera();
	C_InGameSpecialFXManager::Instance().StartPhotoMode();
	return 0;
}

static int EndPhotoCam( lua_State* pState )
{
	AmaxCam::CameraSystem().StopPhotoCamera();
	C_InGameSpecialFXManager::Instance().EndPhotoMode();
	return 0;
}

// LockPhotoCamControls
static S32 LockPhotoCamControls( lua_State* pState )
{
	bool bLockControls = ( lua_type( pState, 1 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 1 ) != 0 ) : false;

	AmaxCam::C_PhotoCamera2* pc2 = AmaxCam::CameraSystem().PhotoCamera2();
	if (pc2)
		pc2->EnableControls(!bLockControls);
	return 0;
}

// SetPhotoFilters
static S32 SetPhotoFilters( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TTABLE || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SetPhotoFilters( table, id ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 2 ) );

	if ( !pObject || ( pObject->GetType() != UI::E_UIGameButtonType_ColourPicker ) )
		return 0;

	UI::C_UIColour hintColour = ((UI::UIColourPicker*)pObject)->GetSelected();

	lua_settop( pState, 1 );

	F32 fBrightness =		Lua::GetDataMember<F32>( pState, "brightness" );
	F32 fContrast =			Lua::GetDataMember<F32>( pState, "contrast" );
	F32 fSaturation = 		Lua::GetDataMember<F32>( pState, "saturation" );
	F32 fHintAmount = 		Lua::GetDataMember<F32>( pState, "hint_amount" );
	F32 fShutterAngle =		Lua::GetDataMember<F32>( pState, "shutter_angle" );
	F32 fMaxShutterAngle =	Lua::GetDataMember<F32>( pState, "max_shutter_angle" );
	F32 fIDepth =			Lua::GetDataMember<F32>( pState, "idepth" );
	F32 fAperture =			Lua::GetDataMember<F32>( pState, "aperture" );
	
	lua_pop( pState, 1 );

	UI::C_UiPhotoMode::Instance().SetBrightness( fBrightness );
	UI::C_UiPhotoMode::Instance().SetContrast( fContrast );
	UI::C_UiPhotoMode::Instance().SetSaturation( fSaturation );
	UI::C_UiPhotoMode::Instance().SetHintColour( hintColour );
	UI::C_UiPhotoMode::Instance().SetHintAmount( fHintAmount );
	UI::C_UiPhotoMode::Instance().SetShutterAngle( fShutterAngle * 0.5f / fMaxShutterAngle ); // 0 to 1/60 sec shutter speed
	UI::C_UiPhotoMode::Instance().SetIDepth( fIDepth );
	UI::C_UiPhotoMode::Instance().SetAperture( fAperture );

	return 0;
}

static int TakePhoto( lua_State* pState )
{
	S32 samples = 0;
	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
		samples = (S32)lua_tonumber( pState, 1 );

	bool hideCapture = true;
	if ( lua_type( pState, 2 ) == LUA_TBOOLEAN )
		hideCapture = (lua_toboolean( pState, 2 ) != 0);

	bool forceDisableUI = false;
	if ( lua_type( pState, 3 ) == LUA_TBOOLEAN )
		forceDisableUI = (lua_toboolean( pState, 3 ) != 0);

	PhotoMode::AppPhotoCapture().TakePhoto( samples, hideCapture, false, forceDisableUI );

	return 0;
}

// GetDrivingAssists
static S32 GetDrivingAssists( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetDrivingAssists( localPlayerIndex ) - incorrect arguments" );
		return 0;
	}

	S32 sLocalPlayerIndex = (S32)lua_tonumber( pState, 1 ) - 1;
	S32 sVehicleIndex = UI::GetPlayerVehicleIndex( sLocalPlayerIndex );

	bool bDrivingAssist = false;
	bool bBrakingAssist = false;

	Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor( sVehicleIndex );

	if ( pLogicActor )
	{
		Game::Actors::C_VehicleActor* pVehicleActor = pLogicActor->GetGraphicalActor( sVehicleIndex );

		if ( pVehicleActor )
		{
			bDrivingAssist = pVehicleActor->m_corneringAssist;
			bBrakingAssist = pVehicleActor->m_brakeAssist;
		}
	}

	lua_pushboolean( pState, bDrivingAssist );
	lua_pushboolean( pState, bBrakingAssist );

	return 2;
}

S32 GetNumViewports( lua_State* pState )
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	
	lua_pushnumber( pState, levelState.mNumViewports );

	return 1;
}

S32 SetNumViewports( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SetNumViewports( S32 ) - incorrect arguments" );
		return 0;
	}

	if( !UI::GetAmaxWorldLayer() )
		return 0;

	if( !UI::GetAmaxWorldLayer()->GetGameWorld() )
		return 0;

	S32 numViews = (S32)lua_tonumber( pState, 1 );

	UI::GetAmaxWorldLayer()->GetGameWorld()->SetNumViews(numViews);

	return 0;
}

S32 GetPlayerToBeEliminated( lua_State* pState )
{
	Game::C_RaceLogicLayer& rll = *C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	int lastIndex =-1;
	int lastPos =-1;
	int firstIndex =-1;
	int firstPos =-1;

	int numVehicles = rll.GetNumVehicles();
	for(int i=0;i<numVehicles;i++)
	{
		Actors::C_VehicleLogicActor* pLogicActor = rll.GetLogicVehicleActor(i);
		if(pLogicActor->GetPositionInfo().mRacePosition > lastPos && levelState.mCars[i].mState == E_VEHICLE_STATE_RACING)
		{
			lastIndex = i;
			lastPos = pLogicActor->GetPositionInfo().mRacePosition;
		}

		if(pLogicActor->GetPositionInfo().mRacePosition > lastPos && levelState.mCars[i].mState == E_VEHICLE_STATE_RACING)
		{
			lastIndex = i;
			lastPos = pLogicActor->GetPositionInfo().mRacePosition;
		}

		if( ((firstIndex == -1) && levelState.mCars[i].mState == E_VEHICLE_STATE_RACING ) || (pLogicActor->GetPositionInfo().mRacePosition < firstPos && levelState.mCars[i].mState == E_VEHICLE_STATE_RACING) )
		{
			firstIndex = i;
			firstPos = pLogicActor->GetPositionInfo().mRacePosition;
		}
	}
	
	if(lastIndex>=0)
	{
		lastIndex = UI::GetLocalPlayerIndex(lastIndex);
	}

	if(firstIndex >= 0)
	{
		firstIndex = UI::GetLocalPlayerIndex(firstIndex);
	}

	lua_pushnumber( pState, (lastIndex == firstIndex) ? -1 : lastIndex+1 );

	return 1;
}

S32 GetPlayersToBeEliminated( lua_State* pState )
{
	Game::C_RaceLogicLayer& rll = *C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	//Game::LevelStateSplitScreenSettings& splitScreenSettings = levelState.GetSplitScreenSettings();

	//int safePosition = rll.GetNumVehicles() - C_RaceRulesManager::Instance().GetNumEliminated() - splitScreenSettings.mPlayersToEliminate;
	int safePosition = rll.GetNumVehicles() - C_RaceRulesManager::Instance().GetNumEliminated();

	lua_newtable( pState );

	int numVehicles = rll.GetNumVehicles();
	int currentNum = 1;
	for(int i=0;i<numVehicles;i++)
	{
		Actors::C_VehicleLogicActor* pLogicActor = rll.GetLogicVehicleActor(i);
		if(pLogicActor->GetPositionInfo().mRacePosition > safePosition && levelState.mCars[i].mState == E_VEHICLE_STATE_RACING)
		{
			lua_newtable( pState );
			Lua::AddDataMember( pState, "index", i+1 );
			lua_rawseti( pState, -2, currentNum );
			currentNum++;
		}
	}

	return 1;
}

// GetHudConfig
static S32 GetHudConfig( lua_State* pState )
{
	lua_newtable( pState );

	UI::C_UiXtConfig* pUiConfig = UI::C_UiXtConfig::Instance();

	if ( !pUiConfig )
		return 1;

	UI::C_UiHudConfig_Pieces* pConfig = NULL;


	if ( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer() )
	{
		/*
		UI::E_RaceType eRaceStyle = UI::GetLevelState().m_MultiplayerType;

		for ( S32 i = 0; i < pUiConfig->m_HudConfig.m_MultiPlayer.count; i++ )
		{
			if ( pUiConfig->m_HudConfig.m_MultiPlayer[ i ].m_eRaceType == eRaceStyle )
			{
				pConfig = &pUiConfig->m_HudConfig.m_MultiPlayer[ i ].m_Pieces;
				break;
			}
		}
		*/
	}
	else
	{
		// do we want to or all of the rules together?
		Game::RaceModeRuleBase* pRulePass = Game::C_RaceRulesManager::Instance().GetRaceRulePass( 0 );

		if ( pRulePass )
		{
			Game::RaceModeRuleType eRuleType = pRulePass->GetType();

			for ( S32 i = 0; i < pUiConfig->m_HudConfig.m_SinglePlayer.count; i++ )
			{
				if ( pUiConfig->m_HudConfig.m_SinglePlayer[ i ].m_eRaceRule == eRuleType )
				{
					pConfig = &pUiConfig->m_HudConfig.m_SinglePlayer[ i ].m_Pieces;
					break;
				}
			}
		}
	}

	if ( pConfig )
	{
		Lua::AddDataMember( pState, "current", pConfig->m_bCurrent );
		Lua::AddDataMember( pState, "target", pConfig->m_bTarget );
		Lua::AddDataMember( pState, "lap_timer", pConfig->m_bLapTimerOrTimeLimit );
		Lua::AddDataMember( pState, "lap_counter", pConfig->m_bLapCounter );

		UI::C_TextStringID currentId( pConfig->m_CurrentOverrideId.id );

		if ( currentId.IsValid() && currentId.IsLocString() )
			Lua::AddDataMember( pState, "current_override_loc_id", currentId.Value() );
	}

	return 1;
}

// DistanceFromDriverFailing
static S32 DistanceFromDriverFailing( lua_State* pState )
{
	lua_pushboolean( pState, UI::C_HudDataFinder::Instance().DistanceFromDriverFailing() );
	return 1;
}

// NumRulesActive
static S32 NumRulesActive( lua_State* pState )
{
	lua_pushnumber( pState, UI::C_HudDataFinder::Instance().NumActiveRules() );
	return 1;
}

// IsSinglePlayerRaceTimed
static S32 IsSinglePlayerRaceTimed( lua_State* pState )
{
	bool bTimed = ( UI::GetLevelState().mTimeLimit != 0 );

	// check for timer expired special events
	for ( S32 i = 0; i < Game::C_RaceRulesManager::Instance().GetNumFailRules(); i++ )
	{
		Game::RaceModeRuleBase* pRuleFail = Game::C_RaceRulesManager::Instance().GetRaceRuleFail( i );

		if ( !pRuleFail )
			continue;
		
		if ( pRuleFail->GetType() == Game::E_RaceRule_TimerExpired )
		{
			bTimed = true;
			break;
		}
	}

	lua_pushboolean( pState, bTimed );
	return 1;
}

static S32 IsRaceMpDestruction( lua_State* pState )
{
	bool isDestruction = false;

	Game::C_RaceBookLayer* pLayer = UI::GetRaceBookLayer();
	if( pLayer && pLayer->GetGameModePtr()->IsMultiplayer() )
	{
		Game::C_RaceModeBase* pRaceMode = pLayer->GetRaceModePtr();
		isDestruction = (pRaceMode && pRaceMode->GetID() == Game::E_RaceMode_Multiplayer_Destruction);
	}

	lua_pushboolean( pState, (isDestruction) ? 1 : 0 );
	return 1;
}

// IsRaceDestruction
static S32 IsRaceDestruction( lua_State* pState )
{
	bool bDestruction = false;

	Btl::Vector<Game::RaceActionBase*>& raceActions = Game::C_RaceRulesManager::Instance().GetTimeActions();

	for ( S32 i = 0; i < raceActions.Size(); i++ )
	{
		if ( raceActions[ i ] && ( raceActions[ i ]->GetType() == Game::E_Action_EliminateHitByPerks ) )
		{
			bDestruction = true;
			break;
		}
	}

	lua_pushboolean( pState, bDestruction );
	return 1;
}

// HasStartCutScene
static S32 HasStartCutScene( lua_State* pState )
{
	lua_pushboolean( pState, 0 ); // Exists?
	lua_pushboolean( pState, 0 ); // Dummy?

	return 2;
}

// HasStartScriptedCamera
static S32 HasStartScriptedCamera( lua_State* pState )
{
	lua_pushboolean( pState, 0 ); // Exists?
	lua_pushboolean( pState, 0 ); // Dummy?

	return 2;
}

// HasRestartScriptedCamera
static S32 HasRestartScriptedCamera( lua_State* pState )
{
	lua_pushboolean( pState, 0 ); // Exists?
	lua_pushboolean( pState, 0 ); // Dummy?

	return 2;

}

// HasPassedCutScene
static S32 HasPassedCutScene( lua_State* pState )
{
	lua_pushboolean( pState, 0 ); // Exists?
	lua_pushboolean( pState, 0 ); // Dummy?

	return 2;
}

// HasPassedScriptedCamera
static S32 HasPassedScriptedCamera( lua_State* pState )
{
	lua_pushboolean( pState, 0 ); // Exists?
	lua_pushboolean( pState, 0 ); // Dummy?

	return 2;
}

// HasFailedScriptedCamera
static S32 HasFailedScriptedCamera( lua_State* pState )
{
	lua_pushboolean( pState, 0 ); // Exists?
	lua_pushboolean( pState, 0 ); // Dummy?

	return 2;
}

static S32 GetDebugEvent( lua_State* pState )
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)levelState.mCity];
	AmaxRoute* pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)levelState.mRoute];

	lua_newtable( pState );

	if ( pCity && pRoute )
	{
		Lua::AddDataMember( pState, "name",			UI::C_TextStringID( "DBG_DEBUG_RACE" ).Value() );
		Lua::AddDataMember( pState, "icon",			pCity->icon_name.string );
		Lua::AddDataMember( pState, "city_name",	pCity->loc_string.id );
		Lua::AddDataMember( pState, "route_name",	pRoute->loc_string.id );
	}

	return 1;
}

static S32 AssignCallback( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TSTRING )
	{
		UI::g_pUILua->StackDump( pState, "AssignCallback( id, string ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if ( pObject && pObject->GetType() == UI::E_UIGameButtonType_Checkbox )
	{
		((UI::UICheckbox*)pObject)->AssignCallback( lua_tostring( pState, 2 ) );
	}
	return 0;
}

static S32 GetCheckboxState( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetCheckboxState( id ) - incorrect arguments" );
		return 0;
	}	

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if ( pObject && pObject->GetType() == UI::E_UIGameButtonType_Checkbox )
	{
		lua_pushboolean( pState, ((UI::UICheckbox*)pObject)->IsChecked() );
		return 1;
	}
	return 0;
}

static S32 SetCheckboxState( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TBOOLEAN )
	{
		UI::g_pUILua->StackDump( pState, "SetCheckboxState( id, bool ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	if ( pObject && pObject->GetType() == UI::E_UIGameButtonType_Checkbox )
	{
		((UI::UICheckbox*)pObject)->Checked( lua_toboolean( pState, 2 ) ? true : false );
	}
	return 0;
}

static int AmaxWorldLayerExists( lua_State* pState )
{
	lua_pushboolean( pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() ? 1 : 0 );
	return 1;
}

static int UIWorldLayerExists( lua_State* pState )
{
	lua_pushboolean( pState, C_GameApp::GetInstance()->GetRaceBookLayer()->GetUIWorldLayer() ? 1 : 0 );
	return 1;
}

static int SkipCutscene( lua_State* pState )
{
	if( !UI::GetAmaxWorldLayer() )
		return 0;

	if( !UI::GetAmaxWorldLayer()->GetGameWorld() )
		return 0;

	if( !UI::GetAmaxWorldLayer()->GetGameWorld()->GetActorList() )
		return 0;
		
	ActorMsg::C_CutScene_SkippedMsg msg;
	UI::GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );

	return 0;
}

static int InitialiseAdverts(lua_State* pState)
{
#if defined(ENABLE_IGA) && defined(PLATFORM_PS3)
	Advertising::SetCacheFolder(C_PS3InstallData::Instance().GetDataUsrdirPath());
#endif
	Advertising::LoggedIn();
	return 0;
}

static void SetGammaByIndex( const F32 gamma_index )
{
	const F32 gamma = 2.2f + (3.0f * (gamma_index - 5.0f) / 10.0f);

	// Set the game's debug var (ag!)
	HDR::g_Gamma = gamma;

	// Set the user gamma for UI shader
	UI::gSimple2D.SetGammaCorrection( gamma );
}

static int GetOptionsDefaults( lua_State* pState )
{
	C_GameProfile* pProfile = C_GameProfileManager::Instance().GetPrimaryProfile(); // Do we need to set different profiles here?
	if( !pProfile )
	{
		LOGWARN( g_Log, DBSTR "Profile not found");
		return 0;
	}

	// Get
	if( lua_type( pState, 1 ) == LUA_TNONE )
	{
		lua_newtable(pState);

		pProfile->mOptions.SendDefaultsToLua( pState );

		return 1;
	}
	
	return 0;
}

#ifdef PLATFORM_PC
// [Blur2PC] Retail Blur PC keyboard options (Options > Keyboard, Shared\Options_Control.lua).
// Amax.UpdateCustomKeyboardControls (retail 0x00b83d80). No argument: a table with IsCustom and the key names
// of the custom layout. (table [, layout]): a numeric layout becomes the profile's keyboard layout, then,
// outside split screen, IsCustom and the numeric keys are read from the table. The controls are applied by
// the Amax.Options call that follows (UI::SetupPlayerControls).
static int UpdateCustomKeyboardControls( lua_State* pState )
{
	C_GameProfile* pProfile = C_GameProfileManager::Instance().GetPrimaryProfile();
	if( !pProfile )
		return 0;

	if( lua_type( pState, 1 ) == LUA_TNONE )
	{
		lua_newtable( pState );
		pProfile->mOptions.SendKeyboardToLua( pState );
		return 1;
	}

	if( lua_type( pState, 1 ) != LUA_TTABLE )
	{
		UI::g_pUILua->StackDump( pState, "Set CustomKeyboardControls - incorrect arguments\n");
		return 0;
	}

	if( lua_type( pState, 2 ) == LUA_TNUMBER )
	{
		pProfile->mOptions.mKeyboardLayout = (S8)(S32)lua_tonumber( pState, 2 );
		lua_pop( pState, 1 );
	}

	if( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != Game::E_GameMode_SplitScreen )
		pProfile->mOptions.GetKeyboardFromLua( pState );

	return 0;
}

// Amax.GetPCKeyboard (retail 0x00b86930): the Windows keyboard layout name ("0000040C" is French AZERTY,
// for which the keyboard screen swaps the Q/A and Z/W labels)
static int GetPCKeyboard( lua_State* pState )
{
	char layoutName[ KL_NAMELENGTH ];
	if( GetKeyboardLayoutNameA( layoutName ) )
		lua_pushstring( pState, layoutName );
	else
		lua_pushstring( pState, "" );
	return 1;
}

// [Blur2PC] Retail Amax.SetSplitscreenKeyboards, Blur.exe 1.2 0x00b88cb0.
// With no arguments, the second keyboard occupies the slot after the primary pad.
// The optional third argument shifts physical pads when the primary uses a keyboard;
// this is a PC adaptation for a single XInput controller on port 0.
static int SetSplitscreenKeyboards( lua_State* pState )
{
	const S32 primary = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	S32 keyboard0 = primary;
	S32 keyboard1 = ( primary + 1 ) & 3;
	bool keyboardPrimary = false;
	if( lua_gettop( pState ) != 0 )
	{
		if( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
		{
			UI::g_pUILua->StackDump( pState, "SetSplitscreenKeyboards - incorrect arguments" );
			return 0;
		}
		keyboard0 = (S32)lua_tonumber( pState, 1 );
		keyboard1 = (S32)lua_tonumber( pState, 2 );
		// [Blur2PC] A keyboard primary frees its XInput port for the next local player.
		keyboardPrimary = lua_toboolean( pState, 3 ) != 0;
	}

	C_GameApp* pApp = C_GameApp::GetInstance();
	UI::C_UiXtConfig* pUiConfig = UI::C_UiXtConfig::Instance();
	if( !pApp || !pUiConfig )
		return 0;
	for( S32 i = 0; i < 4; ++i )
	{
		C_GameProfile* pProfile = C_GameProfileManager::Instance().GetProfile( i );
		if( !pProfile )
			continue;
		Input::C_Device* pKeyboard = ( i == keyboard0 || i == keyboard1 ) ? pApp->GetKeyboard( i ) : NULL;
		const S32 padIndex = keyboardPrimary && i != primary ? ( i + 3 ) & 3 : i;
		Input::C_Device* pPad = keyboardPrimary && i == primary ? NULL : pApp->GetPad( padIndex );
		Game::SetupAmaxUserControlsPC( Input::GetGameInput( i ), pPad, pKeyboard, pApp->GetMouse( i ),
			pProfile->mOptions.mKeyboardLayout, i != keyboard0,
			&pUiConfig->m_ControlConfigs[ (S32)pProfile->mOptions.mControlIndex ], i != keyboard0 );
	}
	return 0;
}
#endif

static int Options( lua_State* pState )
{
	C_GameProfile* pProfile = C_GameProfileManager::Instance().GetPrimaryProfile(); // Do we need to set different profiles here?
	if( !pProfile )
	{
		LOGWARN( g_Log, DBSTR "Profile not found");
		return 0;
	}

	// Get
	if( lua_type( pState, 1 ) == LUA_TNONE )
	{
		lua_newtable(pState);

		pProfile->mOptions.SendToLua( pState );

		return 1;
	}
	
	// Read
	if( lua_type( pState, 1 ) == LUA_TTABLE )
	{
		pProfile->mOptions.GetFromLua( pState );
	}
	else
	if( lua_type( pState, 1 ) != LUA_TNIL )
	{
		UI::g_pUILua->StackDump( pState, "Options(none|table|nil) - incorrect arguments\n");
		return 0;
	}

	// Set
	S32 controller = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (0x00b88b90) sets up keyboard and pad from the profile (0x00c2fe70), except in split screen
	if( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != Game::E_GameMode_SplitScreen )
		UI::SetupPlayerControls( controller );
#else
	UI::C_UiXtConfig* pUiConfig = UI::C_UiXtConfig::Instance();
	if( pUiConfig )
		pUiConfig->m_ControlConfigs[ (S32)pProfile->mOptions.mControlIndex ].RemapGameControls( Input::GetGameInput(controller), C_GameApp::GetInstance()->GetPad(controller) );
#endif

	SetGammaByIndex( pProfile->mOptions.mGammaIndex );

	Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eSetUIOptions, 0, &pProfile->mOptions);
	
	return 0;
}

static int UpdateAudioOptions( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TTABLE )
	{
		UI::g_pUILua->StackDump( pState, "UpdateAudioOptions(table) - incorrect arguments\n");
		return 0;
	}

	Game::ProfileOptions options;
	options.GetFromLua( pState );

	Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eSetUIOptions, 0, &options);

	return 0;
}

static int Speakers( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "Speakers(number) - incorrect arguments\n");
		return 0;
	}

	UI::C_GadgetBase * object = UI::gButtons.Find( (S32)lua_tointeger( pState, 1 ) );
	if(!object)
	{
		LOGWARN( g_Log, DBSTR" : Button ID not found\n");
		return 0;
	}
	if( object->GetType() != UI::E_UIGameButtonType_SpeakerSetup )
	{
		LOGWARN( g_Log, DBSTR" : Button not text\n");
		return 0;
	}

	// Get
	UI::UISpeakerSetup* pSpeaker = (UI::UISpeakerSetup*)object;
	if( lua_type( pState, 2 ) == LUA_TNONE )
	{
		lua_pushnumber( pState, (lua_Number)pSpeaker->GetFL() );
		lua_pushnumber( pState, (lua_Number)pSpeaker->GetFR() );
		lua_pushnumber( pState, (lua_Number)pSpeaker->GetRR() );
		lua_pushnumber( pState, (lua_Number)pSpeaker->GetRL() );
		return 4;
	}

	// Set
	if( lua_type( pState, 2) != LUA_TNUMBER || lua_type( pState, 3 ) != LUA_TNUMBER || lua_type( pState, 4 ) != LUA_TNUMBER || lua_type( pState, 5 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "Option_AdvancedAudio(number, none|(number,number,number,number,number) ) - incorrect arguments\n");
		return 0;
	}

	// Set the gadget
	pSpeaker->SetFL( (F32)lua_tonumber( pState, 2 ) );
	pSpeaker->SetFR( (F32)lua_tonumber( pState, 3 ) );
	pSpeaker->SetRR( (F32)lua_tonumber( pState, 4 ) );
	pSpeaker->SetRL( (F32)lua_tonumber( pState, 5 ) );

	return 0;
}

static int GammaCorrection( lua_State* pState )
{
	// Get
	if( lua_type( pState, 1 ) == LUA_TNONE )
	{
		lua_pushnumber(pState, (lua_Number)(F32)HDR::g_Gamma );
		return 1;
	}
	
	// Set
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GammaCorrection(none|number) - incorrect arguments\n");
		return 0;
	}

	const F32 gamma_index = lua_tonumber( pState, 1 );
	SetGammaByIndex( gamma_index );

	return 0;
}

#ifdef PLATFORM_PC
// [Blur2PC] Retail Blur PC graphics options bindings (Blur.exe 1.2 Amax table; see docs/analysis.md
// "Graphics options (PC)"). The retail error strings are kept, including the two that name older functions.

// retail 0x00b88e20: number of entries in the resolution list (entry 0 is not offered by the options screen)
static int GraphicResolutionCount( lua_State* pState )
{
	if( lua_gettop( pState ) != 0 )
	{
		UI::g_pUILua->StackDump( pState, "GetResolutionTable - incorrect arguments ( takes no parameters )\n");
		return 0;
	}

	lua_pushnumber( pState, (lua_Number)Display::g_Resolutions.GetResolutionCount() );
	return 1;
}

// retail 0x00b88e60: number of multisample options
static int GraphicAACount( lua_State* pState )
{
	if( lua_gettop( pState ) != 0 )
	{
		UI::g_pUILua->StackDump( pState, "GetDisplayAATable - incorrect arguments ( takes no parameters )\n");
		return 0;
	}

	lua_pushnumber( pState, (lua_Number)Display::g_Resolutions.GetMultisampleCount() );
	return 1;
}

// retail 0x00b8dbb0: width and height of a resolution entry, or of the back buffer when no valid index is given
static int GetScreenResolution( lua_State* pState )
{
	if( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		const Display::C_Resolution* pResolution = Display::g_Resolutions.GetResolution( (S32)lua_tonumber( pState, 1 ) );
		if( pResolution )
		{
			lua_pushnumber( pState, (lua_Number)pResolution->m_Width );
			lua_pushnumber( pState, (lua_Number)pResolution->m_Height );
			return 2;
		}
	}

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
	lua_pushnumber( pState, (lua_Number)(S32)backBuffer.GetWidth() );
	lua_pushnumber( pState, (lua_Number)(S32)backBuffer.GetHeight() );
	return 2;
}

// retail 0x00b83ed0: applies an options table (the profile options' GetFromLua, which on PC also applies the
// graphics settings), then refreshes the UI for the new back buffer (retail 0x00c358c0; see
// Blur2PCRefreshUIForBackBuffer)
void Blur2PCRefreshUIForBackBuffer();

static int UpdateGraphicsOptions( lua_State* pState )
{
	C_GameProfile* pProfile = C_GameProfileManager::Instance().GetPrimaryProfile();
	if( pProfile )
	{
		if( lua_type( pState, 1 ) != LUA_TTABLE )
		{
			UI::g_pUILua->StackDump( pState, "Set UpdateGraphicsOptions - incorrect arguments\n");
			return 0;
		}

		pProfile->mOptions.GetFromLua( pState );
		Blur2PCRefreshUIForBackBuffer();
	}
	return 0;
}

// [Blur2PC] retail 0x00c358c0: the UI's 2D screen width follows the new back buffer's aspect ratio
// (width * 480 / height), the 2D clip transform is rebuilt for it (0x00799710) and the UI camera is
// re-initialised (0x00798140), whose 3D projection depends on the clip size.
void Blur2PCRefreshUIForBackBuffer()
{
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
	UI::gSimple2D.SetScreenWidth( (F32)backBuffer.GetWidth() * ( UI_VSCREEN_SIZEY / (F32)backBuffer.GetHeight() ) );
	UI::gSimple2D.SetClipSize2D( Display::C_ViewSetup::GetDeviceCoords().Max().X(), Display::C_ViewSetup::GetDeviceCoords().Max().Y() );
	UI::C_UICamera::Instance().Init();
}
#endif

static int GetControllerConfigs( lua_State* pState )
{
	UI::C_UiXtConfig* pUiConfig = UI::C_UiXtConfig::Instance();
	if ( !pUiConfig )
		return 0;

	lua_newtable(pState);
	for( S32 i = 0; i < pUiConfig->m_ControlConfigs.count; i++ )
	{
		lua_newtable( pState );
			pUiConfig->m_ControlConfigs[i].SendToLua( pState );
		lua_rawseti(pState,-2,i+1);
	}

	return 1;
}

static int Has321Started( lua_State* pState)
{
	bool ret = false;
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
		ret = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->HasStarted321Go();
	lua_pushboolean(pState,ret);
	return 1;
}

static int ShowMidPointHelp( lua_State* pState)
{
	bool ret = false;
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
		ret = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->ShowMidPointHelp();
	lua_pushboolean(pState,ret);
	return 1;
}

Debug::Bool g_debugEnableQuickRestart("Amax.QuickRestart.Enable", true);

static int CanQuickRestart( lua_State* pState)
{	
	lua_pushboolean(pState,(bool)g_debugEnableQuickRestart);
	return 1;
}

static int IsFrontEndLoading( lua_State* pState)
{	
	lua_pushboolean(pState,C_GameApp::GetInstance()->GetRaceBookLayer()->GetFrontEndLoading());
	return 1;
}

static int SetFrontEndLoaded( lua_State* pState)
{
	C_GameApp::GetInstance()->GetRaceBookLayer()->SetFrontEndLoading(false);
	return 0;
}

static int SetLoadStateIntoUI( lua_State* pState )
{
	S32 num = (S32)lua_tonumber( pState, 1 );
	C_GameApp::GetInstance()->GetRaceBookLayer()->SetLoadState(Game::E_LoadState_IntoUI);

	return 0;
}

static int SetLoadStateIntoRace( lua_State* pState )
{
	S32 num = (S32)lua_tonumber( pState, 1 );
	C_GameApp::GetInstance()->GetRaceBookLayer()->SetLoadState(Game::E_LoadState_IntoRace);

#ifndef CFG_GOLD
	AmaxDataLogging::LogRaceEnd();
#endif

	return 0;
}

static int SetLoadStateBackToUI( lua_State* pState )
{
	S32 num = (S32)lua_tonumber( pState, 1 );
	C_GameApp::GetInstance()->GetRaceBookLayer()->SetLoadState(Game::E_LoadState_BackToUI);

	return 0;
}

static int SetLoadStateBackToLobby( lua_State* pState )
{
	S32 num = (S32)lua_tonumber( pState, 1 );
	C_GameApp::GetInstance()->GetRaceBookLayer()->SetLoadState(Game::E_LoadState_BackToLobby);

	return 0;
}

static int HandleUISceneReady( lua_State* pState )
{
	if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetUIWorldLayer() &&
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetUIWorldLayer()->GetGameWorld())
	{
		if ( lua_type( pState, 1 ) == LUA_TBOOLEAN)
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetUIWorldLayer()->GetGameWorld()->HandleUISceneReady();
		else
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetUIWorldLayer()->GetGameWorld()->SetHandleUISceneReady();
	}

	return 0;
}


// GetShouldLoadDefaultCar
static S32 GetShouldLoadDefaultCar( lua_State* pState )
{
	UI::C_UiXtConfig* pConfig = UI::C_UiXtConfig::Instance();

	Game::Actors::RaceLogicCore* pRLC = UI::GetRLC();

	if (!pConfig || !pConfig->m_DefaultVehicle || !pRLC)
		return 0;

	lua_pushnumber( pState, (S32)pConfig->m_DefaultVehicle->ID );
	lua_pushboolean( pState, (pRLC->GetNumVehicles() > 0) );

	return 2;
}

// LoadCar
static S32 LoadCar( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "LoadCar( amax_id, [bool] ) - incorrect arguments" );
		return 0;
	}

	AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)lua_tointeger( pState, 1 ) ];

	if ( !pVehicle )
		return 0;

	Game::Actors::RaceLogicCore* pRLC = UI::GetRLC();

	if ( !pRLC )
		return 0;

	pRLC->HideLockedCarSFX();

	bool bMultiplayer = ( ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 2 ) != 0 ) : false );
	bool bTeams = ( ( lua_type( pState, 3 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 3 ) != 0 ) : false );

	U32 teamColour = 0;

	if ( bMultiplayer && bTeams && lua_type( pState, 4 ) == LUA_TNUMBER )
		teamColour = ( (S8)lua_tonumber( pState, 4 ) ) ? C_MultiplayerData::Instance().m_pConfig->m_TeamBPaint.Id() : C_MultiplayerData::Instance().m_pConfig->m_TeamAPaint.Id();

	if (!pRLC->ShouldLoadVehicle(pVehicle))
	{
		// just respray
		U32 paintColour = 0;

		if ( bMultiplayer && bTeams )
			paintColour = teamColour;
		else
		{		
			Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

			if ( pGameProfile )
				paintColour = pGameProfile->ResprayGet( pVehicle->ID, bMultiplayer );
		}

		Game::Actors::C_VehicleResprayMsg msg( 0, paintColour );
		Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );

		return 0;
	}

	// #35482
	if( lua_type( pState, 5 ) != LUA_TBOOLEAN || lua_toboolean( pState, 5 ) != 0 )
	{
		// Make sure boot assets have been sync'ed here
		Messaging::TMsgType msg = Game::E_GameflowMessage_SyncBootAssets;
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( msg );

		RLPickupSpecialFX sfx;
			
		sfx.mGlobalSpecialFX=C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetDatabase()->mRLSystems[0]->mPickupSystem->mParameters->mGarageSpecialFX;
			
		sfx.mDurationLoop.mStart	= 0.0f;
		sfx.mDurationLoop.mEnd		= 0.0f;	
		sfx.mDurationOutro.mStart	= 0.0f;
		sfx.mDurationOutro.mEnd		= 0.0f;
		sfx.mDurationIntro.mStart	= 0.0f;
		sfx.mDurationIntro.mEnd		= 2.0f;
		sfx.mFrequency				= 1.0f;

		UI::C_UiXtConfig* pConfig = UI::C_UiXtConfig::Instance();
		Maths::HMat43 spawnMatrix = Maths::HMat43(Maths::HMat33(Maths::C_Quaternion(Maths::HVec3(0.0f, 1.0f, 0.0f), Maths::DegToRad((float)pConfig->m_CarOrientation))),
			Maths::HVec3((float)pConfig->m_CarPosition.vx, (float)pConfig->m_CarPosition.vy, (float)pConfig->m_CarPosition.vz));

		Game::C_InGameSpecialFXManager::Instance().SetUpWorldSFX(sfx,spawnMatrix,-1,-1,-1,Maths::HVec3(1,1,1));

		UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_GarageSwitchCarActionIn );
	}

	pRLC->StartAmaxVehicleLoad( pVehicle, bMultiplayer, teamColour );

	return 0;
}

// LoadLockedCarModel
static S32 LoadLockedCarModel( lua_State* pState )
{
	Game::Actors::RaceLogicCore* pRLC = UI::GetRLC();

	if ( !pRLC )
		return 0;

	pRLC->ShowLockedCarSFX();	

	UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_GarageSwitchCarLockedActionIn );

	return 0;
}

// GetColourPalette
static S32 GetColourPalette( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetColourPalette( id ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( !pObject || ( pObject->GetType() != UI::E_UIGameButtonType_ColourPicker ) )
		return 0;

	lua_pushnumber( pState, (S32)((UI::UIColourPicker*)pObject)->GetPalette() );
	return 1;
}

// SetColourPaletteLocked
static S32 SetColourPaletteLocked( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SetColourPaletteLocked( id ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( !pObject || ( pObject->GetType() != UI::E_UIGameButtonType_ColourPicker ) )
		return 0;

	S32 sPaletteLocked = 0;

	for ( S32 i = 0; i < UI::E_Palette_Count; i++ )
		if ( lua_type( pState, i + 2 ) == LUA_TBOOLEAN )
			sPaletteLocked |= ( ( lua_toboolean( pState, i + 2 ) != 0 ) ? ( 1 << i ) : 0 );

	((UI::UIColourPicker*)pObject)->SetPaletteLocked( sPaletteLocked );

	return 0;
}

// SetColourPickerFromRespray
static S32 SetColourPickerFromRespray( lua_State* pState )
{
	if ( ( lua_type( pState, 1 ) != LUA_TNUMBER ) || ( lua_type( pState, 2 ) != LUA_TNUMBER ) || ( lua_type( pState, 3 ) != LUA_TNUMBER ) )
	{
		UI::g_pUILua->StackDump( pState, "SetColourPickerFromRespray( id, id_xt, index ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( !pObject || ( pObject->GetType() != UI::E_UIGameButtonType_ColourPicker ) )
		return 0;

	XtId id = (XtId)lua_tointeger( pState, 2 );
	S32 sIndex = (S32)lua_tonumber( pState, 3 );
	bool bMultiplayer = ( ( lua_type( pState, 4 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 4 ) != 0 ) : false );

	((UI::UIColourPicker*)pObject)->SetFromRespray( id, sIndex, bMultiplayer );

	return 0;
}

// ResprayCar
static S32 ResprayCar( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "ReSprayCar( id, [id_amax], [bool], [bool]] ) - incorrect arguments" );
		return 0;
	}

	XtId amaxVehicleId = ( ( lua_type( pState, 2 ) == LUA_TNUMBER ) ? ( (XtId)lua_tointeger( pState, 2 ) ) : 0 );
	bool bMultiplayer = ( ( lua_type( pState, 3 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 3 ) != 0 ) : false );
	bool bDoNotSendMessage = ( ( lua_type( pState, 4 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 4 ) != 0 ) : false );

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if ( !pGameProfile )
		return 0;

	S32 sId = (S32)lua_tonumber( pState, 1 );

	if ( sId == -1 )
	{
		// revert to manufacturer
		if ( !bDoNotSendMessage )
		{
			Game::Actors::C_VehicleResprayMsg msg( 0, 0 );
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );
		}

		// remove from profile
		if ( amaxVehicleId != 0 )
			pGameProfile->ResprayRemove( amaxVehicleId, bMultiplayer );
	}
	else
	{
		UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)sId );

		if ( !pObject || ( pObject->GetType() != UI::E_UIGameButtonType_ColourPicker ) )
			return 0;

		XtId id = ((UI::UIColourPicker*)pObject)->GetSelectedRespray();
		S32 sIndex = ((UI::UIColourPicker*)pObject)->GetColourIndex();

		if ( !bDoNotSendMessage )
		{
			Game::Actors::C_VehicleResprayMsg msg( 0, id );
			C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );
		}

		// store to profile
		if ( amaxVehicleId != 0 )
		{
			pGameProfile->ResprayAdd( amaxVehicleId, id, sIndex, bMultiplayer );

			// push results down
			lua_pushinteger( pState, id );
			lua_pushnumber( pState, sIndex );

			return 2;
		}
	}

	return 0;
}

// GetResprayColour
static S32 GetResprayColour( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetResprayColour( xt_id ) - incorrect arguments" );
		return 0;
	}

	if ( !UI::C_UiXtConfig::Instance() )
		return 0;

	XtHandle< Vehicle::XtPaintJob > paintJob = UI::C_UiXtConfig::Instance()->m_GarageResprays.Find( (XtId)lua_tointeger( pState, 1 ) );

	if ( !paintJob )
		return 0;

	UI::C_UIColour col;
	col.Set01( 1.0f, paintJob->m_Colours[ 0 ].r, paintJob->m_Colours[ 0 ].g, paintJob->m_Colours[ 0 ].b );

	lua_pushstring( pState, col.LuaString() );

	return 1;
}

static int OutroPrepared( lua_State* pState )
{
	bool outroReady = false;

	if (C_GameApp::GetInstance()->GetRaceBookLayer() &&
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer())
	{
		outroReady = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetCutSceneState() == Game::E_CutSceneState_OutroPrepped;		
	}

	lua_pushboolean( pState, outroReady );

	return 1;
}

static int FansInRace(lua_State* pState)
{
	lua_pushboolean(pState,true);			// Always fans if present in actor list
		
	return 1;
}

static int FanDemandsInRace( lua_State* pState )
{
	Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	if( pLayer )
	{
		lua_pushboolean(pState, pLayer->mNumFanPickups > 0 );
	}
	else
	{
		lua_pushboolean(pState, false );
	}

	return 1;
}

static int ValidGoalInRace( lua_State* pState )
{
	lua_pushboolean( pState, Game::C_RaceRulesManager::Instance().ValidPassRules() );

	return 1;
}

static int KickedOutOfGame(lua_State* pState)
{
#ifndef CFG_GOLD
	AmaxDataLogging::LogRaceCancelled();
#endif
	return 0;
}

static S32 NeedTwitterAuthenticate( lua_State* pState )
{
	bool is_auth=Game::C_TwitterManager::Instance().IsAuthenticated();

	lua_pushboolean(pState,!is_auth);

	return 1;
}

static S32 TwitterAuthenticate( lua_State* pState )
{
	Game::C_TwitterManager::Instance().Authenticate();
	return 0;
}

static S32 TwitterUnAuthenticate( lua_State* pState )
{
	Game::C_TwitterManager::Instance().Unauthenticate();
	return 0;
}

static S32 ClearTweet( lua_State* pState )
{
	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		int tw=(int)lua_tonumber(pState, 1);
		//Game::C_TwitterManager::Instance().ClearTweet(tw);
	}

	return 0;
}

static S32 Tweet( lua_State* pState )
{
	Game::C_TwitterManager::Instance().Tweet(Game::C_SocialNetworkManager::Instance().GetTweetString( 0 ));
	return 0;
}

static S32 StoreTwitterUsername( lua_State* pState )
{
	Game::C_TwitterManager::Instance().SetUsername(UI::gDashboardInterface.GetString());
	return 0;
}

static S32 StoreTwitterPassword( lua_State* pState )
{
	Game::C_TwitterManager::Instance().SetPassword(UI::gDashboardInterface.GetString());
	return 0;
}

static S32 InitTwitter( lua_State* pState )
{
	C_TwitterManager::Instance().Init();
	return 0;
}

static S32 TwitterCheckPublisherFile( lua_State* pState )
{
	C_TwitterManager::Instance().CheckPublisherFile();
	return 0;
}

static S32 TwitterHasPublisherFile( lua_State* pState )
{
	lua_pushboolean(pState,C_TwitterManager::Instance().HasPublisherFile() ? 1 : 0);
	return 1;
}

static S32 TwitterPublisherFileCheckFinished( lua_State* pState)
{
	lua_pushboolean(pState,C_TwitterManager::Instance().HasPublisherFileCheckFinished() ? 1 : 0);
	return 1;
}

static S32 GetTweets( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;
	for(S32 i = 0; i < Game::C_SocialNetworkManager::Instance().GetNumSocialItems(); i++)
	{
		lua_newtable(pState);

		Lua::AddDataMember(pState,"index", i);

		lua_rawseti(pState,-2,index);
		index++;
	}

	return 1;
}

static int PumpTwitterCurrentTask( lua_State* pState )
{
	int error=BD_NO_ERROR;

	bool still_pumping=Game::C_TwitterManager::Instance().PumpCurrentTask(&error);

	lua_pushboolean(pState,still_pumping);
	lua_pushnumber(pState,error);

	return 2;
} 


static S32 CreateBlurb( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER || lua_type( pState, 3 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "CreateBlurb( type, network, index ) - incorrect arguments" );
		return 0;
	}

	int type=(int)lua_tonumber(pState, 1);
	int network=(int)lua_tonumber(pState, 2);
	int index=(int)lua_tonumber(pState, 3);
	
	if(network == Game::AMAX_SOCIAL_NETWORK_BLURB)
	{
		Game::C_SocialNetworkManager::Instance().Create((AMAX_SOCIAL_NETWORKING_ITEM_TYPE)type,Game::AMAX_SOCIAL_NETWORK_FACEBOOK,index);

		static UI::C_TextStringID title_id( "CMN_BLUR_SOCIAL" );

#if defined ( PLATFORM_PS3 )

		SceNpBasicMessageDetails msg;
		int ret;

		char mUtfTitle[SCE_NP_BASIC_SUBJECT_CHARACTER_MAX];
		char mUtfSubject[SCE_NP_BASIC_BODY_CHARACTER_MAX];

		memset(mUtfTitle, 0, sizeof(mUtfTitle));
		memset(mUtfSubject, 0, sizeof(mUtfSubject));

		L10nResult res;
		size_t len  = SCE_NP_BASIC_SUBJECT_CHARACTER_MAX;

		size_t wlen = wcslen( title_id.String() );
		res = UCS2stoUTF8s( (uint16_t*)title_id.String(), &wlen, (uint8_t*)mUtfTitle, &len );

		len  = SCE_NP_BASIC_BODY_CHARACTER_MAX;
		wlen = wcslen( Game::C_SocialNetworkManager::Instance().GetCurrentItemText() );
		res = UCS2stoUTF8s( (uint16_t*)Game::C_SocialNetworkManager::Instance().GetCurrentItemText(), &wlen, (uint8_t*)mUtfSubject, &len );

		msg.msgId = 0;
		msg.mainType = SCE_NP_BASIC_MESSAGE_MAIN_TYPE_GENERAL;
		msg.subType = 0;
		msg.msgFeatures = 0;
		msg.npids = NULL;
		msg.count = 0;
		msg.subject = &mUtfTitle[0];
		msg.body = &mUtfSubject[0];
		msg.data = NULL;
		msg.size = 0;

		ret = sceNpBasicSendMessageGui(&msg, SYS_MEMORY_CONTAINER_ID_INVALID);

		Game::C_Sticker* pSticker = Game::C_StickerManager::Instance().GetSticker(Game::BLUR_STICKER_SHARE_FIRST_EXPERIENCE);
		if(pSticker)
		{
			pSticker->AddProgress(1);
		}

#endif
	}
	else
	{
		Game::C_SocialNetworkManager::Instance().Create((AMAX_SOCIAL_NETWORKING_ITEM_TYPE)type,(Game::AMAX_SOCIAL_NETWORK)network,index);
	}

	return 0;
}

static S32 CreateBlurbFromIndex( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "CreateBlurbFromIndex( network, index ) - incorrect arguments" );
		return 0;
	}

	int network=(int)lua_tonumber(pState, 1);
	int index=(int)lua_tonumber(pState, 2);
	
	Game::C_SocialNetworkManager::Instance().CreateFromIndex((Game::AMAX_SOCIAL_NETWORK)network,index);

	return 0;
}



static S32 AddBlurb( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "AddBlurb( enum ) - incorrect arguments" );
		return 0;
	}

	int blurb=(int)lua_tonumber(pState, 1);

	C_SocialNetworkManager::Instance().AddOneOff((AMAX_SOCIAL_NETWORKING_ITEM_TYPE)blurb,0);

	return 0;
}

static S32 GetBlurbs( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetBlurbs( filter, friend_index ) - incorrect arguments" );
		return 0;
	}

	int type=(int)lua_tonumber(pState, 1);
	int friend_index=(int)lua_tonumber(pState, 2);

	lua_newtable(pState);
	int index=1;

	if(type == 0 || type == 1)
	{
		for(int a=0;a<Game::MAX_FRIEND_DEMANDS;a++)
		{
			SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromID(friend_index);

#ifdef PLATFORM_PS3
			bool identical_ids=Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore[a] == bdAuthUtility::getUserID(prof->m_Uid.handle.data);
			if(!wcscmp(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mChallenger, prof->Name()) || identical_ids )
#else
			bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid, Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore[a]) ? true : false;
			if(!wcsicmp(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mChallenger, prof->Name()) || identical_ids )
#endif
			{
				if(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mEventID)
				{
					lua_newtable(pState);
					
					Lua::AddDataMember(pState,"index", a);
					Lua::AddDataMember(pState,"type", AMAX_SOCIAL_NETWORK_BLURB_TYPE_NORMAL);
					
					lua_rawseti(pState,-2,index);
					index++;
				}
			}
		}
	}

	if(type == 0 || type == 2)
	{
		for(U32 i = 0; i < Game::C_MessagingManager::Instance().GetNumberOfMails(); i++)
		{
			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(i)->m_mailBody;
			const bdOnlineUserID fc_id=Game::C_MessagingManager::Instance().GetMailHeader(i)->m_senderID;

			SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromID(friend_index);

#ifdef PLATFORM_PS3
			bool identical_ids=fc_id == bdAuthUtility::getUserID(prof->m_Uid.handle.data);
			if(!wcscmp(fc->mChallenger, prof->Name()) || identical_ids )
#else
			bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid, fc_id) ? true : false;
			if(!wcsicmp(fc->mChallenger, prof->Name()) || identical_ids )
#endif
			{
				lua_newtable(pState);

				Lua::AddDataMember(pState,"index", i);
				Lua::AddDataMember(pState,"type", AMAX_SOCIAL_NETWORK_BLURB_TYPE_SERVER_MESSAGE);

				lua_rawseti(pState,-2,index);
				index++;
			}
		}
	}

	if(type == 3)
	{
		for(int a=0;a<Game::MAX_FRIEND_DEMANDS;a++)
		{
			if(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mEventID == friend_index)
			{
				lua_newtable(pState);
				
				Lua::AddDataMember(pState,"index", a);
				Lua::AddDataMember(pState,"type", AMAX_SOCIAL_NETWORK_BLURB_TYPE_NORMAL);
				
				lua_rawseti(pState,-2,index);
				index++;
			}
		}

		for(U32 i = 0; i < Game::C_MessagingManager::Instance().GetNumberOfMails(); i++)
		{
			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(i)->m_mailBody;

			if(fc->mEventID == friend_index)
			{
				lua_newtable(pState);

				Lua::AddDataMember(pState,"index", i);
				Lua::AddDataMember(pState,"type", AMAX_SOCIAL_NETWORK_BLURB_TYPE_SERVER_MESSAGE);

				lua_rawseti(pState,-2,index);
				index++;
			}
		}

	}

	return 1;
}

static S32 AddBlurbMessage( lua_State* pState )
{
	Game::C_SocialNetworkManager::Instance().mCurrentMessage=UI::gDashboardInterface.GetString();
	return 0;
}

static S32 ClearBlurbMessage( lua_State* pState )
{
	Game::C_SocialNetworkManager::Instance().mCurrentMessage.Clear();
	return 0;
}

static int HasIntroMessage( lua_State* pState )
{
	return 0;
}

static int GetRecommendedVehicles( lua_State* pState )
{
	return 0;
}

// Clone a texture from texture bundle into tempory space
static int LoadTextureClone( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TSTRING )
	{
		UI::g_pUILua->StackDump( pState, "LoadTextureTempClone(string,[number]) - incorrect arguments\n");
		return 0;
	}

	UI::C_UiLayer* pUILayer = C_GameApp::GetInstance()->GetUiLayer();
	if( !pUILayer )
		return 0;

	if( !pUILayer->GetUiManager() )
		return 0;

	// Get handle to clone
	const char* texture_name = (const char *)lua_tostring( pState, 1 );
	Display::C_Texture* handle = (Display::C_Texture*)UI::gUITextureManager.GetTexturePtrByName( texture_name );
	if( !handle )
		return 0;

	// Index override
	U32 index = 0;
	if( lua_type( pState, 2 ) == LUA_TNUMBER )
		index = (U32)lua_tointeger( pState, 2 );

	// Clone
	pUILayer->GetUiManager()->LoadingTextureClone( handle, index );

	return 0;
}

static int Ps3NeedsInstall( lua_State* pState )
{
#ifdef PLATFORM_PS3
	bool showInstallMessage		= C_PS3InstallData::Instance().ShowInstallingMessage();
	bool hasCheckedForFreeSpace = C_PS3InstallData::Instance().GetHasCheckedForFreeSpaceUI();
#else
	bool showInstallMessage		= false;
	bool hasCheckedForFreeSpace = true;
#endif

//	printf("showInstallMessage %d hasCheckedForFreeSpace %d\n",showInstallMessage, hasCheckedForFreeSpace);

	lua_pushboolean( pState, showInstallMessage			? 1 : 0 );
	lua_pushboolean( pState, hasCheckedForFreeSpace		? 1 : 0 );
	return 2;
}

static int Ps3PrimeInstaller( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		UI::g_pUILua->StackDump( pState, "PrimePs3Installer(string) - incorrect arguments\n");
		return 0;
	}

	bool isCopyAllData = ( lua_toboolean( pState, 1 ) != 0 );

#ifdef PLATFORM_PS3
	if (!isCopyAllData) C_PS3InstallData::Instance().CopyAttractMovie();
	else C_PS3InstallData::Instance().CopyMainInstallData();
#endif
	// isCopyAllData == false : when we're ready to copy attract movie
	// isCopyAllData == true  : when we're ready to copy super data
	
	return 0;
}

static int Ps3PollInstaller( lua_State* pState )
{
	bool isReady = true;
#ifdef PLATFORM_PS3
	static bool result = false;
	isReady = (!C_PS3InstallData::Instance().IsCopyInProgress());
//	printf("Ps3PollInstaller: isReady %d\n",isReady );
#endif
	lua_pushboolean( pState, isReady );
	return 1;
}

static int Ps3InstallerProgress( lua_State* pState )
{
#ifdef PLATFORM_PS3
	const float fProgress = C_PS3InstallData::Instance().GetProgressTotal();
#else
	const float fProgress = 0.0f;
#endif

	lua_pushnumber( pState, (lua_Number)fProgress );
	return 1;
}

static int SetLoadingScreen( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		UI::g_pUILua->StackDump( pState, "PrimePs3Installer(string) - incorrect arguments\n");
		return 0;
	}

	bool enableLoadingScreen = ( lua_toboolean( pState, 1 ) != 0 );
	
	if (enableLoadingScreen)
		C_GameApp::GetInstance()->EnableLoadingScreen();
	else
		C_GameApp::GetInstance()->DisableLoadingScreen();

	return 0;
}

static int CarouselScan( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "CarouselScan( number ) - incorrect arguments" );
		return 0;
	}

	// Get gadget
	UI::C_GadgetBase *pGadget = UI::gButtons.Find( (S32)lua_tonumber(pState,1) );
	if( !pGadget )
		return 0;

	if( pGadget->GetType() != UI::E_UIGameButtonType_Carousel )
		return 0;

	((UI::UICarousel*)pGadget)->BranchScan();

	return 0;
}

static int GetAchievementDetails( lua_State* pState )
{
	C_AchievementSystem::Instance().GetAchievementDetails(SysProfile::C_Manager::Instance().GetPrimaryPadNo());
	return 0;
}

static int ShowFanScoring( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN || lua_type( pState, 2 ) != LUA_TBOOLEAN )
	{
		UI::g_pUILua->StackDump( pState, "ShowFanScoring( bool, bool ) - incorrect arguments" );
		return 0;
	}

	UI::C_UiShowFanScoring showScoringMsg( lua_toboolean( pState, 1 ) != 0,  lua_toboolean( pState, 2 ) != 0 );
	_UI_MESSAGE_FULL( showScoringMsg );
	return 0;
}


static S32 GetCityName( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetCityName( number(ID) ) - incorrect arguments" );
		return 0;
	}	

	lua_pushinteger( pState, C_GameData::Instance().mCities[(GAMEDATA_ID)(U32)lua_tointeger( pState, 1 )]->loc_string.id );
	return 1;
}

static S32 GetRouteName( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetRouteName( number(ID) ) - incorrect arguments" );
		return 0;
	}	

	lua_pushinteger( pState, C_GameData::Instance().mRoutes[(GAMEDATA_ID)(U32)lua_tointeger( pState, 1 )]->loc_string.id );
	return 1;
}

static S32 GetCityIconName( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetCityName( number(ID) ) - incorrect arguments" );
		return 0;
	}

	const U32 id = (U32)lua_tointeger( pState, 1 );

	if ( id > 0 )
	{
		lua_pushstring( pState, C_GameData::Instance().mCities[(GAMEDATA_ID)id]->icon_name );
		return 1;
	}
	return 0;
}

static int DeleteTextureBundle( lua_State* pState )
{
	C_GameApp::GetInstance()->GetUiLayer()->GetUiManager()->DeleteTextureBundle(false);
	return 0;
}

static int EnableAIControl( lua_State* pState) 
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		UI::g_pUILua->StackDump( pState, "EnableAIControl( bool ) - incorrect arguments" );
		return 0;
	}

	bool enable = (lua_toboolean( pState, 1 ) != 0);

	if(enable)
	{
		BIZ_ASSERT(C_GameApp::GetInstance());
		BIZ_ASSERT(C_GameApp::GetInstance()->GetRaceBookLayer());
		BIZ_ASSERT(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer());
		BIZ_ASSERT(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld());
		BIZ_ASSERT(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList());
		
		Game::Actors::C_VehicleForceAIControlMsg forceAIControlMsg(0);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&forceAIControlMsg);
	}

	return 0;
}

static int BGMusicCheck( lua_State* pState) 
{
	if(!lua_isnumber(pState,1))
	{
		UI::g_pUILua->StackDump(pState,"BGMusicCheck(slot) - incorrect arguments");
		return 0;
	}

	//if(!C_GameApp::GetInstance()->GetGameAudioLayer()->TitleHasBGMusicControl())
	//{
	//	UI::gMovie[(int)lua_tonumber(pState,1)].SetVolume(0);
	//}

	return 0;
}


static int StartSubtitles( lua_State* pState) 
{
	if ( lua_type( pState, 1 ) != LUA_TSTRING )
	{
		UI::g_pUILua->StackDump( pState, "StartSubtitles( string ) - incorrect arguments" );
		return 0;
	}

	Game::C_SubtitleManager::Instance().StartSubtitles( lua_tostring( pState, 1 ) );

	return 0;
}

static int UpdateSubtitles( lua_State* pState) 
{
	C_SubtitleManager::Instance().Update();
	return 0;
}

static int EndSubtitles( lua_State* pState) 
{
	Game::C_SubtitleManager::Instance().EndSubtitles();
	return 0;
}

static int GameDataLogRetry( lua_State* pState) 
{
#ifndef CFG_GOLD
	Game::C_GameProfile* pProf = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	if(pProf && pProf->mRaceBookProfile)
	{
		UI::C_WideString64 wLastEvStr;
		//if(pProf->mCurrentEvent)
		//	wLastEvStr.Add(pProf->mCurrentEvent->GetEventName());

		AmaxDataLogging::LogRetry(pProf->mNumberOfRetrys, wLastEvStr.AsString().Chars());
	}
	else
	{
		AmaxDataLogging::LogRetry(0,"");
	}
#endif
	return 0;
}

static int GameDataLogDifficultySelected(lua_State* lua)
{
#ifndef CFG_GOLD
	if (lua_type(lua, 1) != LUA_TNUMBER || lua_type(lua, 2) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump(lua, "Amax:GameDataLogDifficultySelected - incorrect arguments\n");
		return 0;
	}

	U8 oldDifficulty = lua_tointeger(lua, 1);
	U8 newDifficulty = lua_tointeger(lua, 2);

	const char* finalName = "";

	for(S8 i = UI::C_ScreenControlManager::Instance().Count()-1; i >= 0; i--)
	{
		const char* name = UI::C_ScreenControlManager::Instance().Get( i ).GetCurrentScreen();
		if(strcmp(name,"") != 0)
		{
			finalName = name;
			break;
		}
	}

	AmaxDataLogging::LogDifficultySelected(finalName, oldDifficulty, newDifficulty);
#endif
	
	return 0;
}

static int GameDataLogControllerLayoutChanged(lua_State* lua)
{
#ifndef CFG_GOLD
	if (lua_type(lua, 1) != LUA_TNUMBER || lua_type(lua, 2) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump(lua, "Amax:LogControllerLayoutChanged - incorrect arguments\n");
		return 0;
	}
	U8 oldLayout = lua_tointeger(lua, 1);
	U8 newLayout = lua_tointeger(lua, 2);

	AmaxDataLogging::LogControllerLayoutChanged(oldLayout, newLayout);
#endif
	return 0;
}

static int GameDataLogBrightnessChanged(lua_State* lua)
{
#ifndef CFG_GOLD
	if (lua_type(lua, 1) != LUA_TNUMBER || lua_type(lua, 2) != LUA_TNUMBER)
	{
		UI::g_pUILua->StackDump(lua, "Amax:LogBrightnessChanged - incorrect arguments\n");
		return 0;
	}
	U8 oldBrightness = lua_tointeger(lua, 1);
	U8 newBrightness = lua_tointeger(lua, 2);

	AmaxDataLogging::LogBrightnessChanged(oldBrightness, newBrightness);
#endif
	return 0;
}

static int GameDataLogDisplaySettings(lua_State* lua)
{
#ifndef CFG_GOLD

	Game::C_GameWorldPostProcess* layer = AmaxCam::GetGameWorld();
	PostProcess::C_Manager* manager = layer ? layer->GetManager() : 0;
	if (!manager)
		return 0;

	const PostProcess::C_RenderTarget& rtarg = manager->GetRenderTarget(0);

	U32 w = rtarg.iWidth;
	U32 h = rtarg.iHeight;

	AmaxDataLogging::LogDisplaySettings(w, h);
	
#endif
	return 0;
}

static int GameInviteStarted( lua_State* pState )
{
	C_ContentServerManager::Instance().KillCurrentTask();
	C_KeyArchiveManager::Instance().KillCurrentTask();
	C_RecordEventManager::Instance().KillCurrentTask();

	return 0;
}

static S32 ChangeChildrenPanel( lua_State* pState ) 
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"ChangeChildrenPanel(id) - incorrect arguments");
		return 0;
	}

	UI::C_GadgetBase *pGadget = UI::gButtons.Find((U32)lua_tointeger(pState,1));
	if(!pGadget)
		return 0;

	// Find all children
	UI::ButtonArray	children;
	pGadget->FindChildren( &children, true );

	// panel to go to
	const UI::PanelIndex panel = (UI::PanelIndex)lua_tonumber( pState, 2 );
	pGadget->m_Panel = panel;
	pGadget->m_Up = UI::gSimple2D.IsPanel3D( pGadget->m_Panel ) ? -1.0f : 1.0f;

	for ( S32 i = 0; i < children.Size(); i++ )
	{
		children[i]->m_Panel = panel;
		children[i]->m_Up = UI::gSimple2D.IsPanel3D( pGadget->m_Panel ) ? -1.0f : 1.0f;
	}
	return 0;
}

static S32 ChangeChildrenLayer( lua_State* pState ) 
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"ChangeChildrenLayer(id, layer) - incorrect arguments");
		return 0;
	}

	UI::C_GadgetBase *pGadget = UI::gButtons.Find((U32)lua_tointeger(pState,1));
	if(!pGadget)
		return 0;

	// Find all children
	UI::ButtonArray	children;
	pGadget->FindChildren( &children, true );

	// panel to go to
	const UI::LayerIndex layer = (UI::LayerIndex)lua_tonumber( pState, 2 );
	pGadget->m_Layer = layer;
	
	for ( S32 i = 0; i < children.Size(); i++ )
		children[i]->m_Layer = layer;
	
	return 0;
}

static S32 AmericanRegion( lua_State* pState ) 
{
	if(UI::C_UILocaliser::Instance().Region() == UI::E_UIRegion_NorthAmerica)
		lua_pushboolean(pState,true);
	else
		lua_pushboolean(pState,false);

	return 1;
}

static S32 SetUICarToMultiplayer( lua_State* pState ) 
{
	if( lua_type(pState,1) != LUA_TBOOLEAN )
	{
		UI::g_pUILua->StackDump(pState,"SetUICarToMultiplayer(bool) - incorrect arguments");
		return 0;
	}

	bool mp=lua_toboolean(pState,1) ? true : false;

	C_GameApp::GetInstance()->GetRaceBookLayer()->SetMpUICar(mp);

	return 0;
}

static S32 NeedFacebookAuthenticate( lua_State* pState )
{
	bool is_auth=Game::C_FacebookManager::Instance().IsAuthenticated();

	lua_pushboolean(pState,!is_auth);

	return 1;
}

static S32 FacebookUnAuthenticate( lua_State* pState )
{
	Game::C_FacebookManager::Instance().Unauthenticate();
	return 0;
}

static S32 StoreFacebookUsername( lua_State* pState )
{
	Game::C_FacebookManager::Instance().SetUsername(UI::gDashboardInterface.GetString());
	return 0;
}

static S32 StoreFacebookPassword( lua_State* pState )
{
	Game::C_FacebookManager::Instance().SetPassword(UI::gDashboardInterface.GetString());
	return 0;
}

static S32 InitFacebook( lua_State* pState )
{
	Game::C_FacebookManager::Instance().Init();
	return 0;
}

static S32 FacebookAuthenticate( lua_State* pState )
{
	Game::C_FacebookManager::Instance().Authenticate();
	return 0;
}

static S32 PublishFacebookStory( lua_State* pState )
{
	Game::C_FacebookManager::Instance().Publish();
	return 0;
}

static S32 UploadPhotoToFacebook( lua_State* pState )
{
	Game::C_FacebookManager::Instance().UploadPhoto(Game::C_ContentServerManager::Instance().GetUploadFileID(),UI::gDashboardInterface.GetString());
	return 0;
}

static int PumpFacebookCurrentTask( lua_State* pState )
{
	int error=BD_NO_ERROR;

	bool still_pumping=Game::C_FacebookManager::Instance().PumpCurrentTask(&error);

	lua_pushboolean(pState,still_pumping);
	lua_pushnumber(pState,error);

	return 2;
} 

static S32 FacebookCheckPublisherFile( lua_State* pState )
{
	C_FacebookManager::Instance().CheckPublisherFile();
	return 0;
}

static S32 FacebookHasPublisherFile( lua_State* pState )
{
	lua_pushboolean(pState,C_FacebookManager::Instance().HasPublisherFile() ? 1 : 0);
	return 1;
}

static S32 FacebookPublisherFileCheckFinished( lua_State* pState)
{
	lua_pushboolean(pState,C_FacebookManager::Instance().HasPublisherFileCheckFinished() ? 1 : 0);
	return 1;
}

static S32 FacebookGetNumStories( lua_State* pState )
{
	lua_pushnumber(pState,Game::C_SocialNetworkManager::Instance().GetNumSocialItems());
	return 1;
}

static S32 CreateNewFacebookStory( lua_State* pState )
{
	//Game::C_FacebookManager::Instance().AddOneOffFacebookStory(AMAX_FACEBOOK_TYPE_JUST_PLAYING,1);
	return 0;
}

static S32 EditFacebookStory( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"EditFacebookStory(id) - incorrect arguments");
		return 0;
	}

	Game::C_SocialNetworkManager::Instance().mCurrentMessage=UI::gDashboardInterface.GetString();

	return 0;
}

static S32 ClearFacebookStory( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"ClearFacebookStory(id) - incorrect arguments");
		return 0;
	}

	//Game::C_FacebookManager::Instance().ClearStory((int)lua_tointeger(pState,1));
	return 0;
}

// GetFacebookStoryType
static S32 GetFacebookStoryType( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetFacebookStoryType( index ) - incorrect arguments" );
		return 0;
	}

	S32 sIndex = (S32)lua_tonumber( pState, 1 );

	//AMAX_FACEBOOK_TYPE eType = AMAX_FACEBOOK_TYPE_JUST_PLAYING;

	//if ( ( sIndex >= 0 ) && ( sIndex < Game::C_FacebookManager::Instance().GetNumStories() ) )
	//{
	//	Game::AmaxFacebookStoryItem* fbs = Game::C_FacebookManager::Instance().GetStory( sIndex );

	//	if ( fbs )
	//		eType = fbs->mType;
	//}

	lua_pushnumber( pState, (S32)AMAX_SOCIAL_NETWORKING_ITEM_TYPE_PLAYING_BLUR );
	return 1;
}

static S32 FacebookIsRegistered( lua_State* pState )
{
	lua_pushboolean(pState,Game::C_FacebookManager::Instance().HasRegistered());
	return 1;
}

static S32 FacebookUnRegister( lua_State* pState )
{
	Game::C_FacebookManager::Instance().UnRegister();
	return 0;
}

static S32 CreateNewTweet( lua_State* pState )
{
	C_SocialNetworkManager::Instance().AddOneOff(AMAX_SOCIAL_NETWORKING_ITEM_TYPE_PLAYING_BLUR,0);
	return 0;
}

static S32 EditTweet( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"EditTweet(id) - incorrect arguments");
		return 0;
	}

	//Game::C_SocialNetworkManager::Instance().GetSocialItem((int)lua_tonumber(pState,1))->mText=UI::gDashboardInterface.GetString();
	//Game::C_TwitterManager::Instance().GetTweet((int)lua_tonumber(pState,1))->mText=UI::gDashboardInterface.GetString();

	Game::C_SocialNetworkManager::Instance().mCurrentMessage=UI::gDashboardInterface.GetString();

	return 0;
}

static S32 RemainingTweetSpace( lua_State* pState )
{
	lua_pushnumber(pState, Game::C_SocialNetworkManager::Instance().GetRemainingTweetSpace() );
	return 1;
}

static S32 CanShareInLobby( lua_State* pState )
{
	//lua_pushboolean(pState, Game::C_MpResults::Instance().m_ResultsValid );
	lua_pushboolean(pState, false );
	return 1;
}

static S32 GetFacebookTimeout( lua_State* pState )
{
	lua_pushnumber(pState,Game::C_FacebookManager::Instance().GetTimeout());
	return 1;
}

static S32 GetTwitterTimeout( lua_State* pState )
{
	lua_pushnumber(pState,Game::C_TwitterManager::Instance().GetTimeout());
	return 1;
}

static S32 GetMovieVolume( lua_State* pState )
{
	F32 volume = 0.0f;
	if( C_GameApp::GetInstance()->GetGameAudioLayer() )
		volume = C_GameApp::GetInstance()->GetGameAudioLayer()->FormatBinkMovieVolume(32768);

	lua_pushnumber(pState, volume );
	return 1;
}


static S32 GetMaxSpinnerItemSize( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"GetMaxSpinnerItemSize(id) - incorrect arguments");
		return 0;
	}	

	UI::C_GadgetBase* pGadget = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );

	F32 sizeX = 0.0f;
	if( pGadget && pGadget->GetType() == UI::E_UIGameButtonType_AmaxSlider )
	{
		sizeX = ((UI::UIAmaxSlider*)pGadget)->GetItemMaxSizeX();
	}

	lua_pushnumber( pState, sizeX );
	return 1;
}

static S32 ViewPhotoSearchGamerCard(lua_State* pState)
{
	return 0;
}

static S32 ViewPhotoTop20GamerCard(lua_State* pState)
{
	return 0;
}


static S32 DeleteStatsOnlyMatchingSession(lua_State* pState)
{
	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
	if (ns.IsInitialised())
	{
		ns.EnterLock();

		// we will be using the network races matchmaking session since it should not be in use in single player
		NetPlay::C_Session& matchSession = ns.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);
		matchSession.Delete();

		ns.Exitlock();
	}
	return 0;
}

static S32 StartCreateStatsOnlyMatchingSession(lua_State* pState)
{
	bool ok = false;

	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
	if (ns.IsInitialised())
	{
		ns.EnterLock();

		// we will be using the network races matchmaking session since it should not be in use in single player
		NetPlay::C_Session& matchSession = ns.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);
		matchSession.Delete();

		const S32 primary_pad = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		
		if (primary_pad > -1)
		{
			const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primary_pad);
			if (fp.Active() && fp.m_State == SysProfile::eProfileStateOnline && !fp.m_IsGuest)
			{
				U32 serverFlags = NetPlay::SESSION_CREATE_SINGLEPLAYER_WITH_STATS | NetPlay::SESSION_CREATE_HOST;

				// build platform specific matching server parameters list for single player mode (basically defaults for all in single player mode)
				RacerNet::NetMatchingParameterList matchingServerParamList;
				RacerNet::BuildMatchingParameters(matchingServerParamList);

				NetPlay::C_Session::SlotConfig matchSlotConfig;
				matchSlotConfig.m_MaxPublic		= 1;  // we only support the primary pad in single player mode so only need 1 player.
				matchSlotConfig.m_PublicInUse	= 0;
				matchSlotConfig.m_MaxPrivate	= 0;
				matchSlotConfig.m_PrivateInUse	= 0;

				// start the async create request.
				ok = matchSession.Create(primary_pad, serverFlags, &matchingServerParamList[0], matchingServerParamList.Size(), matchSlotConfig);
			}
		}

		ns.Exitlock();
	}
	lua_pushboolean(pState, ok);
	return 1;
}


static int ContinueCreateStatsOnlyMatchingSession(lua_State* pState)
{
	bool finished = true;
	bool success = false;

	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
	if (ns.IsInitialised())
	{
		ns.EnterLock();

		NetPlay::C_Session& matchSession = ns.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);
		switch (matchSession.SessionState())
		{
		case NetPlay::C_Session::eStateCreating:
			finished = false;
			break;

		case NetPlay::C_Session::eStateError:
		case NetPlay::C_Session::eStateNotActive:
			matchSession.Delete();
			break;

		case NetPlay::C_Session::eStateRunning:
			{
				// single player stats session created so add local primary players.
				const S32 primary_pad = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
				BIZ_ASSERT(primary_pad > -1);

				const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primary_pad);
				if (fp.Active() && fp.m_State == SysProfile::eProfileStateOnline && !fp.m_IsGuest)
				{
					if (!matchSession.AddPlayer(fp, true))
						matchSession.Delete();
					else
						success = true;
				}
				else
					matchSession.Delete();
			}
			break;
		}

		ns.Exitlock();
	}

	lua_pushboolean(pState, finished);
	lua_pushboolean(pState, success);
	return 2;

}

static S32 StartPlayStatsOnlyMatchingSession(lua_State* pState)
{
	bool sessionStarted = false; 

	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
	if (ns.IsInitialised())
	{
		ns.EnterLock();
		
		// we dont want to call this for network game modes as that has its own calls within lua_netrace
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if (pLayer != NULL && !pLayer->GetGameModePtr()->IsNetwork())
		{
			NetPlay::C_Session& matchSession = ns.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);
			sessionStarted = matchSession.StartPlay();
		}

		ns.Exitlock();
	}

	lua_pushboolean( pState, sessionStarted );
	return 1;
}

static S32 EndPlayStatsOnlyMatchingSession(lua_State* pState)
{
	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
	if (ns.IsInitialised())
	{
		ns.EnterLock();

		// we dont want to call this for network game modes as that has its own calls within lua_netrace
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if (pLayer != NULL && !pLayer->GetGameModePtr()->IsNetwork())
		{
			NetPlay::C_Session& matchSession = ns.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);
			matchSession.EndPlay(); // all stats we have written will now be sent to the server.
		}

		ns.Exitlock();
	}
	return 0;
}

static S32 FadeUpIngameAudio( lua_State* pState )
{
	Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eFadeInSfx);

	return 0;
}

static S32 FadeDownIngameAudio( lua_State* pState )
{
	Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eFadeOutSfx);

	return 0;
}



static S32 StartFriendEnumeration(lua_State* pState)
{
	C_FriendsManager::Instance().StartEnumeration();
	return 0;
}

static S32 GetNumFriends(lua_State* pState)
{
	lua_pushnumber(pState, 	C_FriendsManager::Instance().GetNumFriends());		
	return 1;
}

static S32 GetFriendsIDFromIndex(lua_State* pState)
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetFriendsIDFromIndex( index ) - incorrect arguments" );
		return 0;
	}

	S32 index = (S32)lua_tonumber( pState, 1 );

	lua_pushinteger(pState, 	C_FriendsManager::Instance().GetFriendIDFromIndex(index));
	return 1;
}

static S32 CheckScoreboardToLookAt(lua_State* pState)
{
	Game::C_GameProfile* prof = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(prof, "Could Not Get Profile in Check Scoreboard Type");

	AmaxEvent* ev = prof->GetCurrentEvent();
	BIZ_ASSERT_MSG(ev, "COULD NOT GET SORCERY DATA FOR SP LEADERBOARD READ");
			
	RacerNet::eScoreboards sbId =  ev->mLeaderboardSettings.mScoreboardToWriteTo;

	lua_pushnumber(pState, 	sbId);	

	return 1;
}

static S32 CheckSpScoreboardType(lua_State* pState)
{
	Game::C_GameProfile* prof = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(prof, "Could Not Get Profile in Check Scoreboard Type");

	AmaxEvent* ev = prof->GetCurrentEvent();
	BIZ_ASSERT_MSG(ev, "COULD NOT GET SORCERY DATA FOR SP LEADERBOARD READ");
			
	RacerNet::eSPScoreboardTypes sbType =  ev->mLeaderboardSettings.mScoreboardType;

	lua_pushnumber(pState, 	sbType);
	
	return 1;
}

static S32 SetSPEvent(lua_State* pState)
{
	if (!C_GameProfileManager::Instance().GetPrimaryProfile())
		return 0;

	C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentSPEventId = -1;

	if (lua_isnumber(pState, 1))
		C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentSPEventId = lua_tonumber(pState,1);
	
	return 0;
}

static S32 ShowMpTimer(lua_State* pState)
{
	F32 fTime(0.f);
	Game::C_LevelState& levelState = UI::GetLevelState();

	if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Elimination && levelState.GetMultiplayerSettings().mEliminationEvent == UI::E_EliminationEvent_Time )
		fTime = ((Game::C_RaceModeMultiplayer_Elimination*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr())->GetRaceTimeout();
	else
	if( levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
		fTime = levelState.mTimeLimit - UI::GetRLC()->GetRaceTime();
	else
	{
		// Show default timeout
		Game::C_RaceModeBase* pRaceMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();
		if ( pRaceMode )
			fTime = pRaceMode->GetRaceTimeout();
	}

	const bool showTimer = ( ( levelState.mRaceMode != Game::E_RaceMode_Multiplayer_Racing ) || ( fTime != 0.0f ) );

	lua_pushboolean(pState, showTimer ? 1 : 0 );
	return 1;
}

static S32 GetMultiplayerOptions( lua_State* pState )
{
	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	
	lua_pushnumber( pState, pProfile->mOptions.mMatchingRule );

	return 1;
}

static S32 SetMultiplayerOptions( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SetMultiplayerOptions( num ) - incorrect arguments");
		return 0;
	}

	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	
	pProfile->mOptions.mMatchingRule = (S8)lua_tonumber( pState, 1 );

	//Data gathering time
	Game::C_RaceBookLayer* pLayer = UI::GetRaceBookLayer();
	if(pLayer)
	{
		pLayer->GetDataGathering().FlagPostEvent(DATA_GATHERING_RECORD_TYPE_REGION_SETTING);
	}

	return 0;
}

static S32 RepairVehicle( lua_State* pState )
{
	if (!lua_isnumber(pState, 1))
		return 0;
	
	S32 vehicleIndex = (S32)lua_tonumber( pState, 1 );

	Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	if ( !pCore )
		return 0;

	Game::Actors::C_VehicleActor* pVehicleActor = (Game::Actors::C_VehicleActor*)pCore->GetVehicleActor( vehicleIndex );

	if ( !pVehicleActor )
		return 0;

	pVehicleActor->RepairForFinish();

	return 0;
}

static S32 UpdateMPSessionLeaderboard( lua_State* pState )
{

	C_ScoreboardManager::Instance().UpdateSessionLeaderboard();
	lua_pushnumber( pState, C_ScoreboardManager::Instance().GetSessionScoreboardData().Size() );	

	return 1;
}


static S32 SessionScoreboardGetRowRank( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SessionScoreboardGetRowRank(row) - incorrect arguments");
		return 0;
	}	

	U32 row = (U32)lua_tointeger( pState, 1 );

	Btl::Vector<SessionScoreboardData>& Data = C_ScoreboardManager::Instance().GetSessionScoreboardData();

	if( row > (U32)Data.Size() )
		return 0;

	lua_pushinteger( pState, Data[row-1].m_Rank );
	return 1;
}

static S32 SessionScoreboardGetRowLegend( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SessionScoreboardGetRowLegend(row) - incorrect arguments");
		return 0;
	}	

	U32 row = (U32)lua_tointeger( pState, 1 );

	Btl::Vector<SessionScoreboardData>& Data = C_ScoreboardManager::Instance().GetSessionScoreboardData();

	if( row > (U32)Data.Size() )
		return 0;

	lua_pushinteger( pState, Data[row-1].m_Legend );
	return 1;
}

static S32 SessionScoreboardIsMe( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SessionScoreboardGetRowLegend(row) - incorrect arguments");
		return 0;
	}	

	U32 row = (U32)lua_tointeger( pState, 1 );

	Btl::Vector<SessionScoreboardData>& Data = C_ScoreboardManager::Instance().GetSessionScoreboardData();

	if( row > (U32)Data.Size() )
		return 0;
	
	//Data Gathering time
	Game::C_RaceBookLayer* pLayer = UI::GetRaceBookLayer();
	if(pLayer)
	{
		pLayer->GetDataGathering().StoreSessionboardInfo(row, Data[row-1].m_RacePointsThisSession, (U8)Data[row-1].m_RacesPlayedThisSession);

	}

	lua_pushboolean( pState, Data[row-1].m_IsLocal );
	return 1;
}

static S32 SessionScoreboardTeam( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SessionScoreboardTeam(row) - incorrect arguments");
		return 0;
	}	

	S8 team = -1; //No team
	U32 row = (U32)lua_tointeger( pState, 1 );

	Btl::Vector<SessionScoreboardData>& data = C_ScoreboardManager::Instance().GetSessionScoreboardData();

	if( row > (U32)data.Size() )
		return 0;
	
	team = data[row-1].m_Team;

	lua_pushnumber( pState, team );
	return 1;
}

static S32 ShouldDisplayPumpkin( lua_State* pState )
{
	bool ShouldDisplaySpPumpkin = false;

	if(((Game::C_AchievementsManager::Instance().IsAchievementComplete(BLUR_ACHIEVEMENT_THE_REAL_HUSTLE)) == true))
	{
		ShouldDisplaySpPumpkin = true;
	}	

	lua_pushboolean(pState, ShouldDisplaySpPumpkin);

	return 1;
}

static S32 GetStickerInformation( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetStickerInformation( location, quadrant ) - incorrect arguments" );
		return 0;
	}

	U32 location = (U32)lua_tointeger(pState, 1);
	U32 index = (U32)lua_tointeger(pState, 2);
	C_Sticker* s = NULL;
	if( location < (U32)(Game::C_AchievementsManager::Instance().mStickerBookLocations.Size()) )
	{
		Achievement* a = Game::C_AchievementsManager::Instance().mStickerBookLocations[location];

		if( index < (U32)a->mStickers.Size() )
			s = a->mStickers[index];
	}

	if ( s && lua_type( pState, 3 ) == LUA_TNUMBER )
	{
		UI::C_GadgetBase *pObject = UI::gButtons.Find((U32)lua_tonumber(pState, 3));
		if(pObject && pObject->GetType() == UI::E_UIButtonType_StaticText )
		{
			((UI::UIStaticText*)pObject)->SetDirectText( s->GetUiDescription()->Chars() );
		}
	}

	lua_pushnumber(pState, s ? s->Progress() : -1);
	lua_pushnumber(pState, s ? s->Goal() : -1);
	lua_pushboolean(pState, s ? s->IsOneShot() : false);

	return 3;
}

static S32 HasNewStickers( lua_State* pState )
{
	if( lua_type( pState, 1 ) == LUA_TBOOLEAN )
	{
		Game::C_StickerManager::Instance().GetStickerBlackboard().mHasUnlockedNewStickers = ( lua_toboolean( pState, 1 ) != 0 );
		return 0;
	}
	else
	{
		lua_pushboolean(pState, Game::C_StickerManager::Instance().HasNewStickers());
		return 1;
	}
}

static S32 IsStickerInQuadrantComplete( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "IsStickerInQuadrantComplete( location, quadrant ) - incorrect arguments" );
		return 0;
	}

	U32 location = (U32)lua_tonumber(pState, 1);
	U32 quadrant = (U32)lua_tonumber(pState, 2);

	bool toReturn = false;
	if( location < (U32)(Game::C_AchievementsManager::Instance().mStickerBookLocations.Size()) )
	{
		Achievement* a = Game::C_AchievementsManager::Instance().mStickerBookLocations[location];
		toReturn = Game::C_AchievementsManager::Instance().GetStickerInQuadrant(a,quadrant)->IsComplete();
	}

	lua_pushboolean(pState, toReturn);

	return 1;
}

static S32 DividingLineBetweenQuadrants( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER || lua_type( pState, 3 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "DividingLineBetweenQuadrants( location, quadrant, quadrant ) - incorrect arguments" );
		return 0;
	}

	U32 location = (U32)lua_tonumber(pState, 1);
	U32 quadrant_1 = (U32)lua_tonumber(pState, 2);
	U32 quadrant_2 = (U32)lua_tonumber(pState, 3);

	bool toReturn = false;
	if( location < (U32)(Game::C_AchievementsManager::Instance().mStickerBookLocations.Size()) )
	{
		Achievement* a = Game::C_AchievementsManager::Instance().mStickerBookLocations[location];
		C_Sticker* quad1 = Game::C_AchievementsManager::Instance().GetStickerInQuadrant(a,quadrant_1);
		C_Sticker* quad2 = Game::C_AchievementsManager::Instance().GetStickerInQuadrant(a,quadrant_2);

		// we only want a dividing line if we haven't completed the achievement and if the stickers in adjacent quadrants are not the same
		if( !( Game::C_AchievementsManager::Instance().IsAchievementComplete(a) ) && quad1->Type() != quad2->Type() )
		{
			toReturn = true;
		}
	}

	lua_pushboolean(pState, toReturn);

	return 1;
}

static S32 GetNumStickersInAchievement( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetNumStickersInAchievement( location ) - incorrect arguments" );
		return 0;
	}
	
	U32 location = (U32)lua_tonumber(pState, 1);

	int toReturn = -1;
	if( location < (U32)(Game::C_AchievementsManager::Instance().mStickerBookLocations.Size()) )
	{
		Achievement* a = Game::C_AchievementsManager::Instance().mStickerBookLocations[location];
		toReturn = a->mStickers.Size();
	}

	lua_pushinteger(pState, toReturn);

	return 1;
}

static S32 PopulateAchievementName( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "PopulateAchievementName( location, textId ) - incorrect arguments" );
		return 0;
	}

	U32 location = (U32)lua_tonumber(pState, 1);
	UI::C_GadgetBase* object = UI::gButtons.Find( (S32)lua_tonumber( pState, 2 ) );

	if(object && object->GetType() == UI::E_UIButtonType_StaticText)
	{
		if( location < (U32)(Game::C_AchievementsManager::Instance().mStickerBookLocations.Size()) )
		{
			Achievement* a = Game::C_AchievementsManager::Instance().mStickerBookLocations[location];
			UI::C_TextStringID name( a->mName.id );
			((UI::UIStaticText*)object)->SetDirectText( name.String() );
		}
	}
	else
	{
		UI::g_pUILua->StackDump( pState, "PopulateAchievementName( location, textId ) - couldn't find textId or wasn't a StaticText object" );
	}

	return 0;
}

static S32 GetAchievementScore( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetAchievementScore( location ) - incorrect arguments" );
		return 0;
	}

	U32 location = (U32)lua_tonumber(pState, 1);
	int toReturn = 0;

	if( location < (U32)(Game::C_AchievementsManager::Instance().mStickerBookLocations.Size()) )
	{
		Achievement* a = Game::C_AchievementsManager::Instance().mStickerBookLocations[location];
		toReturn = (int)(Game::C_AchievementSystem::Instance().GetAchievementScore(a->mType));
	}

	lua_pushinteger( pState, toReturn );

	return 1;
}

static S32 GetAchievementTextureInfo( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetAchievementTextureInfo( location ) - incorrect arguments" );
		return 0;
	}

	U32 location = (U32)lua_tonumber(pState, 1);
	int texIdx = -1;
	int achCategory = -1;

	if( location < (U32)(Game::C_AchievementsManager::Instance().mStickerBookLocations.Size()) )
	{
		Achievement* a = Game::C_AchievementsManager::Instance().mStickerBookLocations[location];
		texIdx = (int)a->mTextureIndex;
		achCategory = (int)a->mCategory;
	}

	lua_pushinteger( pState, texIdx );
	lua_pushinteger( pState, achCategory );

	return 2;
}

static S32 GetNumberOfAchievements( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetNumberOfAchievements( filtering ) - incorrect arguments" );
		return 0;
	}

	S32 filtering = (S32)lua_tonumber(pState, 1);

	if( filtering ) // i.e. not set to "all"
	{
		lua_pushinteger(pState, Game::C_AchievementsManager::Instance().GetFilteredStickerLocations( filtering == 1 ).Size() );
	}
	else
	{
		lua_pushinteger(pState, Game::C_AchievementsManager::Instance().GetNumAchievements() );
	}
	
	return 1;
}

static int CheckStickerProgress (lua_State* pState)
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "CheckStickerProgress: No sticker value provided" );
	}
	
	S32 sticker = BLUR_STICKER_INVALID;
	S32 progress = 1;

	sticker = (S32)lua_tonumber( pState, 1 );

	if(lua_type( pState, 2 ) == LUA_TNUMBER)
	{
		progress = (S32)lua_tonumber( pState, 2 );
	}

	Game::C_Sticker* pSticker = Game::C_StickerManager::Instance().GetSticker((Game::BLUR_STICKER)sticker);
	if(pSticker)
	{
		pSticker->AddProgress(progress);
	}

	return 0;
}

// _SP_CheckRaceMode
static Game::C_RaceModeBase* SP_GetRaceMode( const Game::E_RaceMode eRaceMode )
{
	Game::C_RaceBookLayer* pRaceBookLayer = UI::GetRaceBookLayer();

	if ( !pRaceBookLayer || !pRaceBookLayer->GetRaceModePtr() || ( pRaceBookLayer->GetRaceMode() != eRaceMode ) )
		return NULL;

	return pRaceBookLayer->GetRaceModePtr();
}

// SP_IsStreetRace
static S32 SP_IsStreetRace( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_StreetRace ) != NULL ) );
	return 1;
}

// SP_IsCheckpointRace
static S32 SP_IsCheckpointRace( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_Checkpoint ) != NULL ) );
	return 1;
}

// SP_IsDestructionRace
static S32 SP_IsDestructionRace( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_Destruction ) != NULL ) );
	return 1;
}

// SP_IsFanDemandRace
static S32 SP_IsFanDemandRace( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_FanDemand ) != NULL ) );
	return 1;
}

// SP_IsBossRace
static S32 SP_IsBossRace( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_BossBattle ) != NULL ) );
	return 1;
}

// SP_CheckpointRaceParams
static S32 SP_CheckpointRaceParams( lua_State* pState )
{
	Game::C_RaceModeBase* pRaceModeBase = SP_GetRaceMode( Game::E_RaceMode_Checkpoint );

	if ( !pRaceModeBase )
	{
		pRaceModeBase = SP_GetRaceMode( Game::E_RaceMode_FriendDemand_Checkpoint );
		
		if ( !pRaceModeBase )
			return 0;
	}

	Game::C_RaceModeCheckpoint* pRace = (Game::C_RaceModeCheckpoint*)pRaceModeBase;

	AmaxRaceModeCheckpointVars* pVars = pRace->GetDifficultyVars();

	if ( !pVars )
		return 0;

	lua_pushnumber( pState, 0.0f );
	lua_pushnumber( pState, pVars->silverTimeRemaining );
	lua_pushnumber( pState, pVars->goldTimeRemaining );

	return 3;
}

// SP_StreetRaceState
static S32 SP_StreetRaceState( lua_State* pState )
{
	const Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor( 0 );

	if ( !pLogicActor )
		return 0;

	S32 sPosition = pLogicActor->GetPositionInfo().mRacePosition;
	SP_EventState eEventState = kSP_Incomplete;

		 if ( sPosition == 1 )	eEventState = kSP_Gold;
	else if ( sPosition == 2 )	eEventState = kSP_Silver;
	else if ( sPosition == 3 )	eEventState = kSP_Bronze;

	S32 sBestFans = SP_GetEventBestFans( SP_GetCurrentSaveGame(), SP_GetEventById( SP_GetCurrentSaveGame(), UI::GetLevelState().mSpEventId ) );

	lua_pushstring( pState, SP_GetEventStateName( eEventState ) );
	lua_pushnumber( pState, sPosition );
	lua_pushboolean( pState, ( pLogicActor->GetScoring().mFinalScoreForUI > sBestFans ) );

	return 3;
}

// SP_CheckpointRaceState
static S32 SP_CheckpointRaceState( lua_State* pState )
{
	Game::C_RaceModeBase* pRaceModeBase = SP_GetRaceMode( Game::E_RaceMode_Checkpoint );

	if ( !pRaceModeBase )
	{
		pRaceModeBase = SP_GetRaceMode( Game::E_RaceMode_FriendDemand_Checkpoint );
		
		if ( !pRaceModeBase )
			return 0;
	}

	Game::C_RaceModeCheckpoint* pRace = (Game::C_RaceModeCheckpoint*)pRaceModeBase;

	F32 fBestTime = SP_GetEventBestTime( SP_GetCurrentSaveGame(), SP_GetEventById( SP_GetCurrentSaveGame(), UI::GetLevelState().mSpEventId ) );

	lua_pushstring( pState, SP_GetEventStateName( pRace->GetEventState() ) );
	lua_pushnumber( pState, pRace->GetCheckpointsPassed() );
	lua_pushnumber( pState, pRace->GetTimeLeft() );
	lua_pushboolean( pState, ( pRace->GetTimeLeft() > fBestTime ) );

	return 4;
}



// MP_GetTimeout
static S32 MP_GetTimeout( lua_State* pState )
{
	Game::C_RaceBookLayer* pRaceBookLayer = UI::GetRaceBookLayer();

	if ( !pRaceBookLayer )
		return 0;

	Game::C_RaceModeBase* pRaceMode = pRaceBookLayer->GetRaceModePtr();

	if ( !pRaceMode )
		return 0;

	lua_pushnumber( pState, pRaceMode->GetRaceTimeout() );

	return 1;
}

// SP_DestructionRaceState
static S32 SP_DestructionRaceState( lua_State* pState )
{
	Game::C_RaceModeBase* pRaceModeBase = SP_GetRaceMode( Game::E_RaceMode_Destruction );

	if ( !pRaceModeBase )
	{
		pRaceModeBase = SP_GetRaceMode( Game::E_RaceMode_FriendDemand_Destruction );
		
		if ( !pRaceModeBase )
			return 0;
	}

	Game::C_RaceModeDestruction* pRace = (Game::C_RaceModeDestruction*)pRaceModeBase;

	S32 sBestPoints = SP_GetEventBestPoints( SP_GetCurrentSaveGame(), SP_GetEventById( SP_GetCurrentSaveGame(), UI::GetLevelState().mSpEventId ) );

	lua_pushstring( pState, SP_GetEventStateName( pRace->GetEventState() ) );
	lua_pushnumber( pState, pRace->GetNumPoints() );
	lua_pushnumber( pState, pRace->GetTimeLeft() );
	lua_pushboolean( pState, ( pRace->GetNumPoints() > sBestPoints ) );

	return 4;
}

// SP_FanDemandRaceState
static S32 SP_FanDemandRaceState( lua_State* pState )
{
	Game::C_RaceModeBase* pRaceModeBase = SP_GetRaceMode( Game::E_RaceMode_FanDemand );

	if ( !pRaceModeBase )
	{
		pRaceModeBase = SP_GetRaceMode( Game::E_RaceMode_FriendDemand_FanDemand );
		
		if ( !pRaceModeBase )
			return 0;
	}

	Game::C_RaceModeFanDemand* pRace = (Game::C_RaceModeFanDemand*)pRaceModeBase;

	S32 sBestFans = SP_GetEventBestFans( SP_GetCurrentSaveGame(), SP_GetEventById( SP_GetCurrentSaveGame(), UI::GetLevelState().mSpEventId ) );

	lua_pushstring( pState, SP_GetEventStateName( pRace->GetEventState() ) );
	lua_pushnumber( pState, pRace->GetNumDemandsComplete() );
	lua_pushnumber( pState, pRace->GetTimeLeft() );
	lua_pushboolean( pState, ( pRace->GetNumFans() > sBestFans ) );

	return 4;
}

// SP_BossRaceState
static S32 SP_BossRaceState( lua_State* pState )
{
	const Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor( 0 );

	if ( !pLogicActor )
		return 0;

	SP_EventState eEventState = ( ( pLogicActor->GetPositionInfo().mRacePosition == 1 ) ? kSP_Gold : kSP_Incomplete );

	lua_pushstring( pState, SP_GetEventStateName( eEventState ) );

	return 1;
}

// SP_IsFriendDemand
static S32 SP_IsFriendDemand( lua_State* pState )
{
	lua_pushboolean( pState, !( ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_StreetRace ) == NULL )
							&& ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_Destruction ) == NULL )
							&& ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_Checkpoint ) == NULL )
							&& ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_FanDemand ) == NULL )
							&& ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_BossBattle ) == NULL ) ) );
	return 1;
}

// SP_IsStreetRaceFD
static S32 SP_IsStreetRaceFD( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_StreetRace ) != NULL ) );
	return 1;
}
// SP_IsDestructionFD
static S32 SP_IsDestructionFD( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_Destruction ) != NULL ) );
	return 1;
}
// SP_IsCheckpointFD
static S32 SP_IsCheckpointFD( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_Checkpoint ) != NULL ) );
	return 1;
}
// SP_IsFanDemandFD
static S32 SP_IsFanDemandFD( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_FanDemand ) != NULL ) );
	return 1;
}
// SP_IsBossBattleFD
static S32 SP_IsBossBattleFD( lua_State* pState )
{
	lua_pushboolean( pState, ( SP_GetRaceMode( Game::E_RaceMode_FriendDemand_BossBattle ) != NULL ) );
	return 1;
}

// SP_ParamsFD
static S32 SP_ParamsFD( lua_State* pState )
{
	const Game::FriendChallenge* pFC = C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	if ( !pFC )
		return 0;

	lua_pushstring( pState, SP_GetEventStateName( (SP_EventState)pFC->mMedal ) );
	lua_pushnumber( pState, pFC->mParameters[ 0 ].mModifier );
	lua_pushnumber( pState, pFC->mParameters[ 0 ].mParamType );

	return 3;
}

// SP_StateFD
static S32 SP_StateFD( lua_State* pState )
{
	const Game::FriendChallenge* pFC = C_FriendChallenges::Instance().GetAttemptingFriendDemand();

	if ( !pFC )
		return 0;

	Game::FRIEND_CHALLENGE_RESULT_TYPE eResultType = C_FriendChallenges::Instance().AttemptingCriteriaResult();
	bool bPassingCriteria = ( ( eResultType == FRIEND_CHALLENGE_RESULT_TYPE_PASSING ) || ( eResultType == FRIEND_CHALLENGE_RESULT_TYPE_PASSED ) );

	Game::FRIEND_CHALLENGE_RESULT_TYPE eParamType = C_FriendChallenges::Instance().AttemptingParamResult();
	bool bPassingParam = ( ( eParamType == FRIEND_CHALLENGE_RESULT_TYPE_PASSING ) || ( eParamType == FRIEND_CHALLENGE_RESULT_TYPE_PASSED ) );

	lua_pushboolean( pState, bPassingCriteria );
	lua_pushboolean( pState, bPassingParam );

	return 2;
}

// SP_GetCharacterFromVehicle
static S32 SP_GetCharacterFromVehicle( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "SP_GetCharacterFromVehicle( vehicle_index ) - incorrect arguments" );
		return 0;
	}

	S32 sVehicleIndex = (S32)lua_tonumber( pState, 1 );
	U32 uCharacterId = UI::GetLevelState().mCars[ sVehicleIndex ].mCharacterID;

	RaceBook::C_RaceBookProfile* pRaceBookProfile = RaceBook::C_RaceBookManager::Instance().GetProfileByID( (GAMEDATA_ID)uCharacterId );

	if ( !pRaceBookProfile )
		return 0;

	lua_newtable( pState );
	pRaceBookProfile->SendToLua( pState );

	return 1;
}

// SP_GetLevelResult
static S32 SP_GetLevelResult( lua_State* pState )
{
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

	SP_Event* pBaseEvent = SP_GetEventById( SP_GetCurrentSaveGame(), levelState.mSpEventId );
	if( !pBaseEvent )
	{
		LOGWARN( g_Log, DBSTR " Invalid single player event" );
		return 0;
	}

	S32 sSecondaryTarget = pBaseEvent->fanPar;
	SP_EventState eEventState = (SP_EventState)results.mRaceStatistics.mMedal;

	lua_newtable( pState );
	Lua::AddDataMember( pState, "state", SP_GetEventStateName( eEventState ) );

	if ( ( eEventState == kSP_Incomplete ) || ( pBaseEvent->kind == kSP_BossEvent ) )
	{
		Lua::AddDataMember( pState, "secondary", false );
		Lua::AddDataMember( pState, "gfd",  false );
	}
	else
	{
		Lua::AddDataMember( pState, "secondary", ( results.mRaceStatistics.mFansEarned >= sSecondaryTarget ) );
		Lua::AddDataMember( pState, "gfd", results.mRaceStatistics.mGoldenFanDemandComplete );
	}

	if ( results.mNumCars > 0 )
		Lua::AddDataMember( pState, "position", results.mCars[ 0 ].mPosition );

	return 1;
}

// _ShowAiNames
static S32 _ShowAiNames( lua_State* pState )
{
	lua_pushboolean( pState, UI::ShowAiNames() );
	return 1;
}

static S32 SetOnlineRanked( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		return 0;
	}

	const bool ranked = lua_toboolean( pState, 1 ) ? true : false;

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

	if ( pLayer && pLayer->GetGameModePtr()->GetID() == E_GameMode_Online )
	{
		pLayer->GetGameModePtr()->SetRanked( ranked );
	}

	return 0;
}

static S32 PlayerHasLastGasp( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SessionScoreboardGetRowRank(row) - incorrect arguments");
		return 0;
	}	

	U32 vehicleIndex = (U32)lua_tointeger( pState, 1 );
	bool playerHasLastGaspMod = false;

	Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
	Game::C_MultiplayerVehicleAbility* LastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);

	// If we have the last gasp mod
	if(LastGaspAbility->GetActive())
	{	
		playerHasLastGaspMod = true;
	}

	lua_pushboolean( pState,  playerHasLastGaspMod);

	return 1;
}

static S32 PlayerHasJumpTheGun( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SessionScoreboardGetRowRank(row) - incorrect arguments");
		return 0;
	}	

	U32 vehicleIndex = (U32)lua_tointeger( pState, 1 );
	bool playerHasJumpTheGunMod = false;

	Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
	Game::C_MultiplayerVehicleAbility* pJumpTheGunAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Jump_The_Gun);

	// If we have the last gasp mod
	if(pJumpTheGunAbility->GetActive())
	{	
		playerHasJumpTheGunMod = true;
	}

	lua_pushboolean( pState,  playerHasJumpTheGunMod);

	return 1;
}


static S32 PlayerHasECM( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SessionScoreboardGetRowRank(row) - incorrect arguments");
		return 0;
	}	

	U32 vehicleIndex = (U32)lua_tointeger( pState, 1 );
	bool playerHasECM = false;

	Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
	Game::C_MultiplayerVehicleAbility* pECMMod = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_ECM);

	// If we have the last gasp mod
	if(pECMMod->GetActive())
	{	
		playerHasECM = true;
	}

	lua_pushboolean( pState,  playerHasECM);

	return 1;
}

static S32 GetWreckedSequenceDuration( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"GetJumpTheGunWreckedSequenceDuration() - incorrect arguments");
		return 0;
	}	

	U32 vehicleIndex = (U32)lua_tointeger( pState, 1 );

	Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor(vehicleIndex);
	float wreckedSequenceDuration = 0.0f; 
	if(pLogicActor)
	{
		// The logic actor knows how long its wreck should last , based on which mods are / are not equipped
		wreckedSequenceDuration = pLogicActor->GetWreckedSequenceDuration();
	}
	else
	{
		// return default
		wreckedSequenceDuration = UI::GetRLC()->GetRLSystem()->mDamageSystem->mParameters->mResetTimerAfterWrecked;
	}

	lua_pushnumber( pState,  wreckedSequenceDuration );

	return 1;
}

static S32 LastGaspModeActive( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		return 0;
	}	

	U32 vehicleIndex = (U32)lua_tointeger( pState, 1 );
	bool lastGaspModeActive = false;

	// If we have the last gasp mod
	Game::Actors::C_VehicleLogicActor* plogicActor = UI::GetVehicleLogicActor(vehicleIndex);
	if(plogicActor && plogicActor->GetIsInLastGaspMode())
	{	
		lastGaspModeActive = true;
	}

	lua_pushboolean( pState,  lastGaspModeActive);

	return 1;
}

static S32 FireLastGasp( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SessionScoreboardGetRowRank(row) - incorrect arguments");
		return 0;
	}	

	U32 vehicleIndex = (U32)lua_tointeger( pState, 1 );
	bool playerHasLastGaspMod = false;

	Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();	
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities();
	Game::C_MultiplayerVehicleAbility* LastGaspAbility = pAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Last_Gasp);

	// If we have the last gasp mod.. wreck the vehicle and fire off the LG
	if(LastGaspAbility->GetActive())
	{	
		Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor(vehicleIndex);
		if(pLogicActor)
		{
			pLogicActor->FireLastGasp(); 
		}
	}

	return 0;
}

static S32 GetLastGaspTime( lua_State* pState )
{
	// Get the last gasp time from the vehicle logic actor
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		return 0;
	}	

	U32 vehicleIndex = (U32)lua_tointeger( pState, 1 );
	Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor(vehicleIndex);
	if(pLogicActor)
	{
		float time = pLogicActor->GetTimeTillLastGaspForceFire();
		lua_pushnumber(pState, time);
		return 1;
	}
	else
	{
		return 0;
	}
	
}

static S32 CanPlayGameModeVideo( lua_State* pState )
{
	bool canPlayGameModeVideo = false;

	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"CanPlayGameModeVideo( enum ) - incorrect arguments");
		return 0;
	}
	eSPVideoConfig VideoType = (eSPVideoConfig)lua_tointeger( pState, 1 );

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when looking to play sp videos");
	
	canPlayGameModeVideo = pGameProfile->ShouldPlayVideo(VideoType);

	lua_pushboolean(pState, canPlayGameModeVideo);
	return 1;
}

static S32 GetGameModeVideoFile( lua_State* pState )
{
	XtSz8 filename;
	bool canSkip = false;
	bool fullscreen = false;

	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"CanPlayGameModeVideo( enum ) - incorrect arguments");
		return 0;
	}

	eSPVideoConfig VideoType = (eSPVideoConfig)lua_tointeger( pState, 1 );

	switch(VideoType)
	{
		case VIDEO_GAME_MODE_RACE:
			{
				filename	= gUiVideoData->m_GameModes.m_RaceMode.m_Video;
				canSkip		= gUiVideoData->m_GameModes.m_RaceMode.m_CanSkip;
				fullscreen	= gUiVideoData->m_GameModes.m_RaceMode.m_Fullscreen;

			}
			break;
		case VIDEO_GAME_MODE_DESTRUCTION:
			{
				filename	= gUiVideoData->m_GameModes.m_DestructionMode.m_Video;
				canSkip		= gUiVideoData->m_GameModes.m_DestructionMode.m_CanSkip;
				fullscreen	= gUiVideoData->m_GameModes.m_DestructionMode.m_Fullscreen;
			}
			break;
		case VIDEO_GAME_MODE_CHECKPOINT:
			{
				filename	= gUiVideoData->m_GameModes.m_CheckpointMode.m_Video;
				canSkip		= gUiVideoData->m_GameModes.m_CheckpointMode.m_CanSkip;
				fullscreen	= gUiVideoData->m_GameModes.m_CheckpointMode.m_Fullscreen;
			}
			break;
		case VIDEO_GAME_MODE_FAN:
			{
				filename	= gUiVideoData->m_GameModes.m_FanMode.m_Video;
				canSkip		= gUiVideoData->m_GameModes.m_FanMode.m_CanSkip;
				fullscreen	= gUiVideoData->m_GameModes.m_FanMode.m_Fullscreen;
			}
			break;
		case VIDEO_GAME_MODE_ONEONONE:
			{
				filename	= gUiVideoData->m_GameModes.m_OneOnOne.m_Video;
				canSkip		= gUiVideoData->m_GameModes.m_OneOnOne.m_CanSkip;
				fullscreen	= gUiVideoData->m_GameModes.m_OneOnOne.m_Fullscreen;
			}
			break;
		case VIDEO_TUTORIAL_1:
			{
				filename	= gUiVideoData->m_Tutorials.m_Tutorial1.m_Video;
				canSkip		= gUiVideoData->m_Tutorials.m_Tutorial1.m_CanSkip;
				fullscreen	= gUiVideoData->m_Tutorials.m_Tutorial1.m_Fullscreen;
			}
			break;
		case VIDEO_TUTORIAL_2:
			{
				filename	= gUiVideoData->m_Tutorials.m_Tutorial2.m_Video;
				canSkip		= gUiVideoData->m_Tutorials.m_Tutorial2.m_CanSkip;
				fullscreen	= gUiVideoData->m_Tutorials.m_Tutorial2.m_Fullscreen;
			}
			break;
		case VIDEO_FAN_PAR:
			{
				filename	= gUiVideoData->m_FanModes.m_FanPar.m_Video;
				canSkip		= gUiVideoData->m_FanModes.m_FanPar.m_CanSkip;
				fullscreen	= gUiVideoData->m_FanModes.m_FanPar.m_Fullscreen;
			}
			break;
		case VIDEO_FAN_RUN:
			{
				filename	= gUiVideoData->m_FanModes.m_FanRun.m_Video;
				canSkip		= gUiVideoData->m_FanModes.m_FanRun.m_CanSkip;
				fullscreen	= gUiVideoData->m_FanModes.m_FanRun.m_Fullscreen;
			}
			break;
		case VIDEO_FAN_DEMAND:
			{
				filename	= gUiVideoData->m_FanModes.m_FanDemand.m_Video;
				canSkip		= gUiVideoData->m_FanModes.m_FanDemand.m_CanSkip;
				fullscreen	= gUiVideoData->m_FanModes.m_FanDemand.m_Fullscreen;
			}
			break;	
		case VIDEO_END_OF_GAME:
			{
				filename	= gUiVideoData->m_EndOfGame.m_EndCredits.m_Video;
				canSkip		= gUiVideoData->m_EndOfGame.m_EndCredits.m_CanSkip;
				fullscreen	= gUiVideoData->m_EndOfGame.m_EndCredits.m_Fullscreen;
			}
			break;
		case VIDEO_END_PHONECALL:
			{
				filename	= gUiVideoData->m_EndOfGame.m_Phonecall.m_Video;
				canSkip		= gUiVideoData->m_EndOfGame.m_Phonecall.m_CanSkip;
				fullscreen	= gUiVideoData->m_EndOfGame.m_Phonecall.m_Fullscreen;
			}
			break;
		case VIDEO_GLOSSY_INTRO:
			{
				filename	= gUiVideoData->m_GameModes.m_GlossyIntro.m_Video;
				canSkip		= gUiVideoData->m_GameModes.m_GlossyIntro.m_CanSkip;
				fullscreen	= gUiVideoData->m_GameModes.m_GlossyIntro.m_Fullscreen;
			}
			break;
		case VIDEO_POWER_UPS:
			{
				filename	= gUiVideoData->m_GameModes.m_PowerUps.m_Video;
				canSkip		= gUiVideoData->m_GameModes.m_PowerUps.m_CanSkip;
				fullscreen	= gUiVideoData->m_GameModes.m_PowerUps.m_Fullscreen;
			}
			break;

	}

	lua_pushfstring( pState, filename.Text() );
	lua_pushboolean( pState, canSkip);
	lua_pushboolean( pState, fullscreen);

	return 3;
}


static S32 GetUnlocksVideoFile( lua_State* pState )
{
	XtSz8	filename;
	bool	canSkip = false;
	bool	fullscreen = false;

	if( lua_type( pState, 1 ) != LUA_TNUMBER && lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"GetUnlocksVideoFile( enum, int tier ) - incorrect arguments");
		return 0;
	}

	eSPVideoConfig VideoType = (eSPVideoConfig)lua_tointeger( pState, 1 );
	S32 Tier = (S32)lua_tointeger( pState, 2 );

	switch(VideoType)
	{
		case VIDEO_TIER_UNLOCKS:
			{
				filename	= gUiVideoData->m_TierUnlock[Tier].m_Video;
				canSkip		= gUiVideoData->m_TierUnlock[Tier].m_CanSkip;
				fullscreen	= gUiVideoData->m_TierUnlock[Tier].m_Fullscreen;

			}
			break;
		case VIDEO_BOSS_UNLOCKS:
			{
				filename	= gUiVideoData->m_BossUnlock[Tier].m_Video;
				canSkip		= gUiVideoData->m_BossUnlock[Tier].m_CanSkip;
				fullscreen	= gUiVideoData->m_BossUnlock[Tier].m_Fullscreen;

			}
			break;
		case VIDEO_BOSS_BEATEN:
			{
				filename	= gUiVideoData->m_BossBeaten[Tier].m_Video;
				canSkip		= gUiVideoData->m_BossBeaten[Tier].m_CanSkip;
				fullscreen	= gUiVideoData->m_BossBeaten[Tier].m_Fullscreen;

			}
	}

	lua_pushfstring( pState, filename.Text() );
	lua_pushboolean( pState, canSkip);
	lua_pushboolean( pState, fullscreen);

	return 3;
}

static S32 SetVideoPlayedBitInProfile( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"CanPlayGameModeVideo( enum ) - incorrect arguments");
		return 0;
	}
	eSPVideoConfig VideoType = (eSPVideoConfig)lua_tointeger( pState, 1 );

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when looking to play sp videos");
	
	pGameProfile->SetVideoPlayed(VideoType);	

	return 0;
}

static S32 GetComboTimer( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"GetCombo(vehicle_index) - incorrect arguments");
		return 0;
	}

	Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor( UI::GetPlayerVehicleIndex( (S32)lua_tonumber( pState, 1 ) ) );

	if( pLogicActor )
	{
		lua_pushnumber(pState, pLogicActor->GetScoring().mComboTimer);
	}
	else
	{
		lua_pushnumber(pState, 0);
	}

	return 1;
}

static S32 GetRivalsEnabled( lua_State* pState )
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	lua_pushboolean(pState, levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction);

	return 1;
}

static S32 GetTierFromBossId( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"GetTierFromBossId( S32 BossId ) - incorrect arguments");
		return 0;
	}

	XtU32 bossId = (XtU32)lua_tointeger( pState, 1 );

	if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
	{
		if (SP_Boss* boss = SP_GetBossById(sg, bossId))
		{
			lua_pushnumber(pState, boss->name.id);
			return 1;
		}
	}

	return 0;
}

static S32 SetCurrentSpTier( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SetCurrentSpTier( S32 tier ) - incorrect arguments");
		return 0;
	}

	//	Tiers are 1-9 from lua, we want 0-8
	S32 tierNum = (S32)lua_tointeger( pState, 1 ) - 1;

	C_GameApp::GetInstance()->GetRaceBookLayer()->SetCurrentSpTier(tierNum);

	return 0;
}

static S32 GetPOBDLCInfo( lua_State* pState )
{
	static char *dlc[3] = {"Car Pack","City Pack","Power-up Pack"};

	lua_newtable(pState);
	for( S32 i = 0; i < 3; i++ )
	{
		lua_newtable( pState );
			
		Lua::AddDataMember( pState, "Name", dlc[i] );

		lua_rawseti(pState,-2,i+1);
	}

	return 1;
}

static S32 GetPOBFriendDemandInfo( lua_State* pState )
{
	lua_newtable( pState );

	Lua::AddDataMember( pState, "NewChallenges", 4 );
	Lua::AddDataMember( pState, "NewReChallenges", 8 );
	Lua::AddDataMember( pState, "ChallengesComplete", 3 );
	Lua::AddDataMember( pState, "ReChallengesComplete", 2 );

	return 1;
}

static S32 CheckIsNewsNew( lua_State* pState )
{
	bool isNew = false;
	
	MultiplayerConfig* multiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
	
	if(multiplayerConfig)
	{
		U16 newsId = multiplayerConfig->m_LatestBlurNewsItemId;

		Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when looking for new News");
		
		if(newsId > pGameProfile->LatestBlurNewsItemId())
		{
			isNew = true;
		}
		else
		{
			isNew = false;
		}
	}

	lua_pushboolean(pState, isNew);
	return 1;
}

static S32 SetNewNewsId( lua_State* pState )
{
	bool isNew = false;

	MultiplayerConfig* multiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
	BIZ_ASSERT_MSG(multiplayerConfig, "Could not get multiplayer config");

	U16 newsId = multiplayerConfig->m_LatestBlurNewsItemId;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when looking for new News");
	
	pGameProfile->LatestBlurNewsItemId(newsId);
	
	return 0;
}

static S32 CheckIsDLCNew( lua_State* pState )
{
	bool isNew = false;

	MultiplayerConfig* multiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
	BIZ_ASSERT_MSG(multiplayerConfig, "Could not get multiplayer config");
	
	U16 dlcId = multiplayerConfig->m_LatestDlcItemId;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when looking for new DLC");
	
	if(dlcId > pGameProfile->LatestDlcItemId())
	{
		isNew = true;
	}
	else
	{
		isNew = false;
	}

	lua_pushboolean(pState, isNew);
	return 1;
}

static S32 SetNewDlcId( lua_State* pState )
{
	bool isNew = false;

	MultiplayerConfig* multiplayerConfig = C_MultiplayerData::Instance().m_pConfig;
	BIZ_ASSERT_MSG(multiplayerConfig, "Could not get multiplayer config");

	U16 dlcId = multiplayerConfig->m_LatestDlcItemId;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when looking for new dlc");
	
	pGameProfile->LatestDlcItemId(dlcId);
	
	return 0;
}

static S32 CheckIsCommunityNewsNew( lua_State* pState )
{
	bool isNew = false;

	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;
	BIZ_ASSERT_MSG(multiplayerConfig, "Could not get multiplayer config");

	U16 communityNewsId = multiplayerConfig->m_LatestCommunityItemId;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when looking for new Community News");
	
	if(communityNewsId > pGameProfile->LatestCommunityItemId())
	{
		isNew = true;
	}
	else
	{
		isNew = false;
	}

	lua_pushboolean(pState, isNew);
	return 1;
}

static S32 SetNewCommunityNewsId( lua_State* pState )
{
	bool isNew = false;

	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;
	BIZ_ASSERT_MSG(multiplayerConfig, "Could not get multiplayer config");

	U16 communityNewsId = multiplayerConfig->m_LatestCommunityItemId;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when looking for new community news");
	
	pGameProfile->LatestCommunityItemId(communityNewsId);
	
	return 0;
}

static S32 CanShowPreviouslyOnBlur( lua_State* pState )
{
	bool canShowPreviouslyOnBlur = true;

	if( g_DebugPOB == false )
	{
		canShowPreviouslyOnBlur = false;
	}

	lua_pushboolean(pState, canShowPreviouslyOnBlur);
	
	return 1;
}

static S32 HasBeenPlayingSpGameLatest( lua_State* pState )
{		
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when HasBeenPlayingSpGameLatest");
	
	bool selectingSPGame = pGameProfile->SelectingSPGame();

	lua_pushboolean( pState, selectingSPGame);

	return 1;
}

static S32 SetPlayingSpGameLatest( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		UI::g_pUILua->StackDump(pState,"SetPlayingSpGameLatest( bool ) - incorrect arguments");
		return 0;
	}

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	BIZ_ASSERT_MSG(pGameProfile, "Could not get game profile when SetPlayingSpGameLatest");	

	bool playingSpGameLatest =  (lua_toboolean( pState, 1 ) != 0);

	bool shouldProfileSave = pGameProfile->SelectingSPGame(playingSpGameLatest);

	lua_pushboolean( pState, shouldProfileSave );

	return 1;
}

static S32 Lua_CheckSinglePlayerStickers( lua_State* pState)
{
	CheckSinglePlayerStickers();

	return 0;
}

static S32 SliderUpdateArrows( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SliderUpdateArrows( id(num) ) - incorrect arguments");
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );
	if ( pObject && pObject->GetType() == UI::E_UIGameButtonType_AmaxSlider )
	{
		((UI::UIAmaxSlider*)pObject)->UpdateArrows();
		return 0;
	}

	return 0;
}

static S32 ReadEventOwnerStats( lua_State* pState)
{
	C_EventOwnerManager::Instance().ReadFriendsStats();
	return 0;
}

static S32 EventOwnerState( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"EventOwnerState( id ) - incorrect arguments");
		return 0;
	}

	U32 eventId=(U32)lua_tointeger( pState, 1 );
	
	EVENT_OWNER_RESULT state=C_EventOwnerManager::Instance().BeatingFriend(eventId);

	lua_pushnumber(pState, state);

	return 1;
}

static S32 HasSinglePlayerRival( lua_State* pState )
{
	lua_pushboolean(pState, (C_EventOwnerManager::Instance().GetFriend() != 0));
	return 1;
}

static S32 Toggle( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "Toggle( id ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );
	if ( pObject && pObject->GetType() == UI::E_UIGameButtonType_Toggle )
	{
		((UI::UIToggle*)pObject)->Toggle();
		return 0;
	}

	return 0;
}

static S32 ChangeColour( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TSTRING )
	{
		UI::g_pUILua->StackDump( pState, "ChangeColour( id ) - incorrect arguments" );
		return 0;
	}

	UI::C_GadgetBase* pObject = UI::gButtons.Find( (U32)lua_tointeger( pState, 1 ) );
	if ( pObject && pObject->GetType() == UI::E_UIGameButtonType_Toggle )
	{
		S32 arg = 0;
		if ( lua_type( pState, 3 ) == LUA_TNUMBER )
			arg = (S32)lua_tonumber( pState, 3 );

		((UI::UIToggle*)pObject)->ChangeColour( arg, (const char*)lua_tostring( pState, 2 ) );
		return 0;
	}

	return 0;
}

static S32 GetLeaderboardVehicleId(lua_State* pState)
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"GetLeaderboardVehicleId( row(num) ) - incorrect arguments");
		return 0;
	}

	const S32 row = (S32)lua_tointeger( pState, 1 );

	S32 VehicleId = 0;

	if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
	{
		Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
		VehicleId = (U32)data.m_Columns[Game::eSPVehicleId].m_Data.m_S64;
	}

	lua_pushinteger(pState, VehicleId);

	return 1;
}

static S32 ChangeLocationNodeActor(lua_State* pState)
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TSTRING )
	{
		UI::g_pUILua->StackDump(pState,"ChangeLocatorActor( number, string ) - incorrect arguments");
		return 0;
	}

	S32 id = (S32)lua_tointeger( pState, 1 );

	// Find gadget
	UI::C_GadgetBase * pObject = UI::gButtons.Find( id );
	if ( !pObject || pObject->GetType() != UI::E_UIGameButtonType_LocatorNode )
	{
		LOGWARN( g_Log, DBSTR "Invalid gadget[id:%d", id );
		return 0;
	}

	((UI::UILocatorNode*)pObject)->ChangeLocation( lua_tostring( pState, 2 ) );
	return 0;
}

static S32 SetFriendCompare(lua_State* pState)
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"SetFriendCompare( row(num) ) - incorrect arguments");
		return 0;
	}

	S32 row = (S32)lua_tointeger( pState, 1 );

	row-=1;	//lua is 1 -> ... , C++ is 0 -> ...

	if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
	{
		Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );

		C_EventOwnerManager::Instance().SetFriend(data.m_EntityId);
	}

	return 0;
}

static S32 IsFriendCompareReading(lua_State* pState)
{
	lua_pushboolean(pState, C_EventOwnerManager::Instance().ReadingRivalStats());
	return 1;
}

static S32 ResetChromeVehicleGarageCheat(lua_State* pState)
{
	for( int i = 0; i < C_GameData::Instance().mCheatPacks.Number(); i++ )
	{
		AmaxCheatBase* cheatBase = C_GameData::Instance().mCheatPacks[i];
		if(cheatBase->GetType() == E_CHEAT_TYPE_CHROME)
		{
			AmaxCheatChromeVehicle* cheatChrome = (AmaxCheatChromeVehicle*)cheatBase;
			cheatChrome->mCurrentSequenceIdx = 0;			
		}
	}

	return 0;
}

static S32 CheckChromeVehicleGarageCheat(lua_State* pState)
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"CheckChromeVehicleGarageCheat( U32 id ) - incorrect arguments");
		return 0;
	}

	U32 carId				= (S32)lua_tointeger( pState, 1 );
	S32 controller			= SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	C_GameProfile* pProf	= C_GameProfileManager::Instance().GetProfile(controller);
	bool unlocked			= false;

	if(pProf)
	{
		for( int i = 0; i < C_GameData::Instance().mCheatPacks.Number(); i++ )
		{
			AmaxCheatBase* cheatBase = C_GameData::Instance().mCheatPacks[i];
			if(cheatBase->GetType() == E_CHEAT_TYPE_CHROME)
			{
				AmaxCheatChromeVehicle* cheatChrome = (AmaxCheatChromeVehicle*)cheatBase;

				if(cheatChrome->mAmaxVehicle.Object() && cheatChrome->mAmaxVehicle->ID == carId)
				{
					int buttonId		= cheatChrome->mButtonSequence[cheatChrome->mCurrentSequenceIdx];
					bool buttonPressed	= Input::GetGameInput(controller)->GetButtonPressedDebounced(buttonId);
					bool wrongButton	= false;

					for( int i = E_UiButton_Pad_Up; i < E_UiButton_Count; i++ )
					{
						if(Input::GetGameInput(controller)->GetButtonPressedDebounced(i) && i != buttonId)
						{
							wrongButton = true;
							break;
						}
					}

					if(wrongButton)
					{
						cheatChrome->mCurrentSequenceIdx = 0;
					}
					else if(buttonPressed)
					{
						cheatChrome->mCurrentSequenceIdx++;

						if(cheatChrome->mCurrentSequenceIdx >= cheatChrome->mButtonSequence.Size())
						{
							if(cheatChrome->mAmaxVehicleUnlock.Object())
							{
								Btl::InsertionResult< Btl::Set<U32>::Iterator > itRes = pProf->mVehicleUnlocks.Insert(cheatChrome->mAmaxVehicleUnlock->ID);

								if(itRes.inserted)
								{
									unlocked = true;
									C_MultiplayerData::Instance().PreCacheData();
								}
							}
						}
					}
				}
			}
		}
	}

	lua_pushboolean( pState, unlocked );
	return 1;
}

static S32 CanUseShare( lua_State* pState )
{
	const U8 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);

	bool online_possible = false;

#if defined ( PLATFORM_PS3 )
	
	online_possible = SysProfile::C_Manager::Instance().NetworkPlatformAccountAvailable() & NetPlay::C_Core::Instance().ValidLocalAddress();

#elif defined ( PLATFORM_PC )

	char* userName = Game::C_GameProfileManager::Instance().GetDemonwareUserName();
	char* password = Game::C_GameProfileManager::Instance().GetDemonwarePassword();
	if (userName != NULL && password != NULL)
	{
		online_possible = true;
	}

#endif

	if(fp.m_CommunicationsPrivilege && (Game::C_TwitterManager::Instance().HasPublisherFile() || 
		Game::C_FacebookManager::Instance().HasPublisherFile() || online_possible) )
	{
		lua_pushboolean(pState, true);
	}
	else
	{
		lua_pushboolean(pState, false);
	}
	
	return 1;
}

static S32 CacheGlobalAnimationResources( lua_State* pState )
{
//	MEMORY_TRACKER_PUSH("game.cachedresources");
//	C_GameApp::GetInstance()->CacheGlobalAnimationResources();
//	MEMORY_TRACKER_POP("game.cachedresources");

	return 0;
}

static S32 IsBootPreCachingComplete( lua_State* pState )
{
	lua_pushboolean( pState, C_GameApp::GetInstance()->IsBootPreCachingComplete() ? 1 : 0 );

	return 1;
}

static S32 IsInstallerScreenEnabled( lua_State* pState )
{
	lua_pushboolean( pState, C_GameApp::GetInstance()->IsInstallerScreenEnabled() ? 1 : 0 );

	return 1;
}

static S32 IsLoadingSegEnabled( lua_State* pState )
{
	lua_pushboolean( pState, C_GameApp::GetInstance()->IsLoadingSegEnabled() ? 1 : 0 );

	return 1;
}

static S32 BootUiSceneReady( lua_State* pState )
{
	lua_pushboolean( pState, C_GameApp::GetInstance()->UiSceneReady() ? 1 : 0 );

	return 1;
}

static S32 PostRaceSinglePlayer( lua_State* pState )
{
	const Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	C_GameProfile *game_prof=C_GameProfileManager::Instance().GetPrimaryProfile();

	if(!results.mFake)
	{
		game_prof->UploadPlayerStats();
		
		C_EventOwnerManager::Instance().WritePlayersStats();
	}

	SP_SaveGame* sg=SP_GetCurrentSaveGame();
	SP_Tier* tier=SP_GetTier(sg, 8);	//apologies for the hard coded 8
	if(tier)
	{
		SP_Boss* boss = tier->boss;
		if(boss)
		{
			if(SP_IsBossUnlocked(sg, boss))
			{
				Game::C_AvatarAwardSystem::Instance().GiveAvatarAward(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), Game::BLUR_AVATAR_AWARD_NINTH_BOSS_TEE);
			}
		}
	}

	if(results.mRaceStatistics.mMedal == kSP_Gold)
	{
		C_PictureAwardSystem::Instance().GivePictureAward(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), BLUR_PICTURE_AWARD_GOLD);
	}

	int complete=0;
	int count=0;

	for(int a=0;a<SP_GetTierCount(sg);a++)
	{
		SP_Tier *tier=SP_GetTier(sg,a);

		if(tier)
		{
			for(int b=0;b<tier->events.count;b++)
			{
				count++;

				if(SP_GetEventDifficulty(sg,tier->events[b]) == GAME_DIFFICULTY_HARD)
				{
					if(SP_GetEventState(sg, tier->events[b]) == kSP_Gold)
					{
						complete++;
					}
				}
			}
		}
	}

	if(count == complete)
	{
		C_PictureAwardSystem::Instance().GivePictureAward(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), BLUR_PICTURE_AWARD_HARD_GOLD);
	}

	return 0;
}

// FakeOpenFriendsList
static S32 FakeOpenFriendsList( lua_State* pState )
{
	_UI_MESSAGE_PAD( UI::E_UiMsg_CtrlButton_Back, SysProfile::C_Manager::Instance().GetPrimaryPadNo(), true, 1.0, 0, 0xffffffff );
	return 0;
}

// CleanPS3Install
static S32 CleanPS3Install( lua_State* pState )
{
#ifdef PLATFORM_PS3
	C_PS3InstallData::Instance().ReleaseResources();
#endif
	return 0;
}

static S32 GetNumberOfPeopleInRaceSummaryResults(lua_State* pState)
{
	S32 numPeopeInRaceSummary = 0;

	Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

	if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
	{
		Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;

		if( !socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
		{
			for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
				vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
			{
				if( (vehicleIterator) != NULL && ((vehicleIterator)->mSystemID != 0) )
				{	
					numPeopeInRaceSummary++;
				}
			}
		}				
	}			

	lua_pushinteger(pState, numPeopeInRaceSummary);

	return 1;
}


static S32 CanShareSticker(lua_State* pState)
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump(pState,"CanShareSticker( U32 index ) - incorrect arguments");
		return 0;
	}

	U32 index = (S32)lua_tointeger( pState, 1 );

	Achievement* achievement = Game::C_AchievementsManager::Instance().mStickerBookLocations[index];

	int complete=0;
	for(int a=0;a<achievement->mStickers.Size();a++)
	{
		if(achievement->mStickers[a]->IsComplete())
		{
			lua_pushboolean(pState, true);
			return 1;
		}
	}

	lua_pushboolean(pState, false);
	return 1;
}

static S32 BootFinished(lua_State* pState)
{
	PerformanceLogging::UpdateCounters(PerformanceLogging::kGameSubSessionLoaded);
	return 0;
}

static S32 RequestRivalRemotePicture(lua_State* pState)
{
	bdOnlineUserID friendid=C_EventOwnerManager::Instance().GetFriend();
	if(friendid)
	{
		UI::I_GamerPictureManager* pGamerPictureManager = UI::C_ProfileInterface::Instance().GamerPictureManager();
		if(pGamerPictureManager && C_FriendsManager::Instance().GetFriendProfileFromOnlineID(friendid))
		{
			pGamerPictureManager->ClearRemotePicture(0);
			pGamerPictureManager->RequestRemotePicture( SysProfile::C_Manager::Instance().GetPrimaryPadNo(), *C_FriendsManager::Instance().GetFriendProfileFromOnlineID(friendid), 0 );
		}
	}

	return 0;
}

static S32 InstallTrophies(lua_State* pState)
{
#ifdef PLATFORM_PS3
//	Game::C_AchievementSystem::Instance().Install();
#endif
	return 0;
}

static S32 TrophiesInstalled(lua_State* pState)
{
	bool installed = true;
#ifdef PLATFORM_PS3
	installed = Game::C_AchievementSystem::Instance().GetInstalled();
#endif
	lua_pushboolean( pState, installed );
	return 1;
}

// IsEnumeratingFriends
static S32 IsEnumeratingFriends( lua_State* pState )
{
	lua_pushboolean( pState, Game::C_FriendsManager::Instance().IsEnumerating() );
	return 1;
}

static S32 CanViewUserContent( lua_State* pState )
{
	const U8 padIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(padIndex);

	if(fp.m_ViewUserContentPrivilege)
		lua_pushboolean(pState, true);
	else
		lua_pushboolean(pState, false);

	return 1;
}

static S32 EnterNetworkLoad( lua_State* pState )
{
	C_GameApp::GetInstance()->GetRaceBookLayer()->EnterNetworkLoad();

	return 0;
}

static S32 ExitNetworkLoad( lua_State* pState )
{
	C_GameApp::GetInstance()->GetRaceBookLayer()->ExitNetworkLoad();

	return 0;
}

/**********************************************************************************************************************************************************************/
/**********************************************************************************************************************************************************************/
static const luaL_reg amax_lib[] = 
{
	{"GetLevelData",				GetLevelData},

	{"StartTheRace",				StartTheRace},
	{"SetGlobalFade",				SetGlobalFade},

	{"CheckChromeVehicleGarageCheat",	CheckChromeVehicleGarageCheat},
	{"ResetChromeVehicleGarageCheat",	ResetChromeVehicleGarageCheat},

	{"StartAnimatedCamera",			StartAnimatedCamera},
	{"AutoSelectAnimatedCamera",	AutoSelectAnimatedCamera},
	{"RestartCarsToStartPositions",	RestartCarsToStartPositions},
	{"DisableCarControls",			DisableCarControls},
	{"BlendAnimatedCamera",			BlendAnimatedCamera},
	{"GetAnimatedCameraParams",		GetAnimatedCameraParams},
	{"ChangeUiCamera",				ChangeUiCamera},
	{"ChangeUiSplineCamera",		ChangeUiSplineCamera},
	{"UpdateCameraActor",			UpdateCameraActor},
	{"ResetViewpointVisibility",	ResetViewpointVisibility},

	{"SetGameMode",					SetGameMode},
	{"GetGameMode",					GetGameMode},
	{"SetPlayMode",					SetPlayMode},
	{"GetPlayMode",					GetPlayMode},

	{"StartScoreboardWrite",		StartScoreboardWrite},
	{"ScoreboardWriteComplete",		ScoreboardWriteComplete},

	{"StartSPScoreboardWrite",		StartSPScoreboardWrite},
	{"SPScoreboardWriteComplete",	SPScoreboardWriteComplete},
	{"SPScooreboardWriteSuccess",	SPScooreboardWriteSuccess},

	{"StartYouScoreboardRead",		StartYouScoreboardRead},
	{"StartFriendScoreboardRead",	StartFriendScoreboardRead},
	{"StartLobbyScoreboardRead",	StartLobbyScoreboardRead},
	{"StartScoreboardRead",			StartScoreboardRead},
	{"ScoreboardReadComplete",		ScoreboardReadComplete},
	{"ScoreboardNumCurrentRows",	ScoreboardNumCurrentRows},
	{"ScoreboardLookAt",			ScoreboardLookAt},
	{"ScoreboardGetRowRank",		ScoreboardGetRowRank},
	{"ScoreboardGetRowLegend",		ScoreboardGetRowLegend},

	{"SetVehicleByID",			SetVehicleByID},
	{"GetVehicleByID",			GetVehicleByID},
	{"GetPlayerVehicleIndex",	GetPlayerVehicleIndex},
	{"GetCurrentCarClass",		GetCurrentCarClass},
	{"GetCurrentCarID",			GetCurrentCarID},

	{"IsGameModeNetworked",						IsGameModeNetworked},
	{"IsGameModeOnline",						IsGameModeOnline},
	{"IsGameModeRanked",						IsGameModeRanked},
	{"IsGameModeMultiplayer",					IsGameModeMultiplayer},
	{"IsGameModeSplitScreen",					IsGameModeSplitScreen},
	{"ShouldDisablePosition",					ShouldDisablePosition},
	{"AreAllSplitScreenHumanPlayersWrecked",	AreAllSplitScreenHumanPlayersWrecked},
	{"AreAllSplitScreenHumanPlayersEliminated",	AreAllSplitScreenHumanPlayersEliminated},
	{"EnsureVehicleStateValid",					EnsureVehicleStateValid},
	{"SetupRace",								SetupRace},
	{"GetNumViewports",							GetNumViewports},
	{"SetNumViewports",							SetNumViewports},
	{"GetPlayerToBeEliminated",					GetPlayerToBeEliminated},
	{"GetPlayersToBeEliminated",				GetPlayersToBeEliminated},
	{"SetOnlineRanked",							SetOnlineRanked},
	
	{"SetupResults",			SetupResults},
	{"SendMessage",				SendMessage},

	{"UseDebugHud",				UseDebugHud},
	{"GetDebugHudSettings",		GetDebugHudSettings},

	{"GetNumRules",				GetNumRules},
	{"HasPassedRule",			HasPassedRule},

	{"GetPassRuleString",		GetPassRuleString},
	{"GetFailRuleString",		GetFailRuleString},

	{"SetCurrentEvent",			SetCurrentEvent},
	{"SetCurrentCar",			SetCurrentCar},

	{"SetupDebugRace",			SetupDebugRace},
	{"SetDebugEvent",			SetDebugEvent},	

	{"ResetSubtitle",			ResetSubtitle},

	{"FinishIntro",				FinishIntro},
	{"FinishOutro",				FinishOutro},
	{"IsNetworkRace",			IsNetworkRace},
	{"IsGamePaused",			IsGamePaused},

	{"GetViewportVelocities",	GetViewportVelocities},
	{"ShowRevs",				ShowRevs},
	{"ShowCameraHelpers",		ShowCameraHelpers},
	{"GetRevs01",				GetRevs01},

	{"FillStreamer",			FillStreamer},
	{"WaitForStreamer",			WaitForStreamer},
	{"StreamerBlocksLoad",		StreamerBlocksLoad},

	{"RenderUIScene",			RenderUIScene},
	
	{"ChangeEnvironmentSettings",	ChangeEnvironmentSettings},
	{"DebugCutSceneStart",				DebugCutSceneStart},
	{"DebugCutSceneEnd",				DebugCutSceneEnd},
	{"GetDebugEvent",					GetDebugEvent},

	{"GetSetDebugDevMode",			GetSetDebugDevMode},
	{"ShouldPlayCutScene",			ShouldPlayCutScene},

	{"ParseHDDForReplays",			ParseHDDForReplays},
	{"GetReplayFilesFromHDD",		GetReplayFilesFromHDD},
	{"SetLoadReplayIndex",			SetLoadReplayIndex},
	{"SetEditCutsceneIndex",		SetEditCutsceneIndex},
	{"SetConvertReplayIndex",		SetConvertReplayIndex},

	{"CreateUiResources",			CreateUiResources},
	{"DestroyUiResources",			DestroyUiResources},

	{"GetNumVehicles",				GetNumVehicles},
	{"GetCurrentEvent",				GetCurrentEvent},
	{"GetRaceRewards",				GetRaceRewards},

	{"GetCurrentDebugEventID",		GetCurrentDebugEventID},
	{"SetCurrentDebugEventID",		SetCurrentDebugEventID},

	{"StartPhotoCam",				StartPhotoCam},
	{"EndPhotoCam",					EndPhotoCam},
	{"LockPhotoCamControls",		LockPhotoCamControls},
	{"SetPhotoFilters",				SetPhotoFilters},
	{"TakePhoto",					TakePhoto},

	{"PlayPostEventCheer",			PlayPostEventCheer},
	{"PlayVoiceMailMessage",		PlayVoiceMailMessage},
	{"StopVoiceMailMessage",		StopVoiceMailMessage},
	{"GetMessagePopup",				GetMessagePopup},
	
	{"CanSpectateVehicle",			CanSpectateVehicle},
	{"SpectatedVehicleIsEliminated",SpectatedVehicleIsEliminated},
	{"SpectateVehicle",				SpectateVehicle},
	{"GetSpectateVehicleData",		GetSpectateVehicleData},
	{"ToggleIngameScoreboard",		ToggleIngameScoreboard},
	{"GetWreckerModLoadout",		GetWreckerModLoadout},
	{"EndWreckedEffect",			EndWreckedEffect},
	{"IsMpTimeoutActive",			IsMpTimeoutActive},

	{"GetDrivingAssists",			GetDrivingAssists},
	{"GetHudConfig",				GetHudConfig},
	{"DistanceFromDriverFailing",	DistanceFromDriverFailing},
	{"NumRulesActive",				NumRulesActive},
	{"IsSinglePlayerRaceTimed",		IsSinglePlayerRaceTimed},
	{"IsRaceMpDestruction",			IsRaceMpDestruction},
	{"IsRaceDestruction",			IsRaceDestruction},

	// cut scenes
	{"HasStartCutScene",			HasStartCutScene},						// ToDo - THESE ARE DUMMY FUNCTIONS AND NEED REMOVING
	{"HasStartScriptedCamera",		HasStartScriptedCamera},
	{"HasRestartScriptedCamera",	HasRestartScriptedCamera},
	{"HasPassedCutScene",			HasPassedCutScene},
	{"HasPassedScriptedCamera",		HasPassedScriptedCamera},
	{"HasFailedScriptedCamera",		HasFailedScriptedCamera},
	{"OutroPrepared",				OutroPrepared},
	
	// PR - if we need many more like this we could probably create a Lua_GameButtons library
	{"ChangeText",					ChangeText},
	{"SliderUpdateArrows",			SliderUpdateArrows},
	{"Toggle",						Toggle},
	{"ChangeColour",				ChangeColour},
	{"ResolveAiHud",				ResolveAiHud},	
	{"SetChickletsValue",			SetChickletsValue},

	{"AssignCallback",				AssignCallback},
	{"GetCheckboxState",			GetCheckboxState},
	{"SetCheckboxState",			SetCheckboxState},

	{"AmaxWorldLayerExists",		AmaxWorldLayerExists},
	{"UIWorldLayerExists",			UIWorldLayerExists},	
	{"SkipCutscene",				SkipCutscene},
	{"GetOptionsDefaults",			GetOptionsDefaults},
	{"Options",						Options},
	{"Speakers",					Speakers},
	{"GammaCorrection",				GammaCorrection},
#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC graphics options
	{"UpdateGraphicsOptions",		UpdateGraphicsOptions},
	{"UpdateCustomKeyboardControls",	UpdateCustomKeyboardControls},	// [Blur2PC] retail Blur PC keyboard options
	{"GetPCKeyboard",				GetPCKeyboard},
	{"SetSplitscreenKeyboards",	SetSplitscreenKeyboards},	// [Blur2PC] retail PC split-screen input
	{"GraphicResolutionCount",		GraphicResolutionCount},
	{"GraphicAACount",				GraphicAACount},
	{"GetScreenResolution",			GetScreenResolution},
#endif
	{"UpdateAudioOptions",			UpdateAudioOptions},
	{"GetControllerConfigs",		GetControllerConfigs},
	{"Has321Started",				Has321Started},
	{"ShowMidPointHelp",			ShowMidPointHelp},
	
	{"InitialiseAdverts",			InitialiseAdverts},

	{"CanQuickRestart",				CanQuickRestart},
	{"IsFrontEndLoading",			IsFrontEndLoading},
	{"SetFrontEndLoaded",			SetFrontEndLoaded},
	{"SetLoadStateIntoRace",		SetLoadStateIntoRace},	
	{"SetLoadStateBackToUI",		SetLoadStateBackToUI},	
	{"SetLoadStateBackToLobby",		SetLoadStateBackToLobby},
	{"SetLoadStateIntoUI",			SetLoadStateIntoUI},	
	{"HandleUISceneReady",			HandleUISceneReady},
	{"EnterNetworkLoad",			EnterNetworkLoad},
	{"ExitNetworkLoad",				ExitNetworkLoad},

	{"GetShouldLoadDefaultCar",		GetShouldLoadDefaultCar},
	{"LoadCar",						LoadCar},
	{"LoadLockedCarModel",			LoadLockedCarModel},
	
	{"GetColourPalette",			GetColourPalette},
	{"SetColourPaletteLocked",		SetColourPaletteLocked},
	{"SetColourPickerFromRespray",	SetColourPickerFromRespray},
	{"ResprayCar",					ResprayCar},
	{"GetResprayColour",			GetResprayColour},

	{"FansInRace",					FansInRace},
	{"FanDemandsInRace",			FanDemandsInRace},
	{"ValidGoalInRace",				ValidGoalInRace},
	
	{"KickedOutOfGame",				KickedOutOfGame},




	{"NeedTwitterAuthenticate",		NeedTwitterAuthenticate},
	{"TwitterAuthenticate",			TwitterAuthenticate},
	{"TwitterUnAuthenticate",		TwitterUnAuthenticate},
	{"Tweet",						Tweet},
	{"GetTweets",					GetTweets },
	{"PumpTwitterCurrentTask",		PumpTwitterCurrentTask },
	{"ClearTweet",					ClearTweet},

	{"CreateBlurbFromIndex",		CreateBlurbFromIndex},
	{"CreateBlurb",					CreateBlurb},
	{"AddBlurb",					AddBlurb},
	{"GetBlurbs",					GetBlurbs},
	{"AddBlurbMessage",				AddBlurbMessage},
	{"ClearBlurbMessage",			ClearBlurbMessage},

	{"StoreTwitterUsername",		StoreTwitterUsername },
	{"StoreTwitterPassword",		StoreTwitterPassword },

	{"InitTwitter",					InitTwitter },
	{"TwitterCheckPublisherFile",	TwitterCheckPublisherFile },
	{"TwitterHasPublisherFile",		TwitterHasPublisherFile },
	{"TwitterPublisherFileCheckFinished",		TwitterPublisherFileCheckFinished },
	{"CreateNewTweet",				CreateNewTweet },
	{"EditTweet",					EditTweet },

	{"RemainingTweetSpace",			RemainingTweetSpace },
	{"CanShareInLobby",				CanShareInLobby },

	{"NeedFacebookAuthenticate",	NeedFacebookAuthenticate},
	{"FacebookAuthenticate",		FacebookAuthenticate},
	{"FacebookUnAuthenticate",		FacebookUnAuthenticate},
	{"FacebookGetNumStories",		FacebookGetNumStories},
	{"PublishFacebookStory",		PublishFacebookStory},
	{"UploadPhotoToFacebook",		UploadPhotoToFacebook},
	
	{"PumpFacebookCurrentTask",		PumpFacebookCurrentTask },
	
	{"StoreFacebookUsername",		StoreFacebookUsername },
	{"StoreFacebookPassword",		StoreFacebookPassword },
	
	{"InitFacebook",				InitFacebook },
	{"FacebookCheckPublisherFile",	FacebookCheckPublisherFile },
	{"FacebookHasPublisherFile",	FacebookHasPublisherFile },
	{"FacebookPublisherFileCheckFinished",	FacebookPublisherFileCheckFinished},

	{"CreateNewFacebookStory",		CreateNewFacebookStory },
	{"EditFacebookStory",			EditFacebookStory },
	{"ClearFacebookStory",			ClearFacebookStory },
	{"GetFacebookStoryType",		GetFacebookStoryType },

	{"FacebookIsRegistered",		FacebookIsRegistered },
	{"FacebookUnRegister",			FacebookUnRegister },

	{"GetFacebookTimeout",			GetFacebookTimeout },
	{"GetTwitterTimeout",			GetTwitterTimeout },


	{"HasIntroMessage",				HasIntroMessage},

	{"LoadTextureClone",			LoadTextureClone },

	{"Ps3NeedsInstall",				Ps3NeedsInstall },
	{"Ps3PrimeInstaller",			Ps3PrimeInstaller },
	{"Ps3PollInstaller",			Ps3PollInstaller },
	{"Ps3InstallerProgress",		Ps3InstallerProgress },
	{"SetLoadingScreen",			SetLoadingScreen },	

	{"CarouselScan",				CarouselScan },

	{"GetAchievementDetails",		GetAchievementDetails },
	{"ShowFanScoring",				ShowFanScoring },
	{"GetCityName",					GetCityName },
	{"GetRouteName",				GetRouteName },
	{"GetCityIconName",				GetCityIconName },

	{"DeleteTextureBundle",			DeleteTextureBundle },
	{"EnableAIControl",				EnableAIControl },

	{"BGMusicCheck",				BGMusicCheck },
	{"GetRecommendedVehicles",		GetRecommendedVehicles },

	{"StartSubtitles",				StartSubtitles },
	{"UpdateSubtitles",				UpdateSubtitles },
	{"EndSubtitles",				EndSubtitles },
	{"ChangeChildrenPanel",			ChangeChildrenPanel },
	{"ChangeChildrenLayer",			ChangeChildrenLayer },

	{"GameDataLogRetry",					GameDataLogRetry },
	{"GameDataLogDifficultySelected",		GameDataLogDifficultySelected },
	{"GameDataLogControllerLayoutChanged",	GameDataLogControllerLayoutChanged },
	{"GameDataLogBrightnessChanged",		GameDataLogBrightnessChanged },
	{"GameDataLogDisplaySettings",			GameDataLogDisplaySettings },

	{"GameInviteStarted",			GameInviteStarted},
	{"AmericanRegion",				AmericanRegion},

	{"SetUICarToMultiplayer",		SetUICarToMultiplayer},
	{"GetMovieVolume",				GetMovieVolume},
	{"GetMaxSpinnerItemSize",		GetMaxSpinnerItemSize},

	{"ViewPhotoSearchGamerCard",	ViewPhotoSearchGamerCard},
	{"ViewPhotoTop20GamerCard",		ViewPhotoTop20GamerCard},

	{"DeleteStatsOnlyMatchingSession",			DeleteStatsOnlyMatchingSession},
	{"StartCreateStatsOnlyMatchingSession",		StartCreateStatsOnlyMatchingSession},
	{"ContinueCreateStatsOnlyMatchingSession",	ContinueCreateStatsOnlyMatchingSession},
	{"StartPlayStatsOnlyMatchingSession",		StartPlayStatsOnlyMatchingSession},
	{"EndPlayStatsOnlyMatchingSession",			EndPlayStatsOnlyMatchingSession},
	
	{"FadeUpIngameAudio",			FadeUpIngameAudio},
	{"FadeDownIngameAudio",			FadeDownIngameAudio},
	{"StartFriendEnumeration",		StartFriendEnumeration},
	{"GetNumFriends",				GetNumFriends},
	{"GetFriendsIDFromIndex",		GetFriendsIDFromIndex},
	{"ShowMpTimer",					ShowMpTimer},

	{"CheckScoreboardToLookAt", CheckScoreboardToLookAt},
	{"CheckSpScoreboardType", CheckSpScoreboardType},

	{"GetMultiplayerOptions",		GetMultiplayerOptions},
	{"SetMultiplayerOptions",		SetMultiplayerOptions},

	{"RepairVehicle",				RepairVehicle},

	{"SetSPEvent", SetSPEvent},

	{"UpdateSessionLeaderboard",		UpdateMPSessionLeaderboard},
	{"SessionScoreboardGetRowRank",		SessionScoreboardGetRowRank},
	{"SessionScoreboardGetRowLegend",	SessionScoreboardGetRowLegend},
	{"SessionScoreboardIsMe",			SessionScoreboardIsMe},
	{"SessionScoreboardTeam",			SessionScoreboardTeam},	
	
	{"GetRivalsEnabled" ,				GetRivalsEnabled},
	{"MP_GetTimeout",					MP_GetTimeout},
	

	// sticker book
	{ "GetStickerInformation",			GetStickerInformation },
	{ "HasNewStickers",					HasNewStickers },
	{ "IsStickerInQuadrantComplete",	IsStickerInQuadrantComplete },
	{ "DividingLineBetweenQuadrants",	DividingLineBetweenQuadrants },
	{ "GetNumStickersInAchievement",	GetNumStickersInAchievement },
	{ "GetNumberOfAchievements",		GetNumberOfAchievements },
	{ "PopulateAchievementName",		PopulateAchievementName },
	{ "GetAchievementScore",			GetAchievementScore },
	{ "GetAchievementTextureInfo",		GetAchievementTextureInfo },
	{ "CheckStickerProgress",			CheckStickerProgress },

	// single player HUD
	{ "SP_IsStreetRace",				SP_IsStreetRace },
	{ "SP_IsCheckpointRace",			SP_IsCheckpointRace },
	{ "SP_IsDestructionRace",			SP_IsDestructionRace },
	{ "SP_IsFanDemandRace",				SP_IsFanDemandRace },
	{ "SP_IsBossRace",					SP_IsBossRace },

	{ "SP_CheckpointRaceParams",		SP_CheckpointRaceParams },

	{ "SP_StreetRaceState",				SP_StreetRaceState },
	{ "SP_CheckpointRaceState",			SP_CheckpointRaceState },
	{ "SP_DestructionRaceState",		SP_DestructionRaceState },
	{ "SP_FanDemandRaceState",			SP_FanDemandRaceState },
	{ "SP_BossRaceState",				SP_BossRaceState },

	// friend demands
	{ "SP_IsFriendDemand",				SP_IsFriendDemand },
	{ "SP_IsStreetRaceFD",				SP_IsStreetRaceFD },
	{ "SP_IsDestructionFD",				SP_IsDestructionFD },
	{ "SP_IsCheckpointFD",				SP_IsCheckpointFD },
	{ "SP_IsFanDemandFD",				SP_IsFanDemandFD },
	{ "SP_IsBossBattleFD",				SP_IsBossBattleFD },

	{ "SP_ParamsFD",					SP_ParamsFD },
	{ "SP_StateFD",						SP_StateFD },

	{ "SP_GetCharacterFromVehicle",		SP_GetCharacterFromVehicle },
	{ "SP_GetLevelResult",				SP_GetLevelResult },

	{ "ShowAiNames",					_ShowAiNames },

	{"PlayerHasLastGasp",				PlayerHasLastGasp},
	{"FireLastGasp",					FireLastGasp},
	{"LastGaspModeActive",				LastGaspModeActive},
	{"GetLastGaspTime",					GetLastGaspTime},

	{"CanPlayGameModeVideo",			CanPlayGameModeVideo},
	{"GetGameModeVideoFile",			GetGameModeVideoFile},
	{"SetVideoPlayedBitInProfile",		SetVideoPlayedBitInProfile},
	{"GetUnlocksVideoFile",				GetUnlocksVideoFile},

	{"GetComboTimer",					GetComboTimer},

	{"GetRivalsEnabled" ,				GetRivalsEnabled},

	{"GetTierFromBossId",				GetTierFromBossId},
	{"SetCurrentSpTier",				SetCurrentSpTier},

	{"GetPOBDLCInfo",					GetPOBDLCInfo},
	{"GetPOBFriendDemandInfo",			GetPOBFriendDemandInfo},
	{"CanShowPreviouslyOnBlur",			CanShowPreviouslyOnBlur},
	{"CheckSinglePlayerStickers",		Lua_CheckSinglePlayerStickers},

	{"PlayerHasJumpTheGun",				PlayerHasJumpTheGun},
	{"PlayerHasECM",					PlayerHasECM},
	{"GetWreckedSequenceDuration" , 	GetWreckedSequenceDuration},

	{"ReadEventOwnerStats",				ReadEventOwnerStats},
	{"EventOwnerState",					EventOwnerState},
	{"GetLeaderboardVehicleId",			GetLeaderboardVehicleId},
	{"ChangeLocationNodeActor",			ChangeLocationNodeActor},

	{"SetFriendCompare",				SetFriendCompare},
	{"IsFriendCompareReading",			IsFriendCompareReading},

	{"ShouldDisplayPumpkin",			ShouldDisplayPumpkin},

	{"CanUseShare",						CanUseShare},
	{"HasSinglePlayerRival",			HasSinglePlayerRival},
	{"CacheGlobalAnimationResources",	CacheGlobalAnimationResources},
	{"IsBootPreCachingComplete",		IsBootPreCachingComplete},
	{"BootUiSceneReady",				BootUiSceneReady},
	{"IsInstallerScreenEnabled",		IsInstallerScreenEnabled},
	{"IsLoadingSegEnabled",				IsLoadingSegEnabled},

	{"PostRaceSinglePlayer",			PostRaceSinglePlayer},

	{"FakeOpenFriendsList",				FakeOpenFriendsList},
	{"SetNewCommunityNewsId",			SetNewCommunityNewsId},
	{"CheckIsCommunityNewsNew",			CheckIsCommunityNewsNew},
	{"SetNewDlcId",						SetNewDlcId},
	{"CheckIsDLCNew",					CheckIsDLCNew},
	{"SetNewNewsId",					SetNewNewsId},
	{"CheckIsNewsNew",					CheckIsNewsNew},
	{"HasBeenPlayingSpGameLatest",		HasBeenPlayingSpGameLatest},
	{"SetPlayingSpGameLatest",			SetPlayingSpGameLatest},

	{"GetNumberOfPeopleInRaceSummaryResults", GetNumberOfPeopleInRaceSummaryResults},
	
	{"CleanPS3Install",					CleanPS3Install},

	{"CanShareSticker",					CanShareSticker},

	{"BootFinished",					BootFinished},
	{"InstallTrophies",					InstallTrophies},
	{"TrophiesInstalled",				TrophiesInstalled},

	{"RequestRivalRemotePicture",		RequestRivalRemotePicture},

	{"IsEnumeratingFriends",			IsEnumeratingFriends},
	{"CanViewUserContent",				CanViewUserContent},

	{NULL,							NULL}		// end of list
};  


LUALIB_API int luaopen_amax(lua_State *pState)
{
	luaL_openlib(pState,"Amax",amax_lib,0);
	return 1;
}

} // end namespace Game

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
