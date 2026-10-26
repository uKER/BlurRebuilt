#include "stdafx.h"

#include <Game/Game.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <GameLogic\ActorStudioActors\CutSceneMessages.h>
#include <GameLogic\Messaging\InGameDialogueMessages.h>
#include <Game/Messaging\GameFlowMessages.h>
#include <Game/GameActors/Components/Messages/AudioMessages.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic\RaceIntroOutro\RaceIntroOutro.h>
#include <GameLogic/ActorStudioActors/DynamicLightActor.h>
#include <Game/GameWorld/EnvironmentSettings.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <Graphics/CutSceneLights.h>
#include <UI/UIMessages.h>
#include <GameLogic/Structures/LevelState.h>

#include "CutSceneProxyActor.h"
#include "Ui/SoundEffects.h"
#include <Ui/Game/Gadgets/ButtonMessages.h>


//=========================================================================
//					Debug Options
//=========================================================================

static void ExitCutSceneFunc(void *)
{
	_UI_MESSAGE( ActorMsg::E_CutSceneMsg_Finished );
}
static Debug::Function ExitCutScene("CutScene.General.ExitCutScene", ExitCutSceneFunc, 0);


static void ResetGlobalFadeFunc(void *)
{
	Actors::C_ActorUtils::Instance().gameworld->SetGlobalFade(1.0f);
}
static Debug::Function ResetGlobalFade("Game.ResetGlobalFade", ResetGlobalFadeFunc, 0);


Debug::Bool g_debugDisplayCameraCuts("CutScene.General.DisplayCameraCuts", false);


extern Debug::Bool g_UseCutSceneHeadlightNearCull;


//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::CutSceneProxyCreationData,		"CutSceneProxyActor")
	XT_BASE(Actors::C_GameActorCreate)
//	XT_FIELD(m_OnCutsceneSkip,					"OnCutsceneSkip")
XT_END_TYPE


//=========================================================================
//					Component Creation Setup
//=========================================================================


void Actors::CutSceneProxyCreationData::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
}




//=========================================================================
//					CutScene Proxy Actor Core Routines
//=========================================================================



namespace Actors
{


//=========================================================================

CutSceneProxyActor::CutSceneProxyActor()
{
}

//=========================================================================

CutSceneProxyActor::~CutSceneProxyActor()
{
}

//=========================================================================

bool CutSceneProxyActor::InitialiseImpl()					// Called first
{
//	S32		random_entry;

	if (!C_GameActor::InitialiseImpl())
		return(false);

	const CutSceneProxyCreationData *pCreate = GetCreationData();

	return(true);
}


//=========================================================================

bool CutSceneProxyActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	AddSubscriber(GameMessaging::E_GameMsg_Update);
	AddSubscriber(ActorMsg::E_AudioMsg_DialogueFinish);
	AddSubscriber(Game::E_GameflowMessage_RaceLoaded);
	AddSubscriber(ActorMsg::E_DynamicObjMsg_SorceryUpdate);
//	AddSubscriber(ActorMsg::E_DynamicObjMsg_CameraCut);		// Only does this logic if it's directly sent

	m_NumCameraCuts = 0;
	m_Active = false;
	return(true);
}

//=========================================================================

void CutSceneProxyActor::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
{
	switch(result)
	{
		case Resource::E_Notify_DataReady:
			break;
		
		case Resource::E_Notify_DataError:
			break;
		
		case Resource::E_Notify_NoData:
			break;
	}
}

