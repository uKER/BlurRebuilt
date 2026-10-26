#include "stdafx.h"
#include <Network/Misc/NetDefines.h>	// [Blur2PC]
#include <Ui/Game/Multiplayer/MultiplayerDefines.h>	// [Blur2PC]
#include <Game/Game.h>	// [Blur2PC]
#include <GameLogic/Profile/GameProfileManager.h>	// [Blur2PC]

#include "GameLogic\Profile\MultiplayerProfile.h"
#include <Game\Achievements\AchievementSystem.h>
#include "Structures\MpResults.h"

extern Debug::Logger &gLUAMessageLogger;
Debug::Int g_MultiplayerProfileFanOverrideNum("Multiplayer.PrimaryProfile.OverrideFansNum", 0, Debug::Range(0, 1000000));
Debug::Function g_MultiplayerProfileFanOverride("Multiplayer.PrimaryProfile.OverrideFans", Game::C_MultiplayerProfile::DebugOverrideFans, 0);
Debug::Logger& g_MultiplyerUnlocksLog = Debug::GetLogger("Amax.Multiplayer.Unlocks");
Debug::Bool s_dumpMultiplayerUnlocks("Multiplayer.PrimaryProfile.DumpUnlocks", true );

// Daily challenge overrides
Debug::Bool g_OverrideDailyChallengeTimeLapse("Multiplayer.Challenges.DailyChallengeTimeLapseOverride", false ); 
Debug::Int  g_DailyChallengeTimeLapseValueInMinutes("Multiplayer.Challenges.DailyChallengeTimeLapseOverrideInMinutes", 5, Debug::Range(1, 60));

// Constant race bonus data
extern Debug::Bool g_DebugShowMpAbilityActivateSpam;
extern Debug::Logger &g_MultiplayerLog;

Debug::Int g_MaxLegendLevel("Multiplayer.PrimaryProfile.MaxLegendLevel", 10);

DEBUG_ENUM(UI::E_MULTIPLAYER_CHALLENGE_TYPE)
	//Ace challenges
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_ULTIMATE_COMEBACK,		"ULTIMATE_COMEBACK" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_EXTREMELY_EXTREME,		"EXTREMELY_EXTREME" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HIGH_SLIDER,			"HIGH_SLIDER" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SIDEWINDER,				"SIDEWINDER" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_FTL,					"FTL" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHOSEN_ONE,				"CHOSEN_ONE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_RAMPAGE,				"RAMPAGE" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASTER_EXPLODER,		"MASTER_EXPLODER" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_QUICK_DRAW,				"QUICK_DRAW" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSAL_NEXUS,			"CAUSAL_NEXUS" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SKEET_SHOOTER,			"SKEET_SHOOTER" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_TEARAWAY,				"TEARAWAY" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_UNTOUCHABLE,			"UNTOUCHABLE" )	

	//Car challenges					
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES,		"COMPLETED_RACES" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER,					"WINNER" )

	//Daily challenges				
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_OVERTAKE,				"OVERTAKE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PARTICIPANT,			"PARTICIPANT" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HARD_DAYS_WORK,			"HARD_DAYS_WORK" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DEMOLITIONS_EXPERT,		"DEMOLITIONS_EXPERT" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HAT_TRICK,				"HAT_TRICK" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_STYLE,					"STYLE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_FACE_PLANT,				"FACE_PLANT" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PIED_PIPER,				"PIED_PIPER" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HEAD_IN_THE_CLOUDS,		"HEAD_IN_THE_CLOUDS" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HOARDER,				"HOARDER" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHAIN_REACTION,			"CHAIN_REACTION" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_AGGRESSIVE_PERSONALITY,	"AGGRESSIVE_PERSONALITY" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DOMINATOR,				"DOMINATOR" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DISTANCE,				"DISTANCE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAR_CRAZY,				"CAR_CRAZY" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WELL_TRAVELLED,			"WELL_TRAVELLED" )

	//Demolition challenges	 
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_RETRIBUTION,			"RETRIBUTION" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PUNISHER,				"PUNISHER" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WRECKING_BALL,			"WRECKING_BALL" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUISER,				"BRUISER" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CARNAGE,				"CARNAGE" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUTAL_DELUXE,			"BRUTAL_DELUXE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_VENDETTA,				"VENDETTA" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BRUTE_FORCE,			"BRUTE_FORCE" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAUSE_AND_EFFECT,		"CAUSE_AND_EFFECT" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BATTERING_RAM,			"BATTERING_RAM" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SCRAMBLER,				"SCRAMBLER" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_LAST_GASP,				"LAST_GASP" )

	//Driver's Ed. challenges
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BARGE,					"BARGE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHIELD,					"SHIELD" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE,					"MINE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_NITRO,					"NITRO" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOCK,					"SHOCK" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHUNT,					"SHUNT" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BLIND_FIRE,				"BLIND_FIRE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOW_OFF,				"SHOW_OFF" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHUNT_EVADER,			"SHUNT_EVADER" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOCK_EVADER,			"SHOCK_EVADER" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINESWEEPER,			"MINESWEEPER" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_REVERSE_SHUNT,			"REVERSE_SHUNT" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_AIRBRAKE,				"AIRBRAKE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_REVERSE_BLIND_FIRE,		"REVERSE_BLIND_FIRE" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE_LOBBER,			"MINE_LOBBER" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_REAR_GUARD,				"REAR_GUARD" )

	//Specialist challenges
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PACIFIST,				"PACIFIST" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_THICK_SKIN,				"THICK_SKIN" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASOCHIST,				"MASOCHIST" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BARGE_SPECIALIST,		"BARGE_SPECIALIST" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE_SPECIALIST,		"MINE_SPECIALIST" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BLIND_FIRE_SPECIALIST,	"BLIND_FIRE_SPECIALIST" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_BACKWARDS_THINKING,		"BACKWARDS_THINKING" )
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_AFTERBURNER,			"AFTERBURNER" )	
	DE_OPTION( UI::E_MULTIPLAYER_CHALLENGE_TYPE_JUMPER,					"JUMPER" )
