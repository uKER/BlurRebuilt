#include "stdafx.h"
#include <Game/AmaxCam/CameraSystem.h>	// [Blur2PC]
#include "MusicSubsystem.h"	// [Blur2PC]

#include "GameAudioLayer.h"
//#include "StringHash.h"

#include "AppInterfaceLayer.h"
#include "UISubsystem.h"
#include <Ui/SoundEffects.h>
#include <UI/UIMessages.h>

#include "GameAudio\SystemSubsystem.H"

#include <GameApp.h>
#include <Game/Game.h>
#include <Game/Messaging/GameflowMessages.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

#include "Game/GameActors/Components/Messages/AudioMessages.h"
#include "Game\GameActors\Components\Messages\VehicleLogicMessages.h"

#include <GameLogic\Messaging\InGameDialogueMessages.h>

#include "ui/Game/Gadgets/ButtonMessages.h"

#include <Render/debug/DebugRender.h>

#include <GameLogic\Messaging\GameLogicMessages.h>

#include <GameLogic/Structures/LevelState.h>

#include <GameLogic\Profile\GameProfileManager.h>

#include <GameLogic/Profile/GameProfile.h>
#include "GameLogic/Profile/MultiplayerProfile.h"

#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"
#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"

#include <SchedulerConfig.h>

//#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
//#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
//#include <shared_actors/Components/Messages/ResourceMessages.h>
//#include "Game/GameActors/Components/Creation/AudioComponentsCreate.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

Debug::Bool gbUIMessages("Audio.Subsystem.UIMsgsEnabled", true, Debug::Persist());

Debug::Bool g_RenderEqEffect("Audio.Subsystem.Eq.RenderEffect", false, Debug::Persist());

//Debug::Float g_MemoryStatsThreshold("System.MemoryHUD.Threshold",	0.0f, Debug::Range(0.0f, 1500.0f) );
Debug::Float	g_RenderEqYPos("Audio.Subsystem.Eq.YPos", 30.f, Debug::Range(0.f, 2000.f));
Debug::Float	g_RenderEqXPos("Audio.Subsystem.Eq.XPos", 100, Debug::Range(-64.f, 2000.f));
Debug::Float	g_RenderBarScale("Audio.Subsystem.Eq.BarScale", 2, Debug::Range(0.f, 100.f));

Debug::Bool g_bLoadUI("Audio.Subsystem.LoadSubsystem.LoadUI", true, Debug::Persist());

Debug::Bool g_bTriggerUI("Audio.Subsystem.UI.TriggerUISound", false);
Debug::Int	g_UISoundIndex("Audio.Subsystem.UI.UISoundIndex", 2,Debug::Range(0, 100));

Debug::Float	g_MinVoiceMailThreshold("Audio.Subsystem.UI.VOThresholdMin", .1f, Debug::Range(0.f, 1.f));
Debug::Float	g_MaxVoiceMailThreshold("Audio.Subsystem.UI.VOThresholdMax", .8f, Debug::Range(0.f, 1.f));

//Debug::Bool gbStopMusic("Audio.Subsystem.StopMusic", false);



MT::Thread* g_pUISubSystemAsyncLoadThread = NULL;

class C_UISubSystemAsyncLoadThread : public MT::Thread
{
public:
	C_UISubSystemAsyncLoadThread(UISubsystem* pUISubSystem, const Util::C_String& entityName) : 
		MT::Thread(), 
		m_pUISubSystem(pUISubSystem),
		m_entityName(entityName)
	{
		BIZ_ASSERT_SPEED(pUISubSystem);
		m_entityName = entityName;
	}

	virtual ~C_UISubSystemAsyncLoadThread()
	{
	}

	virtual void ThreadMain()
	{
		C_GameApp::GetInstance()->GetGameAudioLayer()->GetAsyncLoadLock().Enter();
		m_pUISubSystem->StartLoadUIEntity(m_entityName);
		m_pUISubSystem->SetUiAudioAsyncPostLoadPending();
		C_GameApp::GetInstance()->GetGameAudioLayer()->GetAsyncLoadLock().Exit();
	}

private:
	UISubsystem* m_pUISubSystem;
	Util::C_String m_entityName;
};

//-----------------------------------------------------------------------------
// UISubsystem::UISubsystem
//-----------------------------------------------------------------------------
UISubsystem::UISubsystem()
{
	firstTimeLoad = true;
	m_bHaveInstalledUI = false;
	m_bShouldInstallUI = false;

	m_RaceInProgress = false;
	m_MPRaceInProgress = false;

	m_VoicemailStarted = false;

	m_GamePaused = false;

	m_trackingVoicemal = false;

	m_startTracking = false;

	m_goingToFrontEnd = false;

	m_hProxy = NULL;

	m_InGarage = false;

	m_hSchedulerGroup = Scheduler::CreateGroup();

	SetSubSystemState(E_GAMEFLOW_STATE_UNKNOWN);
	SetUiAudioLoadedState(eUiAudioNotLoaded);
	m_uiAudioAsyncPostLoadPending = false;
	m_activeUIEntity = eActiveUIEntity_None;

	m_intoUIForFirstTime = true;
	m_syncLoadInitialUpdateNeeded = false;
}

UISubsystem::~UISubsystem()
{
	if (g_pUISubSystemAsyncLoadThread)
	{
		g_pUISubSystemAsyncLoadThread->Join();
		SAFE_DELETE_PTR(g_pUISubSystemAsyncLoadThread);
	}
	Scheduler::DestroyGroup(m_hSchedulerGroup);
}

