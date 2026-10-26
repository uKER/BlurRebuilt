#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic/ScoreboardManager.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <GameLogic\Structures\MpResults.h>
#include <GameLogic/Profile/MultiplayerProfile.h>

#include <Network\Session.h>
#include <Network\ScoreboardWriter.h>
#include <Network\PlayerSkillWriter.h>
#include <Network\PlayerSkill.h>
#include <Network/Core/NetScoreboards.h>
#include <Network/Misc/NetDefines.h>
#include <Network/Core/NetServices.h>
#include <Network/Misc/NetDatagramPacketTypes.h>
#include <Network/Core/NetRace.h>
#include <Network/player/NetPlayerManager.h>

#include <misc_shared\Messaging\GameMessaging.h>

#include <ui\Game\Profiles\ProfileInterface.h>

#include "bdLobby\bdLobbyCommon\bdAuthCommon.h"

#include <Gamelogic\SocialNetworking\FriendsManager.h>



static const S32 MAX_LEADERBOARD_START_INDEX = 0;
static const S32 MAX_SP_LEADERBOARD_COLUMNS = 6;
static const S32 MAX_MP_LEADERBOARD_COLUMNS = 12;



static Debug::Logger &gAmaxScoreboardsLogger = Debug::GetLogger("amax.scoreboards");

namespace Game
{

C_ScoreboardManager C_ScoreboardManager::s_instance;

void C_ScoreboardManager::Init()
{
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );

	AddSubscriber( GameMessaging::E_GameMsg_Update );

	mScoreboardRankOffset=0;
}

void C_ScoreboardManager::Update()
{

}

void C_ScoreboardManager::StartScoreboardWrite(void)
{
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	if (levelState.mNetworkRace)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised())
		{
			services.StopScoreboardRequests();
			mScoreboardWriteUserIndex = 0;
			mScoreboardWriteState = eWriteNotActive;

			RacerNet::C_NetRace& race = services.Race();
			if (race.IsInitialised() && race.IsRunning() && race.IsOnline())
			{	
				if (services.PlayerManager().Index(0)->m_RaceData.m_InRaceVehicleIndex < 0)
					return; // local player was not in the race.. maybe spectating.

				// start procedure for scoreboard writing..
				bool updateSkill = false;
				RacerNet::C_NetPlayer* pHost = services.PlayerManager().HostPlayer(false);
				if (pHost && pHost->LocalPlayer())
					updateSkill = true;
				
				// only host needs to update skill values since we dont run ranked sessions.
				mScoreboardWriteState = (updateSkill == true) ? eWriteSkillStart : eWriteStatsStart;
			}
		}
	}
}

bool C_ScoreboardManager::ScoreboardWriteComplete(void)
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised())
	{
		NetPlay::C_Session& raceSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);
		Scoreboard::C_Writer& writer = services.ScoreboardWriter();
		NetPlay::C_PlayerSkillWriter& skillWriter = services.PlayerSkillWriter();

		//has scoreboard writing procedure finished.
		switch (mScoreboardWriteState)
		{
		case eWriteSkillStart:
			ScoreboardSkillStart(skillWriter, raceSession);
			break;

		case eWriteSkillWait:
			ScoreboardSkillContinue(skillWriter);
			break;

		case eWriteStatsStart:
			ScoreboardStatsStart(writer, raceSession);
			break;

		case eWriteStatsWait:
			ScoreboardStatsContinue(writer);
			break;

		case eWriteEndSessionStart:
			ScoreboardEndSessionStart(raceSession);
			break;

		case eWriteEndSessionWait:
			ScoreboardEndSessionContinue(raceSession);
			break;
		}
	}
	else
		mScoreboardWriteState = eWriteNotActive;
	
	// all finished.
	return mScoreboardWriteState == eWriteNotActive ? true : false;
}

void C_ScoreboardManager::StartSPScoreboardWrite()
{
	//Default to not being active
	mScoreboardWriteState = eWriteNotActive;

	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	if (pLayer != NULL && !pLayer->GetGameModePtr()->IsNetwork()) //Make sure that we are in single player
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		if (services.IsInitialised()) //Check that we are online
		{
			services.StopScoreboardRequests();
			mScoreboardWriteUserIndex = 0;
			//We want to write to stats leaderboard only
			mScoreboardWriteState = eWriteStatsStart;			
		}
	}
}

bool C_ScoreboardManager::SPScoreboardWriteComplete(bool reTry)
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised())
	{
		NetPlay::C_Session& raceSession = services.MatchmakingSession(RacerNet::C_NetServices::eMatchmakingSessionTypeRace);
		Scoreboard::C_Writer& writer = services.ScoreboardWriter();
	
		//has scoreboard writing procedure finished.
		switch (mScoreboardWriteState)
		{
	
		case eWriteStatsStart:
			SPScoreboardStatsStart(writer, raceSession, reTry);
			break;

		case eWriteStatsWait:
			SPScoreboardStatsContinue(writer);
			break;
		}
	}
	else
		mScoreboardWriteState = eWriteNotActive;
	
	// all finished.
	return mScoreboardWriteState == eWriteNotActive ? true : false;
}

bool C_ScoreboardManager::GetWriteSuccess()
{
	bool success = (mScoreboardWriteState != eWriteFailed)? true : false;
	return success;
}

void C_ScoreboardManager::StartScoreboardRead(eScoreboardTypes type)
{

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised())
	{
		services.StopScoreboardRequests();
		mScoreboardWriteUserIndex = 0;

		if(type == eScoreboardTypeFriends)
			mScoreboardReadState = eReadFriendsStart;
		else if(type == eScoreboardTypeYou)
			mScoreboardReadState = eReadPivotStatsStart;
		else if (type == eScoreboardTypeLobby)
			mScoreboardReadState = eReadLobbyStart;
		else
			mScoreboardReadState = eReadStatsStart;
	}
}