END_DEBUG_ENUM

Debug::Enum<UI::E_MULTIPLAYER_CHALLENGE_TYPE>	g_ChallengeType("Multiplayer.Challenges.ChallengeType", UI::E_MULTIPLAYER_CHALLENGE_TYPE_PACIFIST);
Debug::Int g_ChallengeValueToAdd("Multiplayer.Challenges.ChallgengeValueToAdd", 0, Debug::Range(0, 10));
Debug::Function g_AddToChallenge("Multiplayer.Challenges.AddToChallenge", Game::C_MultiplayerProfile::DebugAddToChallenge, 0);
Debug::Int g_ChallengeCarToAddTo("Multiplayer.Challenges.ChallengeCarToAddTo", 0);

namespace Game
{

bool DebugOverrideFanCall = false; 

void C_MultiplayerProfile::Init()
{
	mLegendLevel = 0;

	mMultiplayerStatistics.Init();
	mFavouriteGroups.Initialise();
	mChallengeSystem.Initialise();
	mPreviouslyOnBlurStats.Clear();

	// garage resprays
	mResprays_MP.Clear();
	mRespraysLastUsed = ResprayData( 0, 0 );
	
	mLastCarUsed=0;

	Reset();
}

void C_MultiplayerProfile::Reset()
{
	mChallengeSystem.Reset();

	mCanUnlockLegend=false;
	mNumFans=0;
	mNumPreviousFans=0;
	mCurrentLevel = 0;
	mPreviousLevel = 0;

	ClearTempRaceFans();

	const MultiplayerConfig* config=C_MultiplayerData::Instance().m_pConfig;

	// This could be called before sorcery is loaded noobus
	if ( !config )
		return;

	mCarRewardFanBonusPercentage = config->m_CarRewardFanBonusPercentage;

	// make the last car used one from your garage to start
	mLastCarUsed = config->m_LevelingData[0].mVehicleUnlocks[0]->ID;

	// default last car used per class based on leveling data as we should have at least on car per class at the start
	for ( S32 i = 0; i < mLastCarUsedClass.Size(); i++ )
		mLastCarUsedClass[ i ] = 0;

	for ( S32 i = 0; i < config->m_LevelingData[ 0 ].mVehicleUnlocks.count; i++ )
	{
		const AmaxVehicle* pVehicle = config->m_LevelingData[ 0 ].mVehicleUnlocks[ i ].Object();
		if(pVehicle && pVehicle->m_ParentVehicle == NULL) // If it has a parent vehicle.. then its a boss / legend car.. dont assign these by default!
		{
			mLastCarUsedClass[ config->m_LevelingData[ 0 ].mVehicleUnlocks[ i ]->carClass ] = config->m_LevelingData[ 0 ].mVehicleUnlocks[ i ]->ID;
		}
	}

	mRankedLoadouts.Init();

	mRecentUnlockHistory.Init(config);
}

void C_MultiplayerProfile::ClearTempRaceFans()
{
	mRaceFinishBonus=0;
	mOnTrackFans=0;
	mTotalChallengeFans=0;
	mFinishBonus=0;
	mFanFavouriteBonus=0;
	mTotalNewFans=0;
	mCarRewardFanBonus=0;
	mTeamBonus=0;
}

//	[GavinY] - TO DO: Get rid of these hard-coded values and get them from Sorcery (probably...)
const S32 C_MultiplayerProfile::NumPickupSlots()const
{
	return 3;
}

const bool C_MultiplayerProfile::AllowAnyPerkType()const
{
	return true;
}

const bool C_MultiplayerProfile::AllowManualPerkChange()const
{
	return true;
}

const bool C_MultiplayerProfile::AllowShield() const
{
	return true;
}

const S32 C_MultiplayerProfile::NumShieldSlots() const
{
	return 3;
}

const S32 C_MultiplayerProfile::NumAdditionalHealthSlots() const
{
	return 0;
}

void C_MultiplayerProfile::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );

	out << mMultiplayerStatistics << mCurrentLevel <<  mLegendLevel << mNumFans << mLastCarUsed;

	mRankedLoadouts.Serialise(stream);

	out << mMiscChallengeDataBlackboard;
	
	//Serialise our favourite groups
	out << mFavouriteGroups;
	out << mRecentEvents;

	// Serialise Recent Unlock History
	mRecentUnlockHistory.Serialise(stream); 

	mChallengeSystem.Serialise( stream );

	// garage resprays
	out << mResprays_MP.Size();
	for ( ResprayMap::Iterator it = mResprays_MP.Begin(); it != mResprays_MP.End(); ++it )
		out << it->GetKey() << it->GetValue().first << it->GetValue().second;

	out << mRespraysLastUsed.first << mRespraysLastUsed.second;

	////	"Previously on Blur..." stats
	mPreviouslyOnBlurStats.CalculateDeltaValues();
	mPreviouslyOnBlurStats.Serialise(stream);
}