////-----------------------------------------------------------------------------
//// LoadUI
////-----------------------------------------------------------------------------
//void UISubsystem::LoadUI()
//{
//	if(firstTimeLoad)
//	{
//		firstTimeLoad = false;
//
//		if(!g_bLoadUI)
//			return;
//
//		// create a music entity
//		Util::C_String string = "UI";
//		Audio::XtEntityProxy* pXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(string);
//		if (pXtEntity)
//		{
//			pXtEntity->SetupEntityType();
//
//			if (pXtEntity->GetEntityTypeHnd().IsValid())
//			{
//				AudioEntityHandle hEntity;
//				Audio::InterfaceController::Get().CreateEntity(pXtEntity->GetEntityTypeHnd(), hEntity);
//				//Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(hEntity, "StartActionIn");
//
//				//store def handle
//				m_pDefHandle = pXtEntity->GetEntityDefHnd();
//
//				//// connect to renderer
//			//	m_pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(pXtEntity->GetEntityDefHnd());
//				//if (pDef && pDef->LookupInstanceComponent("OutputToRenderer") != 0xffffffff)
//				//	Audio::InterfaceController::Get().SetMaster(hEntity, "OutputToRenderer");
//
//				// connect to system entity
//				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &hEntity);
//
//				//Audio::InterfaceController::Get().TriggerEntityActionIn(hEntity, hActionIn);
//
//
//				//set up VoFinished callback
//				Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(hEntity, "VOFinished");		
//
//				if(hActionOut != -1)
//					Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hEntity,eAudioStopped), hActionOut);
//
//
//				// temp. store handle in AudioSystem class
//				//C_GameApp::GetInstance()->GetGameAudioLayer()->SetActorEntityHnd(Game::C_GameAudioLayer::eUISubsystem,hEntity);
//
//				m_hUIAudioEntity = hEntity;
//			}
//
//			m_Bin1 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin1");
//			m_Bin2 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin2");
//			m_Bin3 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin3");
//			m_Bin4 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin4");
//			m_Bin5 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin5");
//			m_Bin6 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin6");
//			m_Bin7 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin7");
//			m_Bin8 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin8");
//
//			g_hBin1 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin1);
//			g_hBin2 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin2);
//			g_hBin3 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin3);
//			g_hBin4 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin4);
//			g_hBin5 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin5);
//			g_hBin6 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin6);
//			g_hBin7 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin7);
//			g_hBin8 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin8);
//
//			m_VOPosParamOutHandle= Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BlurbVoicemails_0SoundActive");							
//		}
//
//		SetSubscriberSystem( &Game::C_Game::GetInstance()->GetSubscriberSystem() );
//		AddSubscriber( GameMessaging::E_GameMsg_Update );
//		AddSubscriber( UI::E_UiMsg_PlaySoundEffect );
//
//		AddSubscriber( Game::E_GameLogicMessage_Failed_Rule );
//		AddSubscriber( Game::E_GameLogicMessage_Passed_All_Rule );
//		AddSubscriber( Game::E_GameflowMessage_IntroStarted );
//		AddSubscriber( Game::E_GameflowMessage_IntroFinished );
//		AddSubscriber( Game::E_GameflowMessage_RaceFinished );
//		AddSubscriber( Game::E_GameflowMessage_VehicleFinished );
//
//		AddSubscriber( ActorMsg::E_AudioMsg_PlayUIDialogue );
//		AddSubscriber( ActorMsg::E_AudioMsg_StopUIDialogue );
//
//		AddSubscriber( E_VehicleLogicMsg_CriticalMass );
//
//
//		//SuspendInstances();
//
//		ResumeInstance(UI				);
//		ResumeInstance(MusicController );
//		ResumeInstance(Voicemail		);
//
//		//Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "UI_AudioStopped");
//
//		RegisterActionOuts();
//
//		m_MinHealthLevelThisFrame = 100000;
//
//		m_LevelMusicPlaying = false;
//	}
//
//	m_LevelMusicPlaying = false;
//	for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
//	{
//		Audio::EntityInstance* pInstance = m_aEntityInstance[i];
//		if (pInstance)
//		{
//
//			Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("LevelEndActionIn"));
//			Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
//
//			//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StartActionIn"));
//			//Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
//		}
//
//		
//	}
//
//	ResumeInstance(Voicemail		);
//	m_VOPosition = 0;
//	m_VOPositionPrev = 0;
//	m_VOPosReturnDataHandle = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_VOPosParamOutHandle);
//
//	//start frontend music
//	Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "StartActionIn");
//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
//
//	m_bHaveInstalledUI = true;
//}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------

bool UISubsystem::ProcessSystemMessage(const U32 u32MessageId)
{
	if(!gbUIMessages)
		return false;

	switch (u32MessageId)
	{
		//case Audio::eInit:
		//{

		//}
		//break;
		case Audio::eMute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,0);
				}
			}
			break;

		case Audio::eUnmute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,1);
				}
			}
			break;

		case Audio::ePreLoadInGameUI:
			{
				//Going into the game...
				if (m_activeUIEntity != eActiveUIEntity_InGame)
				{
					LOGINFO(Game::g_Log, "Requesting data UI - InGame (SYNC)");
					LoadUIEntity("InGameUI");
					m_syncLoadInitialUpdateNeeded = true;
					m_activeUIEntity = eActiveUIEntity_InGame;
				}
			}
			break;

		case Audio::ePreLoadFrontEndUI:
			{
				//Ignore this the first time around, we'll do an async load in our initial update.
				if (!m_intoUIForFirstTime && m_activeUIEntity != eActiveUIEntity_FrontEnd)
				{
					LOGINFO(Game::g_Log, "Requesting data UI - FrontEnd (SYNC)");
					LoadUIEntity("FrontEndUI");
					m_syncLoadInitialUpdateNeeded = true;
					m_activeUIEntity = eActiveUIEntity_FrontEnd;
				}
			}
			break;

		case Audio::eLoadUI:
			{
				//load all UI on first time going into game. hack to allow PS3 to cache streams
				//let update know to load UI
				m_bShouldInstallUI = true;

				m_LevelMusicPlaying = false;

			//	if the frontEnd is already loaded, don't reload
				if(GetSubSystemState() == E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_LEAVING_LEVEL)
					break;

				if(GetSubSystemState() == E_GAMEFLOW_STATE_UNKNOWN)
					SetSubSystemState(E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_INIT);
				else
					SetSubSystemState(E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_LEAVING_LEVEL);

				//load frontEnd UI
				m_goingToFrontEnd = true;



			}
		break;

		case Audio::eBeginLevel:
			{

			//	LoadUIFrontEnd("UI");
				if(GetSubSystemState() == E_GAMEFLOW_STATE_INGAME_LOADED_FROM_LEAVING_UI)
					break;
				else
					SetSubSystemState(E_GAMEFLOW_STATE_INGAME_LOADED_FROM_LEAVING_UI);

				//let update know to load UI
				m_bShouldInstallUI = true;

				//load ingame UI
				m_goingToFrontEnd = false;

				//send slot abilities
				//Need a profile
				Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
				Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				bool isRanked = !pLayer || !pLayer->GetGameModePtr() || pLayer->GetGameModePtr()->IsRanked();
	
				UI::E_MULTIPLAYER_LOADOUT_TYPE loadOutType = pMultiplayerProfile->GetLoadouts(isRanked).GetCurrentLoadOutType();
				const Game::MultiplayerLoadOut& loadOut = pMultiplayerProfile->GetLoadouts(isRanked).GetLoadOut(loadOutType);

				for( S32 j = 0; j < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; j++)
				{				
					bool empty = true;
					abilitySlots[j] = loadOut.GetAbilityInSlot( (UI::E_MULTIPLAYER_LOADOUT_SLOT)j);
				}
			}
			break;

	case Audio::eLoadLevel:	
		{
			
				//stop UI frontend Music
				for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
				{
					Audio::EntityInstance* pInstance = m_aEntityInstance[i];
					if (pInstance)
					{
						Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StopActionIn"));
						Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
						//printf("\nSTOPACTION IN FROM eLOADLEVEL\n");
					}

				//					
				}

			//SuspendInstance(Voicemail		);				
			m_RaceInProgress = true;

			//release current UI Entity, frontEnd
			ReleaseUIEntity();

			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::LoadingStartActionIn);

			//CreateLoadingUI();

			//m_MPRaceInProgress = true;
			}
				
		break;

	case Audio::eEndLevel:
		{

				m_LevelMusicPlaying = false;

				for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
				{
					Audio::EntityInstance* pInstance = m_aEntityInstance[i];
					if (pInstance)
					{
						Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("RaceQuitActionIn"));
						Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);

						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("LevelEndActionIn"));
						Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);

					}
				}

				m_RaceInProgress = false;

		//		Audio::InterfaceController::Get().UntrackValue(m_VOPosReturnDataHandle);


				//release in game entity
				//ReleaseUIEntity();

				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::LoadingStartActionIn);

				//CreateLoadingUI();
				//m_MPRaceInProgress = false;
		}
		break;

	case Audio::eRaceStarted:
				for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					//if(gbPlayMusic)
					{
						Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("RaceStartedActionIn"));
						Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);

					}
				}
			}
				m_RaceInProgress = true;
				//m_MPRaceInProgress = true;
		break;

	}
	return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProcessCustomMessage
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool UISubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data , void* pData)
{

	U32 stoppedInstance = u32MessageId;
	
	U32 resumedInstance = u32MessageId - NumActionOutIds;

	//don't want to suspend the first 3 instances
	if(u32MessageId > Theme_AudioStopped && u32MessageId < NumActionOutIds)
		SuspendInstance(stoppedInstance);
	else
		if(u32MessageId >= NumActionOutIds)
		ResumeInstance(resumedInstance);

	//send custom messages from other systems directly here
	switch(u32MessageId)
	{
		//send health
		case eCurrentHealth:
		{

			if(Game::C_Game::GetInstance()->GetLevelState().mNumViewports == 1) // if one player
			{
				
					Audio::EntityDataMemberHandle  hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CurrentHealthPercent");
					Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity,hParamIn, *((S32*)pData));

					//dont want to hear wrecked sounds when race has finished and on the score screen, hard code to 1
					if(!m_RaceInProgress)
							Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hParamIn,1000);

			}



		}
		break;

		//send the health level enum
		case eCurrentHealthLevel:
		{
			if(Game::C_Game::GetInstance()->GetLevelState().mNumViewports == 1) // if one player
			{
					Audio::EntityDataMemberHandle  hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CurrentHealthLevel");
					Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity,hParamIn, *((S32*)pData));

					//if(u32Data == -1)//if wrecked
					//{
					//	Audio::EntityDataMemberHandle  hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "WreckedActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity,hActionIn);

					//}
						

					if(!m_RaceInProgress && u32Data == 0)
							Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hParamIn,1);
				

			}
			else
			if( u32Data < m_MinHealthLevelThisFrame && u32Data != -1.f)
				m_MinHealthLevelThisFrame = u32Data;

		}
		break;
	}




	return false;
}



