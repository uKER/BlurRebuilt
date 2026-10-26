#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>
#include <GameSystems.h>

#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic\Pickups\PickupBase.h>
#include <game\gameworld\GameWorld.h>
#include <dynamic\track\Track.h>
#include <game/AmaxCam/CameraSwitcher.h>

#include <Game/Messaging/GameflowMessages.h>
#include <GameLogic\Messaging\GameLogicMessages.h>
#include <GameLogic\Structures\VehiclePickupInfo.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <ui\Game\UILocaliser.h>

#include <misc_shared\Messaging\MessageMultiHandler.h>
#include <GameLogic\Replays\ReplayDirector.h>
#include <GameLogic\Replays\ReplayBuffer.h>
#include <GameLogic\Replays\ReplayVehicleManager.h>
#include <GameLogic\Replays\ReplaySaveManager.h>
#include <GameLogic\Replays\ReplayMessages.h>
#include <GameLogic\Replays\ReplayDefines.h>
#include <GameLogic\Replays\ReplayPacket.h>
#include <GameLogic\Replays\ReplayUtilities.h>
#include <GameLogic\Replays\MemChunk.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <System\Memory.h>

#include "misc_shared/archive/archive.h"
#include "misc_shared/archive/btl.h"
#include "btl/utility/string.h"
#include "system/serialise/C_FileSystem.h"
#include "system/serialise/C_FileBlock.h"


//#define TURN_OFF_REPLAY_OPTIMISATIONS
#if defined(TURN_OFF_REPLAY_OPTIMISATIONS)
#pragma optimize("", off)
#endif

Debug::Logger &g_ReplayDirectorLog( Debug::GetLogger("amax.replay") );
Debug::Logger &g_ReplayTimeLog( Debug::GetLogger("amax.replay.time") );

Debug::Bool g_RecordingOn("Replays.Recording.RecordingON", false, Debug::Persist());
Debug::Bool g_debugExportVehiclePositions("Replays.Cutscenes.ExportVehiclePositions", false);
Debug::Bool g_debugLoadCutsceneForEditing("Replays.Cutscenes.LoadCutsceneForEDITING", false);
Debug::Bool g_debugPauseForPhotoOp("Replays.PauseForPhotoOpportunity", false, Debug::Persist());
Debug::Float g_debugPrePhotoOpTimestamp("Replays.Recording.PrePhotoOpTimestamp", 0.5f, Debug::Range(0.0f, 2.0f), Debug::Step(0.01f));
Debug::Bool g_debugKeepRecordingAfterRaceFinish("Replays.Recording.KeepRecordingAfterRaceFinish", false);
Debug::Float g_debugReplaySkipPauseDelay("Replays.SkipPauseDelay", 0.15f, Debug::Range(0.0f, 2.0f), Debug::Persist());

static int g_DebugFramePauseDelay = 0;

extern Debug::Bool g_RecordEveryFrame;
extern Debug::String g_replayDebugSaveFilename;
extern Debug::String g_cutsceneSaveFilename;

static void StopCutsceneRecord(void *)
{
	g_RecordingOn = false;
	Game::C_ReplayDirector::Instance().StopRecording();
}

//Debug::Function StartNewCutscene("Replays.Cutscenes.StartRecordingCutscene", StartCutsceneRecord, 0);
Debug::Function StopRecordingCutscene("Replays.Cutscenes.StopRecordingCutscene", StopCutsceneRecord, 0);

namespace AmaxCam
{
	extern Debug::Bool LookAtEnable;
	extern Debug::Int LookAtIndex;
	extern Debug::Float LookOffsetX;
	extern Debug::Float LookOffsetY;
	extern Debug::Float LookOffsetZ;
	extern Debug::Float LookFov;
}

namespace Cam2
{
	extern Debug::Bool ShakeOverride;
	extern Debug::Float ShakeAmountX;
	extern Debug::Float ShakeAmountY;
	extern Debug::Float ShakeAmountZ;
	extern Debug::Float ShakeFreq;
}

extern Debug::Bool g_UiDebugDoRender;


namespace Game
{


	C_ReplayDirector C_ReplayDirector::s_Instance;

	C_ReplayDirector::C_ReplayDirector()
	{
		for(int i=0; i<m_Vehicles.Size(); i++)
		{
			m_Vehicles[i] = NULL;
		}
		m_pSaveManager = NULL;
		m_pExportSaveFile = NULL;
		m_pExportSamplesFileBlock = NULL;

		m_ReplayTime = 0.0f;

		m_ReplayMode = E_ReplayMode_FrontEnd;
	}

	C_ReplayDirector::~C_ReplayDirector()
	{
		Destroy();
	}

	void C_ReplayDirector::Destroy()
	{
		for(int i=0 ; i< m_Vehicles.Size() ; i++)
		{
			// [Blur2PC] m_Vehicles[i] is NULL for every slot the constructor didn't populate
			// with an active recording (e.g. Destroy() runs on shutdown without ever racing);
			// dereferencing it unconditionally crashed on that path. Retail cannot have this
			// bug (every plain quit-to-desktop would crash), so guard it like every other
			// member below.
			if (m_Vehicles[i])
			{
				m_Vehicles[i]->EraseRecording();
				SAFE_DELETE_PTR(m_Vehicles[i]);
			}
		}
		SAFE_DELETE_PTR(m_MiscPacketBuffers[0]);
		SAFE_DELETE_PTR(m_MiscPacketBuffers[1]);
		SAFE_DELETE_PTR(m_pSaveManager);
		SAFE_DELETE_PTR(m_pExportSaveFile);
		SAFE_DELETE_PTR(m_pExportSamplesFileBlock);

		SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );

		RemoveSubscriber( E_GameflowMessage_RaceLoaded );
		RemoveSubscriber( E_GameflowMessage_IntroFinished );
		RemoveSubscriber( E_GameflowMessage_OutroStarted );
		RemoveSubscriber( E_GameflowMessage_OutroFinished );
		RemoveSubscriber( E_GameflowMessage_RaceStarted );
		RemoveSubscriber( E_GameflowMessage_RaceFinished );
		RemoveSubscriber( E_GameflowMessage_QuitRace );
		RemoveSubscriber( E_GameflowMessage_QuitAndWinRace );
		RemoveSubscriber( E_GameLogicMessage_LapIncremented );
		RemoveSubscriber( E_ReplayMessage_GoToTime );
		RemoveSubscriber( E_ReplayMessage_UpdateSessionGhost );
		RemoveSubscriber( E_ReplayMessage_TimeAttackNewLap );
		RemoveSubscriber( E_ReplayMessage_Pause );
		RemoveSubscriber( E_ReplayMessage_Resume );

		SetSubscriberSystem(NULL);
	}

	void C_ReplayDirector::Init()                                                                                                                                                                                     
	{/*
	 #ifdef PLATFORM_PC
	 //	DEBUG: Clear the replay directory if this debug variable is set - this way other people don't fill their kits up with useless replay files!
	 if(g_DeleteReplayDirectory)
	 {
	 Sys::I_FileSystem& fs = C_GameSystems::Instance().GetFileSystem();
	 ScanHDDForReplays();
	 for(int i=0; i < m_ScannedReplayNames.Size(); i++)
	 {
	 fs.DeleteFileA((char *)GetScannedReplayStringFromIndex(i));
	 }
	 }
	 #endif

	 #ifdef PLATFORM_XENON
	 //	DEBUG: Clear the replay directory if this debug variable is set - this way other people don't fill their kits up with useless replay files!
	 if(g_DeleteReplayDirectory)
	 {
	 Sys::I_FileSystem& fs = C_GameSystems::Instance().GetFileSystem();
	 ScanHDDForReplays();
	 for(int i=0; i < m_ScannedReplayNames.Size(); i++)
	 {
	 fs.DeleteFileA((char *)GetScannedReplayStringFromIndex(i));
	 }
	 }
	 #endif
	 */
		/*if(!C_GameApp::GetInstance()->GetRaceBookLayer()->UtilDirCreatedSuccessfully())
		{
			m_ReplayMode = E_ReplayMode_Disabled;
			return;
		}*/

		SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );

		AddSubscriber( E_GameflowMessage_RaceLoaded );
		AddSubscriber( E_GameflowMessage_IntroFinished );
		AddSubscriber( E_GameflowMessage_OutroStarted );
		AddSubscriber( E_GameflowMessage_OutroFinished );
		AddSubscriber( E_GameflowMessage_RaceStarted );
		AddSubscriber( E_GameflowMessage_RaceFinished );
		AddSubscriber( E_GameflowMessage_Failed );
		AddSubscriber( E_GameflowMessage_Wrecked );
		AddSubscriber( E_GameflowMessage_QuitRace );
		AddSubscriber( E_GameflowMessage_QuitAndWinRace );
		AddSubscriber( E_GameLogicMessage_LapIncremented );
		AddSubscriber( Game::E_GameLogicMessage_GameplayImpact );
		AddSubscriber( E_ReplayMessage_GoToTime );
		AddSubscriber( E_ReplayMessage_UpdateSessionGhost );
		AddSubscriber( E_ReplayMessage_TimeAttackNewLap );
		AddSubscriber( E_ReplayMessage_Pause );
		AddSubscriber( E_ReplayMessage_Resume );
		AddSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );

		m_pLevelState = &C_Game::GetInstance()->GetLevelState();	//	Gets the level state, this persists through game's lifespan and is only cleared and refilled

		m_Loop = true;

		MEMORY_TRACKER_PUSH("game.replay");

		for(int i=0; i<m_Vehicles.Size(); i++)
		{
			m_Vehicles[i] = new C_ReplayVehicleManager();
		}
		m_MiscPacketBuffers[0] = new C_MemChunk(1);	//	Size = 1k
		m_MiscPacketBuffers[0]->SetMaxSize(1000);
		m_MiscPacketBuffers[1] = new C_MemChunk(1);	//	Size = 1k
		m_MiscPacketBuffers[1]->SetMaxSize(1000);

		m_MasterPacketBuffers[0] = new C_MemChunk(1000);	//	Size = 1k
		m_MasterPacketBuffers[0]->SetMaxSize(10000);
		m_MasterPacketBuffers[1] = new C_MemChunk(1000);	//	Size = 1k
		m_MasterPacketBuffers[1]->SetMaxSize(10000);

		m_pSaveManager = new C_ReplaySaveManager();

		MEMORY_TRACKER_POP("game.replay");

		Clear();

#ifndef CFG_GOLD
		Debug_InitialiseArteryConnection();
