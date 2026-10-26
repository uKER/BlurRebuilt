#ifndef REPLAY_MANAGER_H
#define REPLAY_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/Messaging.h>
#include <GameLogic/Replays/ReplayMessages.h>

#include <GameLogic/Structures/LevelState.h>

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <GameLogic\Replays\ReplayTypes.h>
#include <GameLogic/Replays/ReplayUtilities.h>
#include <GameLogic\Pickups\PickupsManager.h>

#include <misc_shared\Messaging\MessageMultiHandler.h>
#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>

#include <system\artery\Artery.h>
#include <system\serialise\I_File.h>

namespace Arch
{
	class BinaryOutputArchive;
}

namespace Game
{
	class C_ReplayBuffer;
	class C_ReplayVehicleManager;
	class C_ReplaySaveManager;
	class C_MemChunk;
}

namespace Game
{

/* 
===============================================================
					class C_ReplayDirector

NOTES: Singleton class that handles and directs all replay recording and playback
===============================================================
*/
class C_ReplayDirector : public Messaging::C_MessageMultiHandler
{
public:

	C_ReplayDirector();
	~C_ReplayDirector();

	//	Singleton accessor method
	static C_ReplayDirector&					Instance() {return s_Instance;};

	//	Gets new RLC and acquires vehicle handles etc for a new race
	void										RaceLoaded();

	void										RaceStarted();

	void										GetRaceHandles(bool p_ResetIndices = true);

	bool										IsEnabled() { return m_ReplayMode != E_ReplayMode_Disabled; }

	//	Set up game-wide message subscriptions etc
	void										Init();

	void										StartRecording();

	void										StopRecording();

	void										Pause();
	void										Resume();

	//	Reset vector iterators to the beginning of the recording
	void										ResetPlayback();

	//	Delete all recording data to prepare for new race
	void										EraseRecording();

	//	Initialise race-specific message subscriptions etc
	void										GameInit(Messaging::C_MessageSubscriberSystem *actor_subscriber_system);

	//	Unsubscribes from game-specific messages when GameWorld is destroyed after race
	void										GameEnd();

	//	Called from destructor - tidies up after class
	void										Destroy();

	//	Updates the replay system
	void										Update(float p_TimestepIn);

	//	Update called within RacebookLayer's ::GlobalUpdate() method - basically just used for checking if the player
	//	has un-paused the replay
	void										GlobalUpdate();

	E_ReplayMode								GetReplayMode() const {return m_ReplayMode;}

	//	Gets the current replay mode of the specified vehicle
	E_ReplayMode								GetVehicleReplayMode(U8 p_Index);

	//	Sets the replay mode of the specified vehicle
	void										SetVehicleReplayMode(U8 p_Index, E_ReplayMode p_Mode);

	//	Check whether it's time to grab an updated state for each vehicle, and also grab controls for each vehicle (regardless of time)
	void										RecordStatesAndControls(U32 p_ReplayTime);

	//	Record the Master states and stick them in the Master packet stream
	void										RecordMasterPackets(U32 p_ReplayTime);

	//	Grab the current position state for all vehicles - regardless of whether it's "time"
	void										GrabStatesAllVehicles(U32 p_ReplayTime);

	void										AddPickupFired(PickupCreationStruct pickupInfo);
	void										AddPickupPostDelay(U32 pickupUid, Maths::HMat43 targetMatrix, float fVal);
	void										AddPickupHit(U8 p_HitIndex, E_PICKUP_TYPE pickupType);
	void										AddShieldedPickup(U8 p_ShieldedIndex, E_PICKUP_TYPE pickupType);
	void										AddKillPickup(U32 pickupUid);
	void										AddPickupStore(U8 index);
	void										AddPickupAdded(U8 index, E_PICKUP_TYPE pickupType);

	//	Add a timestamp telling the system that there's an awesome photo op here.
	void										AddPhotoOpportunity(U32 p_ReplayTime);

	//	Playback the various miscellaneous race event packets stored in the misc stream
	void										PlaybackMiscPackets(U32 p_ReplayTime);