void UISubsystem::Update()
{
	//do loading of UI Entities. because of PS3, need to do it on flag change
	if (/*(! m_bHaveInstalledUI) &&*/ m_bShouldInstallUI && CanInstallUI())
	{
		//Front end loading...
		if(m_goingToFrontEnd && m_activeUIEntity != eActiveUIEntity_FrontEnd)
		{
			if (m_intoUIForFirstTime)
			{
				m_intoUIForFirstTime = false;
				LOGINFO(Game::g_Log, "Requesting data UI - FrontEnd (ASYNC)");
				LoadUIEntityAsync("FrontEndUI");
				m_activeUIEntity = eActiveUIEntity_FrontEnd;
			}
		}
	}

	if (LoadUIEntityAsyncPostLoad() || m_syncLoadInitialUpdateNeeded)
	{
		m_syncLoadInitialUpdateNeeded = false;

		//LoadUI();
		if(m_goingToFrontEnd)
		{
			LOGINFO(Game::g_Log, "Creating UI - FrontEnd");
			//LoadUIEntity("FrontEndUI");


			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{

					//stop any ingae
					//Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("LevelEndActionIn"));
					//Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);

					//start frontend music
					Audio::EntityDataMemberHandle  hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "StartActionIn");
					Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

					//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StartActionIn"));
					//Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
				}

				
			}

		}
		else
		{
			LOGINFO(Game::g_Log, "Creating UI - Ingame");
			//LoadUIEntity("InGameUI");

			//start InGame Music
			Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LevelStartActionIn");
			Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

			Audio::EntityDataMemberHandle hEntityIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "AbilitySlot1");
			Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hEntityIn,  (F32)abilitySlots[0]);

			hEntityIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "AbilitySlot2");
			Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hEntityIn,  (F32)abilitySlots[1]);

			hEntityIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "AbilitySlot3");
			Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hEntityIn,  (F32)abilitySlots[2]);


		}

	}
	
	//Wait until we've got some data.
	if (m_uiAudioLoadedState != eUiAudioLoaded)
	{
		return;
	}


	F32 Bin1 = 0.f;
	Audio::InterfaceController::Get().GetValue(Bin1, g_hBin1);
	F32 Bin2 =0.f;
	Audio::InterfaceController::Get().GetValue(Bin2, g_hBin2);
	F32 Bin3 =  0.f;
	Audio::InterfaceController::Get().GetValue(Bin3, g_hBin3);
	F32 Bin4 =0.f;
	Audio::InterfaceController::Get().GetValue(Bin4, g_hBin4);
	F32 Bin5 = 0.f;
	Audio::InterfaceController::Get().GetValue(Bin5, g_hBin5);
	F32 Bin6 =0.f;
	Audio::InterfaceController::Get().GetValue(Bin6, g_hBin6);
	F32 Bin7 = 0.f;
	Audio::InterfaceController::Get().GetValue(Bin7, g_hBin7);
	F32 Bin8 =   0.f;
	Audio::InterfaceController::Get().GetValue(Bin8, g_hBin8);

	m_BinValues[ 0 ] = Bin1;
	m_BinValues[ 1 ] = Bin2;
	m_BinValues[ 2 ] = Bin3;
	m_BinValues[ 3 ] = Bin4;
	m_BinValues[ 4 ] = Bin5;
	m_BinValues[ 5 ] = Bin6;
	m_BinValues[ 6 ] = Bin7;
	m_BinValues[ 7 ] = Bin8;

	if(m_VoicemailStarted)
	{

		F32 tempPos = 0;

		//get the stream position 
		Audio::InterfaceController::Get().GetValue(tempPos, m_VOPosReturnDataHandle);		


		F32 min = g_MinVoiceMailThreshold;
		F32 max = g_MaxVoiceMailThreshold;

		//we can get into a situation where 0 won't come through for the start of a stream, so start tracking if value is low
		if(tempPos <= min)
		{
			m_startTracking = true;
		}

		if(m_startTracking)
		{

			m_VOPosition = tempPos;
			
			//scale all values so text speeds up
			{
				F32 scrollSpeed = 1.12;
				m_VOPosition *= scrollSpeed;

				m_VOPosition = Maths::Clamp(m_VOPosition, 0.f, 1.f);
			}

			F32 smoothValue1 = .9;
			F32 smoothValue2 = .1;

			m_VOPosition = m_VOPositionPrev * smoothValue1 + m_VOPosition * smoothValue2; //smooth it

			m_VOPositionPrev = m_VOPosition;
		}


	}
	else
	{
		m_startTracking = false;

		m_VOPositionPrev = 0.0;
		m_VOPosition = 0.0;

	}





	#ifndef CFG_GOLD
	if(g_RenderEqEffect)
	{

			S32 red = 0xff;
			S32 green = 0xff;
			S32 blue = 0xff;
			F32 radius = 1;

			Maths::RGBA colourYellow = Maths::RGBA(red, green, 0x00, 0xff);
			Maths::RGBA colourRed = Maths::RGBA(red, 0x00, 0x00, 0xff);
			Maths::RGBA colourBlue = Maths::RGBA(0x00, 0x00, blue, 0xff);
			Maths::RGBA colourGreen = Maths::RGBA(0x00, green, 0x00, 0xff);

			F32 x = 64 +(F32)g_RenderEqXPos;
			F32 y = g_RenderEqYPos;

			Maths::HVec2 pos( x, y);
			Maths::HVec2 size( 50.0f,  Bin1*g_RenderBarScale);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos,size,colourGreen);

			x = 120 +(F32)g_RenderEqXPos;
			y = g_RenderEqYPos;

			Maths::HVec2 pos2( x, y);
			Maths::HVec2 size2( 50.0f,  Bin2*g_RenderBarScale);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos2,size2,colourBlue);

			x = 180 +(F32)g_RenderEqXPos;
			y = g_RenderEqYPos;


			Maths::HVec2 pos3( x, y);
			Maths::HVec2 size3( 50.0f,  Bin3*g_RenderBarScale);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos3,size3,colourRed);

			x = 240 +(F32)g_RenderEqXPos;
			y = g_RenderEqYPos;

			Maths::HVec2 pos4( x, y);
			Maths::HVec2 size4( 50.0f,  Bin4*g_RenderBarScale);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos4,size4,colourYellow);

			x = 300 +(F32)g_RenderEqXPos;
			y = g_RenderEqYPos;


			Maths::HVec2 pos5( x, y);
			Maths::HVec2 size5( 50.0f,  Bin5*g_RenderBarScale);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos5,size5,colourRed);

			x = 360 +(F32)g_RenderEqXPos;
			y = g_RenderEqYPos;

			Maths::HVec2 pos6( x, y);
			Maths::HVec2 size6( 50.0f,  Bin6*g_RenderBarScale);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos6,size6,colourYellow);

			x = 420 +(F32)g_RenderEqXPos;
			y = g_RenderEqYPos;

			Maths::HVec2 pos7( x, y);
			Maths::HVec2 size7( 50.0f,  Bin7*g_RenderBarScale);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos7,size7,colourGreen);

			x = 480 +(F32)g_RenderEqXPos;
			y = g_RenderEqYPos;


			Maths::HVec2 pos8( x, y);
			Maths::HVec2 size8( 50.0f,  Bin8*g_RenderBarScale);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos8,size8,colourRed);

			Display::BasicRender()->DrawColoured2DRect(500,200, 50, 50, Maths::RGBA(64, 64, 64, 192));

	}
	#endif // ifndef CFG_GOLD
	
	Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();
	//if split screen
	if(levelState.mNumViewports > 1)
	{
		//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "SplitScreenHealthPercent");
		//Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hActionIn,m_MinHealthPercentThisFrame);

		//dont want to hear wrecked sounds when race has finished and on the score screen, hard code to 1
		if(!m_RaceInProgress)
			m_MinHealthLevelThisFrame = 2.0;

		Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "SplitScreenHealthLevel");
		Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hActionIn,m_MinHealthLevelThisFrame);

		m_MinHealthLevelThisFrame = 100000; ///high number for the reset, we wont have a health of 100000


	}

	if(g_bTriggerUI)
	{

			g_bTriggerUI = false;
			UI::C_UiSoundEffectMessage msg( UI::E_UiMsg_PlaySoundEffect );
			msg.m_EffectNum = g_UISoundIndex;

			MessageHandler(&msg);
	}

	//if in garage send car positions
	if(m_InGarage && GetSubSystemState() == E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_INIT || GetSubSystemState() == E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_LEAVING_LEVEL)
	{

		Game::C_RaceBookLayer* pRBL = C_GameApp::GetInstance()->GetRaceBookLayer();
		Game::C_AmaxWorldLayer* pAWL = NULL;
		Game::Actors::RaceLogicCore* pRLC = NULL;

		if(pRBL)
		{
			pAWL =  pRBL->GetAmaxWorldLayer();
			if(pAWL)
				pRLC = pAWL->GetRaceLogicCore();
		}

		if(pRLC)
		{
			U8 focusVehicle = pRLC->GetViewportFocusVehicleIndex(0);

			AmaxCam::C_CameraSystem& cameraSystem = AmaxCam::CameraSystem();
			const Cam2::I_Feed& feed = cameraSystem.GameFeed(0);
			Maths::HMat43 camMatrix = feed.Mat43();

			Game::Actors::C_VehicleActor* vehicleActor = pRLC->GetVehicleActor(focusVehicle);

			if(vehicleActor)
			{
				Maths::HMat43 vehMatrix = vehicleActor->GetMatrix();
				Maths::HVec3 pos =  vehMatrix.AW();
				Maths::HVec3 dir =  vehMatrix.AZ();

				Audio::EntityDataMemberHandle  hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CarPosition");
				Audio::InterfaceController::Get().SetEntityVectorIn(m_hUIAudioEntity, hVector,(F32*)&pos);

				 hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CarDirection");
				Audio::InterfaceController::Get().SetEntityVectorIn(m_hUIAudioEntity, hVector,(F32*)&dir);

				Audio::EntityDataMemberHandle  hParam = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CarAudioIndex");
				Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hParam,(F32)vehicleActor->GetAudioGarageID());

				//send positonal vectors to UI

				hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ListenerPosition");
				Audio::InterfaceController::Get().SetEntityVectorIn(m_hUIAudioEntity, hVector,(F32*)&camMatrix.AW());

				 hVector = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ListenerDirection");
				Audio::InterfaceController::Get().SetEntityVectorIn(m_hUIAudioEntity, hVector,(F32*)&camMatrix.AZ());	
			}	
		}
	}


}

