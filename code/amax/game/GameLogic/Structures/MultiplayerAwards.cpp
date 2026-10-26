#include "stdafx.h"
#include <GameLogic/Layers/AmaxWorldLayer.h>	// [Blur2PC]
#include "MultiplayerAwards.h"

#include "Structures\MpResults.h"

#include <GameLogic/GameData/MultiplayerData.h>
#include <algorithm>
#include <utilityai/utilityai.h>
#include <Core/Utility/Utility.h>
#include <GameLogic\GameModes\GameModeBase.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

#include "GameApp.h"
#include "Game/Game.h"

#ifndef CFG_GOLD
DEBUG_ENUM(UI::E_MultiplayerAward)
	DE_OPTION(UI::E_MultiplayerAward_SpeedDemon, 		"SpeedDemon")
	DE_OPTION(UI::E_MultiplayerAward_Tortoise, 			"Tortoise")
	DE_OPTION(UI::E_MultiplayerAward_FanTastic, 		"FanTastic")
	DE_OPTION(UI::E_MultiplayerAward_OldNews, 			"OldNews")
	DE_OPTION(UI::E_MultiplayerAward_Marksman, 			"Marksman")
	DE_OPTION(UI::E_MultiplayerAward_TriggerHappy, 		"TriggerHappy")
	DE_OPTION(UI::E_MultiplayerAward_Squirrel, 			"Squirrel")
	DE_OPTION(UI::E_MultiplayerAward_Gladiator, 		"Gladiator")
	DE_OPTION(UI::E_MultiplayerAward_Drifter, 			"Drifter")
	DE_OPTION(UI::E_MultiplayerAward_CrashTestDummy, 	"CrashTestDummy")
	DE_OPTION(UI::E_MultiplayerAward_BreakNeck, 		"BreakNeck")
	DE_OPTION(UI::E_MultiplayerAward_Agile, 			"Agile")
	DE_OPTION(UI::E_MultiplayerAward_Brawler, 			"Brawler")
	DE_OPTION(UI::E_MultiplayerAward_CleanRacer, 		"CleanRacer")
	DE_OPTION(UI::E_MultiplayerAward_WreckingBall, 		"WreckingBall")
	DE_OPTION(UI::E_MultiplayerAward_Survivor, 			"Survivor")
	DE_OPTION(UI::E_MultiplayerAward_Relentless, 		"Relentless")
	DE_OPTION(UI::E_MultiplayerAward_WriteOff, 			"WriteOff")
	DE_OPTION(UI::E_MultiplayerAward_PunchingBag, 		"PunchingBag")
	DE_OPTION(UI::E_MultiplayerAward_DeepSeaDiver, 		"DeepSeaDiver")
	DE_OPTION(UI::E_MultiplayerAward_Cliffhanger, 		"CliffHanger")
	DE_OPTION(UI::E_MultiplayerAward_Environmentalist, 	"Environmentalist")
	DE_OPTION(UI::E_MultiplayerAward_Mastermine,		"Mastermine")
	DE_OPTION(UI::E_MultiplayerAward_SilentRunning,		"SilentRunning")
	DE_OPTION(UI::E_MultiplayerAward_SafetyNet,			"SafetyNet")
	DE_OPTION(UI::E_MultiplayerAward_Scrambler,			"Scrambler")
	DE_OPTION(UI::E_MultiplayerAward_DecoyDrop,			"DecoyDrop")
	DE_OPTION(UI::E_MultiplayerAward_Batterer,			"Batterer")
	DE_OPTION(UI::E_MultiplayerAward_StableFrame,		"StableFrame")
	DE_OPTION(UI::E_MultiplayerAward_IronFist,			"IronFist")
	DE_OPTION(UI::E_MultiplayerAward_ButterFingers,		"ButterFingers")
	DE_OPTION(UI::E_MultiplayerAward_Dexterous,			"Dexterous")
	DE_OPTION(UI::E_MultiplayerAward_Dominator,			"Dominator")
	DE_OPTION(UI::E_MultiplayerAward_LongJumper,		"LongJumper")
	DE_OPTION(UI::E_MultiplayerAward_CloseEncounters,	"CloseEncounters")
	DE_OPTION(UI::E_MultiplayerAward_Booster,			"Booster")
	DE_OPTION(UI::E_MultiplayerAward_SafeHands,			"SafeHands")
	DE_OPTION(UI::E_MultiplayerAward_Defender,			"Defender")
	DE_OPTION(UI::E_MultiplayerAward_LitterBug,			"LitterBug")
	DE_OPTION(UI::E_MultiplayerAward_CrimsonCarapace,	"CrimsonCarapace")
	DE_OPTION(UI::E_MultiplayerAward_Hunter,			"Hunter")

END_DEBUG_ENUM