#endif
	}

	void C_ReplayDirector::GameInit(Messaging::C_MessageSubscriberSystem *actor_subscriber_system)
	{
		m_ActorSubscriberSystem=actor_subscriber_system;

		SetSubscriberSystem(m_ActorSubscriberSystem);

		AddSubscriber( E_VehicleLogicMsg_FiredPerk );
		AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	}

	void C_ReplayDirector::GameEnd()
	{
		SetSubscriberSystem(m_ActorSubscriberSystem);

		RemoveSubscriber( E_VehicleLogicMsg_FiredPerk );
		RemoveSubscriber( E_VehicleLogicMsg_HitByPerk );

		RemoveCurrentSubscriberSystem();

		m_ActorSubscriberSystem = NULL;

		//	There really shouldn't be any reason why we'd need to call this, but will
		//	protect against the game bailing  back to the UI mid-cutscene (like if player signs out mid-load...)
		StopCsReplay();
	}

	void C_ReplayDirector::Clear()
	{
		EraseRecording();
		m_pRLC = NULL;
		m_Paused = false;
		m_PauseTrackTimer = 0.0f;
		m_CutscenePaused = false;
		m_CutsceneResumeDelay = 0;
		for(int i=0; i<m_Vehicles.Size(); i++)
		{
			m_Vehicles[i]->SetReplayMode(E_ReplayMode_Idle);
		}
	}

	void C_ReplayDirector::SwapBuffers()
	{
		m_FirstBufferIsMemory = !m_FirstBufferIsMemory;

		for(int i=0 ; i< m_Vehicles.Size() ; i++)
		{
			m_Vehicles[i]->SwapBuffers();
		}
	}

	void C_ReplayDirector::ResetPlayback()
	{
		m_ReplayTime = 0.0f;
		for(int i=0 ; i< m_Vehicles.Size() ; i++)
		{
			m_Vehicles[i]->ResetPlayback();
		}
	}

	void C_ReplayDirector::EraseRecording()
	{
		for(int i = 0; i < m_Vehicles.Size(); i++)
		{
			m_Vehicles[i]->EraseRecording();
		}
		m_FirstBufferIsMemory = true;
		GetMemoryMiscRec()->ClearMemory();
		GetDiskMiscRec()->ClearMemory();
		GetMemoryMasterRec()->ClearMemory();
		GetDiskMasterRec()->ClearMemory();
	}

	C_MemChunk* C_ReplayDirector::GetMemoryMiscRec() const
	{
		if(m_FirstBufferIsMemory)
		{
			return m_MiscPacketBuffers[0];
		}
		else
			return m_MiscPacketBuffers[1];
	}

	C_MemChunk* C_ReplayDirector::GetDiskMiscRec() const
	{
		if(m_FirstBufferIsMemory)
		{
			return m_MiscPacketBuffers[1];
		}
		else
			return m_MiscPacketBuffers[0];
	}

	C_MemChunk* C_ReplayDirector::GetMemoryMasterRec() const
	{
		if(m_FirstBufferIsMemory)
		{
			return m_MasterPacketBuffers[0];
		}
		else
			return m_MasterPacketBuffers[1];
	}

	C_MemChunk* C_ReplayDirector::GetDiskMasterRec() const
	{
		if(m_FirstBufferIsMemory)
		{
			return m_MasterPacketBuffers[1];
		}
		else
			return m_MasterPacketBuffers[0];
	}

	void C_ReplayDirector::WipeMemoryBuffers()
	{
		for(int i = 0; i < m_Vehicles.Size(); i++)
		{
			m_Vehicles[i]->GetMemoryBuffer()->Clear();
		}
		GetMemoryMiscRec()->ClearMemory();
		GetMemoryMasterRec()->ClearMemory();
	}


	void C_ReplayDirector::WipeDiskBuffers()
	{
		for(int i = 0; i < m_Vehicles.Size(); i++)
		{
			m_Vehicles[i]->GetDiskBuffer()->Clear();
		}
		GetDiskMiscRec()->ClearMemory();
		GetDiskMasterRec()->ClearMemory();
	}

	void C_ReplayDirector::RaceLoaded()
	{
		//	When this method is called the race should have been fully loaded so it's safe to grab things like the RLC
		//	and handles to the vehicles
		GetRaceHandles();

		switch(m_ReplayMode)
		{
		case E_ReplayMode_PrePlay:
			{
				ProcessMasterPackets();
				m_pRLC->SetViewportFocusVehicleIndex(0, 0);	//	Focus the camera on vehicle with index 0
			}
			break;
		case E_ReplayMode_PreTimeAttack:
			{
				//	Do our post-RLC-creation setup stuff for Time Attack (tell the ghosts to...ghost, etc)
				for(int i=1; i<m_NumVehicles; i++)
				{
					m_Vehicles[i]->SetGhost(true);
				}
			}
			break;
		}
	}

	void C_ReplayDirector::RaceStarted()
	{
		//	All this method really does it change the replay mode from a Pre-MODE to MODE,
		//	now that the race has started and we're ready to being incrementing our replay timer
		switch(m_ReplayMode)
		{
		case E_ReplayMode_FrontEnd:
			{
				m_ReplayMode = E_ReplayMode_Idle;
			}
			break;
		case E_ReplayMode_PrePlay:
			{
				m_ReplayMode = E_ReplayMode_Play;
			}
			break;
		case E_ReplayMode_PreCutsceneEdit:
			{
				m_ReplayMode = E_ReplayMode_CutsceneEdit;
			}
			break;
		case E_ReplayMode_PreTimeAttack:
			{
				m_ReplayMode = E_ReplayMode_TimeAttack;
			}
			break;
		}
	}


	void C_ReplayDirector::GetRaceHandles(bool p_ResetIndices)
	{
		AcquireRLC();

		m_NumVehicles = m_pRLC->GetNumVehicles();

		for(int i = 0; i < m_NumVehicles; i++)
		{
			if(p_ResetIndices)
				m_Vehicles[i]->SetVehicleIndex(i);
			m_Vehicles[i]->AcquireVehicleHandle();
			m_Vehicles[i]->AcquireVehicleActor();
		}
	}

	E_ReplayReturnCode C_ReplayDirector::AcquireRLC()
	{
		m_pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		if(m_pRLC)
			return E_ReplayReturnCode_Success;
		else
			return E_ReplayReturnCode_Failed;
	}

	void C_ReplayDirector::StartRecording()
	{
		if(!m_pSaveManager)
		{
			MEMORY_TRACKER_PUSH("game.replay");
			m_pSaveManager = new C_ReplaySaveManager();
			MEMORY_TRACKER_POP("game.replay");
		}
		E_ReplayReturnCode startResult = m_pSaveManager->StartNewReplay();

		if(startResult != E_ReplayReturnCode_Success)
		{
			LOGWARN(g_ReplayDirectorLog, "Failed to begin recording replay - speak to GavinY :(");
			return;
		}

		m_ReplayTime = 0.0f;
		EraseRecording();
		m_ReplayMode = E_ReplayMode_Record;
		m_FirstFrame = true;
		for(int i = 0; i < m_NumVehicles; i++)
		{
			m_Vehicles[i]->SetReplayMode(E_ReplayMode_Record);
		}


		if(g_debugExportVehiclePositions)
		{
			Export_StartNewFile();
		}
	}

	void C_ReplayDirector::StopRecording()
	{
		if(g_debugExportVehiclePositions)
		{
			Export_FinishFile();
		}
		if(m_ReplayMode == E_ReplayMode_CutsceneEdit)
		{
			m_pSaveManager->FinishCutsceneEdit(GetScaledReplayTime());
		}
		else
		{
			//	Do all the stuff we need to do when completing a replay file
			m_pSaveManager->FinishSaveFile(GetScaledReplayTime());
		}

		//	Set the mode to Idle for when we go to the menu, or keep playing
		//	(latter only really applies for promo videos/cutscenes etc)
		m_ReplayMode = E_ReplayMode_Idle;
	/*	for(int i=0; i<m_NumVehicles; i++)
		{
			m_Vehicles[i]->ChangeReplayMode(E_ReplayMode_Idle);
		}*/
	}

	E_ReplayReturnCode C_ReplayDirector::SaveVehicleRecording(C_ReplayVehicleManager* p_pVehicle)
	{
		if(!m_pSaveManager)
		{
			MEMORY_TRACKER_PUSH("game.replay");
			m_pSaveManager = new C_ReplaySaveManager();
			MEMORY_TRACKER_POP("game.replay");
		}
		E_ReplayReturnCode result = m_pSaveManager->SaveVehicleToFile(p_pVehicle);

		return result;
	}

	//	Should m_ReplayTime be updated this frame?
	bool C_ReplayDirector::ShouldUpdate()
	{
		if(m_ReplayMode == E_ReplayMode_Record ||
			m_ReplayMode == E_ReplayMode_Play ||
			m_ReplayMode == E_ReplayMode_CutsceneEdit ||
			m_ReplayMode == E_ReplayMode_TimeAttack &&
			!m_CutscenePaused)
		{
			return true;
		}
		else
			return false;
	}

	void C_ReplayDirector::Update(float p_TimestepIn)
	{
		//	If we're not using the replay system - leave!
		if(m_ReplayMode == E_ReplayMode_Disabled)
			return;

		//	Only increase the replay time if we're actually recording/replaying - otherwise it will screw up timings when
		//	in the front end or whatever
		if(ShouldUpdate())
		{
			m_ReplayTime += p_TimestepIn;
		}

		if(m_CutscenePaused)
		{
			if(m_CutsceneResumeDelay > 0)
				m_CutsceneResumeDelay--;
			else
			{
				m_CutscenePaused = false;

				//	Resume all vehicles
				for(int i = 0; i < m_NumVehicles; i++)
				{
					m_Vehicles[i]->FreezeVehicle(false);
				}
			}
		}
		switch(m_ReplayMode)
		{
		case E_ReplayMode_FrontEnd:
			{
				Clear();
			}
			break;
		case E_ReplayMode_Record:
			{
				if(m_FirstFrame)
				{
					m_ReplayTime = 0.0f;
					RecordMasterPackets(GetScaledReplayTime());
					for(int i = 0; i < m_NumVehicles; i++)
					{
						AddPickupStore(i);
					}
					m_FirstFrame = false;
				}
				RecordStatesAndControls(GetScaledReplayTime());
				//	If it's time to save: swaps the buffers and starts the saving thread
				if(m_pSaveManager->DumpSceneIfItsTime(GetScaledReplayTime()) == E_ReplayReturnCode_Success)
				{
					//	Clear memory chunks etc here ready for new scene
					WipeMemoryBuffers();
					RecordMasterPackets(GetScaledReplayTime());
					for(int i = 0; i < m_NumVehicles; i++)
					{
						AddPickupStore(i);
					}
				}
				if(!g_RecordingOn)
				{
					StopRecording();
				}
				Export_WriteVehiclePositions(p_TimestepIn);
			}
			break;
		case E_ReplayMode_Play:
			{
				if(m_PauseTrackTimer > 0)
				{
					m_PauseTrackTimer -= p_TimestepIn;
					if(m_PauseTrackTimer <= 0)
					{
						Framework::C_ApplicationBase::GetInstance()->Pause( Framework::E_LayerPause_Freeze );
					}
				}
				if(IsItTimeToLoop())
				{
					GoToScene(0);
				}
				if(m_pSaveManager->LoadSceneIfItsTime(GetScaledReplayTime())== E_ReplayReturnCode_Success)
				{
					for(int i=0; i<m_NumVehicles; i++)
					{
						m_Vehicles[i]->NewScene();
					}
				}	
				for(int i=0; i<m_NumVehicles; i++)
				{
					m_Vehicles[i]->Update(GetScaledReplayTime());
				}
				PlaybackMiscPackets(GetScaledReplayTime());
			}
			break;
		case E_ReplayMode_CutsceneEdit:
			{
				if(m_FirstFrame)
				{
					RecordMasterPackets(GetScaledReplayTime());
					m_FirstFrame = false;
				}
				if(m_pSaveManager->SaveAndLoadIfItsTime(GetScaledReplayTime()) == E_ReplayReturnCode_Success)
				{
					GetMemoryMasterRec()->ClearMemory();
					RecordMasterPackets(GetScaledReplayTime());
					for(int i=0; i<m_NumVehicles; i++)
					{
						m_Vehicles[i]->NewScene();
					}
				}
				PlaybackMiscPackets(GetScaledReplayTime());
				for(int i=0; i<m_NumVehicles; i++)
				{
					m_Vehicles[i]->Update(GetScaledReplayTime());
				}
				//	Processing for each frame if we're editing an existing cutscene/replay
				if(ReachedEndOfReplay())
				{
					StopRecording();
				}
				Export_WriteVehiclePositions(p_TimestepIn);
			}
			break;

		//case E_ReplayMode_CutscenePaused:
		//	{
		//		//	[GY] - This code allows for the fact that MakeStatic on bodies is deferred and Activate is not (should allow a Pause() and Resume() in the same frame to work...)
		//		if(!m_CutscenePaused)
		//		{
		//			static U8 deferredFixCounter = 0;
		//			deferredFixCounter++;
		//			if(deferredFixCounter > 1)
		//			{
		//				m_ReplayMode = E_ReplayMode_Play;

		//				//	Resume all vehicles
		//				for(int i = 0; i < m_NumVehicles; i++)
		//				{
		//					m_Vehicles[i]->FreezeVehicle(false);
		//				}
		//				deferredFixCounter = 0;
		//			}
		//		}
		//	}
		//	break;

		case E_ReplayMode_TimeAttack:
			{
				//	Do all Time Attack/ghost updating here...
				//-------------------------------------------
				//	All vehicles->Update()
				for(int i=0; i<m_NumVehicles; i++)
				{
					m_Vehicles[i]->Update(GetScaledReplayTime());
				}


				//	If the player quits, save out their session best (for now)
			}
			break;
		case E_ReplayMode_Idle:
			{
				if(g_RecordingOn)
				{
					StartRecording();
				}
			}
			break;
		default:
			break;
		}
	}

	void C_ReplayDirector::GlobalUpdate()
	{
#ifndef CFG_GOLD
		Debug_ProcessArteryMessages();
#endif
		/*if(m_ReplayMode == E_ReplayMode_Paused)
		{
			if(!m_Paused)
			{
				Framework::C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_Freeze );	
				m_ReplayMode = E_ReplayMode_Play;
			}
		}*/
	}

	void C_ReplayDirector::RecordStatesAndControls(U32 p_ReplayTime)
	{
		for(int i = 0; i < m_NumVehicles ; i++ )
		{
			m_Vehicles[i]->RecordStatesAndControls(p_ReplayTime);
		}
	}

	void C_ReplayDirector::RecordMasterPackets(U32 p_ReplayTime)
	{
		//	Vehicle states + damage:
		C_ReplayPacketMaster_VehicleStates masterStatePacket;
		masterStatePacket.SetTime(p_ReplayTime);
		for(int i = 0; i < m_NumVehicles ; i++ )
		{
			m_Vehicles[i]->GetNetworkState(&masterStatePacket.m_CompressedNetworkStates[i]);
			C_ReplayPacketMaster_VehicleDamage masterDamagePacket;
			masterDamagePacket.SetTime(p_ReplayTime);
			masterDamagePacket.m_VehicleIndex = m_Vehicles[i]->GetIndex();
			m_Vehicles[i]->GetVehicleDamage(masterDamagePacket.m_DamageState);
			GetMemoryMasterRec()->Push((U8*)&masterDamagePacket, sizeof(masterDamagePacket));
		}
		U32 size = sizeof(masterStatePacket);
		U32 sizeM = sizeof(C_ReplayPacketMaster_VehicleStates);
		GetMemoryMasterRec()->Push((U8*)&masterStatePacket, sizeof(masterStatePacket));

		if(!m_ItsACutscene)
		{
			U32 pickupsMemSize = 0;
			//	On-track pickup types:
			Btl::Map<U32, Actor::C_ActorHandle>::Iterator it = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->mPickupPositionActorMap.Begin();
			while(it != C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->mPickupPositionActorMap.End())
			{
				Actor::C_ActorHandle handle = (*it).GetValue();
				::Actors::PickupPositionActor* pActor = (::Actors::PickupPositionActor*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(handle);
				C_ReplayPacketMaster_PickupOnTrack masterPickupPacket;
				masterPickupPacket.m_PickupId = pActor->GetCreationData()->m_SorceryUid;
				masterPickupPacket.m_PickupType = pActor->GetEnumPickupType();
				if(masterPickupPacket.m_PickupType != E_PICKUP_TYPE_REPAIR)
				{
					GetMemoryMasterRec()->Push((U8*)&masterPickupPacket, sizeof(masterPickupPacket));
					pickupsMemSize += sizeof(masterPickupPacket);
				}
				it++;
			}
		}
	}

	void C_ReplayDirector::GrabStatesAllVehicles(U32 p_ReplayTime)
	{
		for(int i=0; i< m_NumVehicles; i++)
		{
			//if(m_Vehicles[i]->GetReplayMode() == E_ReplayMode_Record)
			m_Vehicles[i]->AddState(p_ReplayTime, true);
		}
	}

	void C_ReplayDirector::AddPickupFired( PickupCreationStruct pickupInfo )
	{
		if(m_ReplayMode == E_ReplayMode_Record)
		{
			if(m_Vehicles[pickupInfo.mFiringIdx]->GetReplayMode() == E_ReplayMode_Record)
			{
				C_ReplayPacketPerkFire perkFire;
				perkFire.m_TargetIdx = pickupInfo.mTargetIdx;
				perkFire.m_PickupType = pickupInfo.mPickupType;
				perkFire.m_SubType = pickupInfo.mPickupSubType;
				perkFire.m_Delay = pickupInfo.mDelay;
				perkFire.m_PickupUid = pickupInfo.mPickupId;
				perkFire.SetTime(GetScaledReplayTime());
				m_Vehicles[pickupInfo.mFiringIdx]->AddPerkFiredEvent(perkFire);
			}
		}
	}

	void C_ReplayDirector::AddPickupPostDelay( U32 pickupUid, Maths::HMat43 targetMatrix, float fVal )
	{
		if(m_ReplayMode == E_ReplayMode_Record)
		{
			C_ReplayPacketPerkPostDelay postDelay;
			postDelay.m_PickupUid = pickupUid;
			Maths::Store(targetMatrix, &postDelay.m_TargetMatrix);
			postDelay.m_fVal = fVal;
			postDelay.SetTime(GetScaledReplayTime());

			GetMemoryMiscRec()->Push((U8*)&postDelay, sizeof(postDelay));
		}
	}

	void C_ReplayDirector::AddPickupHit( U8 p_HitIndex, E_PICKUP_TYPE pickupType )
	{
		if(m_ReplayMode == E_ReplayMode_Record)
		{
			if(m_Vehicles[p_HitIndex]->GetReplayMode() == E_ReplayMode_Record)
			{
				C_ReplayPacketPerkHit perkHit;
				perkHit.m_PickupType = pickupType;
				perkHit.SetTime(GetScaledReplayTime());
				m_Vehicles[p_HitIndex]->AddPerkHitEvent(perkHit);
			}
		}
	}

	void C_ReplayDirector::AddShieldedPickup( U8 p_ShieldedIndex, E_PICKUP_TYPE pickupType )
	{
		if(m_ReplayMode == E_ReplayMode_Record)
		{
			if(m_Vehicles[p_ShieldedIndex]->GetReplayMode() == E_ReplayMode_Record)
			{
				C_ReplayPacketShieldedPerk shieldedPerk;
				shieldedPerk.m_PickupType = pickupType;
				shieldedPerk.SetTime(GetScaledReplayTime());
				m_Vehicles[p_ShieldedIndex]->AddShieldedPerkEvent(shieldedPerk);
			}
		}
	}

	void C_ReplayDirector::AddKillPickup( U32 pickupUid )
	{
		if(m_ReplayMode == E_ReplayMode_Record)
		{
			C_ReplayPacketKillPickup kill;
			kill.m_PickupUid = pickupUid;
			kill.SetTime(GetScaledReplayTime());

			GetMemoryMiscRec()->Push((U8*)&kill, sizeof(kill));
		}
	}

	void C_ReplayDirector::AddPickupStore( U8 index )
	{
		if(m_ReplayMode == E_ReplayMode_Record && m_Vehicles[index]->GetReplayMode() == E_ReplayMode_Record)
		{
			Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(index);
			if(pLogicActor)
			{
				C_ReplayPacketPickupsStore store;
				VehiclePickupInfo& info = pLogicActor->GetPickupInfo();
				store.m_StoreIndex = info.mPickupStoreIndex;
				for(int i = 0; i < 3; i++)
				{
					if(i < info.mPickupStore.Size())
					{
						store.m_PickupStore[i]	= info.mPickupStore[i].mType;
						store.m_NumShots[i]		= info.mPickupStore[i].mNumShots;
					}
					else
					{
						store.m_PickupStore[i]	= E_PICKUP_TYPE_NONE;
						store.m_NumShots[i]		= 0;
					}
				}
				store.SetTime(GetScaledReplayTime());
				m_Vehicles[index]->AddPickupStoreEvent(store);
			}
		}
	}

	void C_ReplayDirector::AddPickupAdded( U8 index, E_PICKUP_TYPE pickupType )
	{
		if(m_ReplayMode == E_ReplayMode_Record && m_Vehicles[index]->GetReplayMode() == E_ReplayMode_Record)
		{
			C_ReplayPacketPickupAdded pickup;
			pickup.m_PickupType = pickupType;
			pickup.SetTime(GetScaledReplayTime());

			m_Vehicles[index]->AddPickupAddedEvent(pickup);
		}
	}

	void C_ReplayDirector::AddPhotoOpportunity( U32 p_ReplayTime )
	{
		C_ReplayPacketPhotoOp photoOp;
		U32 preHighlightTimestamp = p_ReplayTime - (U32)(g_debugPrePhotoOpTimestamp * REPLAY_RACETIME_SCALER);
		if(preHighlightTimestamp < 0)
			preHighlightTimestamp = 0;
		photoOp.SetTime(preHighlightTimestamp);

		GetMemoryMiscRec()->Push((U8*)&photoOp, sizeof(photoOp));
	}

	void C_ReplayDirector::PlaybackMiscPackets(U32 p_replayTime)
	{
		E_ReplayReturnCode keepChecking = E_ReplayReturnCode_Failed;	//	If this is _Success a packet has just been processed
		do
		{
			keepChecking = E_ReplayReturnCode_Failed;
			E_ReplayPacketType type = PeekAtPacket(GetMemoryMiscRec());
			switch(type)
			{
			case E_ReplayPacketType_PerkPostDelay:
				{
					keepChecking = ProcessPickupPostDelay(p_replayTime);
				}
				break;
			case E_ReplayPacketType_KillPickup:
				{
					keepChecking = ProcessKillPickup(p_replayTime);
				}
				break;
			case E_ReplayPacketType_PhotoOp:
				{
					keepChecking = ProcessPhotoOp(p_replayTime);
				}
				break;
			case E_ReplayPacketType_Unknown:	//	Fall through
			default:
				break;
			}
			GetMemoryMiscRec()->AssertChunk();
		}while(keepChecking == E_ReplayReturnCode_Success);
	}

	E_ReplayReturnCode C_ReplayDirector::ProcessMasterPackets(bool p_OnlyProcessInitialPositions)
	{
		//	Load all the stuff we find in this scene's master packet, depending on replay version #
		U8 replayVersion = m_pSaveManager->GetReplayVersion();

		if(replayVersion > 2)
		{
			U8* currentCheckAddress = GetMemoryMasterRec()->GetStart();
			do
			{
				currentCheckAddress = FindNextPacketOfType(E_ReplayPacketType_Master, GetMemoryMasterRec(), currentCheckAddress);
				if(!currentCheckAddress)
					break;
				C_ReplayPacketMaster* masterPacket = (C_ReplayPacketMaster*)currentCheckAddress;
				switch(masterPacket->GetMasterType())
				{
				case E_ReplayPacketMaster_VehicleStates:
					{
						ProcessMasterVehicleStates(currentCheckAddress);
						currentCheckAddress += sizeof(C_ReplayPacketMaster_VehicleStates);
					}
					break;
				case E_ReplayPacketMaster_VehicleDamage:
					{
						if(!p_OnlyProcessInitialPositions)
						{
							ProcessMasterVehicleDamage(currentCheckAddress);
						}
						currentCheckAddress += sizeof(C_ReplayPacketMaster_VehicleDamage);
					}
					break;
				case E_ReplayPacketMaster_PickupOnTrack:
					{
						if(!p_OnlyProcessInitialPositions && !m_ItsACutscene)
						{
							ProcessMasterPickupsOnTrack(currentCheckAddress);
						}
						currentCheckAddress += sizeof(C_ReplayPacketMaster_PickupOnTrack);
					}
					break;
				case E_ReplayPacketMaster_Unknown:
					{
						//	....damn
						BIZ_ASSERT(false);
					}
					break;
				}
			}while(PeekAtPacket(GetMemoryMasterRec(), currentCheckAddress) != E_ReplayPacketType_Unknown);
		}
		else
			return E_ReplayReturnCode_Failed;
		return E_ReplayReturnCode_Success;
	}

	E_ReplayReturnCode C_ReplayDirector::ProcessMasterVehicleStates(U8* p_pPacketAddress)
	{
		C_ReplayPacketMaster_VehicleStates* masterPacket = (C_ReplayPacketMaster_VehicleStates*)p_pPacketAddress;
		//	Set the replay time to match the start of this scene
		SetScaledReplayTime(masterPacket->GetTime());
		for(int i=0; i < m_NumVehicles; i++)
		{
			RacingPhysics::Car::NetworkState state;
			RacingPhysics::Car::UncompressNetworkState(masterPacket->m_CompressedNetworkStates[i], state);
			Maths::HMat43 worldMatrix = RacingPhysics::ConvertHVec(state.axis);

			m_pLevelState->mCars[i].m_InitialWorldMatrix = worldMatrix;

			U8* firstVehicleState = FindNextPacketOfType(E_ReplayPacketType_VehicleState,
				m_Vehicles[i]->GetMemoryVehicleRec(),
				m_Vehicles[i]->GetMemoryVehicleRec()->GetStart());
			if(!firstVehicleState)
			{
				//	Didn't find a SINGLE Vehicle state packet in memory...something very wrong is going on...
				m_Vehicles[i]->SetInitialNetworkStates(masterPacket->m_CompressedNetworkStates[i], masterPacket->GetTime(),
					masterPacket->m_CompressedNetworkStates[i], masterPacket->GetTime());
			}
			else
			{
				//	This is the (hopefully) normal way things will go - we find an initial state from the Master packet
				//	and one to interpolate towards from the vehicle states memory
				C_ReplayPacketVehicleState* vehiclePacket = (C_ReplayPacketVehicleState*)firstVehicleState;
				m_Vehicles[i]->SetInitialNetworkStates(masterPacket->m_CompressedNetworkStates[i], masterPacket->GetTime(),
					vehiclePacket->m_CompressedNetworkState, vehiclePacket->GetTime());
			}
		}
		return E_ReplayReturnCode_Success;
	}

	E_ReplayReturnCode C_ReplayDirector::ProcessMasterVehicleDamage(U8* p_pPacketAddress)
	{
		C_ReplayPacketMaster_VehicleDamage* masterPacket = (C_ReplayPacketMaster_VehicleDamage*)p_pPacketAddress;
		C_ReplayMsg_SetVehicleDamageMsg* damageMsg = new C_ReplayMsg_SetVehicleDamageMsg(masterPacket->m_VehicleIndex, masterPacket->m_DamageState);
		m_ActorSubscriberSystem->BroadcastMessage(damageMsg);
		delete damageMsg;

		return E_ReplayReturnCode_Success;
	}

	E_ReplayReturnCode C_ReplayDirector::ProcessMasterPickupsOnTrack( U8* p_pPacketAddress )
	{
		if(!m_ItsACutscene)
		{
			C_ReplayPacketMaster_PickupOnTrack* masterPacket = (C_ReplayPacketMaster_PickupOnTrack*)p_pPacketAddress;
			::Actors::PickupPositionActor* pActor = (::Actors::PickupPositionActor*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetPickupPositionActor(masterPacket->m_PickupId);
			if(pActor)
			{
				pActor->SetPickupType(masterPacket->m_PickupType);
			}
		}
		return E_ReplayReturnCode_Success;
	}

	Game::E_ReplayReturnCode C_ReplayDirector::ProcessPickupPostDelay( U32 p_ReplayTime )
	{
		U8* current = GetMemoryMiscRec()->GetCurrent();
		C_ReplayPacketPerkPostDelay* packet = (C_ReplayPacketPerkPostDelay*)current;
		if(p_ReplayTime > packet->GetTime())
		{
			//	We've arrived at a highlight of the replay, so we need to pause here
			C_PickupBase* pPickup = C_PickupsManager::Instance().GetPickup(packet->m_PickupUid);
			//	This check shouldn't fail but just to be on the safe side
			if(pPickup)
			{
				Maths::HMat43 matrix = Maths::Load<Maths::HMat43>(&packet->m_TargetMatrix);
				pPickup->SetPostDelayMatrix(matrix, packet->m_fVal);
			}

			GetMemoryMiscRec()->MoveForward(sizeof(C_ReplayPacketPerkPostDelay));
			return E_ReplayReturnCode_Success;
		}
		else
			return E_ReplayReturnCode_Failed;
	}

	Game::E_ReplayReturnCode C_ReplayDirector::ProcessKillPickup( U32 p_ReplayTime )
	{
		U8* current = GetMemoryMiscRec()->GetCurrent();
		C_ReplayPacketKillPickup* packet = (C_ReplayPacketKillPickup*)current;
		if(p_ReplayTime > packet->GetTime())
		{
			//	We've arrived at a highlight of the replay, so we need to pause here
			C_PickupBase* pPickup = C_PickupsManager::Instance().GetPickup(packet->m_PickupUid);
			//	This check shouldn't fail but just to be on the safe side
			if(pPickup)
			{
				// NEED TO PASS THROUGH SAVED REASONING HERE?
				pPickup->KillPickup(-1, E_PICKUP_CAUSE_OF_DEATH_REPLAY);
			}

			GetMemoryMiscRec()->MoveForward(sizeof(C_ReplayPacketKillPickup));
			return E_ReplayReturnCode_Success;
		}
		else
			return E_ReplayReturnCode_Failed;
	}

	E_ReplayReturnCode C_ReplayDirector::ProcessPhotoOp( U32 p_ReplayTime )
	{
		U8* current = GetMemoryMiscRec()->GetCurrent();
		C_ReplayPacketPhotoOp* packet = (C_ReplayPacketPhotoOp*)current;
		bool highlightReached = false;
		if(p_ReplayTime > packet->GetTime())
		{
			//	We've arrived at a highlight of the replay, so we need to pause here
			if(g_debugPauseForPhotoOp)
			{
				Framework::C_ApplicationBase::GetInstance()->Pause( Framework::E_LayerPause_Freeze );
				m_Paused = true;
			}
			GetMemoryMiscRec()->MoveForward(sizeof(C_ReplayPacketPhotoOp));
			return E_ReplayReturnCode_Success;
		}
		else
			return E_ReplayReturnCode_Failed;
	}

	bool C_ReplayDirector::HasThereBeenARecording()
	{
		//	Check the first vehicle to see if it contains a recording
		if((m_Vehicles[0]) && m_Vehicles[0]->GetMemoryVehicleRec()->GetUsedSize() > 0)
			return true;
		else
			return false;
	}

	void C_ReplayDirector::ScanHDDForReplays()
	{
		Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
		Sys::C_DirectoryInfo info;

		fs.DirectoryInfo("replays\\",&info);

		m_ScannedReplayNames.Destroy();

		for(unsigned int a=0;a<info.NumFiles();a++)
		{
			Sys::C_FileInfo f_info;
			f_info=info.FileInfo(a);

			m_ScannedReplayNames.PushBack(f_info.m_FileName.Chars());
		}
	}

	Btl::String64 *C_ReplayDirector::GetScannedReplayStringFromIndex(int i)
	{
		return &m_ScannedReplayNames[i];
	}

	bool C_ReplayDirector::LoadReplayFromIndex(int p_Index)
	{
		if(p_Index >= m_ScannedReplayNames.Size())
		{
			LOGWARN(g_ReplayDirectorLog, "Attempting to load an invalid replay index");
			return false;
		}
		if(!m_pSaveManager)
		{
			MEMORY_TRACKER_PUSH("game.replay");
			m_pSaveManager = new C_ReplaySaveManager();
			MEMORY_TRACKER_POP("game.replay");
		}

		if(m_pSaveManager->StartLoad((char*)&m_ScannedReplayNames[p_Index]) == E_ReplayReturnCode_Failed)
			return false;
		m_NumVehicles = m_pLevelState->mNumCars;

		//	Set initial vehicle positions into the level state so they are then set through the normal processes

		ProcessMasterPackets(true);

		m_pLevelState->mReplayMode = E_ReplayMode_Play;

		if( !m_pLevelState->mAIController )
		{
			m_pLevelState->mAIController = RLDatabase::Instance().mRLSystems[0]->mAIPickupController->mControllers[0].mController.Object();
		}

		m_pLevelState->Finished();
		for(int i = 0; i<m_NumVehicles; i++)
		{
			m_Vehicles[i]->SetReplayMode(E_ReplayMode_Play);
		}
		m_ReplayMode = E_ReplayMode_PrePlay;	//	Don't set to _Play yet as this will try to access the non-existent RLC!
#ifndef CFG_GOLD
		Debug_SendConstantReplayInfo();
#endif
		return true;
	}

	bool C_ReplayDirector::LoadCutsceneForEditing(int p_Index)
	{
		if(p_Index >= m_ScannedReplayNames.Size())
		{
			LOGWARN(g_ReplayDirectorLog, "Attempting to load an invalid replay index");
			return false;
		}

		if(!m_pSaveManager)
		{
			MEMORY_TRACKER_PUSH("game.replay");
			m_pSaveManager = new C_ReplaySaveManager();
			MEMORY_TRACKER_POP("game.replay");
		}
		if(m_pSaveManager->StartCutsceneEdit((char *) &m_ScannedReplayNames[p_Index]) == E_ReplayReturnCode_Failed)
			return false;
		m_NumVehicles = m_pRLC->GetNumVehicles();

		//	Set initial vehicle positions into the level state so they are then placed through the normal processes
		ProcessMasterPackets(true);
		m_pLevelState->mReplayMode = E_ReplayMode_Play;
		m_pLevelState->Finished();
		for(int i = 0; i<m_NumVehicles; i++)
		{
			m_Vehicles[i]->SetReplayMode(E_ReplayMode_Play);
		}
		m_ReplayMode = E_ReplayMode_PreCutsceneEdit;
		m_FirstFrame = true;

#ifndef CFG_GOLD
		Debug_SendConstantReplayInfo();
#endif

		return true;
	}

	void C_ReplayDirector::SetSaveFilename( Util::C_String p_Filename )
	{
		if(!m_pSaveManager)
		{
			MEMORY_TRACKER_PUSH("game.replay");
			m_pSaveManager = new C_ReplaySaveManager();
			MEMORY_TRACKER_POP("game.replay");
		}
		m_pSaveManager->SetSaveFilename(p_Filename);
	}

	bool C_ReplayDirector::ReachedEndOfReplay()
	{
		if(GetScaledReplayTime() > m_pSaveManager->GetEndReplayTime())
			return true;
		else
			return false;
	}

	bool C_ReplayDirector::IsItTimeToLoop()
	{
		if(m_Loop && ReachedEndOfReplay())
			return true;
		else
			return false;
	}


	E_ReplayMode C_ReplayDirector::GetVehicleReplayMode(U8 p_Index)
	{
		BIZ_ASSERT(p_Index < m_Vehicles.Size());
		return m_Vehicles[p_Index]->GetReplayMode();
	}


	void C_ReplayDirector::SetVehicleReplayMode(U8 p_Index, E_ReplayMode p_Mode)
	{
		BIZ_ASSERT(p_Index < m_Vehicles.Size());
		m_Vehicles[p_Index]->SetReplayMode(p_Mode);
	}

	S8 C_ReplayDirector::FindReplayIndexFromVehicleIndex(S8 p_VehicleIndex) const
	{
		if(!p_VehicleIndex)
			return -1;
		for(int i = 0; i < m_Vehicles.Size(); i++)
		{
			if(m_Vehicles[i]->GetIndex() == p_VehicleIndex)
				return i;
		}
		return -1;
	}

	U32 C_ReplayDirector::GetScaledReplayTime() const
	{
		return (int)(m_ReplayTime * REPLAY_RACETIME_SCALER);
	}

	//	Nasty and results in losing precision but only used when skipping around the replay so it's all good
	void C_ReplayDirector::SetScaledReplayTime(U32 p_ReplayTime)
	{
		//	Adjust for the loss in precision that comes from converting the large U32 to the scaled-down float.
		//	Should avoid situations where vehicles are attempting to interpolate between two states where the first time
		//	is a millisecond or so larger than the current replay time
		U32 adjustedReplayTime = p_ReplayTime + 1;
		float timeToSet = ((float)adjustedReplayTime) / REPLAY_RACETIME_SCALER;
		m_ReplayTime = timeToSet;
	}

	bool C_ReplayDirector::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		bool handledMessage = true;
		if(m_ReplayMode == E_ReplayMode_Disabled)
		{
			return handledMessage;
		}

		switch( pMessage->Type() )
		{
		case Game::E_GameflowMessage_ClearStoredActorPtrs:
			{
				m_pRLC = NULL;
			}
			break;

		case E_GameflowMessage_QuitRace:		//	Fall through
		case E_GameflowMessage_QuitAndWinRace:	//	Fall through
		case E_GameflowMessage_RaceFinished:
			{
				if(!g_debugKeepRecordingAfterRaceFinish)
				{
					switch(m_ReplayMode)
					{
					case E_ReplayMode_Record:		//	Fall-through
					case E_ReplayMode_CutsceneEdit:
						{
							StopRecording();
						}
						break;
						//case E_ReplayMode_TimeAttack:
						//	{
						//		//	TO DO: This is where we'll ask the player if they want to save their best ghost - probably...
						//		/*SwapBuffers();
						//		SaveVehicleRecording(m_Vehicles[E_GhostType_SessionBest]);*/
						//	}
						//	break;
					}
					m_ReplayMode = E_ReplayMode_FrontEnd;
				}
			}
			break;

		case E_GameflowMessage_RaceLoaded:
			{
				if(!m_pLevelState->mInUI)
				{
					RaceLoaded();
				}
			}
			break;

		case E_GameflowMessage_RaceStarted:
			{
				if(!m_pLevelState->mInUI)
				{
					RaceStarted();
				}
			}
			break;

		//case E_GameLogicMessage_LapIncremented:
		//	{
		//		//	Check for player vehicle crossing the line, and therefore resetting all ghosts/updating session best
		//		if(m_ReplayMode == E_ReplayMode_TimeAttack)
		//		{
		//			const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);
		//			//	Check if the player's vehicle has just crossed the start line for the first time (beginning of first lap)
		//			if(pMsg->mVehicleIndex == 0 && pMsg->mLap == 1)
		//			{
		//				m_ReplayTime = 0.0f;
		//				m_Vehicles[0]->SetReplayMode(E_ReplayMode_GhostRecord);
		//			}
		//		}
		//	}
		//	break;

		//case E_VehicleLogicMsg_HitByPerk:
		//	{
		//		if(m_ReplayMode == E_ReplayMode_Record)
		//		{
		//			//	A vehicle's been hit by a perk - find out which one was hit and by what type
		//			const C_VehicleLogicMsg_HitByPerkMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);
		//			if(m_Vehicles[pMsg->mVehicleIndex]->GetReplayMode() == E_ReplayMode_Record)
		//			{
		//				C_ReplayPacketPerkHit perkHit;
		//				perkHit.m_PickupType = pMsg->mPickupType;
		//				perkHit.m_Level = pMsg->mLevel;
		//				perkHit.SetTime(GetScaledReplayTime());
		//				m_Vehicles[pMsg->mVehicleIndex]->AddPerkHitEvent(perkHit);
		//			}
		//		}
		//	}
		//	break;

		//case Game::E_GameLogicMessage_GameplayImpact:
		//	{
		//		if(m_ReplayMode == E_ReplayMode_Record)
		//		{
		//			//	If the player vehicle was in a heavy impact we record this as a possible Photo Opportunity.
		//			const Game::C_GameplayImpactMsg* pMsg = Messaging::MessageCast<Game::C_GameplayImpactMsg>(pMessage);
		//			if(pMsg->mVehicleIdx == 0)	// Eurgh that's a horrible check...
		//			{
		//				if(pMsg->mImpactLevel == E_IMPACT_LEVEL_HEAVY)
		//				{
		//					//AddPhotoOpportunity(GetScaledReplayTime());
		//				}
		//			}
		//		}
		//	}
		//	break;

		case E_ReplayMessage_GoToTime:
			{
				if(m_ReplayMode == E_ReplayMode_Play)
				{
					const C_ReplayMsg_GoToTimeMsg* pMsg = Messaging::MessageCast<C_ReplayMsg_GoToTimeMsg>(pMessage);
					GoToTime(pMsg->m_ReplayTime);
				}
			}
			break;

		case E_ReplayMessage_ChangeModeRecord:
			{
				const C_ReplayMsg_ChangeModeRecordMsg* pMsg = Messaging::MessageCast<C_ReplayMsg_ChangeModeRecordMsg>(pMessage);
				if(pMsg->m_VehicleIndex <= m_NumVehicles)
				{
					m_Vehicles[pMsg->m_VehicleIndex]->ChangeReplayMode(E_ReplayMode_Record, pMsg->m_ControllerIndex);
				}
			}
			break;

	/*	case E_ReplayMessage_UpdateSessionGhost:
			{
				if(m_ReplayMode == E_ReplayMode_TimeAttack)
				{
					UpdateSessionGhost();
				}
			}
			break;

		case E_ReplayMessage_TimeAttackNewLap:
			{
				if(m_ReplayMode == E_ReplayMode_TimeAttack)
				{
					TimeAttackNewLap();
				}
			}
			break;*/

		case E_ReplayMessage_Pause:
			{
				Framework::C_ApplicationBase::GetInstance()->Pause( Framework::E_LayerPause_Freeze );
				m_Paused = true;
			}
			break;

		case E_ReplayMessage_Resume:
			{
				Framework::C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_Freeze );
				m_Paused = false;
			}
			break;
		}

		return handledMessage;
	}

	/*----------------------------------------------	NAVIGATION	-------------------------------------------*/

	E_ReplayReturnCode C_ReplayDirector::GoToScene(U32 p_SceneNum)
	{
		WipeMemoryBuffers();
		if(m_pSaveManager->GoToScene(p_SceneNum) == E_ReplayReturnCode_Failed)
		{
			return E_ReplayReturnCode_Failed;
		}
		ProcessMasterPackets();
		for(int i=0; i < m_NumVehicles; i++)
		{
			m_Vehicles[i]->ChangeReplayMode(E_ReplayMode_Play);

		}
		if(m_Paused)
		{
			Framework::C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_Freeze );	
			m_ReplayMode = E_ReplayMode_Play;
			m_PauseTrackTimer = g_debugReplaySkipPauseDelay;
		}
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetTrackRenderer()->FlushTracks(GetScaledReplayTime());
		C_PickupsManager::Instance().KillAllPickups();
		C_InGameSpecialFXManager::Instance().ClearFXStore();
		return E_ReplayReturnCode_Success;
	}

	E_ReplayReturnCode C_ReplayDirector::GoToTime(float p_ReplayTime)
	{
		return GoToScaledTime((U32)(p_ReplayTime * REPLAY_RACETIME_SCALER));
	}

	E_ReplayReturnCode C_ReplayDirector::GoToScaledTime(U32 p_ReplayTime)
	{
		if(p_ReplayTime > m_pSaveManager->GetEndReplayTime())
		{
			return E_ReplayReturnCode_Skip;
		}
		if((p_ReplayTime > GetScaledReplayTime()) && 
			(p_ReplayTime < m_pSaveManager->GetNextSceneLoadTime()))
		{
			//	We know the time we're jumping to is within the current scene, and ahead of where we are right now.
			//	So we can just set the time and the packets will catch up themselves :)
			SetScaledReplayTime(p_ReplayTime);
		}
		else
		{
			//	We need to go to a different scene, which the save manager searches for by time
			m_pSaveManager->GoToTime(p_ReplayTime);
			ProcessMasterPackets();
			SetScaledReplayTime(p_ReplayTime);
		}
		for(int i=0; i < m_NumVehicles; i++)
		{
			m_Vehicles[i]->ChangeReplayMode(E_ReplayMode_Play);
			m_Vehicles[i]->ResetPlayback();
		}
		if(m_Paused)
		{
			Framework::C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_Freeze );
			m_ReplayMode = E_ReplayMode_Play;
			m_PauseTrackTimer = g_debugReplaySkipPauseDelay;
		}
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetTrackRenderer()->FlushTracks(GetScaledReplayTime());
		C_PickupsManager::Instance().KillAllPickups();
		C_InGameSpecialFXManager::Instance().ClearFXStore();
		return E_ReplayReturnCode_Success;
	}

	E_ReplayReturnCode C_ReplayDirector::TrackXScenes(S32 p_TrackNum)
	{
		U32 currentScene = m_pSaveManager->GetCurrentSceneNumber();
		//	Don't worry if we're going to an illegal scene number, GoToScene will check
		S32 goToScene = currentScene + p_TrackNum;
		if(goToScene < 0)
			goToScene = 0;
		E_ReplayReturnCode result = GoToScene(goToScene);
		return result;
	}

	/*---------------------------------------------------------------------------------------------------------*/

	/*------------------------------------------	TIME ATTACK / GHOSTS	-----------------------------------*/



	/*===============
	NAME: TimeAttackSetup

	NOTE: Makes Time Attack-specific additions to the LevelState (adds ghosts) and sets up the appropriate Replay Director stuff
	Called BEFORE the LevelState is Finished.
	===============*/

	//E_ReplayReturnCode C_ReplayDirector::TimeAttackSetup()
	//{
	//	/*
	//	NOTE - IMPORTANT NOTE!!!!!!!!!!!!!!!!!!
	//	This implementation just assumes a single session ghost and doesn't take into account any form of ghost saving
	//	or loading existing ghosts into "slots"
	//	*/

	//	//	Clear any previous recordings (ghost data, etc, from the last time attack gameworld)
	//	Clear();

	//	m_ReplayMode = E_ReplayMode_PreTimeAttack;

	//	//	Duplicate the player vehicle into the 2nd slot (session ghost)
	//	m_pLevelState->mNumCars++;	//	Increase the number of vehicles to spawn
	//	m_pLevelState->mCars[1].mCarID = m_pLevelState->mCars[0].mCarID;
	//	m_pLevelState->mCars[1].mCharacterID = m_pLevelState->mCars[0].mCharacterID;
	//	m_pLevelState->mCars[1].mStartPosition = m_pLevelState->mCars[0].mStartPosition;
	//	m_pLevelState->mCars[1].mNameAboveCar.Format( L"Session Best" );
	//	m_pLevelState->mCars[1].mHuman = true;
	//	m_pLevelState->mCars[1].mControlType = E_VEHICLE_CONTROL_TYPE_NETWORK;
	//	m_pLevelState->mCars[1].mController = INVALID_CONTROLLER;

	//	for(U32 i=0; i < m_pLevelState->mNumCars;i++)
	//	{
	//		//	Quick hack to make buffer chunks large enough to accommodate more than a 5s scene for each vehicle/ghost
	//		m_Vehicles[i]->GetMemoryBuffer()->GetVehicleRecording()->SetMaxSize(60000);
	//		m_Vehicles[i]->GetMemoryBuffer()->GetControlsRecording()->SetMaxSize(40000);
	//	}

	//	return E_ReplayReturnCode_Success;
	//}

	//void C_ReplayDirector::UpdateSessionGhost()
	//{
	//	m_Vehicles[E_GhostType_SessionBest]->CloneMemoryBuffer(m_Vehicles[E_GhostType_PlayerPlaceholder]);
	//}

	//void C_ReplayDirector::TimeAttackNewLap()
	//{
	//	//	Clear player lap recording
	//	m_ReplayTime = 0.0f;
	//	U32 testVehicleSize = m_Vehicles[0]->GetMemoryVehicleRec()->GetUsedSize();
	//	m_Vehicles[E_GhostType_PlayerPlaceholder]->EraseRecording();
	//	m_Vehicles[E_GhostType_PlayerPlaceholder]->AddState(GetScaledReplayTime());

	//	//	Reset the ghosts
	//	for(int i=1; i < m_NumVehicles; i++)
	//	{
	//		m_Vehicles[i]->ResetPlayback();
	//		m_Vehicles[i]->ChangeReplayMode(E_ReplayMode_Play);
	//	}
	//}



	/*------------------------------------------	CUTSCENE CONTROL FUNCTIONS	-----------------------------------*/