bool UISubsystem::MessageHandler(const Messaging::C_MessageBase *pMessage )
{
	PERF_EVENT("UI.UISubsystem.MessageHandler");

	switch(pMessage->Type())
	{

		case Game::E_GameflowMessage_RaceFinished:
			{
				//if(Game::C_GameProfileManager::Instance().GetPrimaryProfile())
				//{
				//	if(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mPassedEvent)
				//	{
				//		Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RacePassed");
				//		Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

				//	//fwd message on to Music 
				//		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::RaceFinishPassActionIn);
				//		
				//	}
				//	else
				//	{
				//		Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RaceFailed");
				//		Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

				//		//fwd message on to Music 
				//		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::RaceFinishFailActionIn);

				//	}
				//}

				m_RaceInProgress = false;

			}
			break;

		case  Game::E_GameLogicMessage_Failed_Rule:
			{
				
				//Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RaceFailed");
				//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
			}
			break;

		case  Game::E_GameLogicMessage_Passed_All_Rule:
			{

				//Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RacePassed");
				//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
			}
			break;

		case  Game::E_GameflowMessage_IntroStarted:
			{

				Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "IntroStarted");
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
			}
			break;
			
		case  Game::E_GameflowMessage_IntroFinished:
			{

				Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "IntroFinished");
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
			}
			break;

		case  ActorMsg::E_AudioMsg_PlayUIDialogue:
			{
				
				const ActorMsg::C_AudioPlayUIDialogue* msg = Messaging::MessageCast<ActorMsg::C_AudioPlayUIDialogue>(pMessage);

				//m_playDialogue = true;

				////need to insert a delay to the message here due to streaming issues...
				//m_delayFrames = g_debugDelayFrames;

				//m_bankID = msg->mSplitGroup;
				//m_sampleIndex = msg->mSplitIndex;		


				Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "SampleIndex");
				Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hParamIn, (F32)msg->fSampleIndex);

				Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "VOTriggerActionIn");
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

				m_VoicemailStarted = true;

			}
			break;

		case  ActorMsg::E_AudioMsg_StopUIDialogue:
			{
				
				//const ActorMsg::C_AudioStopUIDialogue* msg = Messaging::MessageCast<ActorMsg::C_AudioStopUIDialogue>(pMessage);

 			//	Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "SampleIndex");
				//Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hParamIn, (F32)msg->fSampleIndex);

				Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "VOStopActionIn");
				Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

				m_VoicemailStarted = false;

			}
			break;


		case UI::E_UiMsg_PlaySoundEffect:
		{

				const UI::C_UiSoundEffectMessage* msg = Messaging::MessageCast<UI::C_UiSoundEffectMessage>(pMessage);
				Audio::EntityDataMemberHandle hActionIn;

				// AUDIOTODO: do these lookups once in the init

				switch( msg->m_EffectNum )
				{
				case UI::E_UiSoundEffect_Move:
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MoveActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_Forwards:
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "AcceptActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_Back:
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CancelActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_Toggle:
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ToggleActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_TabsOpen:
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TabsOpenActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_Error:
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ErrorActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_CurrentHealthLevel:
						//if(Game::C_Game::GetInstance()->GetLevelState().mNumViewports == 1) // if one player
						//{
						//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CurrentHealthPercent");
						//	Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hActionIn,msg->m_Value1);

						//	//dont want to hear wrecked sounds when race has finished and on the score screen, hard code to 1
						//	if(!m_RaceInProgress)
						//			Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hActionIn,1000);

						//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CurrentHealthLevel");
						//	Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hActionIn,msg->m_Value2);

						//	if(!m_RaceInProgress && msg->m_Value2 == 0)
						//			Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hActionIn,1);

						//}
						//else
						//if( msg->m_Value2 < m_MinHealthLevelThisFrame && msg->m_Value2 != -1.f)
						//	m_MinHealthLevelThisFrame = msg->m_Value2;

					break;

					case UI::E_UiSoundEffect_TargetLocked:
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TargetLockedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_TargetOnYou:
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TargetOnYouActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_AcceptActionIn:
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "AcceptActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_MenuOpenActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MenuOpenActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_AppOpenActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "AppOpenActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_CancelActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CancelActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_MenuCloseActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CancelActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_AppCloseActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CancelActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_NoticeActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "NoticeActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_ErrorActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ErrorActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_PressStartActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PressStartActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	
						
						//fwd message on to Music 
						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::MenuStartActionIn,0);
					break;														 
																				 
					case UI::E_UiSoundEffect_TabsOpenActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TabsOpenActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_ToggleActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ToggleActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_MoveActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MoveActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_WaitingStartActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "WaitingStartActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_WaitingStopActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "WaitingStopActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_FansCountingActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FansCountingActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_FansSmallCheerActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FansSmallCheerActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_FansMidCheerActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FansMidCheerActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_FansLargeCheerActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FansLargeCheerActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_NewMessageActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "NewMessageActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_NewMessageMinorActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "NewMessageMinorActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_NewFriendActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "NewFriendActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_NewAppActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "NewAppActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_TargetLockedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TargetLockedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_TargetOnYouActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TargetOnYouActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_OSDMessageActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "OSDMessageActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_OSDMessageComboActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "OSDMessageComboActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_OSDMessageBonusActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "OSDMessageBonusActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_OSDMessageRepairActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "OSDMessageRepairActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_OSDMessageNegativeActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "OSDMessageNegativeActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_RaceCountdownActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RaceCountdownActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_RaceGoActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RaceGoActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_PerksCountdownActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PerksCountdownActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_PerksGoActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PerksGoActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_MPCountdownActionIn:

						//WIP
						//if(m_MPRaceInProgress)
						{
							hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MPCountdownActionIn");
							Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
						}
					break;														 
																				 
					case UI::E_UiSoundEffect_RaceTargetAchievingActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RaceTargetAchievingActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_RaceTargetFailingActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RaceTargetFailingActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_MoneyCountingOutActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MoneyCountingOutActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_MoneyCountingInActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MoneyCountingInActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_PhotoTakenActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PhotoTakenActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																	 
					case UI::E_UiSoundEffect_PerkBargeSelectedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PerkBargeSelectedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_PerkBombSelectedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PerkBombSelectedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_PerkNitrousSelectedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PerkNitrousSelectedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_PerkShockSelectedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PerkShockSelectedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_PerkShuntSelectedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PerkShuntSelectedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);	 
					break;														 
																				 
					case UI::E_UiSoundEffect_SpeedCameraFlashActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "SpeedCameraFlashActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_FanDemandPickupActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FanDemandPickupActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_FanDemandCountdownActionIn:
						{

							Game::C_LevelState& levelState	= Game::C_Game::GetInstance()->GetLevelState();
							//not split screen
							if(levelState.mNumViewports <= 1 && m_RaceInProgress)
							{
												 
							hActionIn= Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FanDemandCountdownActionIn");
							Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
							}
							//Audio::InterfaceController::Get().SetEntityValue(m_hUIAudioEntity, hActionIn, (F32)msg->m_Value1);}
						}
					break;

					case UI::E_UiSoundEffect_FanDemandSuccessActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FanDemandSuccessActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_FanDemandFailActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FanDemandFailActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_BuyActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BuyActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;


					case UI::E_UiSoundEffect_ResprayActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ResprayActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_StartInGameMusic:		

						if(	!m_LevelMusicPlaying)
						{
							//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LevelStartActionIn");
							//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

							m_LevelMusicPlaying = true;
						}
					break;

					case UI::E_UiSoundEffect_ModShopAttachActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ModShopAttachActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;


					case UI::E_UiSoundEffect_ModShopRemoveActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ModShopRemoveActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_GarageSwitchCarActionIn:		
						if(m_InGarage)
						{
							hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageSwitchCarActionIn");
							Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
						}
					break;

					case UI::E_UiSoundEffect_GarageSwitchCarLockedActionIn:		
						if(m_InGarage)
						{
							hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageSwitchCarLockedActionIn");
							Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
						}
					break;

					case UI::E_UiSoundEffect_GarageSelectActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageSelectActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					case UI::E_UiSoundEffect_GarageEnterActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageEnterActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

						m_InGarage = true;
					break;

					case UI::E_UiSoundEffect_GarageExitActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageExitActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

						m_InGarage = false;
					break;

					//new FE sounds
					case UI::E_UiSoundEffect_GarageReSprayActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageReSprayActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GarageColourPickerMoveActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageColourPickerMoveActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GarageFinishStyleSelect01ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageFinishStyleSelectActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GarageFinishStyleSelect02ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageFinishStyleSelectActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GarageFinishStyleSelect03ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageFinishStyleSelectActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GarageFinishStyleSelect04ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageFinishStyleSelectActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GarageSelectVehicleActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GarageSelectVehicleActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_LobbyJoinActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LobbyJoinActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_LobbyLeaveActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LobbyLeaveActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_LobbyCountDown10ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LobbyCountDown10ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_LobbyCountDown03ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LobbyCountDown03ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_ChallengeNewItemActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ChallengeNewItemActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

					//new HUD sounds

				case UI::E_UiSoundEffect_LockHardActionIn:		

					if(abilitySlots[2] == UI::E_MultiplayerVehicleAbilities_ECM)
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LockHardECMActionIn");
					else
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LockHardActionIn");

						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_LockSoftActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LockSoftActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_LockHardAIActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LockHardAIActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_LockSoftAIActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LockSoftAIActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_HealthLowActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HealthLowActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_ModUse01ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ModUse01ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_ModUse02ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ModUse02ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_ModUse03ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ModUse03ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_ModDropActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ModDropActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_BankChainActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BankChainActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_ChallengeCompleteActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ChallengeCompleteActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_RankUpActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RankUpActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_FansActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FansActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_ChainMultiplierActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ChainMultiplierActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_CountDownActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CountDownActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_BankFanActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BankFanActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MessageOtherActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MessageOtherActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_PerkSwitchActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PerkSwitchActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_EliminationCountDown10ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "EliminationCountDown10ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_EliminationCountDown03ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "EliminationCountDown03ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_FansCountUpActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FansCountUpActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_FansTotalSmallActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FansTotalSmallActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_FansTotalMediumActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FansTotalMediumActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_FansTotalLargeActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FansTotalLargeActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_RankCountUpActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RankCountUpActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_RankNextActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RankNextActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_PlayerFirstActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PlayerFirstActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_PlayerSecondActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PlayerSecondActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_PlayerThirdActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PlayerThirdActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_LeftActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "UDLRActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_RightActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "UDLRActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_LightTrailsActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LightTrailsActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_RaceTargetFailedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RaceTargetFailedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::RaceFinishFailActionIn);
					break;

				case UI::E_UiSoundEffect_RaceTargetPassActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RaceTargetPassActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::RaceFinishPassActionIn);
					break;

				case UI::E_UiSoundEffect_LobbyVoteCountActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LobbyVoteCountActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_EnrouteToNextEventActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "EnrouteToNextEventActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MotorMashCountActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MotorMashCountActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GraphicMenuTopActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GraphicMenuTopActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GraphicForwardActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GraphicForwardActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GraphicBackwardActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GraphicBackwardActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GraphicPowerButtonActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GraphicPowerButtonActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GraphicStartFlyForwardActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GraphicStartFlyForwardActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GraphicStartFlyBackwardActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GraphicStartFlyBackwardActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GraphicCityActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GraphicCityActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GraphicTextBoxActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GraphicTextBoxActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GraphicPictureBoxActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GraphicPictureBoxActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_RaceStartCountdownActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RaceStartCountdownActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_WreckedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "WreckedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_UnlockContentActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "UnlockContentActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GiftHealthActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GiftHealthActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_GiftPowerUpActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GiftPowerUpActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MPPlayerFirstActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MPPlayerFirstActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MPFinalScoreActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MPFinalScoreActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MPRankAwayActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MPRankAwayActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MPRankInActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MPRankInActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MPFocusOnWinnerActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MPFocusOnWinnerActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MPPositionInActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MPPositionInActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MPNumberGlowActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MPNumberGlowActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MPFinishInActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MPFinishInActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MotorMashLightImpactActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MotorMashLightImpactActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MotorMashHeavyImpactActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MotorMashHeavyImpactActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break;

				case UI::E_UiSoundEffect_MotorMashWreckedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "MotorMashWreckedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_TutorialSplashScreen01ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TutorialSplashScreen01ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_TutorialSplashScreen02ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TutorialSplashScreen02ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_TutorialSplashScreen03ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TutorialSplashScreen03ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_IronFistActivateActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "IronFistActivateActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_CountDown03ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CountDown03ActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_DiscardPickupActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "DiscardPickupActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_DiscardDecoyPickupActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "DiscardDecoyPickupActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PreviouslyOnBlurActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PreviouslyOnBlurActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_DestructionPassActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "DestructionPassActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::RaceFinishPassActionIn);
					break; 

				case UI::E_UiSoundEffect_DestructionFailActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "DestructionFailActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::RaceFinishFailActionIn);
					break; 

				case UI::E_UiSoundEffect_DestructionWreckedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "DestructionWreckedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_DestructionWreckLongshotActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "DestructionWreckLongshotActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PassedStartLineActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PassedStartLineActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PassedCheckpointActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PassedCheckpointActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PressStartButtonScreenActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PressStartButtonScreen");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

						//fwd message on to Music 
						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::PressStartActionIn,0);
					break; 

				case UI::E_UiSoundEffect_PickupsFireErrorActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PickupsFireErrorActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PassedFanRunCheckpointActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PassedFanRunCheckpointActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_LastGaspInitiatedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LastGaspInitiatedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_ScramblerActivatedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ScramblerActivatedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_ScramblerDeactivatedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ScramblerDeactivatedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_Route66ActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Route66ActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_TumbleweedAlleyActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TumbleweedAlleyActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PasseigDeGraciaActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PasseigDeGraciaActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_ElCarmelHeightsActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ElCarmelHeightsActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_CatalanClimbActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CatalanClimbActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_CityBreachActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CityBreachActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_SpeedwayActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "SpeedwayActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PromenadeLoopActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PromenadeLoopActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_SeafrontStripActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "SeafrontStripActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_CoastalCruiseActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CoastalCruiseActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PacificReachActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PacificReachActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_CargoRunActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CargoRunActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_TheMoneyRunActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TheMoneyRunActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_HighRiseRingActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HighRiseRingActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_HarborFreewayActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HarborFreewayActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_ShoreditchSweepSoundActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ShoreditchSweepSoundActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_UrbanBeltActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "UrbanBeltActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_CentralSprintActionIn:													 
					//	hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CentralSprintActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_HollywoodRiftActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HollywoodRiftActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_DowntownVistaActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "DowntownVistaActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_ConcreteBasinActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ConcreteBasinActionIn");
					//	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_StormdrainSurgeActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "StormdrainSurgeActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_AscentActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "AscentActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_ManhattanViewActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ManhattanViewActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_BrooklynTourActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BrooklynTourActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_RussianSteppesActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RussianSteppesActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_GoldenGateRushActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "GoldenGateRushActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_BayAreaTourActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BayAreaTourActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 
					
				case UI::E_UiSoundEffect_BayShoreRouteActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BayShoreRouteActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_WangansenActionIn:													 
						//hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "WangansenActionIn");
						//Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 
				//Boss One on One’s
				case UI::E_UiSoundEffect_CarverActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CarverActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_DraykeActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "DraykeActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_EvilShannonActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "EvilShannonActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_KhanActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "KhanActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_NatalyaActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "NatalyaActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_RhymerActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "RhymerActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_ShannonActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ShannonActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_TalonActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TalonActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_TwinsActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TwinsActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_GamePausedQuitActionIn:													 

						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eGamePausedQuit);
					break; 

				case UI::E_UiSoundEffect_StopwatchCollectedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HudStopwatchActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_MedalIncreaseActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HudGainLightActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_MedalDecreaseActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HudLoseLightActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_MedalInThreatActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HudLightInThreatActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_LapIncrementedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LapIncrementedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_LastLapActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LastLapActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PreviouslyRewardActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PreviouslyRewardActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PreviouslyOutActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PreviouslyOutActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PreviouslySparksActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PreviouslySparksActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_UpActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "UDLRActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_DownActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "UDLRActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_CarouselLeftActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CarouselRLActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_CarouselRightActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CarouselRLActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_StickersPageActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "StickersPageActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_EndOfRacePauseActionIn:	
							Audio::InterfaceController::Get().BroadcastMessage(Audio::eUnmute, 0xffffffff);
							Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eEndofRaceFadeOutSfx);	
					break; 

				case UI::E_UiSoundEffect_EndOfRaceUnPauseActionIn:		
						//Audio::InterfaceController::Get().BroadcastMessage(Audio::eMute, 0xffffffff);
						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eEndofRaceFadeInSfx);	
					break; 


				case UI::E_UiSoundEffect_TabsCloseActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "TabsCloseActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_FilterActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FilterActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_FriendVs1ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FriendsVsActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_FriendVs2ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FriendsVsActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_FriendVs3ActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FriendsVsActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_FriendTaleOfTheTapeActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "FriendTaleOfTheTapeActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_BossDemandIncrementActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BossDemandIncrementActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_BossDemandCompleteActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BossDemandCompleteActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_BossUnlockedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BossUnlockedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_ChooseActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ChooseActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_CascadeActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "CascadeActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_BossRaceWinActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BossRaceWinActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::RaceFinishPassActionIn);
					break; 

				case UI::E_UiSoundEffect_BossRaceLoseActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BossRaceLoseActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);

						Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eMusicSubsystem, MusicSubsystem::RaceFinishFailActionIn);
					break; 

				case UI::E_UiSoundEffect_LoadingScreenNavActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LoadingScreenNavActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_ApplyBossModActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "ApplyBossModActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_HudProgressSlideActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HudProgressSlideActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_HudFanComboEndActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HudFanComboEndActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_HudPositionIncrementActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HudPositionIncrementActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_HudHealthLossActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HudHealthLossActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_HudFanComboActiveActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "HudFanComboActiveActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PostRacePassedLightTrailActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PostRacePassedLightTrailActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PostRacePassedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PostRacePassedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PostRaceFailedActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PostRaceFailedActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PostRaceBestActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PostRaceBestActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PostRaceNotBestActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PostRaceNotBestActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PostRaceGoalSpawnActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PostRaceGoalSpawnActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PostRaceLeaderboardSpawnActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PostRaceLeaderboardSpawnActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PostRaceNewUnlocksActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PostRaceNewUnlocksActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PreviouslyWhooshActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PreviouslyWhooshActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_PreviouslyCounterActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "PreviouslyCounterActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 

				case UI::E_UiSoundEffect_SkipVistaActionIn:													 
						hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "SkipVistaActionIn");
						Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
					break; 


 				}
			}
	}

	return false;

}