extern Debug::Logger &g_MultiplayerLog;
Debug::Bool g_DebugShowAwardSelectionInformation("Multiplayer.Awards.ShowSelectionInformation", false);
Debug::Bool g_DebugShowAllAwardInformation("Multiplayer.Awards.ShowAllAwardInformation", false);

Debug::Bool g_DebugForceAward("Multiplayer.Awards.ForceAwards", false);
Debug::Enum<UI::E_MultiplayerAward>	g_DebugAward("Multiplayer.Awards.Award", UI::E_MultiplayerAward_None);
Debug::Enum<UI::E_MultiplayerAward>	g_DebugAward2("Multiplayer.Awards.Award2", UI::E_MultiplayerAward_None);
Debug::Enum<UI::E_MultiplayerAward>	g_DebugAward3("Multiplayer.Awards.Award3", UI::E_MultiplayerAward_None);

/*
static void CalculateWinnersAndDesirability(void*)
{
	Game::MpVehiclesData& results = Game::C_MpResults::Instance().m_LiveVehicleData;
	S8 playersInRace = 0;
	for (S8 i = 0; i < results.Size(); i++)
	{
		if(results[i].m_State == Game::E_VEHICLE_STATE_UNKNOWN)
		{
			break;
		}

		playersInRace++;
	}

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	Game::C_GameModeNetworkRanked* pGameMode = (Game::C_GameModeNetworkRanked*)pLayer->GetGameModePtr();

	pGameMode->GetRaceAwards().CalculateWinners(playersInRace);
	pGameMode->GetRaceAwards().GetMultiplayerAwards().CalculateDesirability();
}

Debug::Function g_DebugCalculateWinnersAndDesirability( "Multiplayer.Awards.CalculateWinnersAndDesirability", CalculateWinnersAndDesirability, 0 );

static void GiftAward(void*)
{
	if(g_DebugAward != UI::E_MultiplayerAward_None)
	{
		Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		Game::C_GameModeNetworkRanked* pGameMode = (Game::C_GameModeNetworkRanked*)pLayer->GetGameModePtr();

		pGameMode->GetRaceAwards().GiftAward((UI::E_MultiplayerAward)g_DebugAward);
	}
}

Debug::Function g_DebugGiftAward( "Multiplayer.Awards.GiftAward", GiftAward, 0 );
*/
#endif