void C_MultiplayerProfile::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );

	in >> mMultiplayerStatistics >> mCurrentLevel >>  mLegendLevel >> mNumFans >> mLastCarUsed;

	mRankedLoadouts.Deserialise(stream);

	in >> mMiscChallengeDataBlackboard;

	//Read our favourite groups in
	in >> mFavouriteGroups;
	in >> mRecentEvents;
	
	// Read in Recent Unlock History
	mRecentUnlockHistory.Deserialise(stream); 

	mChallengeSystem.Deserialise( stream );

	// garage resprays
	S32 resprayCount = 0;
	XtId amaxVehicleId = 0;
	XtId resprayId = 0;
	S32 resprayIndex = 0;

	in >> resprayCount;
	for ( S32 i = 0; i < resprayCount; i++ )
	{
		in >> amaxVehicleId >> resprayId >> resprayIndex;
		mResprays_MP.Insert( amaxVehicleId, ResprayData( resprayId, resprayIndex ) );
	}

	in >> mRespraysLastUsed.first >> mRespraysLastUsed.second;


	//// "Previously on Blur..." stats
	mPreviouslyOnBlurStats.Deserialise(stream);
	mPreviouslyOnBlurStats.GetInitialStatValues();

	CheckLegendMode();

	mPreviousLevel = 0;
	mNumPreviousFans = 0;
	
	ClearTempRaceFans();
}

void C_MultiplayerProfile::PreRaceUpdate()
{
	mMultiplayerStatistics.PreRaceUpdate();

	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

	mPreviousLevel = mCurrentLevel;
	mNumPreviousFans = mNumFans;

	ClearTempRaceFans();
}

