#include "stdafx.h"
#include "DataGathering.h"
#include "system\serialise\C_MemoryStream.h"
#include "system\serialise\BinarySerialiseAutoEndian.h"
#include "Structures\NetGameVars.h"
#include "Structures\LevelState.h"
#include "Structures\LevelResult.h"
#include "Structures\NetVehicleData.h"
#include "Structures\MpResults.h"
#include "RaceModes\RaceModeMultiplayer.h"
#include "GameApp.h"
#include "GameLogic\Layers\RaceBookLayer.h"
#include "GameLogic\ActorStudioActors\PickupPositionActor.h"
#include "Game\ActorDef\GameActorList.h"
#include "GameLogic\Profile\GameProfileManager.h"
#include <Game/Game.h>
#include "GameLogic/Layers/RaceLogicLayer.h"
#include "Network/Player/NetPlayerManager.h"

extern Debug::Logger &g_MultiplayerLog;

namespace Game
{

DataGathering::DataGathering(C_RecordEventManager& eventManager):
	m_eventManager(eventManager)
{
	mVoteResults.Reset();
	mSessionboardStats.Reset();
	m_FlaggedEvents.ClearBits();
	mTrackPickupData = 0;
	mTrackPickupNumber = 0;
}

void DataGathering::Init(DataGatheringRecords* dataGatheringRecords)
{
	m_RecordsActive = dataGatheringRecords;

	if(!m_RecordsActive)
	{
		LOGERROR(g_MultiplayerLog, "DataGathering::DataGathering - Record Activation Data not found! Data logging will be inactive");
	}
}

void DataGathering::FlagPostEvent(DATA_GATHERING_RECORD_TYPE category)
{
	if(IsRecordTypeActive(category))
	{
		m_FlaggedEvents.SetBit(category);
	}
}

//Call once to post all the currently flagged events to demonware (And then unflasg all events)
void DataGathering::PostFlaggedEvents(bdOnlineUserID& demonwareUserID)
{
	//Do we have any data flagged for sending
	if(m_FlaggedEvents.CountBitsSet() == 0)
	{
		return;
	}

	//Create stream for all data
	static const int initialStreamSize=2048;
	Sys::C_MemoryStream stream(initialStreamSize);
	Sys::BinarySerialiserAutoEndian serial( stream );

	//First information shared between all records
	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	
	// GUID
	serial<<netGameVars.m_StartRaceData.mHostId;
	serial<<netGameVars.m_StartRaceData.mHostStartTime;
	// User ID
	serial<<demonwareUserID;

	//Then check and serialise data for all flagged records
	for(int i = 0; i < 32; i++)
	{
		if(m_FlaggedEvents.GetBitValue(i))
		{
			PostEvent(serial, (DATA_GATHERING_RECORD_TYPE)i);
		}
	}

	//Post the uber-event
	PostRecord(stream.GetBufferPtr(), stream.GetBufferSize(), EVENT_RECORD_CATEGORY_DATA_GATHERING_COMBINED_EVENTS);

	//And reset flagged records
	m_FlaggedEvents.ClearBits();
}

void DataGathering::PostEvent(Sys::BinarySerialiserAutoEndian& serial, DATA_GATHERING_RECORD_TYPE category)
{
	//Write the category of the data
	serial<<'[';
	serial<<(U8)category;
	serial<<'=';
	switch(category)
	{
		case DATA_GATHERING_RECORD_TYPE_VEHICLE_INFORMATION:
			PostVehicleInformation(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_RANK_UP:
			PostRankedUp(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_LOBBY_CHOICE:
			PostLobbyChoice(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_QUIT_RACE:
			PostQuitRace(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_MOD_SELECTION:
			PostModSelection(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_CHECKED_SESSION_BOARD:
			PostCheckedSessionBoard(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_HOST_MIGRATED:
			PostHostMigrated(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_VOTE_RESULTS:
			PostRouteVoteResults(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_CUSTOM_RACE_SETTINGS:
			PostCustomRaceSettings(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_CHALLENGE_COMPLETED:
			PostChallengeCompleted(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_AWARDS_GIVEN:
			PostAwardInformation(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_CAREER_INFORMATION:
			PostPlayerCareerInformation(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_POST_RACE:
			PostPostRaceStats(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_ON_TRACK_PICKUPS:
			PostOnTrackPickups(serial);
			break;
		case DATA_GATHERING_RECORD_TYPE_REGION_SETTING:
			PostRegionSetting(serial);
			break;
	}
	//Write the end of the data for this category
	serial<<'/';
	serial<<']';
}

void DataGathering::StoreVoteResults(U8 winningVotes, U32 winningRoute, U32 losingRoute, U8 losingVotes)
{
	mVoteResults.winningVotes = winningVotes;
	mVoteResults.winningRoute = winningRoute;
	mVoteResults.losingRoute = losingRoute;
	mVoteResults.losingVotes = losingVotes;

	//Don't flag. We will flag this to the host later (But everybody stores now in case of host migration).
}

void DataGathering::StoreSessionboardInfo(U8 position, U16 racePoints, U8 racesPlayed)
{
	mSessionboardStats.position = position;
	mSessionboardStats.racePoints = racePoints;
	mSessionboardStats.racesPlayed = racesPlayed;

	FlagPostEvent(DATA_GATHERING_RECORD_TYPE_CHECKED_SESSION_BOARD);
}

void DataGathering::StoreLobbyChoice(bool createdLobby)
{
	mCreatedLobby = createdLobby;

	FlagPostEvent(DATA_GATHERING_RECORD_TYPE_LOBBY_CHOICE);
}

void DataGathering::StoreTrackPickupInformation()
{
	if(IsRecordTypeActive(DATA_GATHERING_RECORD_TYPE_ON_TRACK_PICKUPS))
	{
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

		if(pLayer && pLayer->GetRaceLogicLayer())
		{
			//Delete existing data if any
			if(mTrackPickupData)
			{
				SAFE_DELETE_PTR(mTrackPickupData);
				mTrackPickupNumber = 0;
			}

			Btl::Map<U32, Actor::C_ActorHandle>& pickupsMap = pLayer->GetRaceLogicLayer()->mPickupPositionActorMap;
			//Store size and create structure
			mTrackPickupNumber = pickupsMap.Size();
			if(mTrackPickupNumber == 0)
			{
				return;
			}
			mTrackPickupData = new TrackPickupData[mTrackPickupNumber];

			Btl::Map<U32, Actor::C_ActorHandle>::Iterator it = pickupsMap.Begin();
			U32 p = 0;
			while( it != pickupsMap.End())
			{
				::Actors::PickupPositionActor* pActor = NULL;

				Actor::C_ActorHandle handle = (*it).GetValue();
				pActor = (::Actors::PickupPositionActor*)pLayer->GetRaceLogicLayer()->m_pGameActorList->GetActorPtr(handle);
				if(pActor)
				{
					mTrackPickupData[p].id = pActor->GetCreationData()->GetSorceryUid();
					mTrackPickupData[p].timesPickedUp = pActor->GetTimesPickedUpByLocalPlayer();
					mTrackPickupData[p].type = pActor->GetEnumPickupType();
				}
				else
				{
					mTrackPickupData[p].id = 0;
					mTrackPickupData[p].timesPickedUp = 0;
					mTrackPickupData[p].type = 0;
				}
				p++;
				it++;
			}
		}

		FlagPostEvent(DATA_GATHERING_RECORD_TYPE_ON_TRACK_PICKUPS);
	}
}

bool DataGathering::IsRecordTypeActive(U32 recordTypeValue)
{
	if(m_RecordsActive)
	{
		U32 activatedRecords = m_RecordsActive->m_ActivatedRecords;
		if(activatedRecords & (1 << recordTypeValue))
		{
			return true;
		}
	}

	return false;
}

void DataGathering::PostRecord(void *data, int size, EVENT_RECORD_CATEGORY category)
{
	m_eventManager.WriteEventRecordBin(data, size, category);
}

void DataGathering::PostRouteVoteResults(Sys::BinarySerialiserAutoEndian& serial)
{
	if(mVoteResults.winningRoute == 0)
	{
		return;
	}

	serial<<mVoteResults.winningVotes;
	serial<<mVoteResults.winningRoute;
	serial<<mVoteResults.losingRoute;
	serial<<mVoteResults.losingVotes;

	mVoteResults.Reset();
}

void DataGathering::PostPostRaceStats(Sys::BinarySerialiserAutoEndian& serial)
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	if(!pLayer || !pProfile)
	{
		return;
	}

	C_RaceModeBase* pRaceMode = pLayer->GetRaceModePtr();
	if(!pRaceMode)
	{
		return;
	}

	Game::C_LevelResult& levelResult = Game::C_Game::GetInstance()->GetLevelResult();
	NetVehicleData vehicleData;
	C_MpResults::Instance().GetPlayerResults(0,vehicleData);

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	if(levelState.mNumCars == 0)
	{
		return;	
	}

	//Level and Vehicle sorcery IDs
	serial<<levelState.mRoute;

	//Rank
	serial<<(U8)(levelState.mCars[0].mMultiplayerRank+1);

	//Game Mode
	serial<<(U8)pRaceMode->GetID();

	//Num vehicles wrecked
	serial<<(U16)levelResult.mCars[0].mStatistics.mNumVehiclesWrecked;

	//Pickups used/hit
	serial<<(U8)(E_PICKUP_TYPE_MAX-1);

	for(int i = 0; i < E_PICKUP_TYPE_MAX-1; i++)
	{
		serial<<(U16)levelResult.mCars[0].mStatistics.mNumPerksFired[i];
	}

	for(int i = 0; i < E_PICKUP_TYPE_MAX-1; i++)
	{
		serial<<(U16)levelResult.mCars[0].mStatistics.mNumPerksHit[i];
	}

	//Players at the start and end of the race
	MpVehiclesData& results = C_MpResults::Instance().m_LiveVehicleData;

	S8 playersAtStart = 0;
	S8 playersAtEnd = 0;
	for (S8 i = 0; i < results.Size(); i++)
	{
		if (results[i].m_State == E_VEHICLE_STATE_UNKNOWN)
		{
			continue;
		}

		playersAtStart++;

		if (results[i].m_State != E_VEHICLE_STATE_LEFT_RACE)
		{
			playersAtEnd++;
		}
	}

	serial<<playersAtStart;
	serial<<playersAtEnd;

	//Times wrecked
	serial<<(U16)levelResult.mCars[0].mStatistics.mNumTimesWrecked;

	//HoG types
	serial << (U8)E_HandOfGodResetType_MAX;
	for(int i = 0; i < E_HandOfGodResetType_MAX; i++)
	{
		serial<<(U8)levelResult.mCars[0].mStatistics.mNumHoGTimes[i];
	}

	//Final position
	serial<<(U8)C_MpResults::Instance().GetPlayerPosition(0);

	//Distance to finish when first players finished
	serial<<(float)levelResult.mCars[0].mStatistics.mDistToFirstOnFinish;

	//Motormash points
	serial<<(U16) Game::C_MpResults::Instance().m_SortedVehicleResults[0].m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];

	//Timed out? (As in didnt finish the race in time)
	if(levelState.mCars[0].mState == E_VEHICLE_STATE_TIMED_OUT)
	{
		serial << (U8)1;
	}
	else
	{
		serial << (U8)0;
	}

	//Timeout length
	serial << (U8)((Game::C_RaceModeMultiplayer_Racing*)pRaceMode)->GetInitialTimeoutTime();

	//Average lap time
	if(levelState.mLaps > 0)
	{
		serial<<(vehicleData.m_Statistics[UI::E_MultiplayerStat_RaceTime] / (F32)levelState.mLaps);
	}
	else
	{
		serial<<0.0f;
	}

	//Total fans player gained in race
	serial<<(U16)vehicleData.m_Statistics[UI::E_MultiplayerStat_Fans];

	if(pProfile)
	{
		//Showy Flourish fans
		const Game::MpRaceSummary& raceSummary = pProfile->GetRaceSummary();
		serial<<(U16)raceSummary.m_ShowyFlourishContribution;

		//Race finish bonus
		serial<<(U16)raceSummary.m_FinishBonus;

		//Fan favourite
		serial<<(U16)raceSummary.m_FanDemandContribution;

		//Challenge fans
		serial<<(U16)raceSummary.m_TotalChallengeFans;

		//Car reward fans
		serial<<(U16)raceSummary.m_CarRewardFanBonus;

		//Total fans
		serial<<(U16)raceSummary.m_TotalNewFans;
	}

	serial<<vehicleData.m_Statistics[UI::E_MultiplayerStat_LongestJump];
	serial<<vehicleData.m_Statistics[UI::E_MultiplayerStat_LongestDrift];
	serial<<vehicleData.m_Statistics[UI::E_MultiplayerStat_TopSpeed];
}

void DataGathering::PostOnTrackPickups(Sys::BinarySerialiserAutoEndian& serial)
{
	if(!mTrackPickupData || mTrackPickupNumber == 0)
	{
		return;
	}

	serial<<mTrackPickupNumber;

	for(U32 i = 0; i < mTrackPickupNumber; i++)
	{
		if(mTrackPickupData[i].id != 0 && mTrackPickupData[i].type >= 0 && mTrackPickupData[i].type < E_PICKUP_TYPE_MAX)
		{
			serial<<mTrackPickupData[i].id;
			serial<<mTrackPickupData[i].timesPickedUp;
			serial<<mTrackPickupData[i].type;
		}
		else
		{
			serial<<(U32)0;
			serial<<(U8)0;
			serial<<(U8)0;
		}
	}

	//Delete data
	if(mTrackPickupData)
	{
		SAFE_DELETE_PTR(mTrackPickupData);
		mTrackPickupNumber = 0;
	}
}

void DataGathering::PostRankedUp(Sys::BinarySerialiserAutoEndian& serial)
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	serial<<(U8)(pProfile->GetCurrentLevel()+1);
	serial<<pProfile->GetMultiplayerStatistics().mTotalPlaytime;
	serial<<pProfile->GetMultiplayerStatistics().mNumEventsPlayed;
}

void DataGathering::PostLobbyChoice(Sys::BinarySerialiserAutoEndian& serial)
{
	serial<<(U8)mCreatedLobby; //0 if they joined an existing one, otherwise 1
}

void DataGathering::PostQuitRace(Sys::BinarySerialiserAutoEndian& serial)
{
	//If we have quit the race then make sure the post race results aren't flagged
	m_FlaggedEvents.ClearBit(DATA_GATHERING_RECORD_TYPE_POST_RACE);

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayerManager& manager = services.PlayerManager();
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	serial<<levelState.mRoute;
	serial<<(U8)levelState.mRaceMode;
	serial<<(U8)manager.TotalSize();
}

void DataGathering::PostModSelection(Sys::BinarySerialiserAutoEndian& serial)
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
	Game::C_RaceBookLayer* rbl=C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if(!pProfile)
	{
		return;
	}

	bool isRanked = !rbl || !rbl->GetGameModePtr() || rbl->GetGameModePtr()->IsRanked();

	UI::E_MULTIPLAYER_LOADOUT_TYPE loadoutType = pProfile->GetLoadouts(isRanked).GetCurrentLoadOutType();
	UI::E_MultiplayerVehicleAbilities mod1 = pProfile->GetLoadouts(isRanked).GetAbilityInCurrent(UI::E_MULTIPLAYER_LOADOUT_SLOT1);
	UI::E_MultiplayerVehicleAbilities mod2 = pProfile->GetLoadouts(isRanked).GetAbilityInCurrent(UI::E_MULTIPLAYER_LOADOUT_SLOT2);
	UI::E_MultiplayerVehicleAbilities mod3 = pProfile->GetLoadouts(isRanked).GetAbilityInCurrent(UI::E_MULTIPLAYER_LOADOUT_SLOT3);

	serial<<(U8)loadoutType;
	serial<<(U8)mod1;
	serial<<(U8)mod2;
	serial<<(U8)mod3;
	serial<<levelState.mCars[0].mUpgrades.GetMultiplayerAbilities()->GetActivationTimes(UI::E_MULTIPLAYER_LOADOUT_SLOT1);
	serial<<levelState.mCars[0].mUpgrades.GetMultiplayerAbilities()->GetActivationTimes(UI::E_MULTIPLAYER_LOADOUT_SLOT2);
	serial<<levelState.mCars[0].mUpgrades.GetMultiplayerAbilities()->GetActivationTimes(UI::E_MULTIPLAYER_LOADOUT_SLOT3);
}

void DataGathering::PostVehicleInformation(Sys::BinarySerialiserAutoEndian& serial)
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if(!pGameProfile)
	{
		return;
	}

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	serial<<levelState.mCars[0].mCarID;
	serial<<pGameProfile->ResprayGet( levelState.mCars[0].mCarID, true );
}


void DataGathering::PostCheckedSessionBoard(Sys::BinarySerialiserAutoEndian& serial)
{
	serial<<mSessionboardStats.position;
	serial<<mSessionboardStats.racePoints;
	serial<<mSessionboardStats.racesPlayed;

	mSessionboardStats.Reset();
}

void DataGathering::PostHostMigrated(Sys::BinarySerialiserAutoEndian& serial)
{
	
}

void DataGathering::PostCustomRaceSettings(Sys::BinarySerialiserAutoEndian& serial)
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

	if(!pGameMode)
	{
		return;
	}

	const Game::C_CustomRace& customRace = pGameMode->GetCustomRace();

	//Normal settings
	serial<<customRace.GetRoute();
	serial<<customRace.GetLapLimit();
	serial<<(U16)customRace.GetTimeLimit();
	serial<<(U8)customRace.GetRaceMode();
	serial<<(U8)customRace.GetCarClass();

	//Advanced settings
	serial<<customRace.GetDamageMode();
	serial<<customRace.GetMods();
	serial<<customRace.GetAI();
	serial<<customRace.GetRespawn();
	serial<<customRace.GetUpgrades();
	serial<<customRace.GetHandicap();
	serial<<customRace.GetTimeout();
	serial<<customRace.GetLockToHostCar();
	serial<<customRace.GetPowerups();
	serial<<customRace.GetPowerupRespawnMode();
	serial<<customRace.GetRandomPowerups();
}

void DataGathering::PostChallengeCompleted(Sys::BinarySerialiserAutoEndian& serial)
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();

	if(!pProfile)
	{
		return;
	}

	Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
	Game::C_MultiplayerChallengeTracker::TChallengeList challengesCompleted = pProfile->GetChallengeSystem().GetTracker().GetChallengesComplete();
	
	serial<<(U8)(pProfile->GetCurrentLevel()+1);
	serial<<pProfile->GetMultiplayerStatistics().mTotalPlaytime;
	serial<<pProfile->GetMultiplayerStatistics().mNumEventsPlayed;
	serial<<levelState.mCars[0].mCarID;
	serial<<challengesCompleted.Size();

	for( Game::C_MultiplayerChallengeTracker::TChallengeList::Iterator it = challengesCompleted.Begin(); it != challengesCompleted.End(); it++ )
	{
		serial<<(U8)(*it).type;
		serial<<(U8)(*it).tier;
		serial<<(pProfile->GetChallengeSystem().IsPackComplete((*it).pack) ? (*it).pack : -1);
	}
}

void DataGathering::PostAwardInformation(Sys::BinarySerialiserAutoEndian& serial)
{
	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();
	if(!pLayer || !pGameMode)
	{
		return;
	}
	const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
	const Game::AwardSelection& awardSelection = awards.GetAwardSelection();

	//Make a note of awards given out and the ones we (Vehicle Idx 0) won, drew or lost (-1)
	for(U8 i = 0; i < E_MultiplayerAward_MaxAwards; i++)
	{
		serial<<(U8)awardSelection.m_Awards[i];
		if(awardSelection.m_Owners[i] == 0 || awardSelection.m_AwardTied[i])
		{
			serial<<awardSelection.m_AwardValues[i];
		}
		else
		{
			serial<<(F32)-1.0f;
		}
	}
}

void DataGathering::PostPlayerCareerInformation(Sys::BinarySerialiserAutoEndian& serial)
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
	Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
	
	if(!pProfile)
	{
		return;
	}

	serial<<(U8)(pProfile->GetCurrentLevel()+1);

	//Driver Score
	serial<<pProfile->GetMultiplayerStatistics().mNumRacePoints;

	//Powerup ratio
	if(pProfile->GetMultiplayerStatistics().mTotalPickupsFired > 0)
	{
		serial<<((float)pProfile->GetMultiplayerStatistics().mTotalPickupsHit / (float)pProfile->GetMultiplayerStatistics().mTotalPickupsFired) * 100.0f;
	}
	else
	{
		serial<<0;
	}

	serial<<pProfile->GetMultiplayerStatistics().mTotalPlaytime;
	serial<<pProfile->GetMultiplayerStatistics().mNumLifetimeFans;
	serial<<pProfile->GetMultiplayerStatistics().mAveragePosition;
	serial<<pProfile->GetMultiplayerStatistics().mCumulativePosition;
	serial<<pProfile->GetMultiplayerStatistics().mNumEventsPlayed;
	serial<<pProfile->GetMultiplayerStatistics().mNumEventsWon;
}

void DataGathering::PostRegionSetting(Sys::BinarySerialiserAutoEndian& serial)
{
	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	if(!pProfile)
	{
		return;
	}

	serial<<pProfile->mOptions.mMatchingRule;
}

#ifndef CFG_GOLD

Debug::Bool g_PostVehicleInfoWhenEnteringGarage("Multiplayer.DataGathering.PostVehicleInformationWhenEnteringGarage", false);
Debug::Bool g_PostRouteInfoWhenEnteringCustomRace("Multiplayer.DataGathering.PostRouteInformationWhenEnteringCustomRace", false);
Debug::Bool g_PostAwardInfoWhenEnteringAwardsScreen("Multiplayer.DataGathering.PostAwardInformationWhenEnteringAwardsScreen", false);
Debug::Bool g_PostChallengeInfoWhenEnteringChallengesScreen("Multiplayer.DataGathering.PostChallengeInformationWhenEnteringChallengesScreen", false);
Debug::Bool g_PostAbilityInfoWhenEnteringModshopScreen("Multiplayer.DataGathering.PostAbilityInformationWhenEnteringModshopScreen", false);

void DataGathering::PostVehicleSorceryInformation()
{
	if(!g_PostVehicleInfoWhenEnteringGarage)
	{
		return;
	}

	for (Btl::Map<U32, S32>::Iterator it = C_MultiplayerData::Instance().m_MultiplayerVehicleMap.Begin(); it != C_MultiplayerData::Instance().m_MultiplayerVehicleMap.End(); ++it)
	{
		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)it->GetKey() ];

		if (!pVehicle)
			continue;

		static const int initialStreamSize=2048;
		Sys::C_MemoryStream stream(initialStreamSize);
		Sys::BinarySerialiserAutoEndian serial( stream );

		serial<<(U8)SORCERY_DATA_TYPE_VEHICLE;
		serial<<pVehicle->ID;
		UI::C_WideString64 locName;
		locName = UI::C_TextStringID(pVehicle->loc_string.id);
		serial<<(U8)locName.Length();
		for(U8 i = 0; i < locName.Length(); i++)
		{
			serial<<(U8)locName[i];
		}
		serial<<(U8)pVehicle->carClass;

		m_eventManager.WriteEventRecordBin(stream.GetBufferPtr(), stream.GetBufferSize(), EVENT_RECORD_CATEGORY_SORCERY_INFORMATION);
	}
}

void DataGathering::PostRouteSorceryInformation()
{
	if(!g_PostRouteInfoWhenEnteringCustomRace)
	{
		return;
	}

	const XtArray< XtHandle<AmaxRoute> >* pRoutes = NULL;

	//First races...
	pRoutes = &C_MultiplayerData::Instance().m_pConfig->m_Routes;
	for (U8 i = 0; i < pRoutes->count; i++)
	{
		const AmaxRoute* pRoute = pRoutes->elements[i];

		if (!pRoute)
			continue;

		static const int initialStreamSize=2048;
		Sys::C_MemoryStream stream(initialStreamSize);
		Sys::BinarySerialiserAutoEndian serial( stream );

		serial<<(U8)SORCERY_DATA_TYPE_ROUTE;
		serial<<pRoute->ID;
		UI::C_WideString64 locName;
		locName = UI::C_TextStringID(pRoute->loc_string.id);
		serial<<(U8)locName.Length();
		for(U8 i = 0; i < locName.Length(); i++)
		{
			serial<<(U8)locName[i];
		}

		m_eventManager.WriteEventRecordBin(stream.GetBufferPtr(), stream.GetBufferSize(), EVENT_RECORD_CATEGORY_SORCERY_INFORMATION);
	}

	//Then arenas.
	pRoutes = &C_MultiplayerData::Instance().m_pConfig->m_Arenas;
	for (U8 i = 0; i < pRoutes->count; i++)
	{
		const AmaxRoute* pRoute = pRoutes->elements[i];

		if (!pRoute)
			continue;

		static const int initialStreamSize=2048;
		Sys::C_MemoryStream stream(initialStreamSize);
		Sys::BinarySerialiserAutoEndian serial( stream );

		serial<<(U8)SORCERY_DATA_TYPE_ROUTE;
		serial<<pRoute->ID;
		UI::C_WideString64 locName;
		locName = UI::C_TextStringID(pRoute->loc_string.id);
		serial<<(U8)locName.Length();
		for(U8 i = 0; i < locName.Length(); i++)
		{
			serial<<(U8)locName[i];
		}

		m_eventManager.WriteEventRecordBin(stream.GetBufferPtr(), stream.GetBufferSize(), EVENT_RECORD_CATEGORY_SORCERY_INFORMATION);
	}

}

void DataGathering::PostAwardSorceryInformation()
{
	if(!g_PostAwardInfoWhenEnteringAwardsScreen)
	{
		return;
	}

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	const Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();
	const MultiplayerAwards& awards = pGameMode->GetRaceAwards().GetAwards();

	for(U8 i = 0; i < awards.Size(); i++)
	{
		static const int initialStreamSize=2048;
		Sys::C_MemoryStream stream(initialStreamSize);
		Sys::BinarySerialiserAutoEndian serial( stream );

		serial<<(U8)SORCERY_DATA_TYPE_AWARD;
		serial<<(U8)awards[i].GetData()->mAwardId;
		UI::C_WideString64 locName;
		locName = UI::C_TextStringID(awards[i].GetData()->mAwardName.id);
		serial<<(U8)locName.Length();
		for(U8 i = 0; i < locName.Length(); i++)
		{
			serial<<(U8)locName[i];
		}

		m_eventManager.WriteEventRecordBin(stream.GetBufferPtr(), stream.GetBufferSize(), EVENT_RECORD_CATEGORY_SORCERY_INFORMATION);

	}
}

void DataGathering::PostChallengeSorceryInformation()
{
	if(!g_PostChallengeInfoWhenEnteringChallengesScreen)
	{
		return;
	}

	if( C_MultiplayerData::Instance().m_pConfig)
	{
		for(U8 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++)
		{
			XtHandle<MultiplayerChallengePack>& challengePack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];

			static const int initialStreamSize=2048;
			Sys::C_MemoryStream stream(initialStreamSize);
			Sys::BinarySerialiserAutoEndian serial( stream );

			serial<<(U8)SORCERY_DATA_TYPE_CHALLENGEPACK;
			serial<<(U8)challengePack->mType;
			UI::C_WideString64 locName;
			locName = UI::C_TextStringID(challengePack->mName.id);
			serial<<(U8)locName.Length();
			for(U8 i = 0; i < locName.Length(); i++)
			{
				serial<<(U8)locName[i];
			}

			m_eventManager.WriteEventRecordBin(stream.GetBufferPtr(), stream.GetBufferSize(), EVENT_RECORD_CATEGORY_SORCERY_INFORMATION);

			for(U8 i = 0; i < challengePack->mChallenges.count; i++)
			{
				static const int initialStreamSize=2048;
				Sys::C_MemoryStream stream(initialStreamSize);
				Sys::BinarySerialiserAutoEndian serial( stream );

				serial<<(U8)SORCERY_DATA_TYPE_CHALLENGE;
				serial<<(U8)challengePack->mChallenges[i]->mType;
				serial<<(U8)challengePack->mType;
				UI::C_WideString64 locName;
				locName = UI::C_TextStringID(challengePack->mChallenges[i]->mName.id);
				serial<<(U8)locName.Length();
				for(U8 i = 0; i < locName.Length(); i++)
				{
					serial<<(U8)locName[i];
				}

				m_eventManager.WriteEventRecordBin(stream.GetBufferPtr(), stream.GetBufferSize(), EVENT_RECORD_CATEGORY_SORCERY_INFORMATION);
			}
		}
	}
}

void DataGathering::PostAbilitySorceryInformation()
{
	if(!g_PostAbilityInfoWhenEnteringModshopScreen)
	{
		return;
	}

	for (U8 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_AbilitySlots.count; i++)
	{
		const MultiplayerAbilitySlot* pPack = C_MultiplayerData::Instance().m_pConfig->m_AbilitySlots[i];

		if(!pPack)
			continue;

		for(U8 i = 0; i < pPack->mAbilities.count; i++)
		{
			static const int initialStreamSize=2048;
			Sys::C_MemoryStream stream(initialStreamSize);
			Sys::BinarySerialiserAutoEndian serial( stream );

			serial<<(U8)SORCERY_DATA_TYPE_ABILITY;
			serial<<(U8)pPack->mAbilities[i]->mAbilityId;
			UI::C_WideString64 locName;
			locName = UI::C_TextStringID(pPack->mAbilities[i]->mAbilityName.id);
			serial<<(U8)locName.Length();
			for(U8 i = 0; i < locName.Length(); i++)
			{
				serial<<(U8)locName[i];
			}

			m_eventManager.WriteEventRecordBin(stream.GetBufferPtr(), stream.GetBufferSize(), EVENT_RECORD_CATEGORY_SORCERY_INFORMATION);
			}
	}
}
#endif //CFG_GOLD



}// namespace Game
