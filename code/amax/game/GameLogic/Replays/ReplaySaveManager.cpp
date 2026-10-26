//	This implementation of C_ReplaySaveManager is intended for the Xbox 360 platform, so will only be included in Xenon builds.

#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <GameSystems.h>

#include <GameLogic\Replays\ReplaySaveManager.h>

#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <GameLogic/Replays/MemChunk.h>

#include <GameLogic/Layers/RaceBookLayer.h>

#include <misc_shared\Messaging\MessageMultiHandler.h>
#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>
#include <GameLogic\Replays\ReplayDirector.h>
#include <GameLogic\Replays\ReplayThreading.h>
#include <GameLogic\Replays\ReplayVehicleManager.h>
#include <GameLogic\Replays\ReplayDefines.h>
#include <GameLogic\Replays\ReplayUtilities.h>
#include <GameLogic\Structures\LevelState.h>

#include <system\time\Time.h>

#include <System\Memory.h>


#ifndef CFG_GOLD

#include <GameLogic\Replays\ReplayPacket.h>
#include <GameLogic\Replays\ReplayTypes.h>
#include <RacingPhysics/RacingPhysics.h>
#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <Vehicle\Damage\DamageLayer.h>

#endif


Debug::Logger &gReplaySaveLogger( Debug::GetLogger("amax.replay.save") );
Debug::String g_replayDebugSaveFilename("Replays.Recording.SaveFilename", "");
Debug::String g_cutsceneSaveFilename("Replays.Cutscenes.SaveFilename", "");
Debug::Bool g_debugDoNotSetEventFromReplayHeader("Replays.DoNotLoadEventsFromReplays", false);

extern Debug::Bool g_AllowMultiplePickupsPerLap;