void C_MultiplayerProfile::PostRaceUpdate()
{	
	const Game::C_LevelResult& results = C_Game::GetInstance()->GetLevelResult();
	const Game::LevelResultCar& levelCar = results.mCars[0]; // This needs to be the player idx
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	
	mOnTrackFans = levelCar.mScore;
	//See if we're punishing the player...bad player...naughty!
	if(levelState.mPunitiveMeasuresInPlace)
	{
		mOnTrackFans *= levelState.mPunitiveFanMultiplier;
	}

	const MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;
	const C_MpResults&	result=C_MpResults::Instance();

	CalculateEventFinishedBonus(mCurrentLevel);
	CalculateTeamBonus(levelState, result, multiplayerConfig);
	CalculateCarRewardFanBonus(mOnTrackFans);

	mTotalChallengeFans = mChallengeSystem.GetTracker().GetFansEarned();
	mTotalNewFans = mOnTrackFans + mTotalChallengeFans + mFinishBonus + mCarRewardFanBonus + mTeamBonus;

	#ifndef CFG_GOLD 

		// Don't call this if we are updating due to a debug call (e.g. override fans)
		if(!DebugOverrideFanCall)
		{
			mMultiplayerStatistics.PostRaceUpdate(mTotalNewFans);

			mNumPreviousFans = mNumFans;
			mNumFans += mTotalNewFans;

			if(multiplayerConfig->m_LevelingData)
			{
				const S32 maxFans = GetMaxLevellingFans();
				mNumFans = Maths::Clamp( mNumFans, 0, maxFans );
			}
		}
		else
		{
			// Finished, set flag back as call finished
			DebugOverrideFanCall = false;  
		}

	#else

		mMultiplayerStatistics.PostRaceUpdate(mTotalNewFans);

		mNumPreviousFans = mNumFans;
		mNumFans += mTotalNewFans;

		if(multiplayerConfig->m_LevelingData)
		{
			S32 maxRank = multiplayerConfig->m_LevelingData.count - 1;
			const S32 maxFans = multiplayerConfig->m_LevelingData[maxRank].mFansNeeded;
			mNumFans = Maths::Clamp( mNumFans, 0, maxFans );
		}

	#endif //CFG_GOLD

	mPreviousLevel = mCurrentLevel;
	mCurrentLevel = CalcCurrentLevel();

	mRankedLoadouts.PostRaceUpdate(*this);

	// Need to update with the player car
	const S32 playerIndex = 0; // surely theres a function to tell me this or at least return 0 so we dont have to write [0] all over the code??
	levelState.mCars[playerIndex].mMultiplayerRank = mCurrentLevel;
	levelState.mCars[playerIndex].mMultiplayerWinningStreak = mMultiplayerStatistics.mWinningStreak;

#if !defined ( CFG_GOLD )
	//--------------------------------------------------------------------------------------
	// Unlock new vehicles
	if(s_dumpMultiplayerUnlocks)
	{
		DumpNewlyUnlockedVehicles(mPreviousLevel+1,mCurrentLevel);
		DumpNewlyUnlockedChallenges(mPreviousLevel+1,mCurrentLevel);
		DumpNewlyUnlockedAbilities(mPreviousLevel+1,mCurrentLevel);
	}

#endif

	CheckLegendMode();

	// Have we possibly unlocked new stuff?
	if(mPreviousLevel != mCurrentLevel)
	{
		MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

		BIZ_ASSERT_MSG(multiplayerConfig!=0, "config cannot be 0");

		// Then Make sure we also flag all the new unlocks as shiny and new
		mRecentUnlockHistory.FlagNewlyUnlockedItems(multiplayerConfig, mPreviousLevel+1,mCurrentLevel); 
	}

	mRaceSummary.m_OnTrackFans					= mOnTrackFans;
	mRaceSummary.m_TotalChallengeFans			= mTotalChallengeFans;
	mRaceSummary.m_FinishBonus					= mFinishBonus;
	mRaceSummary.m_TeamBonus					= mTeamBonus;
	mRaceSummary.m_CarRewardFanBonus			= mCarRewardFanBonus;
	mRaceSummary.m_TotalNewFans					= mTotalNewFans;
	mRaceSummary.m_Position						= result.GetPlayerPosition(0);
	mRaceSummary.m_ShowyFlourishContribution	= levelCar.mStatistics.mShowyFlourishFans;
	mRaceSummary.m_FanDemandContribution		= mFanFavouriteBonus;
	mRaceSummary.m_RankUp						= (mCurrentLevel > mPreviousLevel);
	mRaceSummary.m_Rank							= mCurrentLevel;
}

void C_MultiplayerProfile::CalculateCarRewardFanBonus( const S32 onTrackFans ) 
{
	// work out any bonus fans gained due to having the fan bonus car upgrade equipped
	if(mCarRewardFanBonusEquipped)
	{
		mCarRewardFanBonus += ((onTrackFans * 0.01f) * mCarRewardFanBonusPercentage); // earn a percentage of total fans earned in race
	}
}

void C_MultiplayerProfile::CalculateEventFinishedBonus( const S32 currentLevel )
{
	F32 eventBonus = 0;

	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results = C_Game::GetInstance()->GetLevelResult();
	const Game::LevelResultCar& levelCar = results.mCars[0]; // This needs to be the player idx
	Game::C_MpResults& mpResults = Game::C_MpResults::Instance();	
	const PostRaceBonusData& postRaceBonusData = C_MultiplayerData::Instance().m_pConfig->m_PostRaceBonus;

	if(!DebugOverrideFanCall)
	{
		BIZ_ASSERT_MSG(levelCar.mPosition>0 && levelCar.mPosition<=RacerNet::NET_MAX_VEHICLES, "CalculateEventFinishedBonus");
		BIZ_ASSERT_MSG(postRaceBonusData.m_PositionMultipliers.count == RacerNet::NET_MAX_VEHICLES, "RacerNet::NET_MAX_VEHICLES != position multipliers in sorcery");
		F32 positionMultiplier = postRaceBonusData.m_PositionMultipliers[levelCar.mPosition-1];
		BIZ_ASSERT_MSG_SPEED( positionMultiplier >= 0, "positionMultiplier < 0" );

		F32 eventTime = mpResults.m_SortedVehicleResults[0].m_Statistics[UI::E_MultiplayerStat_RaceTime];
		F32 eventLengthMultiplier = eventTime / postRaceBonusData.m_RaceTimeAverage;
		eventLengthMultiplier = Maths::Clamped<F32>(eventLengthMultiplier,0.0f,postRaceBonusData.m_RaceTimeMultiplierMax);
		BIZ_ASSERT_MSG_SPEED( eventLengthMultiplier >= 0, "eventLengthMultiplier < 0" );

		F32 rankMultiplier = (1.0f + ( (currentLevel+1) - 1.0f ) * postRaceBonusData.m_RankMultiplier );
		BIZ_ASSERT_MSG_SPEED( rankMultiplier >= 0, "rankMultiplier < 0" );

		//	Calculate the final event bonus
		eventBonus = postRaceBonusData.m_BaseScore * positionMultiplier * eventLengthMultiplier * rankMultiplier;

		//And just in case something goes horribly wrong
		if(eventBonus > 5000)
		{
			eventBonus = 5000;
		}

		//	Apply any overall fan multiplier that's in effect - 2x Fan weekends etc
		eventBonus *= C_Game::GetInstance()->GetLevelState().mFanMultiplier;

		mFanFavouriteBonus = 0;
		//Fan Favourite ability - gain extra fans per combo
		Game::C_LevelStateMultiplayerVehicleAbilities* mpMultiplayerAbilities = levelState.mCars[0].mUpgrades.GetMultiplayerAbilities();
		Game::C_MultiplayerVehicleAbility* ability = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Fan_Favourite);	
		if(ability->GetActive())
		{
			mFanFavouriteBonus = (ability->GetValue() - 1.0f) * eventBonus;

			if(g_DebugShowMpAbilityActivateSpam)
			{
				LOGDEBUG(g_MultiplayerLog, "Vehicle %d's Fan Favourite ability has been activated!", 0);
			}
		}
	}

	BIZ_ASSERT_MSG_SPEED(eventBonus + mFanFavouriteBonus>=0.0f,"_MultiplayerProfile::CalculateEventFinishedBonus() eveneBonus should never be less than zero!");
	
	mFinishBonus = eventBonus + mFanFavouriteBonus;
	//See if we're punishing the player...bad player...naughty!
	if(levelState.mPunitiveMeasuresInPlace)
	{
		mFinishBonus *= levelState.mPunitiveFanMultiplier;
	}
}