	//DEBUG
	void										ScanHDDForReplays();

	void										SwapBuffers();

	//	Clear all replay data in the various Memory buffers
	void										WipeMemoryBuffers();

	//	Clear all replay data in the various Disk buffers
	void										WipeDiskBuffers();

	void										SetReplayTime(float p_ReplayTime) {m_ReplayTime = p_ReplayTime;};

	void										SetScaledReplayTime(U32 p_ReplayTime);

	void										SetSaveFilename(Util::C_String p_Filename);

	//	Get a pointer to the RLC for this race
	E_ReplayReturnCode							AcquireRLC();	

	//	Return pointer to a specific vehicle manager
	C_ReplayVehicleManager*						GetVehicle(const int p_Index) const {return m_Vehicles[p_Index];};

	C_MemChunk*									GetMemoryMiscRec() const;

	C_MemChunk*									GetDiskMiscRec() const;

	C_MemChunk*									GetMemoryMasterRec() const;

	C_MemChunk*									GetDiskMasterRec() const;

	//	Quick public check for whether we've recorded anything
	bool										HasThereBeenARecording();

	bool										ShouldUpdate();

	// For game messages
	bool										MessageHandler(const Messaging::C_MessageBase* pMessage);

	bool										LoadReplayFromIndex(int p_Index);

	bool										LoadCutsceneForEditing(int p_Index);

	int											GetNumScannedReplays() const {return m_ScannedReplayNames.Size();};

	S8											FindReplayIndexFromVehicleIndex(S8 p_VehicleIndex) const;

	U32											GetScaledReplayTime() const;

	float										GetReplayTime() const {return m_ReplayTime;};

	Btl::String64*								GetScannedReplayStringFromIndex(int i);

	C_ReplaySaveManager*						GetSaveManager() const {return m_pSaveManager;};

	/*----------------------------------------------	NAVIGATION	-------------------------------------------*/
	
	E_ReplayReturnCode							TrackXScenes(S32 p_TrackNum);
	E_ReplayReturnCode							GoToTime(float p_ReplayTime);

	/*------------------------------------------	TIME ATTACK / GHOSTS	-----------------------------------*/

	//E_ReplayReturnCode							TimeAttackSetup();
	void										UpdateSessionGhost();
	//void										TimeAttackNewLap();
	//void										ScanHDDForGhosts();


	/*----------------------------------------------	CUTSCENE CONTROL	-----------------------------------*/

	//bool										ConvertBigEndianCutsceneReplayToLittleEndian(int p_Index);

	void										StartRecordingCsReplay(void);
	void										LoadCsReplayFile(Btl::String32 p_Filename);
	void										PlayCsReplay(F32 p_StartTime);
	void										StopCsReplay();
	void										PauseCsReplay();
	void										ResumeCsReplay();


private:
	
	static C_ReplayDirector						s_Instance;

	void										Clear();

	//	Sets initial vehicle positions into:
	//	Level state IF p_IntoLevelState = true
	//	Heavy-handed ResetAtWorldPosition IF p_IntoLevelState = false
//	void										SetInitialVehiclePositions(bool p_IntoLevelState);

	bool										ReachedEndOfReplay();

	bool										IsItTimeToLoop();

	E_ReplayReturnCode							SaveVehicleRecording(C_ReplayVehicleManager* p_pVehicle);

	/*----------------------------------------------	NAVIGATION	-------------------------------------------*/

	E_ReplayReturnCode							GoToScene(U32 p_SceneNum);
	E_ReplayReturnCode							GoToScaledTime(U32 p_ReplayTime);

	/*------------------------------------------	MISC PACKET PROCESSING	-----------------------------------*/

	E_ReplayReturnCode							ProcessMasterPackets(bool p_OnlyProcessInitialPositions = false);

	E_ReplayReturnCode							ProcessMasterVehicleStates(U8* p_pPacketAddress);
	E_ReplayReturnCode							ProcessMasterVehicleDamage(U8* p_pPacketAddress);
	E_ReplayReturnCode							ProcessMasterPickupsOnTrack(U8* p_pPacketAddress);

