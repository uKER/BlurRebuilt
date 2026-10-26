#include "stdafx.h"

#include "GameAudioLayer.h"
#include <Game\Messaging\GameflowMessages.h>
#include <GameApp.h>
#include "AppInterfaceLayer.h"
#include "MusicSubsystem.h"
#include "SystemSubsystem.h"	// [Blur2PC]
#include "XtAudio.h"

#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"

#include <GameLogic/RaceModes/RaceModeBase.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

Debug::Bool gbMusicMessages("Audio.Subsystem.MusicMsgsEnabled", true, Debug::Persist());
Debug::Bool g_loggingEntityMessages("Audio.Subsystem.MusicSubsystem.LogMessages", false);

Debug::Int g_CarsInRadiusSize("Audio.Subsystem.System.CarsInRadiusSize", 200);


MT::Thread* g_pMusicSubSystemAsyncLoadThread = NULL;

class C_MusicSubSystemAsyncLoadThread : public MT::Thread
{
public:
	C_MusicSubSystemAsyncLoadThread(MusicSubsystem* pMusicSubsystem, const Util::C_String& playlistName, MusicPlaylist* PLObj) : 
		m_playlistName(playlistName),
		m_pMusicSubsystem(pMusicSubsystem),
		m_PLObj(PLObj),
		MT::Thread()
	{
	}

	virtual ~C_MusicSubSystemAsyncLoadThread()
	{
	}

	virtual void ThreadMain()
	{
		C_GameApp::GetInstance()->GetGameAudioLayer()->GetAsyncLoadLock().Enter();
		m_PLObj->Create(m_playlistName);
		m_pMusicSubsystem->SetMusicAsyncPostLoadPending();
		C_GameApp::GetInstance()->GetGameAudioLayer()->GetAsyncLoadLock().Exit();
	}

private:
	Util::C_String m_playlistName;
	MusicPlaylist* m_PLObj;
	MusicSubsystem* m_pMusicSubsystem;
};



//-----------------------------------------------------------------------------
// MusicSubsystem::MusicSubsystem
//-----------------------------------------------------------------------------
MusicSubsystem::MusicSubsystem()
{
	for(int i = 0; i < E_PLAYLIST_TYPE_MAX; i++)
	{
		m_playlist[i] = NULL;
	}

	m_inRace = false;
	m_isSingleplayer = true;
	m_isSplitScreen = false;
	m_MusicType = 0;

	m_currentState = E_GAMEFLOW_STATE_UNKNOWN;
	m_MusicAsyncPostLoadPending = false;
	m_bMessageCached = false;
}

MusicSubsystem::~MusicSubsystem()
{
	g_pMusicSubSystemAsyncLoadThread->Join();
	SAFE_DELETE_PTR(g_pMusicSubSystemAsyncLoadThread);
	m_MusicAsyncPostLoadPending = false;	

	for(int i = 0; i < E_PLAYLIST_TYPE_MAX; i++)
	{
		delete m_playlist[i];
		m_playlist[i] = NULL;
	}
	
	m_currentState = E_GAMEFLOW_STATE_UNKNOWN;

}