bool C_ScoreboardManager::ScoreboardReadComplete(void)
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	if (services.IsInitialised())
	{
		//has scoreboard writing procedure finished.
		switch (mScoreboardReadState)
		{
		case eReadStatsStart:
			{
				Scoreboard::C_ReaderByRank& reader = services.ScoreboardReaderByRank();
				ScoreboardReadStatsStart(reader);
				break;
			}

		case eReadStatsWait:
			{
				Scoreboard::C_ReaderByRank& reader = services.ScoreboardReaderByRank();
				ScoreboardReadStatsContinue(reader);
				break;
			}
		case eReadFriendsStart:
			{
				ScoreboardReadFriendsStart();
				break;
			}

		case eReadFriendsWait:
			{
				ScoreboardReadFriendsContinue();
				break;
			}
		case eReadFriendsStatsStart:
			{
				Scoreboard::C_ReaderByUsers& reader = services.ScoreboardReaderByUsers();
				ScoreboardReadFriendsStatsStart(reader);
				break;
			}

		case eReadFriendsStatsWait:
			{
				Scoreboard::C_ReaderByUsers& reader = services.ScoreboardReaderByUsers();
				ScoreboardReadFriendsStatsContinue(reader);
				break;
			}
		case eReadLobbyStart:
			{
				ScoreboardReadLobbyStart();
				break;
			}

		case eReadLobbyWait:
			{
				ScoreboardReadLobbyContinue();
				break;
			}
		case eReadLobbyStatsStart:
			{
				Scoreboard::C_ReaderByUsers& reader = services.ScoreboardReaderByUsers();
				ScoreboardReadLobbyStatsStart(reader);
				break;
			}

		case eReadLobbyStatsWait:
			{
				Scoreboard::C_ReaderByUsers& reader = services.ScoreboardReaderByUsers();
				ScoreboardReadLobbyStatsContinue(reader);
				break;
			}
		case eReadPivotStatsStart:
			{
				Scoreboard::C_ReaderByRank& reader = services.ScoreboardReaderByRank();
				ScoreboardReadPivotStatsStart(reader);
				break;
			}

		case eReadPivotStatsWait:
			{
				Scoreboard::C_ReaderByRank& reader = services.ScoreboardReaderByRank();
				ScoreboardReadPivotStatsContinue(reader);
				break;
			}
		}
	}
	else
		mScoreboardReadState = eReadNotActive;
	
	// all finished.
	return mScoreboardReadState == eReadNotActive ? true : false;
}

void C_ScoreboardManager::ScoreboardSkillStart(NetPlay::C_PlayerSkillWriter& writer, NetPlay::C_Session& session)
{
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	writer.StopAndCleanup();

		// we dont use skill on demonware

	// some error has happened, clean up skill writer ans force main stats wwite on next update.
	if (mScoreboardWriteState != eWriteSkillWait)
	{
		writer.StopAndCleanup();
		mScoreboardWriteUserIndex = 0;
		mScoreboardWriteState = eWriteStatsStart;
	}
}

void C_ScoreboardManager::ScoreboardSkillContinue(NetPlay::C_PlayerSkillWriter& writer)
{
	bool error = false;
	if (writer.Continue(error))
	{
		// write finished so move on to main stats
		mScoreboardWriteUserIndex = 0;
		mScoreboardWriteState = eWriteStatsStart;
	}
}