void UISubsystem::SuspendInstances()
{
	for (U32 uId = 0; uId < NumInstanceIds; uId++)
	{
		Audio::EntityDataMemberHandle handle = Audio::InterfaceController::Get().LookupInstance(m_pDefHandle, GetInstanceString(uId));
		Audio::InterfaceController::Get().SuspendInstance(m_hUIAudioEntity,handle);
	//	Audio::InterfaceController::Get().SuspendInstance(m_hUIAudioEntity,Audio::InterfaceController::Get().LookupInstance(m_pDefHandle, GetInstanceString(uId)));
	}
}

void UISubsystem::ResumeInstance(U32 uId)
{
	Audio::EntityDataMemberHandle handle = Audio::InterfaceController::Get().LookupInstance(m_pDefHandle, GetInstanceString(uId));
	Audio::InterfaceController::Get().ResumeInstance(m_hUIAudioEntity,handle);

}

void UISubsystem::SuspendInstance(U32 uId)
{
	Audio::EntityDataMemberHandle handle = Audio::InterfaceController::Get().LookupInstance(m_pDefHandle, GetInstanceString(uId));
	Audio::InterfaceController::Get().SuspendInstance(m_hUIAudioEntity,handle);

}


void UISubsystem::RegisterActionOuts()
{

	for (U32 uId = 0; uId < NumActionOutIds; uId++)
	{
		Audio::EntityDataMemberHandle hActionOut = -1;

		hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent( m_hUIAudioEntity, GetActionOutStringStopped( uId ) );

		if(hActionOut != -1)
			Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(m_hUIAudioEntity, uId), hActionOut);

		////do resumed messages
		//hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent( m_hUIAudioEntity, GetActionOutStringResumed( uId ) );

		//if(hActionOut != -1)
		//	Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(m_hUIAudioEntity, eAudioResumed), hActionOut);

	}

	for (U32 uId = NumActionOutIds; uId < NumActionOutResumedIds; uId++)
	{
		Audio::EntityDataMemberHandle hActionOut = -1;

		hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent( m_hUIAudioEntity, GetActionOutStringResumed( uId ) );

		if(hActionOut != -1)
			Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(m_hUIAudioEntity, uId), hActionOut);
	}
	
}