const MpRaceSummary& C_MultiplayerProfile::GetRaceSummary() const
{
	return mRaceSummary;
}

int C_MultiplayerProfile::CalcCurrentLevel()
{
	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;

	int levelLimit=multiplayerConfig->m_LevelingData.count;

	if(multiplayerConfig->m_LevelingData.count)
	{
		for( int i = 0; i < levelLimit; i++ )
		{
			if( multiplayerConfig->m_LevelingData[i].mFansNeeded > mNumFans )
			{	
				int level = i - 1;

				if(level < 0)
					level = 0;

				return level;
			}
		}
	}

	return levelLimit - 1;
}

void C_MultiplayerProfile::BuildNetworkSharedProfileData( RacerNet::NetPlayerSharedProfile& out, bool joinedSession /*= false*/ )
{
	out.m_Rank = Maths::Clamp(mCurrentLevel,0,255);
	out.m_Legend = Maths::Clamp(mLegendLevel,0,255);
	out.m_RacePointsThisSession = joinedSession ? 0 : mMultiplayerStatistics.mNumRacePointsThisSession;
	out.m_WinningStreak			= joinedSession ? 0 : mMultiplayerStatistics.mWinningStreak;
	out.m_RacesThisSession		= joinedSession ? 0 : mMultiplayerStatistics.mNumRacesThisSession;
	out.m_WinsThisSession		= joinedSession ? 0 : mMultiplayerStatistics.mNumWinsThisSession;
	out.m_BlurDeveloper			= IsBlurDeveloper();	//	used to display a dev icon by our names
	out.m_HasBeatenBizarre		= (out.m_BlurDeveloper || HasBeatenBizarre());	//	You can pass on the achievement if you're a dev, or have the achievement already.

	//	Allows us to enable/disable the developer icon at the end of our Gamertags with a checkbox in Sorcery.
	if(!C_MultiplayerData::Instance().m_pConfig->m_ShowDeveloperIcon)
		out.m_BlurDeveloper = false;
}

void C_MultiplayerProfile::BuildNetworkVehicleUpgradesData( RacerNet::NetPlayerVehicle& out, UI::E_CarUpgrades upgrades )
{
	out.m_PickupSlots = NumPickupSlots();
	out.m_ShieldSlots = NumShieldSlots();

	if( upgrades == UI::E_CarUpgrades_On )
	{
	
		const Btl::BitArray16& carUpgrades = mChallengeSystem.GetCarUpgradesEnabled(out.m_xtVehicleId);

		if( carUpgrades.GetBitValue(UI::E_MultiplayerVehicleUpgrades_FanBonus) )
		{
			// Dont need to tell anyone else about this.
			mCarRewardFanBonusEquipped = true;
		}
		else
		{
			mCarRewardFanBonusEquipped = false;
		}

		if( carUpgrades.GetBitValue( UI::E_MultiplayerVehicleUpgrades_HealthSlot) )
		{
			out.m_AdditionalHealthPips = 1;
		}

		if( carUpgrades.GetBitValue( UI::E_MultiplayerVehicleUpgrades_SpeedBoost) )
		{
			out.m_AdditionalHP = 1;
		}
	}
}