//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool MusicSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{
	if(!gbMusicMessages)
		return false;

	switch (u32MessageId)
	{
		case Audio::eInit:
		{
			
		}
		break;

		case Audio::eBeginLevel:
			{
		
				int gameMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode();

				m_isSplitScreen = gameMode == Game::E_GameMode_SplitScreen;
				m_isSingleplayer = (gameMode != Game::E_GameMode_SplitScreen && gameMode != Game::E_GameMode_SystemLink && gameMode != Game::E_GameMode_Online);

				Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

				if (m_isSingleplayer)
				{
					switch(levelState.mRaceMode)
					{
						case Game::E_RaceMode_Destruction:
						case Game::E_RaceMode_FriendDemand_Destruction:
							{
								MusicPlaylist* m_PLObj = new MusicPlaylist;
								LoadAsync(eMusicLoadingLevel, "Music_Destruction", m_PLObj);
								m_playlist[E_PLAYLIST_TYPE_DYNAMIC] = m_PLObj;
							} 
							break;

						case Game::E_RaceMode_Checkpoint:
						case Game::E_RaceMode_FriendDemand_Checkpoint:
							{
								MusicPlaylist* m_PLObj = new MusicPlaylist;
								LoadAsync(eMusicLoadingLevel, "Music_Checkpoint", m_PLObj);
								m_playlist[E_PLAYLIST_TYPE_DYNAMIC] = m_PLObj;
							}
							break;

						case Game::E_RaceMode_BossBattle:
						case Game::E_RaceMode_FriendDemand_BossBattle:
							{
								MusicPlaylist* m_PLObj = new MusicPlaylist;
								LoadAsync(eMusicLoadingBossLevel, "Music_Boss", m_PLObj);
								m_playlist[E_PLAYLIST_TYPE_DYNAMIC] = m_PLObj;
							}
							break;
					}				
				}

				//send messages to Music object
				if(!g_pMusicSubSystemAsyncLoadThread)
					ProcessPostLoad();	
			}	

			break;

		case Audio::eLoadUI:
			//we don'#t want to trigger this the first time we enter the UI (becasue of the press start screen) but do want each subsequent time.
				switch(m_currentState)
				{
					case E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_LEVEL:
						ProcessCustomMessage(MenuStartActionIn);
						break;

					case E_GAMEFLOW_STATE_UNKNOWN:
						m_currentState = E_GAMEFLOW_STATE_FRONTEND_PENDING;
						break;
				};				
			break;

		case Audio::eRaceStarted:
				ProcessCustomMessage(RaceStartActionIn);
				break;

		case Audio::eEndLevel:
			m_inRace = false;

			ProcessCustomMessage(LevelFinishActionIn);

			if(m_playlist[E_PLAYLIST_TYPE_DYNAMIC])
			{
				delete m_playlist[E_PLAYLIST_TYPE_DYNAMIC];
				m_playlist[E_PLAYLIST_TYPE_DYNAMIC] = NULL;
			}

			if(m_currentState == E_GAMEFLOW_STATE_LEVEL_LOADED_FROM_FRONTEND)
				m_currentState = E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_LEVEL;

			break;

		case Audio::eMute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					static Audio::StringHash strHash = Audio::StringHash("GamePaused");
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), strHash);
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
					static Audio::StringHash strHash = Audio::StringHash("GamePaused");
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), strHash);
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,1);
				}
			}
			break;

		case Audio::eSuspend:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					//Audio::InterfaceController::Get().SuspendEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;

		case Audio::eResume:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
				//	Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;
		default:
			break;
	}

	return true;
}

bool MusicSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data , void* pData)
{
	if(u32MessageId == SetMusicType)
	{
		m_MusicType = u32Data;	
		return true;

	}

	// if the async load thread's active, cache this message and run it when we get a chance
	if (g_pMusicSubSystemAsyncLoadThread)
	{
		// we'll drop messages if one's already been cached
		BIZ_ASSERT(! m_bMessageCached);

		CacheMessage(u32MessageId, u32Data, pData);
		return true;
	}

	for(int i = 0; i < E_PLAYLIST_TYPE_MAX; i++)
	{		
		if(m_playlist[i] == NULL)
			continue;

		switch(u32MessageId)
		{
			//send parameters//
			case BossID:
			case MusicType:
			case BossDistance:
			case BossRelativeSpeed:
			case RacePosition:
			case CheckpointTimeRemaining:	
			case DestructionTimeRemaining:
			case VistaID:
				if(pData)	
					m_playlist[i]->SendParameter(u32MessageId,*((F32 *)pData));	
			break;
				
			//send actions//
			default:
					m_playlist[i]->SendAction(u32MessageId);				
			break;
		}
	}
			
	return true;
}