void UISubsystem::StartLoadUIEntity(Util::C_String string)
{
	if(m_hProxy == NULL)
		m_hProxy = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(string);

	if (m_hProxy)
	{
		m_hProxy->SetupEntityType();

		if (m_hProxy->GetEntityTypeHnd().IsValid())
		{
			AudioEntityHandle hEntity;
			Audio::InterfaceController::Get().CreateEntity(m_hProxy->GetEntityTypeHnd(), hEntity);

			//store def handle
			m_pDefHandle = m_hProxy->GetEntityDefHnd();

			// connect to system entity
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &hEntity);

			//set up VoFinished callback
			Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(hEntity, "VOFinished");		

			if(hActionOut != -1)
				Audio::InterfaceController::Get().SetCallback(new Audio::DefaultEntityCallback(hEntity,eAudioStopped), hActionOut);

			m_hUIAudioEntity = hEntity;		

			 SetSubSystemState(E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_INIT);
		}

/*		m_Bin1 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin1");
		m_Bin2 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin2");
		m_Bin3 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin3");
		m_Bin4 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin4");
		m_Bin5 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin5");
		m_Bin6 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin6");
		m_Bin7 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin7");
		m_Bin8 = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "Bin8");

		g_hBin1 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin1);
		g_hBin2 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin2);
		g_hBin3 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin3);
		g_hBin4 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin4);
		g_hBin5 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin5);
		g_hBin6 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin6);
		g_hBin7 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin7);
		g_hBin8 = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_Bin8);

		m_VOPosParamOutHandle= Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "BlurbVoicemails_0SoundActive");				*/			
	}

	_UI_MESSAGE( UI::E_UiGameMsg_UIAudioPreLoaded );	
}