void C_MultiplayerProfile::ResetSessionStats()
{
	mMultiplayerStatistics.mNumRacePointsThisSession = 0;
	mMultiplayerStatistics.mNumRacesThisSession = 0;
	mMultiplayerStatistics.mNumWinsThisSession = 0;
	mMultiplayerStatistics.mWinningStreak = 0;
}

const S32 C_MultiplayerProfile::GetLegendLevel() const
{
	return mLegendLevel;
}

void C_MultiplayerProfile::UnlockLegendMode()
{
	mLegendLevel++;

	//Save out the amount of time it took you to achive the legend level for the legend leaderboard
	mMultiplayerStatistics.mTimeToAchieveLegend = (mMultiplayerStatistics.mTotalPlaytime / SYS_USEC_PER_SEC);

	// Reset all progress
	Reset();

	mPreviouslyOnBlurStats.LegendUpResetStats();

	// Ensure any legend cars that have been unlocked have unlocked all car upgrades
	UnlockLegendCarChallengeUpgrades();

	C_MultiplayerData::Instance().PreCacheData();
}

void C_MultiplayerProfile::UnlockLegendCarChallengeUpgrades()
{
	C_MultiplayerChallengeSystem& challengeSystem = C_GameProfileManager::Instance().GetPrimaryProfile()->GetMultiplayerProfile()->GetChallengeSystem();
	for ( S32 level = 0; level < C_MultiplayerData::Instance().m_pConfig->m_LegendLevelingData.count; level++ )
	{
		MultiplayerLevel& legendLevelData = C_MultiplayerData::Instance().m_pConfig->m_LegendLevelingData[level];

		// For each car
		for ( S32 i = 0; i < legendLevelData.mVehicleUnlocks.count; i++ )
		{
			// Legend cars  begin with full progress in these challenges (so they get the all the shiny awards - paint, health slots etc)
			challengeSystem.GetCarChallenge(legendLevelData.mVehicleUnlocks[i]->ID ,UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER ).ForceComplete();
			challengeSystem.GetCarChallenge(legendLevelData.mVehicleUnlocks[i]->ID ,UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES ).ForceComplete();
			challengeSystem.ForceCarUpgrades(legendLevelData.mVehicleUnlocks[i]->ID);
		}
	}
}

void C_MultiplayerProfile::CheckLegendMode()
{
	// This could be called before sorcery is loaded noobus
	if ( !C_MultiplayerData::Instance().m_pConfig )
	{
		return;
	}

	mCanUnlockLegend = false;

	int levelCount=C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count;

	if (levelCount-1==mCurrentLevel)
	{
		if (mLegendLevel<g_MaxLegendLevel)
		{
			mCanUnlockLegend=true;
		}
	}
}

const bool C_MultiplayerProfile::CanUnlockLegend() const
{
	return mCanUnlockLegend;
}

void C_MultiplayerProfile::DebugAddToChallenge(void* par)
{
	C_MultiplayerChallengeSystem& challengeSystem = C_GameProfileManager::Instance().GetPrimaryProfile()->GetMultiplayerProfile()->GetChallengeSystem();

	//Oh lovely hard coding!
	if(g_ChallengeType == UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES || g_ChallengeType == UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER)
	{
		//Car based challenge, a bit more complicated to blag
		DebugAddToCarChallenge(challengeSystem);
	}
	else
	{
		//Non-car based challenge
		challengeSystem.IncreaseChallengeAmount(g_ChallengeType, g_ChallengeValueToAdd);
	}
}

void C_MultiplayerProfile::DebugAddToCarChallenge(C_MultiplayerChallengeSystem& challengeSystem)
{
	//Find the car challenge pack
	S32 carPackIndex = -1;
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++ )
	{
		MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];
		BIZ_ASSERT_MSG_SPEED( pPack, "Invalid challenge pack in config (NULL)" );

		// Ignore the car challenges
		if ( pPack->mType == UI::E_ChallengePack_Car )
		{
			carPackIndex = i;
			break;
		}
	}

	// Build up the car challenges
	if ( carPackIndex != -1 )
	{
		MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[carPackIndex];

		for ( S32 level = 0; level < C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count; level++ )
		{
			MultiplayerLevel& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[level];

			// For each car
			for ( S32 i = 0; i < levelData.mVehicleUnlocks.count; i++ )
			{
				if(g_ChallengeCarToAddTo == 0 || (levelData.mVehicleUnlocks[i]->ID == g_ChallengeCarToAddTo) )
				{
					challengeSystem.IncreaseCarChallengeAmount(levelData.mVehicleUnlocks[i]->ID, g_ChallengeType, g_ChallengeValueToAdd);

					if( levelData.mVehicleUnlocks[i]->ID == g_ChallengeCarToAddTo )
						return;
				}
			}
		}
	}

}