void MusicSubsystem::ProcessPostLoad()
{
	ProcessCustomMessage(LevelStartActionIn);

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	AmaxRoute* m_pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)levelState.mRoute];

	if(m_pRoute)
	{
		F32 vistaID = m_pRoute->mAudioVistaID;
		ProcessCustomMessage(VistaID,0,&vistaID);
	}

	m_inRace = true;

	m_currentState = E_GAMEFLOW_STATE_LEVEL_LOADED_FROM_FRONTEND;

	ProcessCustomMessage(MusicType,0,&m_MusicType);				
}


void MusicSubsystem::Update()
{
	if(!gbMusicMessages)
		return;

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if (m_MusicAsyncPostLoadPending)
	{
		//We should still be in the loading state.
		BIZ_ASSERT_SPEED(m_eMusicLoadingState == eMusicLoadingFrontend || 
						m_eMusicLoadingState == eMusicLoadingLevel || 
						m_eMusicLoadingState == eMusicLoadingBossLevel);

		//If this doesn't exist, we're dead.
		BIZ_ASSERT_SPEED(g_pMusicSubSystemAsyncLoadThread);

		g_pMusicSubSystemAsyncLoadThread->Join();
		//Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( Game::E_GameflowMessage_MusicLoadingComplete );
		
		m_MusicAsyncPostLoadPending = false;
		SAFE_DELETE_PTR(g_pMusicSubSystemAsyncLoadThread);

		switch (m_eMusicLoadingState)
		{
			case eMusicLoadingFrontend:
			{
				ProcessCustomMessage(PressStartActionIn);	
				m_currentState = E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_START;
			}
			break;

			case eMusicLoadingBossLevel:
			{
				AmaxEvent* ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];

				if(ev)
				{
					F32 musicID = ev->mAudioMusicID;
					ProcessCustomMessage(BossID,0,&musicID);
				}
			}

			// intentional drop-thru
			default:
			{
				ProcessPostLoad();
			}
			break;
		}

		m_eMusicLoadingState = eMusicLoaded;
	}

	if (m_bMessageCached && (! g_pMusicSubSystemAsyncLoadThread))
	{
		m_bMessageCached = false;
		ProcessCustomMessage(m_u32CachedMessageId, m_u32CachedData, m_pCachedData);
	}

	if (m_currentState == E_GAMEFLOW_STATE_FRONTEND_PENDING && CanInstallUI())
	{
		LOGINFO(Game::g_Log, "Creating Music - Resident");

		MusicPlaylist* m_PLObj = new MusicPlaylist;
		LoadAsync(eMusicLoadingFrontend, "Music", m_PLObj);
		m_playlist[E_PLAYLIST_TYPE_RESIDENT] = m_PLObj;

		m_currentState =  E_GAMEFLOW_STATE_FRONTEND_LOADED_FROM_START;

		return;
	}

	if (m_eMusicLoadingState != eMusicLoaded)
		return;

	F32 nearestDistance = Maths::Constants<Maths::HF32>::Max();
	F32 relativeSpeed = 0.f;
	F32 carsInRadius = 0.f;

	ProcessCustomMessage(MusicType,0,&m_MusicType);

	Game::C_RaceBookLayer* pRBL = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_AmaxWorldLayer* pAWL = NULL;
	Game::Actors::RaceLogicCore* pRLC = NULL;
	Game::C_RaceLogicLayer* pRLL = NULL;

	if(pRBL)
	{
		pAWL =  pRBL->GetAmaxWorldLayer();
		if(pAWL)
			pRLC = pAWL->GetRaceLogicCore();

		pRLL = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	}
	

	if(m_inRace && pRLC && pRLL)
	{
		

			U8 focusVehicle = pRLC->GetViewportFocusVehicleIndex(0);
			Game::Actors::C_VehicleLogicActor* focusVehicleLogicActor	= pRLL->GetLogicVehicleActor( focusVehicle );

			if(focusVehicleLogicActor)
			{
				const VehiclePositionInfo& posInfo = focusVehicleLogicActor->GetPositionInfo();
				Maths::HVec3 focusPosition = posInfo.mWorldPosition;
				F32 focusSpeed = posInfo.mForwardSpeed;

				//go through non focus cars
				for(U32 i = 0; i < pRLC->GetNumVehicles();i++)
				{
					//dont check the focus vehicle
					if(i==focusVehicle)
						continue;

					Game::Actors::C_VehicleLogicActor* bossVehicleLogicActor	= pRLL->GetLogicVehicleActor( i );
					if(bossVehicleLogicActor)
					{
						const VehiclePositionInfo& bossPosInfo = bossVehicleLogicActor->GetPositionInfo();						
					
						F32 distance = bossPosInfo.mWorldPosition.Distance(focusPosition);	
						if(distance < nearestDistance)
						{
							relativeSpeed = Maths::Abs( focusSpeed - bossPosInfo.mForwardSpeed);
							nearestDistance = distance;
						}

						//get no. cars in x distance
						if(distance <= g_CarsInRadiusSize)
							carsInRadius++;
					}							
				}	

				//send on to systemSubsystem
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eNearestCarDistance,0,&nearestDistance);				
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eCarsInRadius,0,&carsInRadius);	

			}
		
	

		if(m_isSingleplayer)
		{
			switch(Game::C_Game::GetInstance()->GetLevelState().mRaceMode)
			{
				case Game::E_RaceMode_Destruction:
				case Game::E_RaceMode_FriendDemand_Destruction:
					{

						U8 focusVehicle = pRLC->GetViewportFocusVehicleIndex(0);
						Game::Actors::C_VehicleLogicActor* pMe	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor( focusVehicle );
						
						F32 racePos = pMe->GetPositionInfo().mRacePosition;
						if(racePos != m_prevPosition)
						{
							ProcessCustomMessage(RacePosition,0,&racePos);
							m_prevPosition = racePos;
						}

						Game::C_RaceModeDestruction* pRace = (Game::C_RaceModeDestruction*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();
						
						F32 time = pRace->GetTimeLeft();
						ProcessCustomMessage(DestructionTimeRemaining,0,&time);
					}
					break;

				case Game::E_RaceMode_Checkpoint:
				case Game::E_RaceMode_FriendDemand_Checkpoint:
					{

						Game::C_RaceModeCheckpoint* pRace = (Game::C_RaceModeCheckpoint*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();
						
						F32 time = pRace->GetTimeLeft();
						ProcessCustomMessage(CheckpointTimeRemaining,0,&time);

						if(pRace->GetCheckpointsPassed() == pRace->GetNumCheckpoints())
						{
							ProcessCustomMessage(LastCheckpointActionIn);
						}
						

					}
					break;

				case Game::E_RaceMode_BossBattle:
				case Game::E_RaceMode_FriendDemand_BossBattle:
					{
						//send nearest distance plus relative speed
						ProcessCustomMessage(BossDistance,0,&nearestDistance);
						ProcessCustomMessage(BossRelativeSpeed,0,&relativeSpeed);
					}
					break;
			}
		}
	}

}