void C_ScoreboardManager::ScoreboardStatsStart(Scoreboard::C_Writer& writer, NetPlay::C_Session& session)
{
	const Game::C_LevelResult& results	= Game::C_Game::GetInstance()->GetLevelResult();
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	writer.StopAndCleanup();

	if (levelState.mNumCars == mScoreboardWriteUserIndex)
	{
		// finished stats write for all local players, move to ending session.
		mScoreboardWriteUserIndex = 0;
		mScoreboardWriteState = eWriteEndSessionStart;
	}
	else
	{
		// is local player 
		if (levelState.mCars[mScoreboardWriteUserIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
		{
			// no stats for guests..
			if (!levelState.mCars[mScoreboardWriteUserIndex].mSystemPlayerId.m_IsGuest)
			{
				RacerNet::MpScoreboardDataForEvent localPlayerData;

				Game::C_GameProfile *prof=Game::C_GameProfileManager::Instance().GetPrimaryProfile();

				localPlayerData.m_GameRank = prof->mMultiplayerProfile.GetCurrentLevel();
				localPlayerData.m_TotalFans = prof->mMultiplayerProfile.GetMultiplayerStatistics().mNumLifetimeFans;
				localPlayerData.m_TotalTimePlayed = prof->mMultiplayerProfile.GetMultiplayerStatistics().mTotalPlaytime;
				localPlayerData.m_RacePoints = prof->mMultiplayerProfile.GetMultiplayerStatistics().mNumRacePoints;
				localPlayerData.m_TopThree = prof->mMultiplayerProfile.GetMultiplayerStatistics().mNumTimesInTopThree;
				localPlayerData.m_TotalRaces = prof->mMultiplayerProfile.GetMultiplayerStatistics().mNumEventsPlayed;
				localPlayerData.m_RaceWins = prof->mMultiplayerProfile.GetMultiplayerStatistics().mNumEventsWon;	
				localPlayerData.m_PowerUpHits = prof->mMultiplayerProfile.GetMultiplayerStatistics().mTotalPickupsHit;
				localPlayerData.m_PowerUpsFired = prof->mMultiplayerProfile.GetMultiplayerStatistics().mTotalPickupsFired;
				localPlayerData.m_CarsWrecked = prof->mMultiplayerProfile.GetMultiplayerStatistics().mNumTimesWreckedOtherDrivers;
				localPlayerData.m_LegendLevel = prof->mMultiplayerProfile.GetLegendLevel();
				localPlayerData.m_TimeToAchieveLegend = prof->mMultiplayerProfile.GetMultiplayerStatistics().mTimeToAchieveLegend;

				localPlayerData.m_TotalFansRating = localPlayerData.m_TotalFans;
				localPlayerData.m_RaceWinsRating = localPlayerData.m_RaceWins;
				localPlayerData.m_PowerUpHitsRating = localPlayerData.m_PowerUpHits;				
				localPlayerData.m_RacePointsRating = localPlayerData.m_RacePoints;

				U64 mask = 0x00FFFFFFFFFFFFFFLL;

				//Some error checking so we don't blitz over the value
				S64 assertMask = 0xFFFFFFFFFC000000LL;
				S64 assertValue = (localPlayerData.m_TimeToAchieveLegend & assertMask);
				BIZ_ASSERT_MSG((assertValue == 0), "MP SCOREBOARD WRITING ERROR, VALUE TOO BIG TO FIT INTO RATING VARIABLE - PLEASE TELL AS ");

				S64 massiveNumber = 0x0000000000FFFFFFLL;

				U64 legendTimeBitField = (massiveNumber - localPlayerData.m_TimeToAchieveLegend);
				U64 legendLevelBitField = (localPlayerData.m_LegendLevel << 26); //we want to store the level in the top 6 bits of the 32 bit value, shift them up by 26

				legendTimeBitField = (mask & legendTimeBitField); //make sure the top 8 bits are all 0

				localPlayerData.m_LegendLevelRating = (legendLevelBitField | legendTimeBitField) ;

				/*
				localPlayerData.m_RacesWonRatio=(S64)(((float)localPlayerData.m_RacesWon/(float)localPlayerData.m_RacesPlayed)*1000000.f);
				localPlayerData.m_ShieldingRatio=(S64)(((float)localPlayerData.m_ShieldedSuccessful/(float)localPlayerData.m_ShieldsUsed)*1000000.f);
				localPlayerData.m_FansRatio=(S64)(((float)localPlayerData.m_TotalFans/(float)localPlayerData.m_RacesPlayed)*1000000.f);
				localPlayerData.m_RacesPointsRatio=(S64)(((float)localPlayerData.m_RacesPoints/(float)localPlayerData.m_RacesPlayed)*1000000.f);

				if(localPlayerData.m_PowerupsUsed)
					localPlayerData.m_PowerupsRatio=(S64)(((float)localPlayerData.m_HitWithPowerups/(float)localPlayerData.m_PowerupsUsed)*1000000.f);
				else
					localPlayerData.m_PowerupsRatio=0.f;
				*/

				bool ok = true;
				RacerNet::ScoreboardColumnWriteList cols;
				Scoreboard::ColumnData* pColumnData = NULL;

				RacerNet::MpBuildScoreboardColumnsToWrite(RacerNet::eMpScoreboardTotalWins, localPlayerData, cols);
				pColumnData = (cols.Size() > 0) ? &cols[0] : NULL;
				
					ok = ok && writer.Queue(	RacerNet::ScoreboardId(RacerNet::eMpScoreboardTotalWins), 
												localPlayerData.m_RacePointsRating,
												Scoreboard::eWriteTypeReplace,
												pColumnData, 
												cols.Size());

					RacerNet::MpBuildScoreboardColumnsToWrite(RacerNet::eMpScoreboardPerkRatio, localPlayerData, cols);
					pColumnData = (cols.Size() > 0) ? &cols[0] : NULL;
					ok = ok && writer.Queue(	RacerNet::ScoreboardId(RacerNet::eMpScoreboardPerkRatio), 
												localPlayerData.m_PowerUpHitsRating,
												Scoreboard::eWriteTypeReplace,
												pColumnData, 
												cols.Size());


					RacerNet::MpBuildScoreboardColumnsToWrite(RacerNet::eMpScoreboardTotalFans, localPlayerData, cols);
					pColumnData = (cols.Size() > 0) ? &cols[0] : NULL;
					ok = ok && writer.Queue(	RacerNet::ScoreboardId(RacerNet::eMpScoreboardTotalFans), 
												localPlayerData.m_TotalFansRating,
												Scoreboard::eWriteTypeReplace,
												pColumnData, 
												cols.Size());

					if(localPlayerData.m_LegendLevel > 0)//we only want to write to the legend leaderboard if the player is a legend!
					{
						RacerNet::MpBuildScoreboardColumnsToWrite(RacerNet::eMpScoreboardLegends, localPlayerData, cols);
						pColumnData = (cols.Size() > 0) ? &cols[0] : NULL;
						ok = ok && writer.Queue(	RacerNet::ScoreboardId(RacerNet::eMpScoreboardLegends), 
													localPlayerData.m_LegendLevelRating,
													Scoreboard::eWriteTypeReplace,
													pColumnData, 
													cols.Size());
					}

					RacerNet::MpBuildScoreboardColumnsToWrite(RacerNet::eMpScoreboardLobby, localPlayerData, cols);
					pColumnData = (cols.Size() > 0) ? &cols[0] : NULL;
					ok = ok && writer.Queue(	RacerNet::ScoreboardId(RacerNet::eMpScoreboardLobby), 
												localPlayerData.m_RaceWinsRating,
												Scoreboard::eWriteTypeReplace,
												pColumnData, 
												cols.Size());
				
				
				if (ok)
				{
					// all scoreboards queued for writing so lets try and write the trump..
					if (writer.Start(levelState.mCars[mScoreboardWriteUserIndex].mSystemPlayerId, session))
						mScoreboardWriteState = eWriteStatsWait; 
					else 
						ok = false;
				}
				
				if (!ok)
					writer.StopAndCleanup();
			}
		}

		// next time around look at this player
		mScoreboardWriteUserIndex += 1; 
	}
}

void C_ScoreboardManager::ScoreboardStatsContinue(Scoreboard::C_Writer& writer)
{
	bool error = false;
	if (writer.Continue(error))
	{
		// write finished so see if we need to write some more local player stats data.
		mScoreboardWriteState = eWriteStatsStart;
	}
}

void C_ScoreboardManager::SPScoreboardStatsStart(Scoreboard::C_Writer& writer, NetPlay::C_Session& session, bool reTry)
{
	const Game::C_LevelResult& results	= Game::C_Game::GetInstance()->GetLevelResult();
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	writer.StopAndCleanup();

	if (levelState.mNumCars == mScoreboardWriteUserIndex)
	{
		// finished stats write for all local players, move to ending session.
		mScoreboardWriteUserIndex = 0;
		mScoreboardWriteState = eWriteEndSessionStart;
	}
	else
	{
		// is local player 
		if (levelState.mCars[mScoreboardWriteUserIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
		{
			// no stats for guests..
			if (!levelState.mCars[mScoreboardWriteUserIndex].mSystemPlayerId.m_IsGuest)
			{
				AmaxEvent* ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];
				BIZ_ASSERT_MSG(ev, "COULD NOT GET SORCERY DATA FOR SP LEADERBOARD WRITE");
				
				RacerNet::eScoreboards sbToWriteTo = ev->mLeaderboardSettings.mScoreboardToWriteTo;
				RacerNet::eSPScoreboardTypes sbType =  ev->mLeaderboardSettings.mScoreboardType;

				RacerNet::SpScoreboardDataForEvent localPlayerData;

				Game::C_GameProfile *prof=Game::C_GameProfileManager::Instance().GetPrimaryProfile();

				const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
				const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

				localPlayerData.m_TotalFans = results.mCars[0].mScore;
				localPlayerData.m_LapTime = results.mCars[0].mStatistics.mFastestLapTime  * 100.0f;
				localPlayerData.m_CompTime = results.mCars[0].mStatistics.mTotalRaceTime   * 100.0f;
				localPlayerData.m_Wrecks = results.mCars[0].mStatistics.mNumVehiclesWrecked;
				localPlayerData.m_VehicleId = levelstate.mCars[0].mCarID;
				localPlayerData.m_Difficulty = levelstate.m_difficulty;

				if(sbType == RacerNet::eSPCheckpointsScoreboard)
				{
					//this is time left and gets filled in by racemodecheckpoint
					localPlayerData.m_CompTime = results.mRaceStatistics.mTotalRaceTime * 100.0f;
				}

				localPlayerData.m_RacesRating = localPlayerData.m_TotalFans;
				localPlayerData.m_FanDemandsRating = localPlayerData.m_TotalFans;
				localPlayerData.m_CheckpointsRating = localPlayerData.m_CompTime;
				localPlayerData.m_DestructionRating =  results.mRaceStatistics.mDestruction_NumPoints;
				localPlayerData.m_1on1Rating = localPlayerData.m_CompTime;

				bool ok = true;
				RacerNet::ScoreboardColumnWriteList cols;
				Scoreboard::ColumnData* pColumnData = NULL;				

				S64 Rating = 0;

				bool shouldWriteLeaderboard = false;

				switch(sbType)
				{
				case RacerNet::eSPRacesScoreboard:
					{
						Rating = localPlayerData.m_RacesRating;
					}			 
					break;	
				case RacerNet::eSPFanDemandsScoreboard:
					{
						Rating = localPlayerData.m_FanDemandsRating;
					}			 
					break;
				case RacerNet::eSPCheckpointsScoreboard:
					{
						Rating = localPlayerData.m_CheckpointsRating;
					}			 
					break;
				case RacerNet::eSPDestructionScoreboard:
					{
						localPlayerData.m_Wrecks = localPlayerData.m_DestructionRating;
						Rating = localPlayerData.m_DestructionRating;
					}			 
					break;
				case RacerNet::eSP1on1Scoreboard:
					{
						S64 massiveNumber = 0x000000001FFFFFFFLL;
						Rating = (massiveNumber - localPlayerData.m_1on1Rating);
					}			 
					break;
				default:
					{
						BIZ_ASSERT_MSG(true, "INVALID SP SCOREBOARD TYPE");
					}			
				}
				
				//This bit of code will ensure that the leaderboards are ordered by difficulty
				S64 mask = 0x000000001FFFFFFFLL;

				//Some error checking so we don't blitz over the value
				S64 assertMask = 0xFFFFFFFFE0000000LL;
				S64 assertValue = (Rating & assertMask);
				BIZ_ASSERT_MSG((assertValue == 0), "SCOREBOARD WRITING ERROR, VALUE TOO BIG TO FIT INTO RATING VARIABLE - PLEASE TELL AS ");

				S64 difficultyBitfield = (localPlayerData.m_Difficulty << 29); //shift the difficulty up to top 3 bits in a 32 bit value (xlast can only handle 32 bit)
				Rating = (mask & Rating); //clear the top 35 bits with the mask
				
				S64 finalRating = (Rating | difficultyBitfield); //Rating Ored with difficulty will create our final rating

				switch(sbType)
				{
				case RacerNet::eSPRacesScoreboard:
					{
						localPlayerData.m_RacesRating = finalRating;
					}			 
					break;	
				case RacerNet::eSPFanDemandsScoreboard:
					{
						localPlayerData.m_FanDemandsRating = finalRating;
					}			 
					break;
				case RacerNet::eSPCheckpointsScoreboard:
					{
						localPlayerData.m_CheckpointsRating = finalRating;
					}			 
					break;
				case RacerNet::eSPDestructionScoreboard:
					{
						localPlayerData.m_DestructionRating = finalRating;
					}			 
					break;
				case RacerNet::eSP1on1Scoreboard:
					{
						localPlayerData.m_1on1Rating = finalRating;
					}			 
					break;
				default:
					{
						BIZ_ASSERT_MSG(true, "INVALID SP SCOREBOARD TYPE");
					}			
				}


				RacerNet::SpBuildScoreboardColumnsToWrite(sbType, localPlayerData, cols);
				pColumnData = (cols.Size() > 0) ? &cols[0] : NULL;


				S64 CurrentRating = GetCurrentScoreOnLeaderboard();

				if(finalRating > CurrentRating)
				{
					shouldWriteLeaderboard = true;
				}
				else
				{
					mScoreboardWriteState = eWriteNotActive;
				}

				if(shouldWriteLeaderboard == true)
				{

					if(reTry == false)
					{
						UpdateLocalLeaderboard(finalRating, pColumnData, cols.Size() );
					}

					ok = ok && writer.Queue(	RacerNet::ScoreboardId(sbToWriteTo), 
												finalRating,
												Scoreboard::eWriteTypeReplace,
												pColumnData, 
												cols.Size());	

					
					
					if (ok)
					{
						// all scoreboards queued for writing so lets try and write the trump..
						if (writer.Start(levelState.mCars[mScoreboardWriteUserIndex].mSystemPlayerId, session))
							mScoreboardWriteState = eWriteStatsWait; 
						else 
							ok = false;
					}
					
					if (!ok)
						writer.StopAndCleanup();

				}
			}
		}

		// next time around look at this player
		mScoreboardWriteUserIndex += 1; 
	}
}

S64 C_ScoreboardManager::GetCurrentScoreOnLeaderboard()
{
	S64 CurrentScore = 0;

	int primaryPad = SysProfile::C_Manager::Instance().GetPrimaryPadNo();

	if(primaryPad != -1)
	{
		SysProfile::FullProfile& profileMe = UI::C_ProfileInterface::Instance().GetPadProfile( primaryPad );

		for(S32 i = 0; i < mTotalUsers; i++)
		{
#ifdef PLATFORM_PS3
			bdOnlineUserID id=bdAuthUtility::getUserID(profileMe.m_Uid.handle.data);
			if(id == mRowData[i].m_EntityId)
#else
			if((SysProfile::UserIdentifier)mRowData[i].m_Userdata == (SysProfile::UserIdentifier)profileMe)
#endif
			{
				CurrentScore = mRowData[i].m_Rating;
			}
		}
	}

	return CurrentScore;
}

void C_ScoreboardManager::UpdateLocalLeaderboard(const S64 rating,  const Scoreboard::ColumnData* pColumnData, const U32 numColumns)
{
	int primaryPad = SysProfile::C_Manager::Instance().GetPrimaryPadNo();

	bool updated = false;

	if(primaryPad != -1)
	{
		SysProfile::FullProfile& profileMe = UI::C_ProfileInterface::Instance().GetPadProfile( primaryPad );

		for(S32 i = 0; i < mTotalUsers; i++)
		{
#ifdef PLATFORM_PS3
			bdOnlineUserID id=bdAuthUtility::getUserID(profileMe.m_Uid.handle.data);
			if(id == mRowData[i].m_EntityId)
#else
			if((SysProfile::UserIdentifier)mRowData[i].m_Userdata == (SysProfile::UserIdentifier)profileMe)
#endif
			{
				updated = true;
				mRowData[i].m_Rating = rating;

				for(S32 j = 0; j < mRowData[i].m_Columns.Size(); j++)
				{
					mRowData[i].m_Columns[j].m_Data.m_S64 = pColumnData[j].m_Data.m_S64;
				}
				
			}
		}

		if(updated == false)
		{
			updated = true;
			Scoreboard::RowData newRow;

			const Game::C_LevelState& levelstate = Game::C_Game::GetInstance()->GetLevelState();
			UI::WideStrCopy( newRow.m_Userdata.m_Name, SYSTEM_PROFILE_NAME_LENGTH, UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() ).m_Name );

			newRow.m_Rating = rating;
			for(S32 i = 0; i < numColumns-1; i++)
			{
				newRow.m_Columns.PushBack(pColumnData[i]);
				newRow.m_Columns[i].m_Data.m_S64 = pColumnData[i].m_Data.m_S64;
			}
			
			if(mTotalRows < MAX_SCOREBOARD_FRIENDS) //add to the end
			{
				mRowData[mTotalRows] = newRow;
				mTotalRows++;
				mTotalUsers++;
			}
			else //overwrite the last one
			{
				mRowData[(MAX_SCOREBOARD_FRIENDS -1)] = newRow;
			}			
		}

		//sort your friends
		for(int a=0;a<MAX_SCOREBOARD_FRIENDS;a++)
		{
			mSortList[a]=a;
		}	

		if (mTotalUsers > 0)
		{
			for(unsigned int a=0;a<mTotalUsers;a++)
			{
				for(unsigned int b=0;b<mTotalUsers;b++)
				{
					if(a != b)
					{
						if(mRowData[mSortList[a]].m_Rating > mRowData[mSortList[b]].m_Rating)
						{
							int temp=mSortList[a];
							mSortList[a]=mSortList[b];
							mSortList[b]=temp;
						}
					}
				}
			}
		}	

		StoreFriendLeaderboardLocally( profileMe );		
	}
}

void C_ScoreboardManager::StoreFriendLeaderboardLocally(SysProfile::UserIdentifier& profileMe)
{
	S32 rowMe = -1;

	//Loop through all of the users to find me in the list
	for( int a = 0; a < mTotalUsers;  a++ )
	{
		if((SysProfile::UserIdentifier)mRowData[mSortList[a]].m_Userdata == (SysProfile::UserIdentifier)profileMe)
		{
			rowMe = a;
			break;
		}
	}

	if((rowMe != -1) && (rowMe != 0))
	{
		if(rowMe == (mTotalRows -1)) //I'm last
		{
			S32 count = 0;
			for( int a = (int)mTotalRows - MAX_FRIENDS_SCOREBOARD_ROWS; a < (int)mTotalRows; a++ )
			{
				if(a >= 0)
				{
					mFriendsRowData[count]=mRowData[mSortList[a]];
					mFriendsRowData[count].m_Rank = (a+1);
					count++;
				}
			}	
			mNumFriendRows = count;
		}
		else // we want the people around us because I'm in the middle of the list
		{
			mFriendsRowData[0]=mRowData[mSortList[rowMe-1]];
			mFriendsRowData[0].m_Rank = rowMe;

			mFriendsRowData[1]=mRowData[mSortList[rowMe]];
			mFriendsRowData[1].m_Rank = (rowMe + 1);

			mFriendsRowData[2]=mRowData[mSortList[rowMe+1]];
			mFriendsRowData[2].m_Rank = (rowMe + 2);

			mNumFriendRows = MAX_FRIENDS_SCOREBOARD_ROWS;
		}
	}
	else
	{	
		mNumFriendRows = 0;
		//Just give me the top friends, I'm not on the scoreboard or I'm at the top
		for(int a=0;a<mTotalRows && a<MAX_FRIENDS_SCOREBOARD_ROWS;a++)
		{
			//copy the top 3 as we show it in the UI and other leadwerboard reads could happen
			mFriendsRowData[a]=mRowData[mSortList[a]];
			mFriendsRowData[a].m_Rank = (a + 1);
			mNumFriendRows++;
		}
	}
}

void C_ScoreboardManager::SPScoreboardStatsContinue(Scoreboard::C_Writer& writer)
{
	bool error = false;
	if (writer.Continue(error))
	{
		if(error == true)
		{
			mScoreboardWriteState = eWriteFailed;
		}
		else
		{
			mScoreboardWriteState = eWriteNotActive;
		}
	}
}

void C_ScoreboardManager::ScoreboardReadStatsStart(Scoreboard::C_ReaderByRank& reader)
{
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	
	reader.StopAndCleanup();


	if(reader.Initialise())
	{
		Scoreboard::ColumnIdentifierList ident_list;
		Scoreboard::ColumnTypeList type_list;

		const S32 NumColumns = IsCurrentScoreboardSP() ? MAX_SP_LEADERBOARD_COLUMNS : MAX_MP_LEADERBOARD_COLUMNS;

		for(int i = MAX_LEADERBOARD_START_INDEX; i < NumColumns; i++)
		{
			ident_list.PushBack(i);
			type_list.PushBack(Scoreboard::Int64);
		}

		reader.AddScoreboard(RacerNet::ScoreboardId((RacerNet::eScoreboards)mLookingAtScoreboard),ident_list,type_list,true);

		bool ok=reader.StartReadByRank(0,1 + mScoreboardRankOffset, 10);
		if(!ok)
		{
			BIZ_ASSERT_FAIL("Error reading leaderboards");
			mScoreboardReadState = eReadNotActive;
		}
		else
		{
			mTotalRows=0;
			mScoreboardReadState = eReadStatsWait;
		}
	}
}

void C_ScoreboardManager::ScoreboardReadStatsContinue(Scoreboard::C_ReaderByRank& reader)
{
	bool error = false;
	if (reader.Continue(error))
	{
		mTotalRows=reader.ExtractRows(RacerNet::ScoreboardId((RacerNet::eScoreboards)mLookingAtScoreboard),mRowData,10,mTotalUsers);
		reader.StopAndCleanup();
		mScoreboardReadState = eReadNotActive;

		//no need to sort as things come back like that
		for(int a=0;a<MAX_SCOREBOARD_FRIENDS;a++)
		{
			mSortList[a]=a;
		}
	}
}

void C_ScoreboardManager::ScoreboardReadPivotStatsStart(Scoreboard::C_ReaderByRank& reader)
{
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	reader.StopAndCleanup();

	if(reader.Initialise())
	{
		Scoreboard::ColumnIdentifierList ident_list;
		Scoreboard::ColumnTypeList type_list;

		const S32 NumColumns = IsCurrentScoreboardSP() ? MAX_SP_LEADERBOARD_COLUMNS : MAX_MP_LEADERBOARD_COLUMNS;

		for(int i = MAX_LEADERBOARD_START_INDEX; i < NumColumns; i++)
		{
			ident_list.PushBack(i);
			type_list.PushBack(Scoreboard::Int64);
		}

		reader.AddScoreboard(RacerNet::ScoreboardId((RacerNet::eScoreboards)mLookingAtScoreboard),ident_list,type_list,true);

		const S32 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);

		bool ok=reader.StartReadByPivot(0, fp, 10);
		if(!ok)
		{
			BIZ_ASSERT_FAIL("Error reading leaderboards");
			mScoreboardReadState = eReadNotActive;
		}
		else
		{
			mTotalRows=0;
			mScoreboardReadState = eReadPivotStatsWait;
		}
	}
}

void C_ScoreboardManager::ScoreboardReadPivotStatsContinue(Scoreboard::C_ReaderByRank& reader)
{
	bool error = false;
	if (reader.Continue(error))
	{
		mTotalRows=reader.ExtractRows(RacerNet::ScoreboardId((RacerNet::eScoreboards)mLookingAtScoreboard),mRowData,10,mTotalUsers);
		//mTotalUsers=mTotalRows;

		reader.StopAndCleanup();
		mScoreboardReadState = eReadNotActive;

		//no need to sort as things come back like that
		for(int a=0;a<MAX_SCOREBOARD_FRIENDS;a++)
		{
			mSortList[a]=a;
		}
	}
}


void C_ScoreboardManager::ScoreboardReadFriendsStart()
{
	const S32 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);

	mFriendsEnumerator.Initialise();

	bool ok=mFriendsEnumerator.StartEnumeration(fp,false);

	if(!ok)
	{
		mTotalUsers=0;
		BIZ_ASSERT_FAIL("Could not start Friends enumeration for leaderboards");
		mScoreboardReadState = eReadNotActive;
	}
	else
	{
		mScoreboardReadState = eReadFriendsWait;
	}
}