bool UISubsystem::LoadUIEntityPreLoad()
{
	//release whatever is currently loaded
	ReleaseUIEntity();
	firstTimeLoad = false;
	m_bShouldInstallUI = false;
	if(!g_bLoadUI)
		return false;

	return true;
}

void UISubsystem::LoadUIEntityAsync(const Util::C_String& entityName)
{
	if (g_pUISubSystemAsyncLoadThread)
	{
		//Block and wait for thread to complete.
		g_pUISubSystemAsyncLoadThread->Join();
		SAFE_DELETE_PTR(g_pUISubSystemAsyncLoadThread);
	}
	//Immediately do anything we need to do prior to load.
	if (!LoadUIEntityPreLoad())
		return;

	//Change state.
	SetUiAudioLoadedState(eUiAudioLoading);
	//Launch async load.
	g_pUISubSystemAsyncLoadThread = new C_UISubSystemAsyncLoadThread(this, entityName);
	g_pUISubSystemAsyncLoadThread->Start();
	//Post loading events will be triggered by our new thread.
}

bool UISubsystem::LoadUIEntityAsyncPostLoad() 
{
	if (m_uiAudioAsyncPostLoadPending)
	{
		m_uiAudioAsyncPostLoadPending = false;

		//We should still be in the loading state.
		BIZ_ASSERT_SPEED(m_uiAudioLoadedState == eUiAudioLoading);
		if (g_pUISubSystemAsyncLoadThread)
		{
			//Join loading thread and destroy.
			g_pUISubSystemAsyncLoadThread->Join();
			SAFE_DELETE_PTR(g_pUISubSystemAsyncLoadThread);
		}
		//Perform post load events.
		LoadUIEntityPostLoad();
		//Change state.
		SetUiAudioLoadedState(eUiAudioLoaded);
		//Return true to indicate that we've performed a post load.
		LOGINFO(Game::g_Log, "UISubsystem::LoadUIEntityAsyncPostLoad() Data Ready");
		return true;
	}

	return false;
}