void MusicSubsystem::LoadAsync(MusicSubsystemLoadingState eState, const Util::C_String& playlistName, MusicPlaylist* PLObj)
{
	BIZ_ASSERT_SPEED(! g_pMusicSubSystemAsyncLoadThread);

	if (g_pMusicSubSystemAsyncLoadThread)
	{
		//Block and wait for thread to complete.
		g_pMusicSubSystemAsyncLoadThread->Join();
		SAFE_DELETE_PTR(g_pMusicSubSystemAsyncLoadThread);
		m_MusicAsyncPostLoadPending = false;	
	}
	//Immediately do anything we need to do prior to load.
	//if (!LoadUIEntityPreLoad())
	//	return;

	//Change state.
	m_eMusicLoadingState = eState;

	//Launch async load.
	g_pMusicSubSystemAsyncLoadThread = new C_MusicSubSystemAsyncLoadThread(this, playlistName, PLObj);
	g_pMusicSubSystemAsyncLoadThread->Start();
	//Post loading events will be triggered by our new thread.
}


void MusicSubsystem::LookupComponentHandles(AudioEntityHandle hEntity, Btl::Vector< Audio::EntityDataMemberHandle > *handleArray)
{
	for (U32 uId = 0; uId < NumMessageIds; uId++)
	{
		Audio::EntityDataMemberHandle memberHandle = -1;
		memberHandle = Audio::InterfaceController::Get().LookupInstanceComponent( hEntity, GetString( uId ) );
		//if (memberHandle == -1)
		//	LOGWARN(Game::g_Log, DBSTR" could not lookup component '%s'", GetString( uId ));	
		handleArray->PushBack( memberHandle );
	}
}