void C_ScoreboardManager::ScoreboardReadFriendsContinue()
{
	bool error=false;

	if(mFriendsEnumerator.ContinueEnumeration(error))
	{
		S32 numFriends = mFriendsEnumerator.ExtractFriends(mFriends,MAX_SCOREBOARD_FRIENDS);

		if((numFriends > 0) && error)
		{
			mTotalUsers=0;
			BIZ_ASSERT_FAIL("Could not start Friends enumeration for leaderboards");
			mScoreboardReadState = eReadNotActive;
		}
		else
		{
			mNumFriends=mFriendsEnumerator.ExtractFriends(mFriends,MAX_SCOREBOARD_FRIENDS);
			mScoreboardReadState = eReadFriendsStatsStart;

			for(int a=0;a<mNumFriends;a++)
			{
				Game::C_FriendsManager::Instance().AddFriend(mFriends[a].m_Profile);
			}
		}
	}
}

void C_ScoreboardManager::ScoreboardReadFriendsStatsStart(Scoreboard::C_ReaderByUsers& reader)
{
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	reader.StopAndCleanup();

	if(reader.Initialise())
	{
		Scoreboard::ColumnIdentifierList ident_list;
		Scoreboard::ColumnTypeList type_list;

		const S32 NumColumns = IsCurrentScoreboardSP() ? MAX_SP_LEADERBOARD_COLUMNS : MAX_MP_LEADERBOARD_COLUMNS;

		for(int i = MAX_LEADERBOARD_START_INDEX; i < NumColumns; i++)
		{
			ident_list.PushBack(i);
			type_list.PushBack(Scoreboard::Int64);
		}
	
		reader.AddScoreboard(RacerNet::ScoreboardId((RacerNet::eScoreboards)mLookingAtScoreboard),ident_list,type_list,true);

		SysProfile::UserIdentifier ids[MAX_SCOREBOARD_FRIENDS];

		const S32 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);

		ids[0].m_IsGuest=fp.m_IsGuest;
		ids[0].m_Uid=fp.m_Uid;

		for(int a=0;a<mNumFriends;a++)
		{
			ids[a+1].m_IsGuest=mFriends[a].m_Profile.m_IsGuest;
			ids[a+1].m_Uid=mFriends[a].m_Profile.m_Uid;
		}

		bool ok=reader.SetUsers(ids,mNumFriends+1);
		ok = ok & reader.Start(0);
		if(!ok)
		{
			BIZ_ASSERT_FAIL("Error reading leaderboards");
			mScoreboardReadState = eReadNotActive;
		}
		else
		{
			mTotalRows=0;
			mScoreboardReadState = eReadFriendsStatsWait;
		}
	}
}