namespace Game
{

//Multiplayer Award

C_MultiplayerAward::C_MultiplayerAward(  const XtHandle<MultiplayerAwardData> pAwardData )
{
	BIZ_ASSERT( pAwardData );
	m_Handle = pAwardData;
}


C_MultiplayerAward::MultiplayerAwardStatInformation C_MultiplayerAward::CalculateWinner(S8 numPlayersAtEndOfRace) const
{
	// Cache values being used frequently
	const MultiplayerAwardData* awardData = GetData(); 
	const enum UI::E_MultiplayerStat theStatImTracking = awardData->mAwardStat;

	// Check we actually have some vehicles in the race....
	BIZ_ASSERT_MSG(numPlayersAtEndOfRace > 0, "C_MultiplayerAwards::CalculateWinner() - no live vehicles in race...");

	// Calculate join id of car that has won
	float currentBest  = Game::C_MpResults::Instance().m_SortedVehicleResults[0].m_Statistics[theStatImTracking];
	U8 winnerPosition = 0;
	bool isSplitscreen = false;
	Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	if(pGameModeBase && pGameModeBase->GetID() == Game::E_GameMode_SplitScreen)
	{
		isSplitscreen = true;
	}


	U8 winnerJoinId;

	if(isSplitscreen)
	{
		winnerJoinId    = 0;
	}
	else
	{
		winnerJoinId    = Game::C_MpResults::Instance().m_SortedVehicleResults[0].m_NetJoinId;
	}

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	bool awardWasTied = false;

	// For each vehicle in the race..  (num live vehicles)
	for (int i = 1 ; i < numPlayersAtEndOfRace; i++ )
	{
		// Cache this vehicle's stats locally
		const Game::NetVehicleData& vehicleData = Game::C_MpResults::Instance().m_SortedVehicleResults[i];


		//Dont give awards to the AI
		if(levelState.mCars[vehicleData.m_LocalIndex].mControlType == E_VEHICLE_CONTROL_TYPE_AI)
		{
			continue;
		}

		// See if this bad boy's dropped out, we know that everyone after them 
		// must too have dropped out so no need to carry on
		if(vehicleData.m_State == E_VEHICLE_STATE_LEFT_RACE)
			break;

		float newScore =  vehicleData.m_Statistics[theStatImTracking];

		if(newScore == currentBest)
			awardWasTied = true;

		// 'best' depends on wether higher or lower is considered better
		if(m_Handle->mAwardStatSortingStrategy == UI::E_MultiplayerStatSortingStrategy_HighestWins)	
		{
			// If this vehicle's score is higher.. its the new best
			if( newScore > currentBest )
			{
				currentBest = newScore; 
				winnerPosition = i;
				awardWasTied = false;
				if(isSplitscreen)
				{
					winnerJoinId = vehicleData.m_LocalIndex;
				}
				else
				{
					winnerJoinId = vehicleData.m_NetJoinId;
				}
			}
		}
		else
		{
			// If this vehicle's score is higher.. its the new best
			if(  newScore < currentBest )
			{
				currentBest = newScore; 
				winnerPosition = i; 
				awardWasTied = false;
				if(isSplitscreen)
				{
					winnerJoinId = vehicleData.m_LocalIndex;
				}
				else
				{
					winnerJoinId = vehicleData.m_NetJoinId;
				}
			}
		}
	}	

	MultiplayerAwardStatInformation winnerInfo;
	winnerInfo.joinId = winnerJoinId;
	winnerInfo.statValue = currentBest;
	winnerInfo.finishPosition = winnerPosition;
	winnerInfo.tied = awardWasTied;

	return winnerInfo;
}

const MultiplayerAwardData* C_MultiplayerAward::GetData() const
{
	BIZ_ASSERT_MSG( m_Handle, "Award has not been created properly" );

	return m_Handle.Object();
}


//Multiplayer Awards

C_MultiplayerAwards::C_MultiplayerAwards()
{
	m_Random.Seed( (U32)Time::GetCurrentTime() );
}

//Pre-cache all of the possible awards
void C_MultiplayerAwards::Init()
{
	ResetSelection();

	//For each piece of award data create the award
	for( int award = 0; award < C_MultiplayerData::Instance().m_pConfig->m_MultiplayerAwards.count; award++ )
	{
		XtHandle<MultiplayerAwardData> hAward = C_MultiplayerData::Instance().m_pConfig->m_MultiplayerAwards[award];

		m_Awards[hAward.Object()->mAwardId] = C_MultiplayerAward( hAward );
	}
}

//Take away everyones awards ready for the next race
void C_MultiplayerAwards::ResetSelection()
{
	m_AwardsToGive = 0;
	m_AwardNumber = 0;
	m_CurrentSelection.Reset();
}

const C_MultiplayerAward& C_MultiplayerAwards::GetAward(const UI::E_MultiplayerAward award) const
{
	return m_Awards[award];
}

//Give an award to somebody
void C_MultiplayerAwards::GiftAward( const S32 awardIndex )
{
#ifndef CFG_GOLD
	if(g_DebugShowAwardSelectionInformation)
	{
		LOGDEBUG(g_MultiplayerLog, "Award with icon name %s chosen. Won by netJoinID %i with a score of %f. Desirability was %f.",
			GetAward(m_AwardResults[awardIndex].mAwardType).GetData()->mIconName.Text(), m_AwardResults[awardIndex].mJoinID, 
			m_AwardResults[awardIndex].mWinnersScore, m_AwardResults[awardIndex].mDesirability);
	}
#endif
	m_CurrentSelection.m_Owners[m_AwardNumber] = m_AwardResults[awardIndex].mJoinID;
	m_CurrentSelection.m_Awards[m_AwardNumber] = m_AwardResults[awardIndex].mAwardType;
	m_CurrentSelection.m_AwardValues[m_AwardNumber] = m_AwardResults[awardIndex].mWinnersScore;
	m_CurrentSelection.m_AwardTied[m_AwardNumber] = m_AwardResults[awardIndex].mTied;

	if(m_AwardNumber++ == m_AwardsToGive)
	{
		//Start again from position 0
		m_AwardNumber = 0;
	}
}

//Return an array of results for each type (Column = player net join ID, row = award type)
void C_MultiplayerAwards::CalculateWinners(S8 numPlayersAtEndOfRace)
{ 
	m_NumPlayersAtEndOfRace = numPlayersAtEndOfRace;

	for(int i = 0; i < UI::E_MultiplayerAward_Count; i++)
	{
		const MultiplayerAwardData *data = GetAward((UI::E_MultiplayerAward)i).GetData();
		C_MultiplayerAward::MultiplayerAwardStatInformation winnerStatInfo = m_Awards[(UI::E_MultiplayerAward)i].CalculateWinner(numPlayersAtEndOfRace);
		m_AwardResults[i].mJoinID = winnerStatInfo.joinId;
		m_AwardResults[i].mWinnersScore = winnerStatInfo.statValue;
		m_AwardResults[i].mFinishPosition = winnerStatInfo.finishPosition;
		m_AwardResults[i].mTied = winnerStatInfo.tied;
		m_AwardResults[i].mAwardCategory = data->mAwardCategory;
		m_AwardResults[i].mAwardType = data->mAwardId;
	}
}

//Actually decide on how to hand out the awards
const AwardSelection& C_MultiplayerAwards::CalculateAwards( U32 seed )
{
	m_CurrentSelection.Reset();

	m_Random.Seed(seed);

	//Until all the awards are done lets just add the completed ones available for selection here
	Btl::Vector< UI::E_MultiplayerAward > availableAwards;
	availableAwards.PushBack(UI::E_MultiplayerAward_FanTastic);

	//Reset the winners for every type
	for(int i = 0; i < UI::E_MultiplayerAward_Count; i++)
	{
		m_AwardResults[i].Init();
	}

	//Assuming we want to decide how many awards to do based on the number of players that started  the race...
	MpVehiclesData& results = C_MpResults::Instance().m_LiveVehicleData;
	Game::Actors::RaceLogicCore*	pRaceLogicCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	S8 vehiclesAtEndOfRace = 0;
	S8 humansAtEndOfRace = 0;
	S8 aiAtEndOfRace = 0;
	for (S8 i = 0; i < results.Size(); i++)
	{
		if( results[i].m_State == E_VEHICLE_STATE_UNKNOWN )
		{
			break;	
		}

		if( !pRaceLogicCore->IsVehicleAIControlled(i) )
		{
			humansAtEndOfRace++;
		}
		else
		{
			aiAtEndOfRace++;
		}
	}

	// total up
	vehiclesAtEndOfRace = humansAtEndOfRace + aiAtEndOfRace;

	// And calculate the new winners
	CalculateWinners(vehiclesAtEndOfRace);

	// Work out which award distribution specification to use based on the current number of players in the race
	MultiplayerAwardDistributionSpec specToFollow = GetAwardDistributionSpec( humansAtEndOfRace );

	// Work out if there is a maximum limit imposed on the number of awards each player can win
	int maxAwardsPerPlayer = specToFollow.mMaxNumAwardsPerPlayer;

	// Work out num awards we want to display on screen
	m_AwardsToGive = specToFollow.mNumAwardsToGift + specToFollow.mNumSpoonsToGift + specToFollow.mNumRandomAwardsOrSpoonsToGift;
	BIZ_ASSERT_MSG(m_AwardsToGive <= E_MultiplayerAward_MaxAwards,"C_MultiplayerAwards::CalculateAwards() < Error.. award distribution spec wants to give out more awards than E_MultiplayerAward_MaxAwards..."); 

	// Work out if the awards are desirable or not based on the winning score
	CalculateDesirability();

	// Disable any awards that dont make sense (i.e. winner of 'Most accurate' was player one but with accuracy of 0)
	DisableNonsensicalAwards(); 

	// Tally up how many awards each player has won, and, if there is a score limit imposed, put any multiple wins above what we want to allow in the 'extremely undesirable , last resort' list. 
	if( maxAwardsPerPlayer >= 1)
	{
		EnforceWinLimit(vehiclesAtEndOfRace ,maxAwardsPerPlayer); 
	}

	// Actually select the awards we want to show!
#ifndef CFG_GOLD
	if(g_DebugForceAward)
	{
		SelectDesirableAwardsDebug();
	}
	else
	{
#endif
	SelectDesirableAwards( specToFollow.mNumAwardsToGift, specToFollow.mNumSpoonsToGift, specToFollow.mNumRandomAwardsOrSpoonsToGift );
#ifndef CFG_GOLD
	}
#endif


	return m_CurrentSelection;
}


void C_MultiplayerAwards::EnforceWinLimit(const U8 numPlayers, const U8 maxAwardsPerPlayer)
{
	// Step 1) - Build a map of finish pos -> vehicle ID
	Btl::Map<U8,U8> LocalIdToFinishPosMap;
	BuildLocalIdToFinishPosMap(LocalIdToFinishPosMap);

	// Step 2) - Tally up how which awards each player has won
	Btl::Vector< Btl::Vector<UI::E_MultiplayerAward > > awardsWonArray; 
	awardsWonArray.Resize(numPlayers);

	TallyUpWinsPerPlayer(&awardsWonArray, LocalIdToFinishPosMap, numPlayers);

	// Step 3) - For each player...
	for(Btl::Vector< Btl::Vector<UI::E_MultiplayerAward> >::Iterator iter = awardsWonArray.Begin(); iter != awardsWonArray.End(); iter++)
	{
		// If that player has won more awards than is allowed
		Btl::Vector<UI::E_MultiplayerAward>* awardsWon = &(*iter);
		if(!awardsWon->Empty() && awardsWon->Size() > maxAwardsPerPlayer)
		{
			DeprioritiseInvalidAwards(awardsWon,maxAwardsPerPlayer);
		}
	}
}

void  C_MultiplayerAwards::BuildLocalIdToFinishPosMap( Btl::Map<U8,U8>& LocalIdToFinishPosMap )
{
	Game::MpVehiclesData* pSortedResults = &Game::C_MpResults::Instance().m_SortedVehicleResults;
	BIZ_ASSERT_MSG(!pSortedResults->Empty(),"BuildFinishPosToLocalIdMap( Btl::Map<U8,U8>& finishPosToLocalIdMap ) < Game::C_MpResults::Instance().m_SortedVehicleResults empty");
	for(U8 i = 0; i < pSortedResults->Size(); i++)
	{
		// Map the player's finish pos to their local id
		LocalIdToFinishPosMap.Insert((*pSortedResults)[i].m_LocalIndex, i);
	}
}

void  C_MultiplayerAwards::DeprioritiseInvalidAwards(Btl::Vector<UI::E_MultiplayerAward>* winnersAwards, const U8 maxAwardsToAllowThisWinner)
{
	// FOR NOW (First pass solution) - just randomly disable awards this player has won until only acceptable limit remain -
	U8 numAwardsToDeprioritise = winnersAwards->Size() - maxAwardsToAllowThisWinner;
	for(U8 i = 0; i < numAwardsToDeprioritise; i++)
	{
		// Randomly select one of the awards this player has earned and disable it!
		int randAwardIndexToDisable = m_Random.Rand( winnersAwards->Size() );
		UI::E_MultiplayerAward awardToDisable = (*winnersAwards)[randAwardIndexToDisable];

		// Disable that award..
		BIZ_ASSERT_MSG(awardToDisable <= m_AwardResults.Size(), "DeprioritiseInvalidAwards(Btl::Vector<UI_E_MultiplayerAward>* winnersAwards, const U8 maxAwardsToAllowThisWinner) < trying to disable an award with enum larger than m_AwardResults.Size()");
		m_AwardResults[ awardToDisable ].mDefinedYet = false;
		winnersAwards->EraseByIndex(randAwardIndexToDisable);
	}
}

void C_MultiplayerAwards::TallyUpWinsPerPlayer( Btl::Vector< Btl::Vector<UI::E_MultiplayerAward> >* numWinsArray, Btl::Map<U8,U8>& LocalIdToFinishPosMap, const U8 numPlayers )
{
	BIZ_ASSERT_MSG(numPlayers >= 1 ,"C_MultiplayerAwards::TallyUpNumWinsPerPlayer() < Num players is zero...");
	bool isSplitscreen = false;
	Game::C_GameModeBase *pGameModeBase = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	if(pGameModeBase && pGameModeBase->GetID() == Game::E_GameMode_SplitScreen)
	{
		isSplitscreen = true;
	}
	
	for(int awardNum = 0; awardNum < m_AwardResults.Size(); awardNum++)
	{
		C_MultiplayerAward* award = &m_Awards[awardNum]; 
		if( m_AwardResults[awardNum].mDefinedYet && !m_AwardResults[awardNum].mTied ) // If already disabled, or tied.. dont worry about it
		{
			U8 localPlayerId = m_AwardResults[awardNum].mJoinID; // This is join id in online, or local id if splitscreen

			// If network game then convert join id to local id
			if(!isSplitscreen)
			{
				localPlayerId = GetLocalIndexFromJoinId(m_AwardResults[awardNum].mJoinID);

				// Find the finish position this player came in
				Btl::Map<U8,U8>::ConstIterator iter = LocalIdToFinishPosMap.Find(localPlayerId); 
				BIZ_ASSERT_MSG(iter != LocalIdToFinishPosMap.End(), "C_MultiplayerAwards::TallyUpWinsPerPlayer < could not find finish pos for player in map");

				// Push back the enum of the award this player won BUG:33383  BY FINISH POS - so same on all kits
				(*numWinsArray)[iter->GetValue()].PushBack(award->GetData()->mAwardId);
			}
			else
			{
				(*numWinsArray)[localPlayerId].PushBack(award->GetData()->mAwardId);
			}
		}
	}	
}

const XtHandle<MultiplayerAwardDistributionSpec> C_MultiplayerAwards::GetAwardDistributionSpec( U8 numPlayersToDistributeTo ) const
{
	XtHandle<MultiplayerAwardDistributionSpec> hSpecToUse;
	bool foundValidSpec = false; 

	BIZ_ASSERT_MSG(C_MultiplayerData::Instance().m_pConfig->m_MultiplayerAwardDistributionSpecs.count > 0, "C_MultiplayerAwards::GetAwardDistributionSpec() < Error no Award Distribution specs present in sorcery..");

	for( int spec = 0; spec < C_MultiplayerData::Instance().m_pConfig->m_MultiplayerAwardDistributionSpecs.count; spec++ )
	{
		XtHandle<MultiplayerAwardDistributionSpec> hSpec = C_MultiplayerData::Instance().m_pConfig->m_MultiplayerAwardDistributionSpecs[spec];

		if(hSpec->mMinNumPlayersRequired <= numPlayersToDistributeTo)
		{
			hSpecToUse = hSpec;
			foundValidSpec = true;
		}
		else
		{
			// player requirement only gets higher from here on in.. we are done. 
			break;
		}
		
	}

	BIZ_ASSERT_MSG(foundValidSpec,"C_MultiplayerAwards::GetAwardDistributionSpec() - No valid award distribution specification could be obtained from sorcery...");
	return hSpecToUse;
}

void C_MultiplayerAwards::DisableNonsensicalAwards()
{
	// If highest score is best.... and the winner won with a score of 0 (i.e. won by default)
	// does not make sense to give them this award.. (e.g. most accurate.. but was a clean non-powerup race!)
	for(int i = 0; i < m_AwardResults.Size(); i++)
	{
		C_MultiplayerAward* award = &m_Awards[i]; 
		if( m_AwardResults[i].mWinnersScore == 0 &&
			award->GetData()->mAwardStatSortingStrategy == UI::E_MultiplayerStatSortingStrategy_HighestWins )
		{
			// Disable as makes no sense to award this!
			m_AwardResults[i].mDefinedYet = false;
		}
	}

	// NOTE: TODO: could improve this check if needed, i.e. with each award record in sorcery, provide a 'validity check' score value and
	// an enum to indicate wether a valid score must be 'greater than' or 'less than' this (i.e. E_GreaterThan and score of 0.0f for 'most Accurate' award 
	// would ensure the award was only valid, and therefore awarded, if somebody actually fired a shot and hit)

}

//When we receive the award selection from the host
void C_MultiplayerAwards::SetAwards(const AwardSelection& awardSelection)
{
	m_CurrentSelection = awardSelection;
}

void C_MultiplayerAwards::SelectDesirableAwards( U8 awardsToChoose, U8 spoonsToChoose, U8  randomAwardsOrSpoonsToChoose )
{
	//Sort awards by desirability
	if(!m_AwardResults.Empty())
	{
		std::sort(m_AwardResults.Begin(), m_AwardResults.End(), SortAwardsByDesirability());
	}

	// Build some lists, containing the indices of the desirable awards and spoons, and undesirable awards and spoons. 
	Btl::Vector< U8 > theDesirableAwardIndices;
	Btl::Vector< U8 > theDesirableSpoonIndices;	
	Btl::Vector< U8 > theUndesirableAwardIndices;
	Btl::Vector< U8 > theUndesirableSpoonIndices;	
	BuildSpoonAndAwardLists(&theDesirableAwardIndices,&theDesirableSpoonIndices,&theUndesirableAwardIndices,&theUndesirableSpoonIndices);

	// Gift X Awards
	GiftRequiredNumberFromSelections( awardsToChoose, &theDesirableAwardIndices, &theUndesirableAwardIndices );

	for(int i = 0; i < randomAwardsOrSpoonsToChoose; i++)
	{
		// Always gift X random spoon or award
		GiftRandomAwardOrSpoon(&theDesirableAwardIndices,&theDesirableSpoonIndices,&theUndesirableAwardIndices,&theUndesirableSpoonIndices); 
	}

	// Gift X Spoons
	GiftRequiredNumberFromSelections( spoonsToChoose, &theDesirableSpoonIndices, &theUndesirableSpoonIndices );	
}

#ifndef CFG_GOLD
//DEBUG FUNCTION
void C_MultiplayerAwards::SelectDesirableAwardsDebug()
{
	Btl::Vector< U8 > theDesirableAwardIndices;
	Btl::Vector< U8 > theUndesirableAwardIndices;	

	theDesirableAwardIndices.PushBack((U8)g_DebugAward);
	theDesirableAwardIndices.PushBack((U8)g_DebugAward2);
	theDesirableAwardIndices.PushBack((U8)g_DebugAward3);

	// Gift X Awards
	U8 numToChoose = 3;
	GiftRequiredNumberFromSelections( numToChoose, &theDesirableAwardIndices, &theUndesirableAwardIndices );
}
#endif

void C_MultiplayerAwards::GiftRequiredNumberFromSelections(U8& numToGift, Btl::Vector< U8 >* desirables, Btl::Vector< U8 >* undesirables)
{
	BIZ_ASSERT_MSG(desirables,"GiftRequiredNumberFromSelections() < pointer to vector of desirable awards is NULL");
	BIZ_ASSERT_MSG(undesirables,"GiftRequiredNumberFromSelections() < pointer to vector of undesirable awards is NULL");

	// Gift desirables where possible)
	if(!desirables->Empty())
	{
		GiftRandomsFromSelection(desirables, numToGift, undesirables);
	}

	// Still more to be gifted? (numToGift variable was passed in by reference, so will be 0 if no more remain!)
	if(numToGift > 0 && !undesirables->Empty())
	{
		GiftRandomsFromSelection(undesirables, numToGift);
	}
}

void C_MultiplayerAwards::GiftRandomAwardOrSpoon( Btl::Vector< U8 >* theDesirableAwardIndices, Btl::Vector< U8 >* theDesirableSpoonIndices,
												  Btl::Vector< U8 >* theUndesirableAwardIndices, Btl::Vector< U8 >* theUndesirableSpoonIndices)
{
	int spoonOrAward = m_Random.Rand( 2 );
	BIZ_ASSERT_MSG(spoonOrAward < 2, "GiftRandomAwardOrSpoon < Random() is returning value greater than valid range max");
	U8 randomNumToAward = 1;
	switch(spoonOrAward )
	{
		case 0:
		{
			// PlanA: give something random from Spoons pool
			GiftRequiredNumberFromSelections( randomNumToAward, theDesirableSpoonIndices, theUndesirableSpoonIndices );

			// PlanB: give something random from awards pool if Plan A wasnt successful
			if(randomNumToAward > 0)
			{
				GiftRequiredNumberFromSelections( randomNumToAward, theDesirableSpoonIndices, theUndesirableSpoonIndices );
			}
			break;
		}
		case 1:
		{
			GiftRequiredNumberFromSelections( randomNumToAward, theDesirableAwardIndices, theUndesirableAwardIndices );
			if(randomNumToAward > 0)
			{
				GiftRequiredNumberFromSelections( randomNumToAward, theDesirableAwardIndices, theUndesirableAwardIndices );
			}
			break;
		}
	}

		//Ok, something has gone wrong, so reset to giving out the single award of breakneck
		if(randomNumToAward > 0)
		{
			m_CurrentSelection.Reset();
			m_AwardNumber = 0;
			for(int i = 0; i < m_AwardResults.Size(); i++)
			{
				if(m_AwardResults[i].mAwardType == UI::E_MultiplayerAward_BreakNeck)
				{
					GiftAward(i);
					break;
				}
			}
		}
}

void C_MultiplayerAwards::GiftRandomsFromSelection(Btl::Vector< U8 >* selection, U8& countRemaining,  Btl::Vector< U8 >* otherSelection)
{
	if(!selection)
	{
		return;
	}

	while(countRemaining > 0 && !selection->Empty())
	{
		// From the list, Gift a random award
		U8  numAwardsToChooseFrom = selection->Size();
		int selectionIndexToTry = m_Random.Rand( numAwardsToChooseFrom );
		
		U8 awardIdxChosen = (*selection)[selectionIndexToTry];

		//Might not always want to spoon losers
		if(GetAward((UI::E_MultiplayerAward)awardIdxChosen).GetData()->mReducedProbabilityPositionPercentage > 0)
		{
			bool reduceChance = m_NumPlayersAtEndOfRace / 100 * (100-GetAward((UI::E_MultiplayerAward)awardIdxChosen).GetData()->mReducedProbabilityPositionPercentage) <  m_AwardResults[awardIdxChosen].mFinishPosition;

			if(reduceChance)
			{
				//Remove from list
				selection->EraseByIndex(selectionIndexToTry); 

				//Add to undesirable spoons list
				otherSelection->PushBack(awardIdxChosen);

				//And carry on
				continue;
			}
		}

		GiftAward(awardIdxChosen);  // This also increments m_CurrentAward

		// Flag so not selected again
		m_AwardResults[awardIdxChosen].mDefinedYet = false;

		// Remove index from this list, its no longer a valid award to choose as its now been chosen!
		selection->EraseByIndex(selectionIndexToTry); 

		// Decrement our count!
		countRemaining--; 
	}
}

void C_MultiplayerAwards::BuildSpoonAndAwardLists(Btl::Vector< U8 >* desirableAwards, Btl::Vector< U8 >* desirableSpoons,
												  Btl::Vector< U8 >* undesirableAwards, Btl::Vector< U8 >* undesirableSpoons)
{
	for(int i = 0; i < m_AwardResults.Size(); i++)
	{
		if(!m_AwardResults[i].mDefinedYet)
		{
			// Dont even look at this award it isnt applicable here!
			continue;
		}

		// Is this award desirable?
		if(m_AwardResults[i].mDesirability >= 100.0f) 
		{
			if(m_AwardResults[i].mAwardCategory == UI::E_MultiplayerAwardCategory_WoodenSpoon)
			{
				// If desirable spoon push index into spoon list, setting false to indicate not yet selected
				desirableSpoons->PushBack(i);
			}
			else
			{
				// If desirable Award push index into award list, setting false to indicate not yet selected
				desirableAwards->PushBack(i);
			}
		}
		else
		{
			if(m_AwardResults[i].mAwardCategory == UI::E_MultiplayerAwardCategory_WoodenSpoon)
			{
				// If undesirable spoon push index into spoon list, setting false to indicate not yet selected
				undesirableSpoons->PushBack(i);
			}
			else
			{
				// If undesirable Award push index into award list, setting false to indicate not yet selected
				undesirableAwards->PushBack(i);
			}
		}
	}
}

//Calculates how "cool" a stat would be to show. Higher = Better.
void C_MultiplayerAwards::CalculateDesirability()
{
	//Is motormash?
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	bool isMotormash = (levelState.mRaceMode == Game::E_RaceMode_Multiplayer_Destruction);

	for( int i = 0; i < m_AwardResults.Size(); i++ )
	{
		//Get award struct
		AwardResult &awardResult = m_AwardResults[i];

		//Get award data
		const MultiplayerAwardData *data = GetAward(awardResult.mAwardType).GetData();

		if(data->mAwardId == UI::E_MultiplayerAward_None)
		{
			continue;
		}

		//If its disabled then move on
		if(data->mDisabledInMotormash && isMotormash ||
		   data->mDisabledInRace && !isMotormash  )
		{
			continue;
		}

		//If spline data is required but we are in motormash then we dont want to use this so skip
		if(data->mRequiresAISplineData && isMotormash)
		{
			continue;
		}

		F32 threshold = data->mThresholdValue;

		if(isMotormash)
		{
			threshold *= data->mMotormashThresholdMultiplier;
		}
		else
		{
			threshold *= data->mRaceThresholdMultiplier;
		}

		F32 desirability = 0.0f;

		// Make sure we avoid a div by zero
		if(threshold > 0.0f)
		{
			desirability = (100.0f / threshold) * awardResult.mWinnersScore;
		}
		else
		{
			// Set to max when threshold value is 0.0f; If 0 then always applies. 
			desirability  = 200.0f; 
		}

		//Cap at 200 (So range becomes 0-200 with 100 being the threshold)
		if(desirability > 200.0f)
		{
			desirability = 200.0f;
		}

		//If its a lowest wins then we need to adjust the desirability
		if(data->mAwardStatSortingStrategy == UI::E_MultiplayerStatSortingStrategy_LowestWins)
		{
			//We don't really want 0 percent accuracy to show up for this so make it undesirable
			if(data->mAwardStat == UI::E_MultiplayerStat_Accuracy && awardResult.mWinnersScore <= 0.0001f)
			{
					desirability += 200;
					continue;
			}
			desirability = 200 - desirability;
		}

		awardResult.mDesirability = desirability;

		//If we get to the end then the award has been defined (And can be considered for use)
		awardResult.mDefinedYet = true;
	}

#ifndef CFG_GOLD
	if(g_DebugShowAllAwardInformation)
	{
		for( int i = 0; i < m_AwardResults.Size(); i++ )
		{
			LOGDEBUG(g_MultiplayerLog, "Award information %s: Won by netJoinID %i with a score of %f. Desirability was %f. Desirable: %i", GetAward(m_AwardResults[i].mAwardType).GetData()->mIconName.Text(), m_AwardResults[i].mJoinID, m_AwardResults[i].mWinnersScore, m_AwardResults[i].mDesirability, m_AwardResults[i].mDefinedYet);
		}
	}
#endif
}

void C_MultiplayerAwards::ConvertToLocalIndices()
{
	//	Override the stored net join IDs with the local index of the vehicle that was given the award
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	for(int i = 0; i < E_MultiplayerAward_MaxAwards; i++)
	{
		if(m_CurrentSelection.m_Owners[i] != 0)
		{
			for(U32 k = 0; k < levelState.mNumCars; k++)
			{
				if(levelState.mCars[k].mNetJoinId == m_CurrentSelection.m_Owners[i])
				{
					m_CurrentSelection.m_Owners[i] = k;
					break;
				}
			}
		}
	}
}

// If the local player was tied with anyone else for one of the selected awards, we make them the owner so that they are suitably impressed with themselves.
void C_MultiplayerAwards::ResolveTiedAwards()
{
	U8 localIndex = 0;
	NetVehicleData resultsData;
	Game::C_MpResults::Instance().GetPlayerResults(localIndex, resultsData);

	for(int i = 0; i < E_MultiplayerAward_MaxAwards; i++)
	{
		if(m_CurrentSelection.m_AwardTied[i] && m_CurrentSelection.m_Owners[i] != localIndex)	//	Only resolve ties if it was actually a tie, and if we didn't already get awarded it
		{
			C_MultiplayerAward award = GetAward(m_CurrentSelection.m_Awards[i]);
			const MultiplayerAwardData* awardData = award.GetData(); 
			const enum UI::E_MultiplayerStat theStatImTracking = awardData->mAwardStat;
			if(resultsData.m_Statistics[theStatImTracking] == m_CurrentSelection.m_AwardValues[i])
			{
				m_CurrentSelection.m_Owners[i] = localIndex;
			}
		}
	}
}

} //End namespace Game