void MusicSubsystem::CacheMessage(const U32 u32MessageId, U32 u32Data , void* pData)
{
	m_u32CachedMessageId = u32MessageId;
	m_u32CachedData = u32Data;
	m_pCachedData = pData;
	m_bMessageCached = true;
}


void MusicPlaylist::Create(Util::C_String& string)
{
	m_pXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(string);
	if (m_pXtEntity)
	{
		//Game::CacheEntityResources(*pXtEntity, "MusicSubsystem");
		m_sEntityName = string;
		m_pXtEntity->SetupEntityType();

		if (m_pXtEntity->GetEntityTypeHnd().IsValid())
		{
			Audio::InterfaceController::Get().CreateEntity(m_pXtEntity->GetEntityTypeHnd(), m_hEntity);
			// connect to system entity
			AudioEntityHandle hEntity = GetEntityHandle();
			Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &hEntity);
		}
	}

	MusicSubsystem::LookupComponentHandles(GetEntityHandle(), GetHandleArray());

	if (m_pXtEntity)
	{
		static Audio::StringHash strHash = Audio::StringHash("StartActionIn");
		Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(m_pXtEntity->GetEntityDefHnd(),strHash);
		SendAction(hActionIn);	
	}

}

void MusicPlaylist::Destroy()
{	
	Audio::InterfaceController::Get().ReleaseEntity(GetEntityHandle());
	GetHandleArray()->Destroy();

	if(m_pXtEntity) 
		m_pXtEntity->ReleaseEntityType();

	m_pXtEntity = NULL;
}

inline void MusicPlaylist::SendAction(S32 u32MessageId)
{
	if(u32MessageId != -1 && HasValidComponentHandle(u32MessageId))
	{
		Audio::InterfaceController::Get().TriggerEntityActionIn(GetEntityHandle(), GetComponentHandle(u32MessageId), true);	

		if(g_loggingEntityMessages)
		{

			Util::C_String string(m_sEntityName);
		//	LOGINFO(Game::g_Log, "Sending %s Action: %s",string, MusicSubsystem::GetString(u32MessageId));
		}
	}
};

inline void MusicPlaylist::SendParameter(S32 u32MessageId,F32 value)
{
	if(u32MessageId != -1 && HasValidComponentHandle(u32MessageId))
	{
		Audio::InterfaceController::Get().SetEntityValue(GetEntityHandle(), GetComponentHandle(u32MessageId), value);

		if(g_loggingEntityMessages)
		{

			Util::C_String string(m_sEntityName);
		//	LOGINFO(Game::g_Log, "Sending %s Parameter: %s\t%f",string, MusicSubsystem::GetString(u32MessageId),value);
		}
	}
};

inline void MusicPlaylist::SendVector(S32 u32MessageId,F32* vector)
{
	if(u32MessageId != -1 && HasValidComponentHandle(u32MessageId))
	{
		Audio::InterfaceController::Get().SetEntityVectorIn(GetEntityHandle(), GetComponentHandle(u32MessageId), vector);

		if(g_loggingEntityMessages)
		{
			Util::C_String string(m_sEntityName);
		//	LOGINFO(Game::g_Log, "Sending %s Vector: %s-:\t",string, MusicSubsystem::GetString(u32MessageId));
		}
	}
};

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