void C_ScoreboardManager::ScoreboardReadFriendsStatsContinue(Scoreboard::C_ReaderByUsers& reader)
{
	bool error = false;
	if (reader.Continue(error))
	{
		mTotalRows=reader.ExtractRows(RacerNet::ScoreboardId((RacerNet::eScoreboards)mLookingAtScoreboard),mRowData,MAX_SCOREBOARD_FRIENDS);
		mTotalUsers=mTotalRows;

		reader.StopAndCleanup();
		mScoreboardReadState = eReadNotActive;

		//sort your friends
		for(int a=0;a<MAX_SCOREBOARD_FRIENDS;a++)
		{
			mSortList[a]=a;
		}

		bool sorting=true;
		while(sorting)
		{
			sorting=false;

			if (mTotalUsers > 0)
			{
				for(unsigned int a=0;a<mTotalUsers-1;a++)
				{
					if(mRowData[mSortList[a]].m_Rank > mRowData[mSortList[a+1]].m_Rank)
					{
						int temp=mSortList[a];
						mSortList[a]=mSortList[a+1];
						mSortList[a+1]=temp;

						sorting=true;
					}
				}
			}
		}
		
		int primaryPad = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		S32 rowMe = -1;

		if(primaryPad != -1)
		{
			SysProfile::FullProfile& profileMe = UI::C_ProfileInterface::Instance().GetPadProfile( primaryPad );
		
			StoreFriendLeaderboardLocally( profileMe );
		}
	
	}
}