//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool CutSceneProxyActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_GameActorMsg_Activate:
		{
			m_Active = true;
			break;
		}

		case ActorMsg::E_GameActorMsg_Deactivate:
		{
			m_Active = false;
			break;
		}

		case Game::E_GameflowMessage_RaceLoaded:
		{
			C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_InitialiseProxy );
			if (!Game::C_Game::GetInstance()->GetLevelState().mInUI)
				C_ActorUtils::Instance().gameworld->SetGlobalFade(0.0f);				// Make sure we're set to black before race starts
			break;
		}

		case ActorMsg::E_CutSceneMsg_StartTheIntro:
		{
//			C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_InitialiseProxy );
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(Game::E_AIMessage_InitialiseIntro);
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(Game::E_GameflowMessage_IntroStarted);
			break;
		}

		case Game::E_GameflowMessage_RaceFinished:					// Should take this out once single 
		case Game::E_GameflowMessage_VehicleFinished:
		{
			C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_RaceEnded );
			break;
		}

		case ActorMsg::E_CutSceneMsg_StartTheRace:
		{
			Game::RaceIntroOutro::Instance().FinishIntro();
			C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( pMessage );			// Pass message on to all actors (especially the vehicle proxys)
			break;
		}

		case ActorMsg::E_CutSceneMsg_Finished:
		{
			LOGINFO( Resource::g_Log, "Cutscene sent 'Finished' message");
			_UI_MESSAGE( ActorMsg::E_CutSceneMsg_Finished );
			break;
		}

		case ActorMsg::E_CutSceneMsg_PlayConversationAndSubtitle:
		{
				ActorMsg::C_PlayConversationAndSubtitleMsg *msg=(ActorMsg::C_PlayConversationAndSubtitleMsg *)pMessage;

				lua_getglobal(UI::g_pUILua->GetState(), "SetAmaxSubtitle");
				lua_pushinteger(UI::g_pUILua->GetState(), msg->mDialogue.id);
				lua_pushstring(UI::g_pUILua->GetState(), "!255 255 255 255");
				lua_pcall(UI::g_pUILua->GetState(), 2, 0, 0);

				if(msg->mSpokenLine != E_AMAXDIALOGUE_NONE)
				{			
					Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( msg->mSpokenLine );
				}
			break;
		}

		case ActorMsg::E_CutSceneMsg_LoadResources:
		{
//			const ActorMsg::C_LoadResourcesMsg* msg = Messaging::MessageCast<ActorMsg::C_LoadResourcesMsg>(pMessage);
			break;
		}

		case ActorMsg::E_CutSceneMsg_DumpResources:
		{
//			const ActorMsg::C_DumpResourcesMsg* msg = Messaging::MessageCast<ActorMsg::C_DumpResourcesMsg>(pMessage);
			break;
		}

		case ActorMsg::E_CutSceneMsg_Dialogue:
		{
			const ActorMsg::C_CutScene_DialogueMsg* msg = Messaging::MessageCast<ActorMsg::C_CutScene_DialogueMsg>(pMessage);

			Game::InGameDialogueEvent dialogueMsg(msg->mSoundIndex, msg->mSoundBank);
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&dialogueMsg);
			break;
		}
		
		case GameMessaging::E_GameMsg_Update:
		{
/*
			if (g_debugDisplayCameraCuts)
			{
				if (m_Active)
					LOGINFO( Resource::g_Log, "Camera Cut #%d", m_NumCameraCuts);
			}
*/
			break;
		}

		case ActorMsg::E_DynamicObjMsg_CameraCut:
		{
			m_NumCameraCuts++;
			if (g_debugDisplayCameraCuts)
			{
				if (m_Active)
					LOGINFO( Resource::g_Log, "Camera Cut #%d", m_NumCameraCuts);
			}
			C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( pMessage );		// Broadcast to the rest of the actors
			break;
		}

		case ActorMsg::E_AudioMsg_DialogueFinish:
		{
//			C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_CutSceneMsg_DialogueFinished );
			ActorMsg::C_CutScene_DialogueFinishedMsg msg;
			C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( &msg );
			break;
		}

		case ActorMsg::E_DynamicObjMsg_OutputDebug:
		{
			const ActorMsg::C_OutputDebugMsg* pMsg = Messaging::MessageCast<ActorMsg::C_OutputDebugMsg>(pMessage);
			LOGINFO( Resource::g_Log, "Debug Message String -> %s", pMsg->m_String.Chars());
			break;
		}

		case ActorMsg::E_DynamicObjMsg_EnableLevelRoute:
		{
			const ActorMsg::C_EnableLevelRouteMsg* pMsg = Messaging::MessageCast<ActorMsg::C_EnableLevelRouteMsg>(pMessage);
			C_ActorUtils::Instance().levelinstance->EnableRoute( pMsg->m_RouteNumber );
			break;
		}

		case ActorMsg::E_DynamicObjMsg_DisableLevelRoute:
		{
			const ActorMsg::C_DisableLevelRouteMsg* pMsg = Messaging::MessageCast<ActorMsg::C_DisableLevelRouteMsg>(pMessage);
			C_ActorUtils::Instance().levelinstance->DisableRoute( pMsg->m_RouteNumber );
			break;
		}

		case ActorMsg::E_CutSceneMsg_EnableRaceAI:
		{
			Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
			Game::Actors::C_VehicleActor* pVehicleActor =  (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(0);
			if (pVehicleActor)
			{
				Game::Actors::C_VehicleForceAIControlMsg controlMsg(0);
				pVehicleActor->SendMessageToActor(pVehicleActor->GetHandle(), &controlMsg);
			}

			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(pMessage);		// Pass on this message to rest of game logic
			break;
		}

		case ActorMsg::E_DynamicObjMsg_SelectLightingRig:
		{
			const ActorMsg::C_SelectLightingRigMsg* pMsg = Messaging::MessageCast<ActorMsg::C_SelectLightingRigMsg>(pMessage);
			m_CutSceneLights = pMsg->m_LightingDefinition;
			if (m_CutSceneLights == NULL)			// Send a null to switch lights off
			{
				CutScene::SetLight(0, MathsStorage::Float3(1.0f, 0.0f, 0.0f), MathsStorage::Float3(0.0f, 0.0f, 0.0f), 1.0f);
				CutScene::SetLight(1, MathsStorage::Float3(1.0f, 0.0f, 0.0f), MathsStorage::Float3(0.0f, 0.0f, 0.0f), 1.0f);
				CutScene::SetLight(2, MathsStorage::Float3(1.0f, 0.0f, 0.0f), MathsStorage::Float3(0.0f, 0.0f, 0.0f), 1.0f);
				CutScene::SetLight(3, MathsStorage::Float3(1.0f, 0.0f, 0.0f), MathsStorage::Float3(0.0f, 0.0f, 0.0f), 1.0f);
			}
			else
			{
				SetupLights();
			}
			break;
		}

		case ActorMsg::E_DynamicObjMsg_SorceryUpdate:
		{
			if (m_Active)
				SetupLights();
			break;
		}

		case ActorMsg::E_DynamicObjMsg_ChangeEnvironmentSettings:
		{
			const ActorMsg::C_ChangeEnvironmentSettingsMsg* pMsg = Messaging::MessageCast<ActorMsg::C_ChangeEnvironmentSettingsMsg>(pMessage);
//			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->CreateEnvironmentSettingsFromSorcery(false, (Game::EnvironmentSettings *)&pMsg->m_NewSettings);
			const Game::EnvironmentSettings *envset = (const Game::EnvironmentSettings *)pMsg->m_NewSettings;
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->CreateEnvironmentSettingsFromSorcery(false, (Game::EnvironmentSettings *)envset);
			if (envset == NULL)
				g_UseCutSceneHeadlightNearCull = false;
			else
				g_UseCutSceneHeadlightNearCull = true;
			break;
		}

		case ActorMsg::E_CutSceneMsg_DisplayCredit:
		{
			_UI_MESSAGE( ActorMsg::E_CutSceneMsg_DisplayCredit );
			break;
		}


		default:				// Any unrecognised messages will now be passed on to the world just like a message sender actor
			C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( pMessage );
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(pMessage);		// Pass on this message to rest of game logic
			break;
	}

	return(true);
}