//	bool C_ReplayDirector::ConvertBigEndianCutsceneReplayToLittleEndian( int p_Index )
//	{
//
//#ifndef CFG_GOLD
//
//		if(p_Index >= m_ScannedReplayNames.Size())
//		{
//			LOGWARN(g_ReplayDirectorLog, "Attempting to load an invalid replay index");
//			return false;
//		}
//		if(!m_pSaveManager)
//		{
//			MEMORY_TRACKER_PUSH("game.replay");
//			m_pSaveManager = new C_ReplaySaveManager();
//			MEMORY_TRACKER_POP("game.replay");
//		}
//
//		//	Where the magic happens!
//		m_pSaveManager->ConvertBigEndianCutsceneReplayToLittleEndian((char*)&m_ScannedReplayNames[p_Index]);
//#endif
//		return true;
//	}

	//	Start recording a new cutscene replay once we're in a level
	void C_ReplayDirector::StartRecordingCsReplay( void )
	{
		Util::C_String savefilename(g_cutsceneSaveFilename.Value());
		Game::C_ReplayDirector::Instance().SetSaveFilename(savefilename);
		g_RecordingOn = true;

		m_ReplayTime = 0.0f;
		EraseRecording();
		m_ReplayMode = E_ReplayMode_Record;
		m_FirstFrame = true;
		GetRaceHandles();
		for(int i = 0; i < m_NumVehicles; i++)
		{
			m_Vehicles[i]->SetReplayMode(E_ReplayMode_Record);
		}
		if(!m_pSaveManager)
		{
			MEMORY_TRACKER_PUSH("game.replay");
			m_pSaveManager = new C_ReplaySaveManager();
			MEMORY_TRACKER_POP("game.replay");
		}

		m_pSaveManager->StartNewCutscene();

		if(g_debugExportVehiclePositions)
		{
			Export_StartNewFile();
		}
		m_ItsACutscene = true;
	}
	//	This method is intended for loading cutscene replay to be played AFTER the level has been set up and loaded independently
	void C_ReplayDirector::LoadCsReplayFile(Btl::String32 p_Filename)
	{
		Util::C_String loadFilename = p_Filename.Chars();

		EraseRecording();

		if(!m_pSaveManager)
		{
			MEMORY_TRACKER_PUSH("game.replay");
			m_pSaveManager = new C_ReplaySaveManager();
			MEMORY_TRACKER_POP("game.replay");
		}

		if(!g_debugLoadCutsceneForEditing)
		{
			if(m_pSaveManager->StartCutsceneLoad(loadFilename) == E_ReplayReturnCode_Failed)
				return;

			ProcessMasterPackets();	//	Don't need to pass "true" for safety as the RLC should be initialised by now

			m_pLevelState->mReplayMode = E_ReplayMode_Play;

			GetRaceHandles(false);

			for(int i = 0; i<m_NumVehicles; i++)
			{
				//	Using CHANGE instead of SET actually goes into the vehicle actor and sets it to be under Network control,
				//	as we can't guarantee that they will be at this point in a cutscene.
				m_Vehicles[i]->ChangeReplayMode(E_ReplayMode_Play);
			}

			m_ReplayMode = E_ReplayMode_Play;

			PauseCsReplay();
		}
		else
		{
			if(m_pSaveManager->StartCutsceneEdit(loadFilename) == E_ReplayReturnCode_Failed)
				return;

			ProcessMasterPackets();

			m_pLevelState->mReplayMode = E_ReplayMode_Play;

			GetRaceHandles(false);

			for(int i = 0; i<m_NumVehicles; i++)
			{
				//	Using CHANGE instead of SET actually goes into the vehicle actor and sets it to be under Network control,
				//	as we can't guarantee that they will be at this point in a cutscene.
				m_Vehicles[i]->ChangeReplayMode(E_ReplayMode_Play);
			}
			m_ReplayMode = E_ReplayMode_CutsceneEdit;
			m_FirstFrame = true;

			PauseCsReplay();
		}
		m_ItsACutscene = true;

#ifndef CFG_GOLD
		Debug_SendConstantReplayInfo();
#endif

		return;
	}

	void C_ReplayDirector::PlayCsReplay(F32 p_StartTime)
	{
		if(m_ReplayMode == E_ReplayMode_Play ||
			m_ReplayMode == E_ReplayMode_CutsceneEdit)
		{
			// TODO [GY] - Ascertain whether this is enough

			ResumeCsReplay();

			GoToTime(p_StartTime);
		}
	}

	void C_ReplayDirector::StopCsReplay()
	{
		// TODO [GY] - Ascertain whether this will work...
		//	Will need to clear any existing recording data before we start recording any race stuff, but might be best
		//	not to do that here...
		if(m_ReplayMode == E_ReplayMode_Play)
		{
			m_ReplayMode = E_ReplayMode_FrontEnd;
		}
		else if(m_ReplayMode == E_ReplayMode_Record ||
			m_ReplayMode == E_ReplayMode_CutsceneEdit)
		{
			g_RecordingOn = false;
			StopRecording();
		}

		for(int i = 0; i < m_NumVehicles; i++)
		{
			m_Vehicles[i]->ChangeReplayMode(E_ReplayMode_Idle, m_pLevelState->mCars[i].mController);
		}
		EraseRecording();

		m_ItsACutscene = false;
		m_pSaveManager->EndPlayback();
	}

	void C_ReplayDirector::PauseCsReplay()
	{
		//m_ReplayMode = E_ReplayMode_CutscenePaused;
		m_CutscenePaused = true;

		//	Freeze all vehicles
		for(int i = 0; i < m_NumVehicles; i++)
		{
			m_Vehicles[i]->FreezeVehicle(true);
		}
	}

	void C_ReplayDirector::ResumeCsReplay()
	{
		m_CutsceneResumeDelay = 1;
	}

	/*------------------------------------------	DEBUG REPLAY TOOL ARTERY STUFF	-----------------------------------*/

	void Debug_MessageProc(ArteryConnection con, void* callback)
	{
		if(!C_ReplayDirector::Instance().m_ReplayToolCon.IsConnected())
		{
			C_ReplayDirector::Instance().m_ReplayToolCon = con;

			ArteryWaitContext ctx;

			while(con.IsConnected())
				con.Wait(ctx);
		}
	}

	bool C_ReplayDirector::Debug_InitialiseArteryConnection()
	{
#ifndef CFG_GOLD
		ArteryRegister("Replay", Debug_MessageProc, NULL);
#endif
		return true;
	}

	void C_ReplayDirector::Debug_ProcessArteryMessages()
	{
#ifndef CFG_GOLD
		while(true)
		{
			ArteryMessage msg;

			if (!m_ReplayToolCon.TryReceive(msg))
				break;

			ArteryMessageReader msgReader(msg);

			const char* msgString(NULL);

			if (msgReader.Read(msgString))
			{
				if (!strcmp(msgString, "REPLAY_UPDATE_TOOL_REQUEST"))
				{
					if(m_ReplayMode == E_ReplayMode_Play ||
						m_ReplayMode == E_ReplayMode_CutsceneEdit ||
						m_ReplayMode == E_ReplayMode_Record)
					{
						ArteryMessageBuilder b;
						b.Write("REPLAY_UPDATE_TOOL");
						b.Write(m_ReplayTime);
						b.Write(m_pSaveManager->GetCurrentSceneNumber());
						b.Write(m_pLevelState->mCars[m_pRLC->GetViewportFocusVehicleIndex(0)].mNameAboveCar.AsString());
						b.Write(m_pLevelState->mCars[AmaxCam::LookAtIndex].mNameAboveCar.AsString());

						ArteryMessage m;
						b.Build(m);
						Debug_SendMessageToTool(m);
					}
				}
				else if(!strcmp(msgString, "REPLAY_CONSTANT_INFO_REQUEST"))
				{
					if(m_ReplayMode == E_ReplayMode_Play ||
						m_ReplayMode == E_ReplayMode_CutsceneEdit ||
						m_ReplayMode == E_ReplayMode_Record)
					{
						Debug_SendConstantReplayInfo();
					}
				}
				else if(!strcmp(msgString, "REPLAY_PAUSE_TOGGLE"))
				{
					if(m_ReplayMode == E_ReplayMode_Play ||
						m_ReplayMode == E_ReplayMode_Paused)
					{
						int result;
						msgReader.Read(result);
						if(result == 0)
						{
							Resume();
						}
						else if(result == 1)
						{
							Pause();
						}
					}
				}
				else if(!strcmp(msgString, "REPLAY_LOOP_TOGGLE"))
				{
					int result;
					msgReader.Read(result);
					if(result == 0)
						m_Loop = false;
					else if(result == 1)
						m_Loop = true;
				}
				else if(!strcmp(msgString, "REPLAY_RESTART"))
				{
					if(m_ReplayMode == E_ReplayMode_Play ||
						m_ReplayMode == E_ReplayMode_Paused)
					{
						GoToScene(0);
					}
				}
				else if(!strcmp(msgString, "REPLAY_PREVIOUS_SCENE"))
				{
					TrackXScenes(-1);
				}
				else if(!strcmp(msgString, "REPLAY_NEXT_SCENE"))
				{
					TrackXScenes(1);
				}
				else if(!strcmp(msgString, "REPLAY_GOTO_TIME"))
				{
					float time;
					msgReader.Read(time);
					GoToTime(time);
				}
				else if(!strcmp(msgString, "REPLAY_CHANGE_FOCUS_VEHICLE"))
				{
					if(m_ReplayMode == E_ReplayMode_Play ||
						m_ReplayMode == E_ReplayMode_CutsceneEdit ||
						m_ReplayMode == E_ReplayMode_Record ||
						m_ReplayMode == E_ReplayMode_Paused)
					{
						int vehicleIndex;
						msgReader.Read(vehicleIndex);
						if(vehicleIndex >= m_NumVehicles)
							break;
						m_pRLC->SetViewportFocusVehicleIndex(0, vehicleIndex);
					}
				}
				else if(!strcmp(msgString, "REPLAY_CHANGE_SCENE_LENGTH"))
				{
					U32 sceneLength;
					msgReader.Read(sceneLength);
					m_pSaveManager->SetSceneLength((float)sceneLength);
				}
				else if(!strcmp(msgString, "REPLAY_RECORD_EVERY_FRAME_TOGGLE"))
				{
					int result;
					msgReader.Read(result);
					if(result == 0)
					{
						g_RecordEveryFrame = false;
					}
					else if(result == 1)
					{
						g_RecordEveryFrame = true;
					}
				}
				else if(!strcmp(msgString, "REPLAY_SET_SAVE_FILENAME"))
				{
					const char * saveFilename(NULL);
					msgReader.Read(saveFilename);
					m_pSaveManager->SetSaveFilename(saveFilename);
				}
				else if(!strcmp(msgString, "REPLAY_CHANGE_CAMERA"))
				{
					const char* camType(NULL);
					msgReader.Read(camType);
					if(!strcmp(camType, "Game Feed"))
					{
						AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_GameFeed);
					}
					else if(!strcmp(camType, "Fly"))
					{
						AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_FlyCamera);
					}
					else if(!strcmp(camType, "Relative Fly"))
					{
						AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_RelativeFlyCamera);
					}
					else if(!strcmp(camType, "Fixed"))
					{
						AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_FixedCamera);
					}
					else if(!strcmp(camType, "Relative"))
					{
						AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_RelativeCamera);
					}
					else if(!strcmp(camType, "Script Cam"))
					{
						AmaxCam::Switcher().SetType(AmaxCam::E_DebugCam_ScriptCamera);
					}
				}
				else if(!strcmp(msgString, "REPLAY_LOOKAT_TOGGLE"))
				{
					int result;
					msgReader.Read(result);
					if(result)
						AmaxCam::LookAtEnable = true;
					else
						AmaxCam::LookAtEnable = false;
				}
				else if(!strcmp(msgString, "REPLAY_LOOKATX"))
				{
					float result;
					msgReader.Read(result);
					AmaxCam::LookOffsetX = result; 
				}
				else if(!strcmp(msgString, "REPLAY_LOOKATY"))
				{
					float result;
					msgReader.Read(result);
					AmaxCam::LookOffsetY = result; 
				}
				else if(!strcmp(msgString, "REPLAY_LOOKATZ"))
				{
					float result;
					msgReader.Read(result);
					AmaxCam::LookOffsetZ = result; 
				}
				else if(!strcmp(msgString, "REPLAY_LOOKAT_FOV"))
				{
					float result;
					msgReader.Read(result);
					AmaxCam::LookFov = result;
				}
				else if(!strcmp(msgString, "REPLAY_CHANGE_LOOKAT_TARGET"))
				{
					int result;
					msgReader.Read(result);
					AmaxCam::LookAtIndex = result;
				}

				else if(!strcmp(msgString, "REPLAY_SHAKEOVERRIDE_TOGGLE"))
				{
					int result;
					msgReader.Read(result);
					if(result)
						Cam2::ShakeOverride = true;
					else
						Cam2::ShakeOverride = false;
				}
				else if(!strcmp(msgString, "REPLAY_SHAKEX"))
				{
					float result;
					msgReader.Read(result);
					Cam2::ShakeAmountX = result;
				}
				else if(!strcmp(msgString, "REPLAY_SHAKEY"))
				{
					float result;
					msgReader.Read(result);
					Cam2::ShakeAmountY = result;
				}
				else if(!strcmp(msgString, "REPLAY_SHAKEZ"))
				{
					float result;
					msgReader.Read(result);
					Cam2::ShakeAmountZ = result;
				}
				else if(!strcmp(msgString, "REPLAY_SHAKEFREQUENCY"))
				{
					float result;
					msgReader.Read(result);
					Cam2::ShakeFreq = result;
				}
				else if(!strcmp(msgString, "REPLAY_DEBUG_LOAD_FILE"))
				{
					const char * filename(NULL);
					msgReader.Read(filename);
					m_DebugLoadFilename = filename;
				}
				else if(!strcmp(msgString, "REPLAY_SHOWUI_TOGGLE"))
				{
					int result;
					msgReader.Read(result);
					if(result)
						g_UiDebugDoRender = true;
					else
						g_UiDebugDoRender = false;
				}
				else if(!strcmp(msgString, "REPLAY_FIREPERK"))
				{
					const char * type(NULL);
					int level, index;
					msgReader.Read(type);
					msgReader.Read(level);
					msgReader.Read(index);

					Actor::C_ActorHandle actorHandle;
					Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(index);
					if(index >= m_NumVehicles)
						break;

					if(!strcmp(type, "Nitrous"))
					{
						pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_NITROUS;
					}
					else if(!strcmp(type, "Shock"))
					{
						pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_SHOCK;
					}
					else if(!strcmp(type, "Shunt"))
					{
						pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_SHUNT;
					}
					else if(!strcmp(type, "Barge"))
					{
						pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_BARGE;
					}
					else if(!strcmp(type, "Bomb"))
					{
						pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_BOMB;
					}
					else if(!strcmp(type, "Repair"))
					{
						pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_REPAIR;
					}
					else
					{
						pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_NONE;
					}

					for(int i = 0; i < level; i++)
					{
						C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
					}
				}

				else if(!strcmp(msgString, "REPLAY_SKIP_FRAME"))
				{
					if(m_pRLC)
					{
						//m_ReplayMode = E_ReplayMode_Play;
						//g_DebugFramePauseDelay = 1;
						m_pRLC->SkipFrames(1);
					}
				}
				else if(!strcmp(msgString, "REPLAY_TAKE_CONTROL"))
				{
					if(m_ReplayMode == E_ReplayMode_CutsceneEdit)
					{
						int vehicleIndex;
						msgReader.Read(vehicleIndex);
						if(vehicleIndex >= m_NumVehicles)
							break;
						m_Vehicles[vehicleIndex]->ChangeReplayMode(E_ReplayMode_Record, 0);
					}
				}
				else if(!strcmp(msgString, ""))
				{
				}
			}
		}