//--------------------------------------------------------------------

void C_ScoreboardManager::ScoreboardReadLobbyStart()
{
	const S32 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();

	bool ok = false;

	if(services.IsInitialised())
	{
		RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();
		mNumLobbyProfiles = 0;

		ok = true;

		for( S32 i = 0; i < mgr.TotalSize(); i++ )
		{
			RacerNet::C_NetPlayer* pPlayer = mgr.Index(U8(i));

			if(pPlayer)
			{
				mLobbyProfiles[mNumLobbyProfiles] = pPlayer->m_User;
				mNumLobbyProfiles++;
				
			}
		}		
	}

	if(!ok)
	{
		mTotalUsers=0;
		BIZ_ASSERT_FAIL("Could not start Lobby enumeration for leaderboards");
		mScoreboardReadState = eReadNotActive;
	}
	else
	{
		mScoreboardReadState = eReadLobbyWait;
	}
}

void C_ScoreboardManager::ScoreboardReadLobbyContinue()
{
	//Do I need this function at all?
	bool error=false;

	if(error)
	{
		mTotalUsers=0;
		BIZ_ASSERT_FAIL("Could not start Friends enumeration for leaderboards");
		mScoreboardReadState = eReadNotActive;
	}
	else
	{
		mScoreboardReadState = eReadLobbyStatsStart;
	}
	
}