//=========================================================================

void CutSceneProxyActor::SetupLights()
{
	if (m_CutSceneLights != NULL)
	{
		SetupLight(0, m_CutSceneLights->m_Heading0 + m_CutSceneLights->m_MasterHeading, m_CutSceneLights->m_Elevation0, m_CutSceneLights->m_Wrap0, m_CutSceneLights->m_Colour0.a, m_CutSceneLights->m_Colour0.r, m_CutSceneLights->m_Colour0.g, m_CutSceneLights->m_Colour0.b);
		SetupLight(1, m_CutSceneLights->m_Heading1 + m_CutSceneLights->m_MasterHeading, m_CutSceneLights->m_Elevation1, m_CutSceneLights->m_Wrap1, m_CutSceneLights->m_Colour1.a, m_CutSceneLights->m_Colour1.r, m_CutSceneLights->m_Colour1.g, m_CutSceneLights->m_Colour1.b);
		SetupLight(2, m_CutSceneLights->m_Heading2 + m_CutSceneLights->m_MasterHeading, m_CutSceneLights->m_Elevation2, m_CutSceneLights->m_Wrap2, m_CutSceneLights->m_Colour2.a, m_CutSceneLights->m_Colour2.r, m_CutSceneLights->m_Colour2.g, m_CutSceneLights->m_Colour2.b);
		SetupLight(3, m_CutSceneLights->m_Heading3 + m_CutSceneLights->m_MasterHeading, m_CutSceneLights->m_Elevation3, m_CutSceneLights->m_Wrap3, m_CutSceneLights->m_Colour3.a, m_CutSceneLights->m_Colour3.r, m_CutSceneLights->m_Colour3.g, m_CutSceneLights->m_Colour3.b);
	}
}

//=========================================================================

void CutSceneProxyActor::SetupLight(U32 index, F32 heading, F32 elevation, F32 wrap, F32 intensity, F32 red, F32 green, F32 blue)
{
	CutScene::SetLight(index, MathsStorage::Float3(cosf(heading * Maths::Pi / 180.0f) * cosf(elevation * Maths::Pi / 180.0f), sinf(elevation * Maths::Pi / 180.0f), sinf(heading * Maths::Pi / 180.0f) * cosf(elevation * Maths::Pi / 180.0f)), MathsStorage::Float3(red*intensity, green*intensity, blue*intensity), 1.5f - wrap);
}



} // namespace Actors