	E_ReplayReturnCode							ProcessPickupPostDelay(U32 p_ReplayTime);
	E_ReplayReturnCode							ProcessKillPickup(U32 p_ReplayTime);
	E_ReplayReturnCode							ProcessPhotoOp(U32 p_ReplayTime);

	Messaging::C_MessageSubscriberSystem*		m_ActorSubscriberSystem;

	Game::C_LevelState*							m_pLevelState;

	Game::Actors::RaceLogicCore*				m_pRLC;								//	Pointer to RLC, must be reacquired each race

	int											m_NumVehicles;						//	No. of vehicles in race (stored to save querying RLC every time)
	
	float										m_PauseTrackTimer;					//	Timer countdown activated when playback is skipped while paused
	float										m_ReplayTime;						//	Counter of elapsed time of the replay, used in both recording and playback

	//	Vector holding a vehicle manager object for each individual vehicle in the race (holds recordings)
	Btl::Vector<C_ReplayVehicleManager*, Btl::VectorParameters_Array<MAX_VEHICLES> > m_Vehicles;

	Btl::Vector<Btl::String64>					m_ScannedReplayNames;

	Btl::Vector<Btl::String64>					m_ScannedGhostNames;

public:
	E_ReplayMode								m_ReplayMode;						//	All-conquering mode for replay system (copied into LevelState at start of race)

private:
	C_MemChunk*									m_MiscPacketBuffers[2];				//	Memory chunks to hold misc. replay packet data (non vehicle-specific)

	C_MemChunk*									m_MasterPacketBuffers[2];			//	Memory chunks to store Master Packets

	bool										m_FirstBufferIsMemory;				//	Flag denoting whether the first of the two buffers is for writing to memory or saving to disk
	bool										m_FirstFrame;						//	Are we recording the first frame of a replay? If so, might need to do some special stuff
	bool										m_Loop;								//	Should the Director loop the replay one playback reaches the end?
	bool										m_Paused;
	bool										m_ItsACutscene;						//	Flag for if we're doing cutscene stuff just now - so don't do pickups/damage stuff.
	bool										m_CutscenePaused;					//	Special case pause for when we only want to freeze the vehicles, not the whole game world.
	U8											m_CutsceneResumeDelay;				//	Stupid ridiculous workaround for the fact that Activate() on bodies is deferred but MakeStatic() is not!

	C_ReplaySaveManager*						m_pSaveManager;




	/*------------------------------------------	DEBUG REPLAY TOOL ARTERY STUFF	-----------------------------------*/

public:
	//	Used by replay  tool to load a replay file - filename in m_DebugLoadFilename
	bool										Debug_LoadReplay();

	ArteryConnection							m_ReplayToolCon;
	Util::C_String								m_DebugLoadFilename;

private:
	bool										Debug_InitialiseArteryConnection();
	void										Debug_ProcessArteryMessages();
	void										Debug_SendMessageToTool(ArteryMessage &m);
	void										Debug_SendMessageToTool(const char * p_string);
	void										Debug_SendConstantReplayInfo();


	/*-------------------------------	REPLAY VEHICLE WORLD MATRIX EXPORT...THING. STUFF.	---------------------------*/

private:
	bool										Export_StartNewFile();
	void										Export_WriteVehiclePositions(F32 p_DeltaTime);
	void										Export_FinishFile();

	Sys::I_File*								m_pExportSaveFile;
	Sys::C_FileBlock*							m_pExportHeaderFileBlock;
	Sys::C_FileBlock*							m_pExportSamplesFileBlock;
	Arch::BinaryOutputArchive*					m_pExportSamplesArchive;
	Sys::C_StreamPos							m_ExportSamplesBlockStartPos;

	U32											m_ExportNumSamples;
public:
	Util::C_String								m_ExportSaveFilename;
};

} // namespace Game

#endif // REPLAY_MANAGER_H