void C_ScoreboardManager::ScoreboardReadLobbyStatsStart(Scoreboard::C_ReaderByUsers& reader)
{
	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	reader.StopAndCleanup();

	if(reader.Initialise())
	{
		Scoreboard::ColumnIdentifierList ident_list;
		Scoreboard::ColumnTypeList type_list;

		const S32 NumColumns = IsCurrentScoreboardSP() ? MAX_SP_LEADERBOARD_COLUMNS : MAX_MP_LEADERBOARD_COLUMNS;

		for(int i = MAX_LEADERBOARD_START_INDEX; i < NumColumns; i++)
		{
			ident_list.PushBack(i);
			type_list.PushBack(Scoreboard::Int64);
		}

		reader.AddScoreboard(RacerNet::ScoreboardId((RacerNet::eScoreboards)mLookingAtScoreboard),ident_list,type_list,true);

		SysProfile::UserIdentifier ids[MAX_SCOREBOARD_LOBBYPROFILES];

		const S32 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);

		//ids[0].m_IsGuest=fp.m_IsGuest;
		//ids[0].m_Uid=fp.m_Uid;

		for(int a=0;a<mNumLobbyProfiles;a++)
		{
			ids[a].m_IsGuest=mLobbyProfiles[a].m_IsGuest;
			ids[a].m_Uid=mLobbyProfiles[a].m_Uid;
		}

		bool ok=reader.SetUsers(ids,mNumLobbyProfiles+1);
		ok = ok & reader.Start(0);
		if(!ok)
		{
			BIZ_ASSERT_FAIL("Error reading leaderboards");
			mScoreboardReadState = eReadNotActive;
		}
		else
		{
			mTotalRows=0;
			mScoreboardReadState = eReadLobbyStatsWait;
		}
	}
}

