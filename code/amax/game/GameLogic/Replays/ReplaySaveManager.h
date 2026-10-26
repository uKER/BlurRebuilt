#ifndef REPLAY_SAVE_H
#define REPLAY_SAVE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include <Core\containers\String.h>

#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>

#include <GameLogic/Replays/ReplayTypes.h>
#include <GameLogic\Replays\ReplayHeadersAndFooters.h>
#include <GameLogic\Replays\ReplayVehicleManager.h>

namespace Game
{
	class C_MemChunk;
	class C_ReplaySavingThread;
	class C_ReplayLoadingThread;	
	class C_ReplaySaveAndLoadThread;
}

namespace Game
{

class C_ReplaySaveManager
{
public:
	C_ReplaySaveManager();
	~C_ReplaySaveManager() {Destroy();};

	void										Clear();
	void										Destroy();

	void										SetSceneLength(float p_LengthSeconds);
	void										SetSceneLengthScaled(U32 p_LengthScaled);

	E_ReplayReturnCode							StartNewReplay();
	E_ReplayReturnCode							StartLoad(Util::C_String p_Filename);
	
	//	Methods for starting using Amax cutscenes in various modes
	E_ReplayReturnCode							StartNewCutscene();
	E_ReplayReturnCode							StartCutsceneEdit(Util::C_String p_Filename);
	E_ReplayReturnCode							StartCutsceneLoad(Util::C_String p_Filename);

	E_ReplayReturnCode							WriteNewReplayScene(U32 p_NewSceneSaveTime);
	E_ReplayReturnCode							LoadNewReplayScene();

	E_ReplayReturnCode							DumpSceneIfItsTime(U32 p_ReplayTime);
	E_ReplayReturnCode							LoadSceneIfItsTime(U32 p_ReplayTime);
	//	Saves and loads (for cutscene/replay editing)
	E_ReplayReturnCode							SaveAndLoadIfItsTime(U32 p_ReplayTime);

	//	Writes any remaining replay data to the save file, writes some end of file data and does some nice cleaning up
	E_ReplayReturnCode							FinishSaveFile(U32 p_ReplayTime);
	//	Does much the same as FinishSaveFile(), but also renames the new version of the file and deletes the old one etc
	E_ReplayReturnCode							FinishCutsceneEdit(U32 p_ReplayTime);
	//	Tidies up any threads etc when we're finished with playback
	void										EndPlayback();

	E_ReplayReturnCode							SaveVehicleToFile(C_ReplayVehicleManager* p_pVehicle);

	U8											GetReplayVersion() const { return m_Header->m_Version; }
	U32											GetEndReplayTime() const {return m_LoadFooter->m_EndOfReplayTime;};
	U32											GetNextSceneLoadTime() const { return m_NextSceneLoadTime; }
	U32											GetCurrentSceneNumber() const;
	U32											GetNumScenes() const { return m_LoadFooter->m_NumScenes; }
	Util::C_String								GetLoadFilename() const { return m_LoadFilename; }
	void										SetSaveFilename(Util::C_String p_Filename) { m_SaveFilename = p_Filename; }

	/*------------------------------------------	NAVIGATION METHODS	---------------------------------------*/

	E_ReplayReturnCode							GoToScene(U32 p_SceneNum);
	E_ReplayReturnCode							GoToTime(U32 p_ReplayTime);

	/*---------------------------------------------------------------------------------------------------------*/

#ifndef CFG_GOLD
	/*-----------------------------------	ENDIAN-CONVERTING HACKERY METHODS	-------------------------------*/

	E_ReplayReturnCode							ConvertBigEndianCutsceneReplayToLittleEndian(Util::C_String p_Filename);
	void										ConvertChunk(U8* mem, U8* end);
	void										DoSwaps(U8* address, const char* desc);

	/*---------------------------------------------------------------------------------------------------------*/
#endif


private:

	/*------------------------------------------	NAVIGATION METHODS	---------------------------------------*/

	E_ReplayReturnCode							SkipScene();
	//	Checks the time that the next scene begins at
	U32											CheckTimestamp();


	/*---------------------------------------------------------------------------------------------------------*/
	
	void										FillHeader();
	void										SetFromHeader();

	void										FillVehicleHeader(U8 p_VehicleIndex, U32 p_ReplayTime);
	void										SetFromVehicleHeader(U8 p_VehicleIndex);

	C_ReplayHeader*								m_Header;

	C_ReplayFooter*								m_SaveFooter;

	C_ReplayFooter*								m_LoadFooter;

	C_ReplayVehicleHeader*						m_VHeader;

	Util::C_String								m_SaveFilename;

	Util::C_String								m_LoadFilename;

	Sys::I_File*								m_pSaveFile;

	Sys::I_File*								m_pLoadFile;

	U32											m_LastSceneSaveTime;

	U32											m_NextSceneLoadTime;

	U32											m_NumScenesLoaded;

	U32											m_ReplaySceneLengthScaled;

	bool										m_LoadedAllScenes;

	C_ReplayLoadingThread*						m_pLoadingThread;

	C_ReplaySavingThread*						m_pSavingThread;

	C_ReplaySaveAndLoadThread*					m_pSaveAndLoadThread;
};

} // namspace Game



#endif	//	REPLAY_SAVE_H