void C_MultiplayerProfile::DebugOverrideFans(void* par)
{
	if (C_MultiplayerData::Instance().m_pConfig)
	{
		//To fix asserts happening, clamp the maximum fans to the value set in sorcery
		S32 maxFans = GetMaxLevellingFans();

		//See if we're trying to add more fans than is possible
		if(g_MultiplayerProfileFanOverrideNum > maxFans)
		{
			g_MultiplayerProfileFanOverrideNum = maxFans; 
		}
		C_GameProfileManager::Instance().GetPrimaryProfile()->GetMultiplayerProfile()->DebugOverrideFansNonStatic(g_MultiplayerProfileFanOverrideNum);
	}
}

void C_MultiplayerProfile::DebugOverrideFansNonStatic(int fans)
{

	mNumFans=fans;
	mNumPreviousFans=fans;

	// Set debug fan override flag so we dont try to do a real post race update
	DebugOverrideFanCall = true; 
	PostRaceUpdate();

	mPreviousLevel=mCurrentLevel;
}


#if !defined ( CFG_GOLD )

void C_MultiplayerProfile::DumpNewlyUnlockedVehicles(int startLevel, int endLevel)
{
	TLevelData levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;
	int levelingDataCount = levelData.count;

	// clamp - just in case.
	endLevel = levelingDataCount>endLevel?endLevel:(levelingDataCount-1);

	for(int i=startLevel; i<=endLevel; i++)
	{
		for(int j=0; j < levelData[i].mVehicleUnlocks.count;j++)
		{
			AmaxVehicle* pVehicle = levelData[i].mVehicleUnlocks[j].Object();

			LOGWARN(g_MultiplyerUnlocksLog,"VEHICLE UNLOCKED - Level %d: %s",i+1, UI::C_TextStringID( pVehicle->loc_string.id ).Name());
		}
	}
}

void C_MultiplayerProfile::DumpNewlyUnlockedChallenges(int startLevel, int endLevel)
{
	TLevelData levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;
	int levelingDataCount = levelData.count;

	// clamp - just in case.
	endLevel = levelingDataCount>endLevel?endLevel:(levelingDataCount-1);

	for(int i=startLevel; i<=endLevel; i++)
	{
		for( int pack = 0; pack < levelData[i].mChallengeUnlocks.count; pack++ )
		{
			MultiplayerChallengePack* pChallengePack = levelData[i].mChallengeUnlocks[pack].Object();

			//LOGWARN(g_MultiplyerUnlocksLog,"CHALLENGE PACK UNLOCKED - Level %d: %s",i+1, UI::C_TextStringID( pChallengePack->mName.id ).Name());

			for(int j=0; j<pChallengePack->mChallenges.count; j++)
			{
				MultiplayerChallenge& challenge = pChallengePack->mChallenges[j];
				LOGWARN(g_MultiplyerUnlocksLog,"CHALLENGE UNLOCKED - Level %d: %s",i+1, UI::C_TextStringID( challenge.mName.id ).Name());
			}
		}
	}
}

void C_MultiplayerProfile::DumpNewlyUnlockedAbilities(int startLevel, int endLevel)
{
	TLevelData& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;
	int levelingDataCount = levelData.count;

	// clamp - just in case.
	endLevel = levelingDataCount > endLevel ? endLevel: (levelingDataCount-1);

	for(int i=startLevel; i<=endLevel; i++)
	{
		for(int j=0; j < levelData[i].mAbilityUnlocks.count; j++)
		{
			MultiplayerAbilityData* pAbility = levelData[i].mAbilityUnlocks[j];

			if (pAbility)
			{
				LOGWARN(g_MultiplyerUnlocksLog,"ABILITY UNLOCKED - Level %d: %s",i+1, UI::C_TextStringID( pAbility->mAbilityName.id ).Name());
			}
			else
			{
				BIZ_WARNING_FAIL("A multiplayer ability is set to nul!!");
			}
		}
	}
} 
#endif //*#if*/ !defined ( CFG_GOLD )
MiscellaneousChallengeBlackboardData& C_MultiplayerProfile::GetChallengeBlackboard()
{
	return mMiscChallengeDataBlackboard;
}

FavouriteGroups& C_MultiplayerProfile::GetFavouriteEvents()
{
	return mFavouriteGroups;
}

RecentEventsStore&	C_MultiplayerProfile::GetRecentEvents()
{
	return mRecentEvents;
}

C_MultiplayerLoadouts& C_MultiplayerProfile::GetLoadouts(bool ranked)
{
	if(ranked)
	{
		return mRankedLoadouts;
	}
	else
	{
		return mUnrankedLoadouts;
	}
}

const int C_MultiplayerProfile::GetCurrentLevel() const
{
	return mCurrentLevel;
}

const int C_MultiplayerProfile::GetPreviousLevel() const
{
	return mPreviousLevel;
}

