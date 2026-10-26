#include "stdafx.h"

#include <GameLogic/GameLoggingManager.h>

#include <Game/Game.h>
#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic\ContentServer\RecordEventManager.h>

#include <Game\gameactors\vehiclelogic\VehicleLogicActor.h>

#include <GameLogic/Structures/LevelResult.h>
#include <GameLogic/Profile/GameProfileManager.h>

#include <Racebook\RaceBookManager.h>

#include <Ui\game\Multiplayer\MultiplayerCache.h>
#include <GameLogic\Structures\MpResults.h>

#include <system/serialise/C_FileSystem.h>
#include <system/serialise/TextSerialise.h>

#include <Ui\game\Multiplayer\MultiplayerDefines.h>
#include "bdLobby\bdLobbyCommon\bdAuthCommon.h"

Debug::Logger &g_DemonwareGameLogging( Debug::GetLogger("Demonware.GameLogging") );

namespace Game
{

/*
	These check that the data structures we upload are multiples of 4 bytes. This ensures that
	when stored in memory there's no risk of writing floats to addresses that span a cache line
	(data misalignment crash)
*/
BIZ_STATIC_ASSERT((sizeof(AmaxGameLogHeader) % 4) == 0 );
BIZ_STATIC_ASSERT((sizeof(AmaxGameVehicleLog) % 4) == 0 );

C_GameLoggingManager C_GameLoggingManager::s_instance;

void C_GameLoggingManager::DebugExportEnumNames()
{
}

void C_GameLoggingManager::UploadGameLog()
{
	//	TO DO: This method really needs to only upload data about local vehicles, because we don't possess very many stats for remote vehicles.
	const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	if ((levelState.mNumCars == 0) || (levelState.mCars[0].mController == INVALID_CONTROLLER))
		return;

	const Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();

	if (results.mFake)
		return;

	RaceBook::C_RaceBookProfile *rb_prof = RaceBook::C_RaceBookManager::Instance().GetPlayerProfile();
	C_GameProfile *game_prof=C_GameProfileManager::Instance().GetPrimaryProfile();

	Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();

	memset(&mGameLog, sizeof(AmaxGameLog), 0);

	//sort out the header

	mGameLog.mHeader.mVersion=AMAX_GAME_LOG_VERSION;
	LOGDEBUG(g_DemonwareGameLogging, "--- Starting Log Game (Version %d)", mGameLog.mHeader.mVersion);

	mGameLog.mHeader.mCommunityGroupID=UI::C_MultiplayerCache::Instance().GetCurrentEventID();
	LOGDEBUG(g_DemonwareGameLogging, "GameLog : Community Group ID %ld", mGameLog.mHeader.mCommunityGroupID);

	mGameLog.mHeader.mRaceMode=levelState.mRaceMode;
	LOGDEBUG(g_DemonwareGameLogging, "GameLog : RaceMode %d", mGameLog.mHeader.mRaceMode);
	
	mGameLog.mHeader.mCity=levelState.mCity;
	LOGDEBUG(g_DemonwareGameLogging, "GameLog : City %d", mGameLog.mHeader.mCity);

	mGameLog.mHeader.mRoute=levelState.mRoute;
	LOGDEBUG(g_DemonwareGameLogging, "GameLog : Route %d", mGameLog.mHeader.mRoute);

	mGameLog.mHeader.mNumCars=levelState.mNumCars;
	LOGDEBUG(g_DemonwareGameLogging, "GameLog : Num Cars %d", mGameLog.mHeader.mNumCars);

	mGameLog.mHeader.mLaps=levelState.mLaps;
	LOGDEBUG(g_DemonwareGameLogging, "GameLog : Laps %d", mGameLog.mHeader.mLaps);
	
	mGameLog.mHeader.mTimeLimit=levelState.mTimeLimit;
	LOGDEBUG(g_DemonwareGameLogging, "GameLog : Time Limit %d", mGameLog.mHeader.mTimeLimit);

	for(U32 a=0;a<mGameLog.mHeader.mNumCars;a++)
	{
		NetVehicleData result;
		C_MpResults::Instance().GetPlayerResults(a,result);

		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d", a);

		wcsncpy(mGameLog.mVehicles[a].mCharacterString,levelState.mCars[a].mNameAboveCar,32);
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Name %S", a, mGameLog.mVehicles[a].mCharacterString);

#ifndef PLATFORM_PS3
		mGameLog.mVehicles[a].mSystemID=levelState.mCars[a].mSystemPlayerId.m_Uid;
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : System ID %ld", a, mGameLog.mVehicles[a].mSystemID);
#else
		mGameLog.mVehicles[a].mSystemID=bdAuthUtility::getUserID(levelState.mCars[a].mSystemPlayerId.m_Uid.handle.data);
#endif

		mGameLog.mVehicles[a].mHealthRemaining=result.m_Statistics[UI::E_MultiplayerStat_HealthRemaining];
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Health Left %0.2f", a, mGameLog.mVehicles[a].mHealthRemaining);

		mGameLog.mVehicles[a].mDistanceTravelled=result.m_Statistics[UI::E_MultiplayerStat_DistanceTravelled];
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Distance Travelled %0.2f", a, mGameLog.mVehicles[a].mDistanceTravelled);

		if(levelState.mCars[a].mLoadOuts.Size() == 3)
		{
			mGameLog.mVehicles[a].mLoadOut1=levelState.mCars[a].mLoadOuts[0];
			LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Loadout 1 %d", a, mGameLog.mVehicles[a].mLoadOut1);
			mGameLog.mVehicles[a].mLoadOut2=levelState.mCars[a].mLoadOuts[1];
			LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Loadout 2 %d", a, mGameLog.mVehicles[a].mLoadOut2);
			mGameLog.mVehicles[a].mLoadOut3=levelState.mCars[a].mLoadOuts[2];
			LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Loadout 3 %d", a, mGameLog.mVehicles[a].mLoadOut3);
		}
		else
		{
			mGameLog.mVehicles[a].mLoadOut1=-1;
			mGameLog.mVehicles[a].mLoadOut2=-1;
			mGameLog.mVehicles[a].mLoadOut3=-1;
		}

		mGameLog.mVehicles[a].mRank=levelState.mCars[a].mMultiplayerRank;
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Rank %d", a, mGameLog.mVehicles[a].mRank);

		mGameLog.mVehicles[a].mLegendLevel=levelState.mCars[a].mMultiplayerLegendLevel;
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Legend Level %d", a, mGameLog.mVehicles[a].mLegendLevel);

		mGameLog.mVehicles[a].mCarID=levelState.mCars[a].mCarID;
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : CarID %d", a, mGameLog.mVehicles[a].mCarID);

		mGameLog.mVehicles[a].mManufacturerID=levelState.mCars[a].mManufacturerID;
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : ManufacturerID %d", a, mGameLog.mVehicles[a].mManufacturerID);

		mGameLog.mVehicles[a].mTotalFansGained=result.m_Statistics[UI::E_MultiplayerStat_Fans];
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Total Fans Gained %d", a, mGameLog.mVehicles[a].mTotalFansGained);

		mGameLog.mVehicles[a].mNumPowerups=E_PICKUP_TYPE_MAX;
		for(int b=0;b<E_PICKUP_TYPE_MAX;b++)
		{
			mGameLog.mVehicles[a].mPowerupsUsed[b]= 0;//results.mCars[a].mStatistics.mNumPerksFired[b];
			LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Power Up %d : Fired %d", a, b, mGameLog.mVehicles[a].mPowerupsUsed[b]);

			mGameLog.mVehicles[a].mPowerUpsHit[b]= 0;//results.mCars[a].mStatistics.mNumPerksHit[b];
			LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Power Up %d : Hit %d", a, b, mGameLog.mVehicles[a].mPowerUpsHit[b]);
		}

		mGameLog.mVehicles[a].mStartPosition=levelState.mCars[a].mStartPosition;
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Start Position %d", a, mGameLog.mVehicles[a].mStartPosition);

		mGameLog.mVehicles[a].mFinalPosition=result.m_Position;
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Final Position %d", a, mGameLog.mVehicles[a].mFinalPosition);

		mGameLog.mVehicles[a].mFinalState=result.m_State;
		LOGDEBUG(g_DemonwareGameLogging, "GameLog : Vehicle %d : Final State %d", a, mGameLog.mVehicles[a].mFinalState);
	}
	LOGDEBUG(g_DemonwareGameLogging, "--- Ending Log Game");

	C_RecordEventManager::Instance().WriteEventRecordBin(&mGameLog, sizeof(AmaxGameLog), EVENT_RECORD_CATEGORY_RACELOG);
}

}; // namespace Game