void C_ScoreboardManager::ScoreboardReadLobbyStatsContinue(Scoreboard::C_ReaderByUsers& reader)
{
	bool error = false;
	if (reader.Continue(error))
	{
		mTotalRows=reader.ExtractRows(RacerNet::ScoreboardId((RacerNet::eScoreboards)mLookingAtScoreboard),mRowData,MAX_SCOREBOARD_LOBBYPROFILES);
		mTotalUsers=mTotalRows;

		reader.StopAndCleanup();
		mScoreboardReadState = eReadNotActive;

		//sort your friends
		for(int a=0;a<MAX_SCOREBOARD_LOBBYPROFILES;a++)
		{
			mSortList[a]=a;
		}

		bool sorting=true;
		while(sorting)
		{
			sorting=false;

			if (mTotalUsers > 0)
			{
				for(unsigned int a=0;a<mTotalUsers-1;a++)
				{
					if(mRowData[mSortList[a]].m_Rank > mRowData[mSortList[a+1]].m_Rank)
					{
						int temp=mSortList[a];
						mSortList[a]=mSortList[a+1];
						mSortList[a+1]=temp;

						sorting=true;
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------
void C_ScoreboardManager::ScoreboardEndSessionStart(NetPlay::C_Session& session)
{	
	if (session.EndPlay())
		mScoreboardWriteState = eWriteEndSessionWait;
	else
		mScoreboardWriteState = eWriteNotActive;
}

void C_ScoreboardManager::ScoreboardEndSessionContinue(NetPlay::C_Session& session)
{
	switch (session.SessionState())
	{
	case NetPlay::C_Session::eStateRunning:
	case NetPlay::C_Session::eStateError:
		mScoreboardWriteState = eWriteNotActive;
		mScoreboardWriteUserIndex = 0;
		break;
	}
}


bool C_ScoreboardManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case GameMessaging::E_GameMsg_Update:
			{
				mDeltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
				Update();
			}
			break;

		default :
			handledMessage = false;
			break;
	}

	return handledMessage;
}

void C_ScoreboardManager::UpdateSessionLeaderboard()
{
	//if we already have something in the scoreboard vector then we want to get rid of it
	if(mSessionScoreboard.Size() > 0)
	{
		mSessionScoreboard.Destroy();
	}

	//Now that we have a clear leaderboard vector we want to fill it with the data from everyone currently in the session
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayerManager& PlayerManager =  services.PlayerManager();
	U8 numTotalPlayers = PlayerManager.TotalSize();

	//for each of the players in the lobby
	for(int i = 0; i < numTotalPlayers; i++)
	{
		RacerNet::C_NetPlayer* pNetPlayer = PlayerManager.Index(i);
		if(pNetPlayer) //Just in case - they can leave the lobby at any time making this pointer invalid
		{
			SessionScoreboardData data;
	
			data.m_Name = pNetPlayer->m_User.Name();

#if defined(PLATFORM_PS3)
			data.m_Uid = pNetPlayer->m_User.m_Uid;
#endif
			data.m_IsLocal = pNetPlayer->LocalPlayer();
			data.m_Legend = pNetPlayer->m_ProfileData.m_Game.m_Legend;
			data.m_RacePointsThisSession = pNetPlayer->m_ProfileData.m_Game.m_RacePointsThisSession;
			data.m_RacesPlayedThisSession = pNetPlayer->m_ProfileData.m_Game.m_RacesThisSession;
			data.m_RaceWinsThisSession = pNetPlayer->m_ProfileData.m_Game.m_WinsThisSession;
			data.m_Rank = pNetPlayer->m_ProfileData.m_Game.m_Rank;
			data.m_Team = pNetPlayer->m_RaceData.m_Team;

			//store their data in the vector
			mSessionScoreboard.PushBack(data);
		}
	}

	//Bubble Sort - dirty
	for(int i = 0; i < (mSessionScoreboard.Size()-1); i++)
	{
		for(int j = 0; j < (mSessionScoreboard.Size()-1); j++)
		{
			if(mSessionScoreboard[j].m_RacePointsThisSession < mSessionScoreboard[j+1].m_RacePointsThisSession)
			{
				//Swap them
				SessionScoreboardData data;

				data = mSessionScoreboard[j];//store temp
				mSessionScoreboard[j] = mSessionScoreboard[j+1]; //swap them over
				mSessionScoreboard[j+1] = data;
	
			}
		}
	}

	//now we need to sort all of the data ready to be read by the UI
	//TODO Sort the data based on points this session

}

bool C_ScoreboardManager::IsCurrentScoreboardSP() 
{
	bool scoreboardIsSP = true;

		RacerNet::eScoreboards currentScoreboard = (RacerNet::eScoreboards)mLookingAtScoreboard;

		//check if we are using any of the multiplayer scoreboards as there are way less of those
		if(currentScoreboard == RacerNet::eMpScoreboardTotalWins || currentScoreboard == RacerNet::eMpScoreboardPerkRatio || currentScoreboard == RacerNet::eMpScoreboardTotalFans || currentScoreboard == RacerNet::eMpScoreboardLegends || currentScoreboard == RacerNet::eMpScoreboardLobby)
		{
			scoreboardIsSP = false;
		}

	return scoreboardIsSP;
}

Btl::Vector<SessionScoreboardData>& C_ScoreboardManager::GetSessionScoreboardData()
{
	return mSessionScoreboard;
}

}; // namespace Game