namespace Game
{

const int replaySaveVersion = 8; // Increment if this class is changed
const int replayCompletedMagicNumber = 121;	//	Magic number we write at the end of a replay and read at the beginning of a load to check it's valid
const float	defaultSceneLength = 5.0f;

C_ReplaySaveManager::C_ReplaySaveManager()
{
	m_pSaveFile = NULL;
	m_pLoadFile = NULL;
	m_pLoadingThread = NULL;

	MEMORY_TRACKER_PUSH("game.replaySaveManager");
	m_Header = new C_ReplayHeader();
	m_SaveFooter = new C_ReplayFooter();
	m_LoadFooter = new C_ReplayFooter();
	Clear();

	m_pSavingThread = new C_ReplaySavingThread();
	m_pLoadingThread = new C_ReplayLoadingThread();
	m_pSaveAndLoadThread = new C_ReplaySaveAndLoadThread();
	MEMORY_TRACKER_POP("game.replaySaveManager");
	
	m_ReplaySceneLengthScaled = (int)(defaultSceneLength * REPLAY_RACETIME_SCALER);	//	Default to scenes of 5 seconds
}

void C_ReplaySaveManager::Destroy()
{
	Clear();

	SAFE_DELETE_PTR(m_pLoadingThread);
	SAFE_DELETE_PTR(m_pSavingThread);
	SAFE_DELETE_PTR(m_pSaveAndLoadThread);
	SAFE_DELETE_PTR(m_Header);
	SAFE_DELETE_PTR(m_SaveFooter);
	SAFE_DELETE_PTR(m_LoadFooter);
}

void C_ReplaySaveManager::Clear()
{
	m_Header->Clear();
	m_SaveFooter->Clear();
	m_LoadFooter->Clear();

	m_SaveFilename = "";
	m_LoadFilename = "";

	m_NumScenesLoaded = 0;
	m_LoadedAllScenes = false;

	SAFE_DELETE_PTR(m_pSaveFile);
	SAFE_DELETE_PTR(m_pLoadFile);
}

void C_ReplaySaveManager::FillHeader()
{
	if(!m_Header)
	{
		m_Header = new C_ReplayHeader();
	}
	m_Header->m_Version = replaySaveVersion;
	m_Header->m_SceneLength = m_ReplaySceneLengthScaled;

	Game::C_LevelState* levelState = &C_Game::GetInstance()->GetLevelState();

	m_Header->m_NumCars = levelState->mNumCars+ levelState->mNumAdditionalCars;
	m_Header->m_Event = levelState->mEvent;
	m_Header->m_Route = levelState->mRoute;
	m_Header->m_RouteNumber = levelState->mRouteNumber;
	m_Header->m_NumLaps = levelState->mLaps;

	//	Only set for the #cars we actually have, the others will remain zero
	for(int i = 0; i < m_Header->m_NumCars; i++)
	{
		//	Grab vehicle Sorcery stuff from LevelState
		m_Header->m_CarInfo[i].m_CarID = levelState->mCars[i].mCarID;
		m_Header->m_CarInfo[i].m_CharacterID = levelState->mCars[i].mCharacterID;
		m_Header->m_CarInfo[i].m_NameAboveCar = levelState->mCars[i].mNameAboveCar.AsString();
		m_Header->m_CarInfo[i].m_ResprayId = levelState->mCars[i].mResprayID;
	}
}

void C_ReplaySaveManager::SetFromHeader()
{
	Game::C_LevelState* levelState = &C_Game::GetInstance()->GetLevelState();

	if(!m_Header)
	{
		return;
	}
	levelState->mNumCars = m_Header->m_NumCars;

	if(m_Header->m_Event != 0 && !g_debugDoNotSetEventFromReplayHeader)
		levelState->mEvent = m_Header->m_Event;
	levelState->mRoute = m_Header->m_Route;
	levelState->mCity = C_GameData::Instance().mRoutes[(GAMEDATA_ID)m_Header->m_Route]->city->ID;
	levelState->mRouteNumber = m_Header->m_RouteNumber;
	levelState->mLaps = m_Header->m_NumLaps;
	levelState->mNumViewports = 1;

	for(int i=0; i < m_Header->m_NumCars; i++)
	{
		//	Send vehicle Sorcery stuff to LevelState
		levelState->mCars[i].mCarID = m_Header->m_CarInfo[i].m_CarID;
		levelState->mCars[i].mCharacterID = m_Header->m_CarInfo[i].m_CharacterID;
		levelState->mCars[i].mNameAboveCar.Format(L"%S", m_Header->m_CarInfo[i].m_NameAboveCar.Chars());
		if(m_Header->m_CarInfo[i].m_ResprayId != 0)
			levelState->mCars[i].mResprayID = m_Header->m_CarInfo[i].m_ResprayId;
		levelState->mCars[i].mHuman = true;

		levelState->mCars[i].mControlType = E_VEHICLE_CONTROL_TYPE_NETWORK;
		levelState->mCars[i].mController = INVALID_CONTROLLER;

		levelState->mCars[i].mUpgrades.NumPickupSlots(3);
	}
	g_AllowMultiplePickupsPerLap = true;	//	Goodness that's a horrible hack, but oh well.
}

void C_ReplaySaveManager::SetSceneLength(float p_LengthSeconds)
{
	if(p_LengthSeconds > 1.5f)
	{
		m_ReplaySceneLengthScaled = (int)(p_LengthSeconds * REPLAY_RACETIME_SCALER);
	}
}


void C_ReplaySaveManager::SetSceneLengthScaled( U32 p_LengthScaled )
{
	m_ReplaySceneLengthScaled = p_LengthScaled;
}


/*-------------------------------------------	SAVING METHODS	-------------------------------------------*/

E_ReplayReturnCode C_ReplaySaveManager::StartNewReplay()
{
	Clear();

	FillHeader();

	m_LastSceneSaveTime = (int)(REPLAY_START_TIME * REPLAY_RACETIME_SCALER);

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	fs.CreateSingleDirectory("Replays\\");

	Util::C_String debugFilename(g_replayDebugSaveFilename.Value());
	if((debugFilename == "") ||
		(fs.FileExists("Replays\\" + debugFilename + ".amrp")))
	{
		m_SaveFilename = Time::FormatDate_TimeString(Time::C_DecomposedTime(Time::GetCurrentTime())) + ".amrp";
	}
	else
	{
		m_SaveFilename = debugFilename + ".amrp";
	}
	m_pSaveFile = fs.Open("Replays\\" + m_SaveFilename, Sys::E_FileAccess_Append);
	if (m_pSaveFile == 0)
		return E_ReplayReturnCode_Failed;

	Sys::BinarySerialiserAutoEndian serial(*m_pSaveFile);

	m_Header->Serialise(serial);

	return E_ReplayReturnCode_Success;
}

E_ReplayReturnCode C_ReplaySaveManager::WriteNewReplayScene(U32 p_NewSceneSaveTime)
{
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	if(m_pSaveFile == 0)
	{
		LOGERROR(gReplaySaveLogger, "Accessing replay file to append failed");
		return E_ReplayReturnCode_Failed;
	}
	//	Grab the sizes for all the various memory chunk data in the scene we're ready to write to the file.
	//	Need to do this so we can calculate that total size of the whole scene - that way we can skip past entire scenes
	U32 totalSceneSize = 0;
	U32 CarChunkSizes[MAX_VEHICLES][2];
	U32 miscChunkSize = 0;
	U32 masterChunkSize = 0;
	for(int i=0; i < m_Header->m_NumCars; i++)
	{
		CarChunkSizes[i][0] = C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->GetUsedSize();
		CarChunkSizes[i][1] = C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->GetUsedSize();
		//	Accumulate the sizes of the memory chunks we're going to write, but we're also going to write 
		//	those sizes before each chunk within the scene so we have to include the sizes of those...sizes
		totalSceneSize+= CarChunkSizes[i][0] + CarChunkSizes[i][1] + 
			sizeof(CarChunkSizes[i][0]) + sizeof(CarChunkSizes[i][1]);
	}
	miscChunkSize = C_ReplayDirector::Instance().GetDiskMiscRec()->GetUsedSize();
	totalSceneSize+= miscChunkSize + sizeof(miscChunkSize);
	masterChunkSize = C_ReplayDirector::Instance().GetDiskMasterRec()->GetUsedSize();
	totalSceneSize+= masterChunkSize + sizeof(masterChunkSize);
	//	Also add the size of the replay time that this scene started, which we'll use to know when to load it
	totalSceneSize += sizeof(m_LastSceneSaveTime);
	//	Start writing the scene to file, starting with the total length of this scene in the file
	Sys::BinarySerialiserAutoEndian serial(*m_pSaveFile);

	serial << totalSceneSize;
	serial << m_LastSceneSaveTime;

	for(int i=0; i < m_Header->m_NumCars; i++)
	{
		U32 test, test2;
		//	Write vehicle state chunk size and contents
		serial << CarChunkSizes[i][0];
		test = CarChunkSizes[i][0];
		m_pSaveFile->Write(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->GetStart(), CarChunkSizes[i][0]);

		//	Write vehicle controls chunk size and contents
		serial << CarChunkSizes[i][1];
		test2 = CarChunkSizes[i][1];
		m_pSaveFile->Write(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->GetStart(), CarChunkSizes[i][1]);
	}

 	serial << miscChunkSize;
	m_pSaveFile->Write(C_ReplayDirector::Instance().GetDiskMiscRec()->GetStart(), miscChunkSize);
	serial << masterChunkSize;
	m_pSaveFile->Write(C_ReplayDirector::Instance().GetDiskMasterRec()->GetStart(), masterChunkSize);
	//	Set the new save time
	m_LastSceneSaveTime = p_NewSceneSaveTime;
	//	Increase the scene number counter
	m_SaveFooter->m_NumScenes++;

	return E_ReplayReturnCode_Success;
}

E_ReplayReturnCode C_ReplaySaveManager::FinishSaveFile(U32 p_ReplayTime)
{
	m_pSavingThread->Join();

	//	Grab a final vehicle position state for every vehicle
	C_ReplayDirector::Instance().GrabStatesAllVehicles(p_ReplayTime);

	//	Swap the buffers to grab the final scene data
	C_ReplayDirector::Instance().SwapBuffers();

	//	Write the last scene
	WriteNewReplayScene(p_ReplayTime);


	Sys::BinarySerialiserAutoEndian serial(*m_pSaveFile);

	//	Set and serialise the ReplayFooter stuff
	m_SaveFooter->m_EndOfReplayTime = m_LastSceneSaveTime;
	m_SaveFooter->m_CompletedMarker = replayCompletedMagicNumber;	//	Magic number we check for when loading in a replay file - if it doesn't match this then the replay wasn't cleanly finished
	m_SaveFooter->Serialise(serial);

	//	Flush the remaining serialisation data to ensure everything gets written before we delete the file handle
	m_pSaveFile->Flush();

	SAFE_DELETE_PTR(m_pSaveFile);
	return E_ReplayReturnCode_Success;
}

E_ReplayReturnCode C_ReplaySaveManager::DumpSceneIfItsTime(U32 p_ReplayTime)
{
	if(m_pSavingThread->IsActive())
		return E_ReplayReturnCode_Skip;
	//	Check whether it's time to dump a new scene to file

	if(p_ReplayTime > (m_LastSceneSaveTime + m_ReplaySceneLengthScaled))
	{
		//	Grab a position state for every vehicle before we dump the scene
		C_ReplayDirector::Instance().GrabStatesAllVehicles(p_ReplayTime);
		C_ReplayDirector::Instance().SwapBuffers();
		//	Dump the ReplayDirector's current memory to file
		m_pSavingThread->Enter(p_ReplayTime);
		return E_ReplayReturnCode_Success;
	}
	else
		return E_ReplayReturnCode_Skip;
}



/*--------------------------------------------	LOADING METHODS	-------------------------------------------*/

E_ReplayReturnCode C_ReplaySaveManager::StartLoad(Util::C_String p_Filename)
{
	Clear();
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	if(!fs.FileExists(p_Filename))
	{
		LOGERROR(gReplaySaveLogger, "Replay file " + p_Filename + " doesn't appear to exist");
		return E_ReplayReturnCode_Failed;
	}
	m_LoadFilename = p_Filename;
	m_pLoadFile = fs.Open(m_LoadFilename, Sys::E_FileAccess_Read);
	if(m_pLoadFile == 0)
	{
		LOGERROR(gReplaySaveLogger, "Couldn't open replay file " + m_LoadFilename);
		return E_ReplayReturnCode_Failed;
	}

	if(!m_Header)
	{
		m_Header = new C_ReplayHeader();
	}
	Sys::BinaryDeserialiserAutoEndian serial(*m_pLoadFile);

	serial >> m_Header->m_Version;
	m_Header->Deserialise(serial);
	if(m_Header->m_Version != replaySaveVersion)
	{
		LOGWARN(gReplaySaveLogger, "Older replay save with version %d - current version is %d", m_Header->m_Version, replaySaveVersion);
		return E_ReplayReturnCode_Failed;
		//	Don't need to fail here anymore as *in theory* we should handle replay files from previous versions OK
	}
	SetFromHeader();

	//	Go to the end of the file and then back the size of the ReplayFooter struct
	m_pLoadFile->SetPosition(m_pLoadFile->Length()-(sizeof(C_ReplayFooter)));
	m_LoadFooter->Deserialise(serial);
	if((m_LoadFooter->m_EndOfReplayTime <= 0) || (m_LoadFooter->m_NumScenes <= 0) || (m_LoadFooter->m_CompletedMarker != replayCompletedMagicNumber))
	{
		LOGERROR(gReplaySaveLogger, "ReplayFooter data is not within expected parameters - file was not cleanly finished");
		return E_ReplayReturnCode_Failed;
	}
	//	Go to the first scene in the replay
	GoToScene(0);

	return E_ReplayReturnCode_Success;
}

E_ReplayReturnCode C_ReplaySaveManager::LoadNewReplayScene()
{
	//	Shouldn't get in here if this is true but check it just in case
	if(m_LoadedAllScenes)
	{
		return E_ReplayReturnCode_Skip;
	}
	//PERF_EVENT("C_ReplaySaveManager::LoadNewReplayScene");
	C_ReplayDirector::Instance().WipeDiskBuffers();

	if(m_pLoadFile == 0)
	{
		LOGERROR(gReplaySaveLogger, "Accessing replay file to load next scene failed");
		return E_ReplayReturnCode_Failed;
	}
	if(m_pLoadFile->GetPosition() == 0)
	{
		LOGERROR(gReplaySaveLogger, "Accessing replay file to load next scene failed");
		return E_ReplayReturnCode_Failed;
	}

	Sys::BinaryDeserialiserAutoEndian serial(*m_pLoadFile);

	U64 filePointerPos = m_pLoadFile->GetPosition();

	U32 sceneSize;

 	serial >> sceneSize;

	serial >> m_NextSceneLoadTime;

	for(int i=0; i < m_Header->m_NumCars; i++)
	{
		U32 stateSize;
		serial >> stateSize;
		//	Only resize memory chunks in buffers if they are too small, should cut down on NEWing
		if(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->GetSize() < stateSize)
			C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->Resize(stateSize);
		m_pLoadFile->Read(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->GetStart(), stateSize);

		U32 controlSize;
		serial >> controlSize;
		//	Only resize memory chunks in buffers if they are too small, should cut down on NEWing
		if(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->GetSize() < controlSize)
			C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->Resize(controlSize);
		m_pLoadFile->Read(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->GetStart(), controlSize);
	}

	U32 miscChunkSize;
	serial >> miscChunkSize;
	if(C_ReplayDirector::Instance().GetDiskMiscRec()->GetSize() < miscChunkSize)
		C_ReplayDirector::Instance().GetDiskMiscRec()->Resize(miscChunkSize);
	m_pLoadFile->Read(C_ReplayDirector::Instance().GetDiskMiscRec()->GetStart(), miscChunkSize);

	U32 masterChunkSize;
	serial >> masterChunkSize;

	if(C_ReplayDirector::Instance().GetDiskMasterRec()->GetSize() < masterChunkSize)
		C_ReplayDirector::Instance().GetDiskMasterRec()->Resize(masterChunkSize);
	m_pLoadFile->Read(C_ReplayDirector::Instance().GetDiskMasterRec()->GetStart(), masterChunkSize);

	m_NumScenesLoaded++;

	return E_ReplayReturnCode_Success;
}

E_ReplayReturnCode C_ReplaySaveManager::LoadSceneIfItsTime(U32 p_ReplayTime)
{
	PERF_EVENT("C_ReplaySaveManager::LoadSceneIfItsTime()");

	//	If load thread is still running skip and check again next frame
	if(m_pLoadingThread->IsActive())
		return E_ReplayReturnCode_Skip;
	m_pLoadingThread->Join();
	//	Did the last runthrough of the loading thread find the end of the replay? If so just return
	if(m_LoadedAllScenes)
	{
		return E_ReplayReturnCode_Skip;
	}
	if(p_ReplayTime >= m_NextSceneLoadTime)
	{
		// We've already loaded the next scene into the Disk buffer, so we swap and
		// it's now the Memory buffer - and the Director plays it
		C_ReplayDirector::Instance().SwapBuffers();
		C_ReplayDirector::Instance().WipeDiskBuffers();
		//	Check if we've now loaded - and swapped in - all the scenes (i.e. are about to reach the end of the replay)
		//	m_NumScenesLoaded should only increase here so should never be > m_Footer.m_NumScenes but might as welll be safe
		if(m_NumScenesLoaded >= m_LoadFooter->m_NumScenes)
		{
			m_LoadedAllScenes = true;
		}
		//	Load the next scene so it's ready
		m_pLoadingThread->Enter();
		return E_ReplayReturnCode_Success;
	}
	else
		return E_ReplayReturnCode_Skip;
}


/*------------------------------------------	CUTSCENE METHODS	---------------------------------------*/

/*
===============
NAME: StartNewCutscene

NOTE: Starts a new cutscene recording. This is identical to a normal replay, but it's placed in a different directory
		and has a different file extension :)
===============
*/
E_ReplayReturnCode C_ReplaySaveManager::StartNewCutscene()
{
	//	Before we Clear the save manager, we grab the save filename that has possibly been set before recording this cutscene.

	Util::C_String tempSaveFilename = m_SaveFilename;

	Clear();

	m_SaveFilename = tempSaveFilename;

	//	5-second scenes.
	SetSceneLength(5.0f);

	FillHeader();

	m_LastSceneSaveTime = 0;

	//	TO DO: Do platform-specific directory creation stuff here

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	fs.CreateSingleDirectory("Replays\\cutsceneReplays\\");

	//	If we're creating a cutscene, we should have specified a filename for this replay already
	if(m_SaveFilename == "" || fs.FileExists("Replays\\cutsceneReplays\\" + m_SaveFilename + ".amcs"))
	{
		Util::C_String debugFilename(g_cutsceneSaveFilename.Value());
		if((debugFilename == "") ||
			(fs.FileExists("Replays\\cutsceneReplays\\" + debugFilename + ".amcs")))
		{
			m_SaveFilename = Time::FormatDate_TimeString(Time::C_DecomposedTime(Time::GetCurrentTime())) + ".amcs";
		}
		else
		{
			m_SaveFilename = debugFilename + ".amcs";
		}
	}
	else
	{
		m_SaveFilename += ".amcs";
	}
	m_pSaveFile = fs.Open("Replays\\cutsceneReplays\\" + m_SaveFilename, Sys::E_FileAccess_Append);
	if (m_pSaveFile == 0)
		return E_ReplayReturnCode_Failed;

	Sys::BinarySerialiserAutoEndian serial(*m_pSaveFile);
	m_Header->Serialise(serial);
	return E_ReplayReturnCode_Success;
}

E_ReplayReturnCode C_ReplaySaveManager::StartCutsceneEdit(Util::C_String p_Filename)
{
	StartCutsceneLoad(p_Filename);

	/*--------------	Start save file for new cutscene	---------------*/

	SetSceneLengthScaled(m_Header->m_SceneLength);
	m_LastSceneSaveTime = 0;

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	fs.CreateSingleDirectory("Replays\\cutsceneReplays\\");

	//	TO DO: Do platform-specific directory creation stuff here

	m_SaveFilename = Time::FormatDate_TimeString(Time::C_DecomposedTime(Time::GetCurrentTime())) + ".amcs";
	m_pSaveFile = fs.Open("Replays\\cutsceneReplays\\" + m_SaveFilename, Sys::E_FileAccess_Append);
	if (m_pSaveFile == 0)
		return E_ReplayReturnCode_Failed;

	Sys::BinarySerialiserAutoEndian out(*m_pSaveFile);

	//	Make sure the edited replay-cutscene we're saving out has the latest version number,
	//	but keep the version # we loaded in since we'll do different stuff when loading scenes depending on it.
	if(m_Header->m_Version != replaySaveVersion)
	{
		U8 loadVersion = m_Header->m_Version;
		m_Header->m_Version = replaySaveVersion;
		m_Header->Serialise(out);
		m_Header->m_Version = loadVersion;
	}
	else
		m_Header->Serialise(out);
	return E_ReplayReturnCode_Success;
}

E_ReplayReturnCode C_ReplaySaveManager::SaveAndLoadIfItsTime(U32 p_ReplayTime)
{
	if(m_pSaveAndLoadThread->IsActive())
		return E_ReplayReturnCode_Skip;

	//	Did the last runthrough of the loading thread find the end of the replay? If so just return
	if(m_LoadedAllScenes)
	{
		return E_ReplayReturnCode_Skip;
	}
	if(p_ReplayTime >= m_NextSceneLoadTime)
	{
		//	Grab a position state for every vehicle before we dump the scene
		C_ReplayDirector::Instance().GrabStatesAllVehicles(p_ReplayTime);
		C_ReplayDirector::Instance().SwapBuffers();
		if(m_NumScenesLoaded >= m_LoadFooter->m_NumScenes)
		{
			m_LoadedAllScenes = true;
		}
		m_pSaveAndLoadThread->Enter(p_ReplayTime);
		return E_ReplayReturnCode_Success;
	}
	return E_ReplayReturnCode_Skip;
}

E_ReplayReturnCode C_ReplaySaveManager::FinishCutsceneEdit(U32 p_ReplayTime)
{
	m_pSaveAndLoadThread->Join();

	//	Grab a final vehicle position state for every vehicle
	C_ReplayDirector::Instance().GrabStatesAllVehicles(p_ReplayTime);

	//	Swap the buffers to grab the final scene data
	C_ReplayDirector::Instance().SwapBuffers();

	//	Write the last scene
	WriteNewReplayScene(p_ReplayTime);

	Sys::BinarySerialiserAutoEndian serial(*m_pSaveFile);

	//	Set and serialise the ReplayFooter stuff
	m_SaveFooter->m_EndOfReplayTime = m_LastSceneSaveTime;
	m_SaveFooter->m_CompletedMarker = replayCompletedMagicNumber;	//	Magic number we check for when loading in a replay file - if it doesn't match this then the replay wasn't cleanly finished
	m_SaveFooter->Serialise(serial);

	//	Flush the remaining serialisation data to ensure everything gets written before we delete the file handle
	m_pSaveFile->Flush();

	//	Now it's time for the workaround to renaming this new edited file to the old name (for version control)
	SAFE_DELETE_PTR(m_pLoadFile);
	SAFE_DELETE_PTR(m_pSaveFile);

	//	TO DO: Replace this messy #ifdef with a proper portable way of doing this. At the very least find out how we do this
	//	on PS3 and make the PS3 version of ReplaySaveManager.cpp
	//	Or not bother...as this isn't a retail feature...
#if defined( COMPILER_VC )

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
//	fs.DeleteFileA(m_LoadFilename);
	MoveFile("GAME:\\Replays\\cutsceneReplays\\" + m_SaveFilename, m_LoadFilename);
#endif

	return E_ReplayReturnCode_Success;
}



Game::E_ReplayReturnCode C_ReplaySaveManager::StartCutsceneLoad(Util::C_String p_Filename)
{
	Clear();
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	if(!fs.FileExists(p_Filename))
	{
		LOGERROR(gReplaySaveLogger, "Cutscene Replay file " + p_Filename + " doesn't appear to exist");
		return E_ReplayReturnCode_Failed;
	}
	m_LoadFilename = p_Filename;
	m_pLoadFile = fs.Open(m_LoadFilename, Sys::E_FileAccess_Read);
	if(m_pLoadFile == 0)
	{
		LOGERROR(gReplaySaveLogger, "Couldn't open cutscene replay file " + m_LoadFilename);
		return E_ReplayReturnCode_Failed;
	}

#if defined(PLATFORM_PC)
	//	IMPORTANT! This code assumes that ALL Amax cutscenes are recorded on Xenon, and so for PC must be Endian-swapped.
	//	This should be the only cross-platform use of replay files...
	m_pLoadFile->SetEndianSwapFlag(true);
#endif

	if(!m_Header)
	{
		m_Header = new C_ReplayHeader();
	}
	Sys::BinaryDeserialiserAutoEndian serial(*m_pLoadFile);

	serial >> m_Header->m_Version;
	m_Header->Deserialise(serial);

	BIZ_ASSERT_MSG_SPEED(m_Header->m_Version == replaySaveVersion, "Older replay save with version %d - current version is %d. Playback WILL continue but might well explode...", m_Header->m_Version, replaySaveVersion);

	//	Set the replay vehicle proxies with the correct vehicle indices based on the character XTIDs in the replay file.
	for(int i =0; i < m_Header->m_NumCars; i++)
	{
		S8 vehicleIndex = -1;
		vehicleIndex = FindLocalIndexFromCharacterID(m_Header->m_CarInfo[i].m_CharacterID);
		if(vehicleIndex == -1)
		{
			LOGERROR(gReplaySaveLogger, "Couldn't find a vehicle with the Sorcery Character ID %d specified in the replay file, bailing out", m_Header->m_CarInfo[i].m_CharacterID);
			return E_ReplayReturnCode_Failed;
		}
		//	We've found the vehicle index from the level state that matches this character ID, so we set the replay proxy to use that index.
		C_ReplayDirector::Instance().GetVehicle(i)->SetVehicleIndex(vehicleIndex);
	}

	//	Go to the end of the file and then back the size of the ReplayFooter struct
	m_pLoadFile->SetPosition(m_pLoadFile->Length()-(sizeof(C_ReplayFooter)));
	m_LoadFooter->Deserialise(serial);
	if((m_LoadFooter->m_EndOfReplayTime <= 0) || (m_LoadFooter->m_NumScenes <= 0) || (m_LoadFooter->m_CompletedMarker != replayCompletedMagicNumber))
	{
		LOGERROR(gReplaySaveLogger, "ReplayFooter data is not within expected parameters - file was not cleanly finished");
		return E_ReplayReturnCode_Failed;
	}
	//	Go to the first scene in the replay
	GoToScene(0);

	return E_ReplayReturnCode_Success;
}


void C_ReplaySaveManager::EndPlayback()
{
	m_pLoadingThread->Join();
	return;
}

/*-----------------------------------------	VEHICLE SAVING METHODS	---------------------------------------*/

void C_ReplaySaveManager::FillVehicleHeader(U8 p_VehicleIndex, U32 p_ReplayTime)
{
	Game::C_LevelState* levelState = &C_Game::GetInstance()->GetLevelState();

	if(!m_VHeader)
		m_VHeader = new C_ReplayVehicleHeader();
	m_VHeader->m_VehicleClass = 0;	//	TO DO: Implement storing the time attack vehicle class once this is built into the game
	m_VHeader->m_Padding1 = 0;
	m_VHeader->m_Padding2 = 0;
	m_VHeader->m_Padding3 = 0;

	m_VHeader->m_RouteNumber = levelState->mRouteNumber;
	m_VHeader->m_ReplayTimeLength = p_ReplayTime;
	m_VHeader->m_CarID = levelState->mCars[p_VehicleIndex].mCarID;
	m_VHeader->m_CharacterID = levelState->mCars[p_VehicleIndex].mCharacterID;
}


E_ReplayReturnCode C_ReplaySaveManager::SaveVehicleToFile(C_ReplayVehicleManager* p_pVehicle)
{
	if(!p_pVehicle)
	{
		return E_ReplayReturnCode_Failed;
	}

	FillVehicleHeader(p_pVehicle->GetIndex(), p_pVehicle->GetReplayLength());

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	fs.CreateSingleDirectory("ghosts\\");

	Util::C_String vehicleSaveFilename = Time::FormatDate_TimeString(Time::C_DecomposedTime(Time::GetCurrentTime())) + ".amgh";

	Sys::I_File* pVehicleSaveFile = fs.Open("ghosts\\" + vehicleSaveFilename, Sys::E_FileAccess_Write);

	if(pVehicleSaveFile == 0)
		return E_ReplayReturnCode_Failed;

	Sys::BinarySerialiserAutoEndian out(*pVehicleSaveFile);

	//	Write the vehicle header to the file
	m_VHeader->Serialise(out);

	//	Write vehicle state chunk size and contents
	U32 stateSize = p_pVehicle->GetDiskVehicleRec()->GetUsedSize();
	stateSize = FindNextPacketOfType(E_ReplayPacketType_Unknown, p_pVehicle->GetDiskVehicleRec(), p_pVehicle->GetDiskVehicleRec()->GetStart()) - p_pVehicle->GetDiskVehicleRec()->GetStart();
	out << stateSize;
	pVehicleSaveFile->Write(p_pVehicle->GetDiskVehicleRec()->GetStart(), stateSize);

	//	Write vehicle controls chunk size and contents
	U32 controlSize = p_pVehicle->GetDiskControlsRec()->GetUsedSize();
	controlSize = FindNextPacketOfType(E_ReplayPacketType_Unknown, p_pVehicle->GetDiskControlsRec(), p_pVehicle->GetDiskControlsRec()->GetStart()) - p_pVehicle->GetDiskControlsRec()->GetStart();
	out << controlSize;
	pVehicleSaveFile->Write(p_pVehicle->GetDiskControlsRec()->GetStart(), controlSize);

	//	The whole ghost data (for the single lap) is saved all at once, so we don't need to add any post-save data
	//	like we do with saving a race replay.

	SAFE_DELETE_PTR(pVehicleSaveFile);

	return E_ReplayReturnCode_Success;
}





/*------------------------------------------	NAVIGATION METHODS	---------------------------------------*/

//	Get the number of the scene we're currently playing
U32 C_ReplaySaveManager::GetCurrentSceneNumber() const
{
	S32 currentScene = 0;
	if(m_NumScenesLoaded == (m_LoadFooter->m_NumScenes) && (m_LoadedAllScenes == true))
		currentScene = m_NumScenesLoaded - 1;	//	We count the scenes from #0
	else
		currentScene = m_NumScenesLoaded - 2;	//	-1 since we load a scene ahead, and -1 again since we count from scene #0
	if(currentScene < 0)
		currentScene = 0;
	return currentScene;
}

//	Skip the next scene of the replay
E_ReplayReturnCode C_ReplaySaveManager::SkipScene()
{
	Sys::BinaryDeserialiserAutoEndian serial(*m_pLoadFile);

	U64 currentPos = m_pLoadFile->GetPosition();

	//	Check we're not actually reading the footer - highly unlikely that this could happen but better safe than sorry!
	if(m_pLoadFile->GetPosition() == (m_pLoadFile->Length() - sizeof(C_ReplayFooter)))
	{
		return E_ReplayReturnCode_Failed;
	}

	U32 sceneSize;

	serial >> sceneSize;

	U64 nextPos = m_pLoadFile->GetPosition();

	//	Check that this isn't the last scene in the file (don't let people skip this scene or there'll be nothing to load!)
	if((m_pLoadFile->GetPosition() + sceneSize) == (m_pLoadFile->Length() - sizeof(C_ReplayFooter)))
	{
		m_pLoadFile->SetPosition(currentPos);	//	Go back to before the scene size so LoadNewReplayScene will read it correctly
		return E_ReplayReturnCode_Failed;
	}

	m_pLoadFile->SetPosition(nextPos + sceneSize);

	m_NumScenesLoaded++;

	return E_ReplayReturnCode_Success;
}

U32 C_ReplaySaveManager::CheckTimestamp()
{
	U32 result = 0;
	U32 sceneSize = 0;
	U64 currentPos = m_pLoadFile->GetPosition();
	Sys::BinaryDeserialiserAutoEndian serial(*m_pLoadFile);
	serial >> sceneSize;
	serial >> result;
	m_pLoadFile->SetPosition(currentPos);
	return result;
}

//	Jump to a given scene
E_ReplayReturnCode C_ReplaySaveManager::GoToScene(U32 p_SceneNum)
{
	if((p_SceneNum >= m_LoadFooter->m_NumScenes) || (p_SceneNum < 0))
	{
		//	If we tried to jump to a scene that doesn't exist, just go to the start of the replay
		p_SceneNum = 0;
	}
	C_ReplayDirector::Instance().WipeDiskBuffers();

	//m_pLoadFile->SetPosition(sizeof(C_ReplayHeader));	//	Return to the end of the ReplayHeader at the beginning of the file
	U64 headerSize = m_Header->GetHeaderSize();
	m_pLoadFile->SetPosition(headerSize);	//	Return to the end of the ReplayHeader at the beginning of the file
	m_NumScenesLoaded = 0;
	for(U32 i=0; i< p_SceneNum; i++)
	{
		SkipScene();
	}

	m_LoadedAllScenes = false;

	//	We should now be at the start of the scene we want to play from, so we (mostly) repeat
	//	the steps carried out when loading the first scene in a replay

	if(!LoadNewReplayScene())
	{
		return E_ReplayReturnCode_Failed;
	}

	//	Set the replay time here, but we'll (hopefully) get it from a Master packet anyway later this frame
	C_ReplayDirector::Instance().SetScaledReplayTime(m_NextSceneLoadTime);

	C_ReplayDirector::Instance().SwapBuffers();

	if(m_NumScenesLoaded >= m_LoadFooter->m_NumScenes)
	{
		m_LoadedAllScenes = true;
	}

	//	Load in the next scene so we can swap it into play and be happy at the end of the scene we've jumped to
	LoadNewReplayScene();

	return E_ReplayReturnCode_Success;
}

E_ReplayReturnCode C_ReplaySaveManager::GoToTime(U32 p_ReplayTime)
{
	m_NumScenesLoaded = 0;
	//m_pLoadFile->SetPosition(sizeof(C_ReplayHeader));	//	Return to the end of the ReplayHeader at the beginning of the file
	U64 headerSize = m_Header->GetHeaderSize();
	m_pLoadFile->SetPosition(headerSize);	//	Return to the end of the ReplayHeader at the beginning of the file
	U64 lastScenePosition = m_pLoadFile->GetPosition();
	//	Allow for a single-scene replay/cutscene
	if(m_LoadFooter->m_NumScenes > 1)
	{
		U32 checkResult = 0;
		do
		{
			lastScenePosition = m_pLoadFile->GetPosition();
			SkipScene();
			checkResult = CheckTimestamp();
		}while(checkResult < p_ReplayTime);
		if(m_NumScenesLoaded > 0)
			m_NumScenesLoaded--;	//	SkipScene() would have increased this once more than we want, as we've actually gone back one scene 

	}

	//	At this point, lastScenePosition should point to the start of the scene the given time is part of
	m_pLoadFile->SetPosition(lastScenePosition);
	
	C_ReplayDirector::Instance().WipeDiskBuffers();

	m_LoadedAllScenes = false;

	if(!LoadNewReplayScene())
	{
		return E_ReplayReturnCode_Failed;
	}

	C_ReplayDirector::Instance().SwapBuffers();

	if(m_NumScenesLoaded >= m_LoadFooter->m_NumScenes)
	{
		m_LoadedAllScenes = true;
	}

	//	Load in the next scene so we can swap it into play and be happy at the end of the scene we've jumped to
	LoadNewReplayScene();

	return E_ReplayReturnCode_Success;
}




#ifndef CFG_GOLD
/*-----------------------------------	ENDIAN-CONVERTING HACKERY METHODS	-------------------------------*/

struct EndianFlip
{
	E_ReplayPacketType type;
	const char* description;
	int size;
};

struct EndianMasterFlip
{
	E_ReplayPacketMaster type;
	const char* description;
	int size;
};

EndianFlip EndianFlipData[] =
{
	{ E_ReplayPacketType_Unknown,			"",			0 },
	{ E_ReplayPacketType_VehicleState,		"QQQ",		sizeof(RacingPhysics::Vehicle::CompressedNetworkState)},
	{ E_ReplayPacketType_Controls,          "D",		sizeof(RacingPhysics::Vehicle::CompressedNetworkControls) },
	{ E_ReplayPacketType_PerkFired,			"DBBBB",	sizeof(E_PICKUP_TYPE) + 4 },
	{ E_ReplayPacketType_PerkHit,			"DBBBB",	sizeof(E_PICKUP_TYPE) + 4 },
	{ E_ReplayPacketType_PhotoOp,			"",			0 },
	{ E_ReplayPacketType_Master,			"D",		sizeof(E_ReplayPacketMaster) },

};

EndianMasterFlip EndianMasterFlipData[] =
{
	{ E_ReplayPacketMaster_VehicleStates,	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ",	MAX_VEHICLES * sizeof(RacingPhysics::Vehicle::CompressedNetworkState) },
	{ E_ReplayPacketMaster_VehicleDamage,	"BBBBDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD", 4 + sizeof(Vehicle::C_DamageState) },
	{ E_ReplayPacketMaster_PickupOnTrack,	"DD", 2 * sizeof(U32) },
};

Game::E_ReplayReturnCode C_ReplaySaveManager::ConvertBigEndianCutsceneReplayToLittleEndian( Util::C_String p_Filename )
{
	//	SETUP
	Clear();
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	if(!fs.FileExists(p_Filename))
	{
		LOGERROR(gReplaySaveLogger, "Replay file " + p_Filename + " doesn't appear to exist");
		return E_ReplayReturnCode_Failed;
	}
	m_LoadFilename = p_Filename;
	m_pLoadFile = fs.Open(m_LoadFilename, Sys::E_FileAccess_Read);
	if(m_pLoadFile == 0)
	{
		LOGERROR(gReplaySaveLogger, "Couldn't open replay file " + m_LoadFilename);
		return E_ReplayReturnCode_Failed;
	}

	m_SaveFilename = Time::FormatDate_TimeString(Time::C_DecomposedTime(Time::GetCurrentTime())) + ".amcs";
	m_pSaveFile = fs.Open("Replays\\cutsceneReplays\\" + m_SaveFilename, Sys::E_FileAccess_Write);
	if (m_pSaveFile == 0)
		return E_ReplayReturnCode_Failed;

	//	Set the EndianSwap flag so that stuff we serialize normally will already be E-swapped.
	m_pLoadFile->SetEndianSwapFlag(true);

	if(!m_Header)
	{
		m_Header = new C_ReplayHeader();
	}

	Sys::BinaryDeserialiserAutoEndian loadSerial(*m_pLoadFile);
	Sys::BinarySerialiserAutoEndian saveSerial(*m_pSaveFile);


	//	GRABBING THE HEADER
	loadSerial >> m_Header->m_Version;
	m_Header->Deserialise(loadSerial);

	//	GRABBING THE FOOTER
	//	Go to the end of the file and then back the size of the ReplayFooter struct
	m_pLoadFile->SetPosition(m_pLoadFile->Length()-(sizeof(C_ReplayFooter)));
	m_LoadFooter->Deserialise(loadSerial);
	if((m_LoadFooter->m_EndOfReplayTime <= 0) || (m_LoadFooter->m_NumScenes <= 0) || (m_LoadFooter->m_CompletedMarker != replayCompletedMagicNumber))
	{
		LOGERROR(gReplaySaveLogger, "ReplayFooter data is not within expected parameters - file was not cleanly finished");
		return E_ReplayReturnCode_Failed;
	}

	//	Write out header to new converted file
	m_Header->Serialise(saveSerial);


	U64 headerSize = m_Header->GetHeaderSize();
	m_pLoadFile->SetPosition(headerSize);	//	Return to the end of the ReplayHeader at the beginning of the file

	//	DO THE MAGIC!

	for(U32 i = 0; i < m_LoadFooter->m_NumScenes; i++)
	{
		// load in scene as normal
		U32 sceneSize;

		loadSerial >> sceneSize;

		saveSerial << sceneSize;

		loadSerial >> m_NextSceneLoadTime;

		saveSerial << m_NextSceneLoadTime;

		for(int i=0; i < m_Header->m_NumCars; i++)
		{
			U32 stateSize;
			
			loadSerial >> stateSize;
			saveSerial << stateSize;

			//	READ
			//	Only resize memory chunks in buffers if they are too small, should cut down on NEWing
			if(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->GetSize() < stateSize)
				C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->Resize(stateSize);
			m_pLoadFile->Read(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->GetStart(), stateSize);

			//	CONVERT
			ConvertChunk(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->GetStart(), 
				C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->GetStart() + stateSize);

			//	WRITE
			m_pSaveFile->Write(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskVehicleRec()->GetStart(), stateSize);


			U32 controlSize;

			loadSerial >> controlSize;
			saveSerial << controlSize;

			//	READ
			//	Only resize memory chunks in buffers if they are too small, should cut down on NEWing
			if(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->GetSize() < controlSize)
				C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->Resize(controlSize);
			m_pLoadFile->Read(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->GetStart(), controlSize);

			//	CONVERT
			ConvertChunk(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->GetStart(), 
				C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->GetStart() + controlSize);

			//	WRITE
			m_pSaveFile->Write(C_ReplayDirector::Instance().GetVehicle(i)->GetDiskControlsRec()->GetStart(), controlSize);
		}

		U32 miscChunkSize;

		loadSerial >> miscChunkSize;
		saveSerial << miscChunkSize;

		//	READ
		if(C_ReplayDirector::Instance().GetDiskMiscRec()->GetSize() < miscChunkSize)
			C_ReplayDirector::Instance().GetDiskMiscRec()->Resize(miscChunkSize);
		m_pLoadFile->Read(C_ReplayDirector::Instance().GetDiskMiscRec()->GetStart(), miscChunkSize);

		//	CONVERT
		ConvertChunk(C_ReplayDirector::Instance().GetDiskMiscRec()->GetStart(), 
			C_ReplayDirector::Instance().GetDiskMiscRec()->GetStart() + miscChunkSize);

		//	WRITE
		m_pSaveFile->Write(C_ReplayDirector::Instance().GetDiskMiscRec()->GetStart(), miscChunkSize);


		U32 masterChunkSize;

		loadSerial >> masterChunkSize;
		saveSerial << masterChunkSize;

		//	READ
		if(C_ReplayDirector::Instance().GetDiskMasterRec()->GetSize() < masterChunkSize)
			C_ReplayDirector::Instance().GetDiskMasterRec()->Resize(masterChunkSize);
		m_pLoadFile->Read(C_ReplayDirector::Instance().GetDiskMasterRec()->GetStart(), masterChunkSize);

		//	CONVERT
		ConvertChunk(C_ReplayDirector::Instance().GetDiskMasterRec()->GetStart(),
			C_ReplayDirector::Instance().GetDiskMasterRec()->GetStart() + masterChunkSize);

		//	WRITE
		m_pSaveFile->Write(C_ReplayDirector::Instance().GetDiskMasterRec()->GetStart(), masterChunkSize);
	}
	
	
	//	Serialise the now-Endian-swapped LOAD footer
	m_LoadFooter->Serialise(saveSerial);

	m_pSaveFile->Flush();

	//	Delete the file pointers to close the files :)
	SAFE_DELETE_PTR(m_pLoadFile);
	SAFE_DELETE_PTR(m_pSaveFile);


	return E_ReplayReturnCode_Success;
}

void C_ReplaySaveManager::ConvertChunk( U8* mem, U8* end )
{
	while(mem < end)
	{
		Ut::DoEndianSwap::EndianSwap(*(U32*)mem);

		C_ReplayPacketBase* basePacket = ((C_ReplayPacketBase*)(mem));

		E_ReplayPacketType type = basePacket->GetType();

		BIZ_ASSERT_MSG(type != E_ReplayPacketType_Unknown, "Unknown replay packet type");

		mem += sizeof(U32);

		for(int i = 0; i < sizeof(EndianFlipData)/sizeof(EndianFlip); i++)
		{
			if (EndianFlipData[i].type == type)
			{
				DoSwaps(mem, EndianFlipData[i].description);
				if(type == E_ReplayPacketType_Master)
				{
					E_ReplayPacketMaster masterType = (E_ReplayPacketMaster)(*mem);
					
					BIZ_ASSERT_MSG(masterType != E_ReplayPacketMaster_VehicleStates, "Master_VehicleStates packet found");
					
					U32 size = EndianFlipData[i].size;
					mem += size;
					for(int j = 0; j < sizeof(EndianMasterFlipData)/ sizeof(EndianMasterFlip); j++)
					{
						if(EndianMasterFlipData[j].type == masterType)
						{
							DoSwaps(mem, EndianMasterFlipData[j].description);
							U32 sizeM = EndianMasterFlipData[j].size;
							mem += sizeM;
							break;
						}
					}
				}
				else
				{
					mem += EndianFlipData[i].size;
				}
				break;
			}

			BIZ_ASSERT_MSG(i != (sizeof(EndianFlipData)/sizeof(EndianFlip) - 1), "Couldn't convert this replay packet to little Endian - grab GavinY");
		}
	}
}

void C_ReplaySaveManager::DoSwaps( U8* address, const char* desc )
{
	while(*desc != '\0')
	{
		switch(*desc)
		{
		case 'B': address += 1; break;
		case 'S': Ut::DoEndianSwap::EndianSwap(*(U16*)address); address += 2; break;
		case 'D': Ut::DoEndianSwap::EndianSwap(*(U32*)address); address += 4; break;
		case 'Q': Ut::DoEndianSwap::EndianSwap(*(U64*)address); address += 8; break;
		}
		desc++;
	}
}

#endif


} // namespace Game