MultiplayerStatistics& C_MultiplayerProfile::GetMultiplayerStatistics()
{
	return mMultiplayerStatistics;
}

void C_MultiplayerProfile::SetLastCarUsed(U32 val)
{
	mLastCarUsed = val;

	// update car class last used
	AmaxVehicle* pVehicleAM = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)val ];

	if ( pVehicleAM )
		mLastCarUsedClass[ pVehicleAM->carClass ] = val;
}

U32 C_MultiplayerProfile::GetLastCarUsed() const
{
	return mLastCarUsed;
}

U32 C_MultiplayerProfile::GetLastCarUsed( const AmaxCarClass carClass ) const
{
	if(carClass == AmaxCarClass_ANY)	//	If all classes are valid, return the last Class A vehicle used.
		return mLastCarUsed;
	else
		return mLastCarUsedClass[ carClass ];
}


bool C_MultiplayerProfile::IsBlurDeveloper()
{
	return false;
}

bool C_MultiplayerProfile::HasBeatenBizarre()
{
	return false;	// avoids checking anything on PS3/PC since this doesn't exist as a trophy.
}

ResprayMap& C_MultiplayerProfile::GetRespraysMP()
{
	return mResprays_MP;
}

ResprayData& C_MultiplayerProfile::GetRespraysLastUsed()
{
	return mRespraysLastUsed;
}

S32 C_MultiplayerProfile::GetMaxLevellingRank()
{
	MultiplayerConfig* m_pConfig = C_MultiplayerData::Instance().m_pConfig;
	S32 maxRank = m_pConfig->m_LevelingData.count - 1;
	return maxRank;
}

S32 C_MultiplayerProfile::GetMaxLevellingFans()
{
	MultiplayerConfig* m_pConfig = C_MultiplayerData::Instance().m_pConfig;
	S32 rank = GetMaxLevellingRank();

	return m_pConfig->m_LevelingData[rank].mFansNeeded;
}

bool C_MultiplayerProfile::GetFeatureUnlocked(const UI::E_MultiplayerFeature feature) const
{
	TLevelData levels = C_MultiplayerData::Instance().m_pConfig->m_LevelingData;
	int levelingDataCount = levels.count;

	int level=mCurrentLevel;
	if (level>levels.count)
		level=levels.count;

	for(int i=0; i<=level; i++)
	{
		XtArray<XtHandle<MultiplayerFeature > >& featureUnlocks=levels[i].mFeatureUnlocks;
		for(int j=0; j<featureUnlocks.count; j++)
		{
			if (featureUnlocks[j]->mFeatureId==feature)
				return true;
		}
	}
	return false;
}

const S32 C_MultiplayerProfile::GetTeamBonus() const
{
	return mTeamBonus;
}

void C_MultiplayerProfile::CalculateTeamBonus(const Game::C_LevelState& levelState, const Game::C_MpResults& result, const MultiplayerConfig* config)
{
	mTeamBonus=0;

	if (levelState.mTeams)
	{
		const S32 playerIndex = 0; // surely theres a function to tell me this or at least return 0 so we dont have to write [0] all over the code??
		U8 playerTeam=levelState.mCars[playerIndex].mTeam;

		if (result.GetWinningTeam()==playerTeam)
		{
			U8 otherTeam=1-playerTeam;

			BIZ_ASSERT(result.m_Teams.Size()==2);
			int diff=result.m_Teams[otherTeam].m_RacerOrder.Size()-result.m_Teams[playerTeam].m_RacerOrder.Size();

			float mult=1.f+diff*config->m_WinnerTeamFanBonusBalancingMultiplier;

			if (mult<0.f)
				mult=0.f;

			mTeamBonus=mult*config->m_WinnerTeamFanBonus;
		}
	}
}

}; // namespace Game

bool MP_IsLockedLegendCar(XtId vehicleId)
{
	MultiplayerConfig*		config	= C_MultiplayerData::Instance().m_pConfig;
	Game::C_GameProfile*	profile	= Game::C_GameProfileManager::Instance().GetPrimaryProfile();

	if (!config || !profile)
		return false;

	Game::C_MultiplayerProfile*	mulProf = profile->GetMultiplayerProfile();

	if (!mulProf)
		return false;

	AmaxVehicle* amaxVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)vehicleId];

	if (!amaxVehicle || !amaxVehicle->m_ParentVehicle) // Legend and boss cars have parent vehicles (yuck, nasty)
		return false;

	TLevelData*	legendLevelData	= &config->m_LegendLevelingData;

	for(int i = mulProf->GetLegendLevel() + 1; i < legendLevelData->count; i++)
	{
		MultiplayerLevel* level = &(*legendLevelData)[i];

		for(int j = 0; j < level->mVehicleUnlocks.count; j++)
		{
			if (level->mVehicleUnlocks[j].Id() == amaxVehicle->ID)
				return true;
		}
	}

	return false;
}