#endif
	}

	void C_ReplayDirector::Debug_SendMessageToTool(ArteryMessage &m)
	{
#ifndef CFG_GOLD
		m_ReplayToolCon.Send(m);
#endif
	}

	void C_ReplayDirector::Debug_SendMessageToTool(const char * p_string)
	{
#ifndef CFG_GOLD
		ArteryMessageBuilder b;
		b.Write(p_string);

		ArteryMessage m;

		if (b.Build(m))
			Debug_SendMessageToTool(m);
#endif
	}

	void C_ReplayDirector::Debug_SendConstantReplayInfo()
	{
#ifndef CFG_GOLD
		ArteryMessageBuilder b;
		b.Write("REPLAY_CONSTANT_INFO");
		b.Write(m_pSaveManager->GetLoadFilename());
		b.Write(((float)m_pSaveManager->GetEndReplayTime()) / REPLAY_RACETIME_SCALER);
		b.Write(m_pSaveManager->GetNumScenes());
		b.Write(m_NumVehicles);

		ArteryMessage m;

		if (b.Build(m))
			Debug_SendMessageToTool(m);
#endif
	}

	bool C_ReplayDirector::Debug_LoadReplay()
	{
		if(!m_pSaveManager)
		{
			MEMORY_TRACKER_PUSH("game.replay");
			m_pSaveManager = new C_ReplaySaveManager();
			MEMORY_TRACKER_POP("game.replay");
		}
		if(m_pSaveManager->StartLoad(m_DebugLoadFilename) == E_ReplayReturnCode_Failed)
			return false;
		m_NumVehicles = m_pLevelState->mNumCars;

		//	Set initial vehicle positions into the level state so they are then set through the normal processes

		ProcessMasterPackets(true);

		m_pLevelState->mReplayMode = E_ReplayMode_Play;
		m_pLevelState->Finished();
		for(int i = 0; i<m_NumVehicles; i++)
		{
			m_Vehicles[i]->SetReplayMode(E_ReplayMode_Play);
		}
		m_ReplayMode = E_ReplayMode_PrePlay;	//	Don't set to _Play yet as this will try to access the non-existent RLC!
#ifndef CFG_GOLD
		Debug_SendConstantReplayInfo();
#endif
		return true;
	}

	/*-------------------------------	REPLAY VEHICLE WORLD MATRIX EXPORT...THING. STUFF.	---------------------------*/

	bool C_ReplayDirector::Export_StartNewFile()
	{
		Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

		Btl::Vector<C_GameObjectRecord> myActorTable;
		Btl::String carString = "car";
		for(int i = 0; i < m_NumVehicles; i++)
		{
			myActorTable.PushBack();
			Btl::String indexString = carString + Util::unsignedIntegerToCString(i).Chars();
			myActorTable.Back().name = indexString;
		}
		fs.CreateSingleDirectory("Replays\\cutsceneReplays\\");
		Util::C_String exportFilename = Time::FormatDate_TimeString(Time::C_DecomposedTime(Time::GetCurrentTime())) + ".csrp";
		m_pExportSaveFile = fs.Open(
			"Replays\\cutsceneReplays\\" + exportFilename, 
			Sys::E_FileAccess_Append);
		if (!m_pExportSaveFile)
			return false;

		U32 endianDetector = 1;
		m_pExportSaveFile->Write(&endianDetector, 4);

		//	File header
		m_pExportHeaderFileBlock = new Sys::C_FileBlock(m_pExportSaveFile, "objsmp");
		{
			Arch::BinaryOutputArchive ar(*m_pExportHeaderFileBlock, Ut::NoEndianSwap::swap );
			U32 version = 1;
			ar << version;
		}

		//	Table of object records
		{
			Sys::C_FileBlock gameDescBlock(m_pExportHeaderFileBlock, "objdesc");
			Arch::BinaryOutputArchive ar(gameDescBlock, Ut::NoEndianSwap::swap );
			ar << m_NumVehicles;
			for (int i = 0; i < m_NumVehicles; ++i)
				myActorTable[i].Serialise(ar);
		}

		//	Start block for vehicle position samples
		{
			m_ExportNumSamples = 0;
			m_pExportSamplesFileBlock = new Sys::C_FileBlock(m_pExportHeaderFileBlock, "samples");
			m_pExportSamplesArchive = new Arch::BinaryOutputArchive(*m_pExportSamplesFileBlock, Ut::NoEndianSwap::swap);

			U32 kSamplesNotSetPlaceholder = 0xffffffff;
			//Write placeholder number of samples
			m_ExportSamplesBlockStartPos = m_pExportSamplesFileBlock->GetPosition();
			(*m_pExportSamplesArchive) << kSamplesNotSetPlaceholder;
		}

		return true;
	}

	//	Write the positions of all the vehicles to memory
	void C_ReplayDirector::Export_WriteVehiclePositions(F32 p_DeltaTime)
	{
		if(!m_pExportSaveFile || !m_pExportSamplesFileBlock)
			return;

		Btl::Vector<MathsStorage::Float3> actorTrans;
		Btl::Vector<MathsStorage::Float4> actorQuats;

		for(int i = 0; i < m_NumVehicles; i++)
		{
			RacingPhysics::Vehicle::CompressedNetworkState compVehicleState;
			RacingPhysics::Car::NetworkState vehicleState;

			m_Vehicles[i]->GetNetworkState(&compVehicleState);
			RacingPhysics::Car::UncompressNetworkState(compVehicleState, vehicleState);
			actorTrans.PushBack(MathsStorage::Float3(vehicleState.axis.Position().X(),
				vehicleState.axis.Position().Y(),
				vehicleState.axis.Position().Z()));

			const MathsStorage::Matrix44& tempAxis = vehicleState.axis.GetMatrix();
			Maths::HVec4 quaternionVector = Maths::C_Quaternion(Maths::HMat33(Maths::Load<Maths::HMat44>(&tempAxis)));
			actorQuats.PushBack(MathsStorage::Float4(quaternionVector.X(), quaternionVector.Y(), quaternionVector.Z(), quaternionVector.W()));
		}

		C_GameObjectSample thisSampleData;
		thisSampleData.time = m_ReplayTime;
		thisSampleData.pPos = &actorTrans[0];
		thisSampleData.pQuat = &actorQuats[0];

		thisSampleData.Serialise(*m_pExportSamplesArchive, m_NumVehicles);

		m_ExportNumSamples++;
	}

	void C_ReplayDirector::Export_FinishFile()
	{
		if(!m_pExportSamplesFileBlock || !m_pExportHeaderFileBlock || !m_pExportSamplesFileBlock || !m_pExportSamplesArchive)
			return;

		Sys::C_StreamPos currentPos = m_pExportSamplesFileBlock->GetPosition();
		m_pExportSamplesFileBlock->SetPosition(m_ExportSamplesBlockStartPos);
		m_pExportSamplesFileBlock->Write(&m_ExportNumSamples, sizeof(U32));
		m_pExportSamplesFileBlock->SetPosition(currentPos);

		SAFE_DELETE_PTR(m_pExportSamplesArchive);
		SAFE_DELETE_PTR(m_pExportSamplesFileBlock);
		SAFE_DELETE_PTR(m_pExportHeaderFileBlock);
		SAFE_DELETE_PTR(m_pExportSaveFile);
	}

	void C_ReplayDirector::Pause()
	{
		Framework::C_ApplicationBase::GetInstance()->Pause( Framework::E_LayerPause_Freeze );
		m_Paused = true;
	}

	void C_ReplayDirector::Resume()
	{
		Framework::C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_Freeze );
		m_Paused = false;
	}

};	//	namespace Game