void UISubsystem::LoadUIEntity(Util::C_String string)
{
	//Immediately do anything we need to do prior to load.
	if (!LoadUIEntityPreLoad())
		return;
	StartLoadUIEntity(string);
	LoadUIEntityPostLoad();
	SetUiAudioLoadedState(eUiAudioLoaded);
	//Return true to indicate that we've performed a post load.
	LOGINFO(Game::g_Log, "UISubsystem::LoadUIEntity() Data Ready");
}

void UISubsystem::LoadUIEntityPostLoad()
{
	SetSubscriberSystem( &Game::C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( UI::E_UiMsg_PlaySoundEffect );

	AddSubscriber( Game::E_GameLogicMessage_Failed_Rule );
	AddSubscriber( Game::E_GameLogicMessage_Passed_All_Rule );
	AddSubscriber( Game::E_GameflowMessage_IntroStarted );
	AddSubscriber( Game::E_GameflowMessage_IntroFinished );
	AddSubscriber( Game::E_GameflowMessage_RaceFinished );
	AddSubscriber( Game::E_GameflowMessage_VehicleFinished );

	AddSubscriber( ActorMsg::E_AudioMsg_PlayUIDialogue );
	AddSubscriber( ActorMsg::E_AudioMsg_StopUIDialogue );

	//SuspendInstances();

	ResumeInstance(UI				);
	ResumeInstance(MusicController );
	ResumeInstance(Voicemail		);

	RegisterActionOuts();

	m_MinHealthLevelThisFrame = 100000;

	m_LevelMusicPlaying = false;

	ResumeInstance(Voicemail		);
/*	m_VOPosition = 0;
	m_VOPositionPrev = 0;
	m_VOPosReturnDataHandle = Audio::InterfaceController::Get().TrackValue(m_hUIAudioEntity, m_VOPosParamOutHandle);	*/
}

void UISubsystem::ReleaseUIEntity()
{
	if (g_pUISubSystemAsyncLoadThread)
	{
		g_pUISubSystemAsyncLoadThread->Join();
		SAFE_DELETE_PTR(g_pUISubSystemAsyncLoadThread);
		LoadUIEntityAsyncPostLoad();
	}	

		if (m_hUIAudioEntity.IsValid())
		{

			//untrack all PO's
			//Audio::InterfaceController::Get().UntrackValue(g_hBin1);
			//Audio::InterfaceController::Get().UntrackValue(g_hBin2);
			//Audio::InterfaceController::Get().UntrackValue(g_hBin3);
			//Audio::InterfaceController::Get().UntrackValue(g_hBin4);
			//Audio::InterfaceController::Get().UntrackValue(g_hBin5);
			//Audio::InterfaceController::Get().UntrackValue(g_hBin6);
			//Audio::InterfaceController::Get().UntrackValue(g_hBin7);
			//Audio::InterfaceController::Get().UntrackValue(g_hBin8);
		
		//	Audio::InterfaceController::Get().UntrackValue(m_VOPosReturnDataHandle);

			RemoveSubscriber( GameMessaging::E_GameMsg_Update );
			RemoveSubscriber( UI::E_UiMsg_PlaySoundEffect );

			RemoveSubscriber( Game::E_GameLogicMessage_Failed_Rule );
			RemoveSubscriber( Game::E_GameLogicMessage_Passed_All_Rule );
			RemoveSubscriber( Game::E_GameflowMessage_IntroStarted );
			RemoveSubscriber( Game::E_GameflowMessage_IntroFinished );
			RemoveSubscriber( Game::E_GameflowMessage_RaceFinished );
			RemoveSubscriber( Game::E_GameflowMessage_VehicleFinished );

			RemoveSubscriber( ActorMsg::E_AudioMsg_PlayUIDialogue );
			RemoveSubscriber( ActorMsg::E_AudioMsg_StopUIDialogue );

			Audio::InterfaceController::Get().ReleaseEntity(m_hUIAudioEntity);
			
			 m_hUIAudioEntity = AudioEntityHandle();

		}

		if ( m_hProxy != NULL)
		{
			m_hProxy->ReleaseEntityType();

			m_hProxy = NULL;
		}
			
	SetUiAudioLoadedState(eUiAudioNotLoaded);
	m_activeUIEntity = eActiveUIEntity_None;
}

void UISubsystem::CreateLoadingUI()
{
	//load loading UI
	LoadUIEntity("LoadingUI");//LoadingUI

	LOGINFO(Game::g_Log, "Creating UI - Loading UI");

	//start InGame Music
	Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_hUIAudioEntity, "LevelStartActionIn");
	Audio::InterfaceController::Get().TriggerEntityActionIn(m_hUIAudioEntity, hActionIn);
}

void UISubsystem::ReleaseLoadingUI()
{
	ReleaseUIEntity();
}

UISubsystem::UiSubsystemLoadingState UISubsystem::GetUiAudioLoadedState() const
{
	return m_uiAudioLoadedState;
}

void UISubsystem::SetUiAudioLoadedState(UISubsystem::UiSubsystemLoadingState state)
{
	if (state != m_uiAudioLoadedState)
	{
		m_uiAudioLoadedState = state;
		LOGINFO(Game::g_Log, "UISubsystem::SetUiAudioLoadedState() changed state to %u", (U32)state);
	}
}

void UISubsystem::SetUiAudioAsyncPostLoadPending()
{
	m_uiAudioAsyncPostLoadPending = true;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


