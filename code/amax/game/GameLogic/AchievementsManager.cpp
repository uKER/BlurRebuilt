#include "stdafx.h"

#include <Game/Game.h>
#include "GameLogic\AchievementsManager.h"
#include <GameLogic\StickerManager.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Structures/Sticker.h>

#include <Game\Achievements\AchievementSystem.h>

#include "Ui/UiMessages.h"

#include <SystemProfile\SystemProfileManager.h>
#include <ui/Game/Profiles/ProfileInterface.h>

DEBUG_ENUM(Game::BLUR_STICKER)
	DE_OPTION( Game::BLUR_STICKER_WIN_ANY_PLAYLIST,						"WIN_ANY_PLAYLIST" )	
	DE_OPTION( Game::BLUR_STICKER_WIN_HARDCORE_RACING,					"WIN_HARDCORE_RACING" )					
	DE_OPTION( Game::BLUR_STICKER_WIN_TEAM,								"WIN_CHAMPIONSHIP" )						
	DE_OPTION( Game::BLUR_STICKER_BARGE_CARS,							"BARGE_CARS" )					
	DE_OPTION( Game::BLUR_STICKER_BARGE_OFF_WATER_OR_CLIFF,				"BARGE_OFF_WATER_OR_CLIFF" )
	DE_OPTION( Game::BLUR_STICKER_BARGE_INCOMING_ATTACK,				"BARGE_INCOMING_ATTACK" )
	DE_OPTION( Game::BLUR_STICKER_BARGE_THREE_AT_ONCE,					"BARGE_THREE_AT_ONCE" )
	DE_OPTION( Game::BLUR_STICKER_BEAT_BIZ_DEV,							"BEAT_BIZ_DEV" )					
	DE_OPTION( Game::BLUR_STICKER_FIRST_FRIEND_DEMAND,					"FIRST_FRIEND_DEMAND" )			
	DE_OPTION( Game::BLUR_STICKER_ACCEPT_FRIEND_DEMAND,					"ACCEPT_FRIEND_DEMAND" )
	DE_OPTION( Game::BLUR_STICKER_BEAT_FRIEND_DEMAND,					"BEAT_FRIEND_DEMAND" )
	DE_OPTION( Game::BLUR_STICKER_SEND_CHALLENGE_BACK,					"SEND_CHALLENGE_BACK" )
	DE_OPTION( Game::BLUR_STICKER_WIN_SPLIT_SCREEN,						"WIN_SPLIT_SCREEN" )				
	DE_OPTION( Game::BLUR_STICKER_THREE_BOLT_WRECK,						"THREE_BOLT_WRECK" )				
	DE_OPTION( Game::BLUR_STICKER_FOUR_BOLT_HIT,						"FOUR_BOLT_HIT" )
	DE_OPTION( Game::BLUR_STICKER_BOLT_INCOMING_POWERUPS,				"BOLT_INCOMING_POWERUPS" )
	DE_OPTION( Game::BLUR_STICKER_THREE_BOLT_DIFFERENT,					"THREE_BOLT_DIFFERENT" )
	DE_OPTION( Game::BLUR_STICKER_WRECK_FIRST_BOSS,						"WRECK_FIRST_BOSS" )				
	DE_OPTION( Game::BLUR_STICKER_MAXI_LEADERBOARD_FRIEND,				"MAXI_LEADERBOARD_FRIEND" )		
	DE_OPTION( Game::BLUR_STICKER_CHALLENGE_PACK_FULL_DONE,				"CHALLENGE_PACK_FULL_DONE" )		
	DE_OPTION( Game::BLUR_STICKER_CHALLENGE_PACK_THREE_HALF_DONE,		"CHALLENGE_PACK_THREE_HALF_DONE" )
	DE_OPTION( Game::BLUR_STICKER_CHALLENGE_X_DONE,						"CHALLENGE_X_DONE" )
	DE_OPTION( Game::BLUR_STICKER_CHALLENGE_Y_DONE,						"CHALLENGE_Y_DONE" )
	DE_OPTION( Game::BLUR_STICKER_STOPWATCH_POWERUPS,					"STOPWATCH_POWERUPS" )			
	DE_OPTION( Game::BLUR_STICKER_NITRO_EVERY_CHECKPOINT,				"NITRO_EVERY_CHECKPOINT" )
	DE_OPTION( Game::BLUR_STICKER_GOLD_WITHOUT_NITRO,					"GOLD_WITHOUT_NITRO" )
	DE_OPTION( Game::BLUR_STICKER_GOLD_WITHOUT_STOPWATCH,				"GOLD_WITHOUT_STOPWATCH" )
	DE_OPTION( Game::BLUR_STICKER_EVERY_PRIMARY_LIGHT,					"EVERY_PRIMARY_LIGHT" )			
	DE_OPTION( Game::BLUR_STICKER_EVERY_FAN_PAR,						"EVERY_FAN_PAR" )
	DE_OPTION( Game::BLUR_STICKER_EVERY_LIGHT,							"EVERY_LIGHT" )
	DE_OPTION( Game::BLUR_STICKER_FIRST_FAN_RUN,						"FIRST_FAN_RUN" )					
	DE_OPTION( Game::BLUR_STICKER_FIRST_AWARD,							"FIRST_AWARD" )					
	DE_OPTION( Game::BLUR_STICKER_WIN_TWO_AWARDS,						"WIN_TWO_AWARDS" )
	DE_OPTION( Game::BLUR_STICKER_WIN_THREE_AWARDS,						"WIN_THREE_AWARDS" )			
	DE_OPTION( Game::BLUR_STICKER_BEAT_RHYMER,							"BEAT_RHYMER" )					
	DE_OPTION( Game::BLUR_STICKER_COMPLETE_FAN_DEMANDS,					"COMPLETE_FAN_DEMANDS" )			
	DE_OPTION( Game::BLUR_STICKER_ALL_FAN_DEMANDS_IN_EVENT,				"ALL_FAN_DEMANDS_IN_EVENT" )
	DE_OPTION( Game::BLUR_STICKER_UNLOCK_SHANNON,						"UNLOCK_SHANNON" )				
	DE_OPTION( Game::BLUR_STICKER_COMPLETE_FAN_RUNS,					"COMPLETE_FAN_RUNS" )				
	DE_OPTION( Game::BLUR_STICKER_COMPLETE_ALL_FAN_RUNS,				"COMPLETE_ALL_FAN_RUNS" )
	DE_OPTION( Game::BLUR_STICKER_THROUGH_FAN_GATES,					"THROUGH_FAN_GATES" )
	DE_OPTION( Game::BLUR_STICKER_PHOTO_FINISH,							"PHOTO_FINISH" )		
	DE_OPTION( Game::BLUR_STICKER_FIRST_FAN_PAR,						"FIRST_FAN_PAR" )					
	DE_OPTION( Game::BLUR_STICKER_BEAT_A_RANK_FIFTY,					"BEAT_A_RANK_FIFTY" )				
	DE_OPTION( Game::BLUR_STICKER_RACE_EACH_LOCATION,					"RACE_EACH_LOCATION" )			
	DE_OPTION( Game::BLUR_STICKER_BEAT_CARVER,							"BEAT_CARVER" )					
	DE_OPTION( Game::BLUR_STICKER_BEAT_SHANNON,							"BEAT_SHANNON" )					
	DE_OPTION( Game::BLUR_STICKER_DAMAGE_CARVER_MORE,					"DAMAGE_CARVER_MORE" )			
	DE_OPTION( Game::BLUR_STICKER_AVOID_NATALYAS_POWERUPS,				"AVOID_NATALYAS_POWERUPS" )
	DE_OPTION( Game::BLUR_STICKER_SMASH_KHAN,							"SMASH_KHAN" )
	DE_OPTION( Game::BLUR_STICKER_SHOCK_LEONARD,						"SHOCK_LEONARD" )
	DE_OPTION( Game::BLUR_STICKER_HARUNA_GOLD_NO_DAMAGE,				"HARUNA_GOLD_NO_DAMAGE" )			
	DE_OPTION( Game::BLUR_STICKER_COMPLETE_GAME_HARD,					"COMPLETE_GAME_HARD" )			
	DE_OPTION( Game::BLUR_STICKER_MINE_CARS,							"MINE_CARS" )						
	DE_OPTION( Game::BLUR_STICKER_MINE_WITH_LOBBED,						"MINE_WITH_LOBBED" )
	DE_OPTION( Game::BLUR_STICKER_DESTROY_MINES,						"DESTROY_MINES" )
	DE_OPTION( Game::BLUR_STICKER_MINE_INCOMING_ATTACKS,				"MINE_INCOMING_ATTACKS" )
	DE_OPTION( Game::BLUR_STICKER_CUSTOMISE_A_LOADOUT,					"CUSTOMISE_A_LOADOUT" )			
	DE_OPTION( Game::BLUR_STICKER_WIN_WITH_CUSTOM_LOADOUT,				"WIN_WITH_CUSTOM_LOADOUT" )
	DE_OPTION( Game::BLUR_STICKER_WIN_MOTOR_MASH,						"WIN_MOTOR_MASH" )				
	DE_OPTION( Game::BLUR_STICKER_WRECK_RIVAL,							"WRECK_RIVAL" )
	DE_OPTION( Game::BLUR_STICKER_POINTS_IN_ONE_MASH,					"POINTS_IN_ONE_MASH" )
	DE_OPTION( Game::BLUR_STICKER_NITRO_OVERTAKE_CARS,					"NITRO_OVERTAKE_CARS" )			
	DE_OPTION( Game::BLUR_STICKER_AIRBRAKE_RIFT_CARS,					"AIRBRAKE_RIFT_CARS" )
	DE_OPTION( Game::BLUR_STICKER_AIRBRAKE_AND_BOLT,					"AIRBRAKE_AND_BOLT" )
	DE_OPTION( Game::BLUR_STICKER_WIN_AFTER_AIRBRAKES,					"WIN_AFTER_AIRBRAKES" )
	DE_OPTION( Game::BLUR_STICKER_BEAT_KHAN,							"BEAT_KHAN" )						
	DE_OPTION( Game::BLUR_STICKER_RESPRAY_A_CAR,						"RESPRAY_A_CAR" )					
	DE_OPTION( Game::BLUR_STICKER_TOP_THREE_IN_PINK,					"TOP_THREE_IN_PINK" )
	DE_OPTION( Game::BLUR_STICKER_RESPRAY_WITH_UNLOCKED,				"RESPRAY_WITH_UNLOCKED" )
	DE_OPTION( Game::BLUR_STICKER_UNLOCK_ALL_PLAYLISTS,					"UNLOCK_ALL_PLAYLISTS" )			
	DE_OPTION( Game::BLUR_STICKER_UNLOCK_ALL_CHALLENGE_PACKS,			"UNLOCK_ALL_CHALLENGE_PACKS" )
	DE_OPTION( Game::BLUR_STICKER_UNLOCK_ALL_MODS,						"UNLOCK_ALL_MODS" )
	DE_OPTION( Game::BLUR_STICKER_REACH_RANK_FIFTY,						"REACH_RANK_FIFTY" )
	DE_OPTION( Game::BLUR_STICKER_REPAIR_HEALTH,						"REPAIR_HEALTH" )					
	DE_OPTION( Game::BLUR_STICKER_REPAIR_JUST_IN_TIME,					"REPAIR_JUST_IN_TIME" )
	DE_OPTION( Game::BLUR_STICKER_WIN_WITH_REPAIR_AND_HALF_PIP,			"WIN_WITH_REPAIR_AND_HALF_PIP" )
	DE_OPTION( Game::BLUR_STICKER_REPAIR_MORE_THAN_MAX_HEALTH,			"REPAIR_MORE_THAN_MAX_HEALTH" )
	DE_OPTION( Game::BLUR_STICKER_SHIELD_ATTACKS,						"SHIELD_ATTACKS" )				
	DE_OPTION( Game::BLUR_STICKER_SHIELD_ATTACKS_SINGLE_SHIELD,			"SHIELD_ATTACKS_SINGLE_SHIELD" )
	DE_OPTION( Game::BLUR_STICKER_SHIELD_CLOSE_RANGE,					"SHIELD_CLOSE_RANGE" )
	DE_OPTION( Game::BLUR_STICKER_HOLD_SHIELD_FOR_LAP,					"HOLD_SHIELD_FOR_LAP" )
	DE_OPTION( Game::BLUR_STICKER_SHOCK_CARS,							"SHOCK_CARS" )					
	DE_OPTION( Game::BLUR_STICKER_SHOCK_EVERY_DOME,						"SHOCK_EVERY_DOME" )
	DE_OPTION( Game::BLUR_STICKER_SHOCK_YOURSELF_AND_AVOID,				"SHOCK_YOURSELF_AND_AVOID" )
	DE_OPTION( Game::BLUR_STICKER_SHOCK_CAR_TWICE,						"SHOCK_CAR_TWICE" )
	DE_OPTION( Game::BLUR_STICKER_FIRST_BOSS_DEMAND,					"FIRST_BOSS_DEMAND" )				
	DE_OPTION( Game::BLUR_STICKER_MEET_BOSS_DEMANDS,					"MEET_BOSS_DEMANDS" )
	DE_OPTION( Game::BLUR_STICKER_COMPLETE_FIRST_LEONARD_DEMAND,		"COMPLETE_FIRST_LEONARD_DEMAND" )
	DE_OPTION( Game::BLUR_STICKER_COMPLETE_THIRD_SHOWDOWN_DEMAND,		"COMPLETE_THIRD_SHOWDOWN_DEMAND" )
	DE_OPTION( Game::BLUR_STICKER_SHUNT_CARS,							"SHUNT_CARS" )					
	DE_OPTION( Game::BLUR_STICKER_REVERSE_SHUNT_CARS,					"REVERSE_SHUNT_CARS" )
	DE_OPTION( Game::BLUR_STICKER_DESTROY_INCOMING_SHUNTS,				"DESTROY_INCOMING_SHUNTS" )
	DE_OPTION( Game::BLUR_STICKER_AVOID_SHUNTS_BY_DRIVING,				"AVOID_SHUNTS_BY_DRIVING" )
	DE_OPTION( Game::BLUR_STICKER_EVADE_FIRST_POWERUP,					"EVADE_FIRST_POWERUP" )			
	DE_OPTION( Game::BLUR_STICKER_SHARE_FIRST_EXPERIENCE,				"SHARE_FIRST_EXPERIENCE" )		
	DE_OPTION( Game::BLUR_STICKER_HIGH_SPEED_CLASS_D,					"HIGH_SPEED_CLASS_D" )			
	DE_OPTION( Game::BLUR_STICKER_HIGH_SPEED_CLASS_C,					"HIGH_SPEED_CLASS_C" )
	DE_OPTION( Game::BLUR_STICKER_HIGH_SPEED_CLASS_B,					"HIGH_SPEED_CLASS_B" )
	DE_OPTION( Game::BLUR_STICKER_HIGH_SPEED_CLASS_A,					"HIGH_SPEED_CLASS_A" )				   
	DE_OPTION( Game::BLUR_STICKER_WRECK_YOUR_FIRST,						"WRECK_YOUR_FIRST" )						
	DE_OPTION( Game::BLUR_STICKER_BEAT_DRAKE,							"BEAT_DRAKE" )							
	DE_OPTION( Game::BLUR_STICKER_BEAT_TALON,							"BEAT_TALON" )							
	DE_OPTION( Game::BLUR_STICKER_BEAT_SHANNON_AGAIN,					"BEAT_SHANNON_AGAIN" )			
	DE_OPTION( Game::BLUR_STICKER_BEAT_NATALYA,							"BEAT_NATALYA" )					
	DE_OPTION( Game::BLUR_STICKER_VIEW_BRIGHTON_FIREWORKS,				"VIEW_BRIGHTON_FIREWORKS" )		
	DE_OPTION( Game::BLUR_STICKER_AMBOY_OILFIELDS_SMOKING_CAR,			"AMBOY_OILFIELDS_SMOKING_CAR" )
	DE_OPTION( Game::BLUR_STICKER_NITRO_THROUGH_GAS_STATION,			"NITRO_THROUGH_GAS_STATION" )
	DE_OPTION( Game::BLUR_STICKER_HOLLYWOOD_NITRO_NO_DAMAGE,			"HOLLYWOOD_NITRO_NO_DAMAGE" )
	DE_OPTION( Game::BLUR_STICKER_BEAT_AYUMI_HARUMI,					"BEAT_AYUMI_HARUMI" )				
	DE_OPTION( Game::BLUR_STICKER_DESTRUCTION_EVERY_BOLT_GOLD,			"DESTRUCTION_EVERY_BOLT_GOLD" )	
	DE_OPTION( Game::BLUR_STICKER_WRECK_CARS_IN_DESTRUCTION,			"WRECK_CARS_IN_DESTRUCTION" )
	DE_OPTION( Game::BLUR_STICKER_LONGSHOTS_IN_DESTRUCTION,				"LONGSHOTS_IN_DESTRUCTION" )
	DE_OPTION( Game::BLUR_STICKER_WRECK_RED_CARS,						"WRECK_RED_CARS" )
END_DEBUG_ENUM

DEBUG_ENUM(Game::BLUR_ACHIEVEMENT)
	DE_OPTION( Game::BLUR_ACHIEVEMENT_ALL_ROUNDER,						"ALL_ROUNDER" )					
	DE_OPTION( Game::BLUR_ACHIEVEMENT_BARGE_MASTER,						"BARGE_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_BEAT_BIZARRE,						"BEAT_BIZARRE" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_BETTER_ENEMIES,					"BETTER_ENEMIES" )	
	DE_OPTION( Game::BLUR_ACHIEVEMENT_BLUR_PARTY,						"BLUR_PARTY" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_BOLT_MASTER,						"BOLT_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_BREAK_YOU,						"BREAK_YOU" )		
	DE_OPTION( Game::BLUR_ACHIEVEMENT_CHALLENGE_MASTER,					"CHALLENGE_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_CHALLENGER,						"CHALLENGER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_CHECKPOINT_MASTER,				"CHECKPOINT_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_COMPLETIST,						"COMPLETIST" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_CONE_GATES,						"CONE_GATES" )		
	DE_OPTION( Game::BLUR_ACHIEVEMENT_DOMINATOR,						"DOMINATOR" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_FAN_BOY,							"FAN_BOY" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_FAN_DEMAND_MASTER,				"FAN_DEMAND_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_FIRST_TEST,						"FIRST_TEST" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_FOR_THE_FANS,						"FOR_THE_FANS" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_PHOTO_FINISH,						"PHOTO_FINISH" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_GETTING_STARTED,					"GETTING_STARTED" ) 
	DE_OPTION( Game::BLUR_ACHIEVEMENT_GIANT_KILLER,						"GIANT_KILLER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_GLOBETROTTER,						"GLOBETROTTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_GOING_GREY,						"GOING_GREY" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_LEARNING_THE_ROPES,				"LEARNING_THE_ROPES" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_LIGHTS_OUT_MEATBALL,				"LIGHTS_OUT_MEATBALL" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_MASTER_OF_THE_MACHINE,			"MASTER_OF_THE_MACHINE" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_MEET_THE_MITCHELLS,				"MEET_THE_MITCHELLS" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_MINE_MASTER,						"MINE_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_MODDER,							"MODDER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_MOTOR_MAN,						"MOTOR_MAN" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_NITRO_MASTER,						"NITRO_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_NOT_SO_PRETTY_BOY,				"NOT_SO_PRETTY_BOY" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_PAINT_JOB,						"PAINT_JOB" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_RACE_MASTER,						"RACE_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_RANK_FIFTY,						"RANK_FIFTY" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_REPAIR_MASTER,					"REPAIR_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_SHIELD_MASTER,					"SHIELD_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_SHOCK_MASTER,						"SHOCK_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_SHOW_WHOS_BOSS,					"SHOW_WHOS_BOSS" )	
	DE_OPTION( Game::BLUR_ACHIEVEMENT_SHUNT_MASTER,						"SHUNT_MASTER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_SKILLS,							"SKILLS" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_SOCIALISER,						"SOCIALISER" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_SPEED_DEMON,						"SPEED_DEMON" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_SWING_OF_THINGS,					"SWING_OF_THINGS" )	
	DE_OPTION( Game::BLUR_ACHIEVEMENT_SYBERIAN_EXPRESS,					"SYBERIAN_EXPRESS" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_TALONTED,							"TALONTED" )		
	DE_OPTION( Game::BLUR_ACHIEVEMENT_THE_REAL_HUSTLE,					"THE_REAL_HUSTLE" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_THORNY_ISSUE,						"THORNY_ISSUE" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_TOURIST,							"TOURIST" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_TWIN_TROUBLE,						"TWIN_TROUBLE" )
	DE_OPTION( Game::BLUR_ACHIEVEMENT_WRECKING_MACHINE,					"WRECKING_MACHINE" )
END_DEBUG_ENUM

Debug::Bool g_DebugOutputAchievementCompletion("Achievements.AchievementCompletionDebugOutput", false, Debug::Persist() );
Debug::Bool g_DebugOutputStickerCompletion("Achievements.StickerCompletionDebugOutput", false, Debug::Persist() );

Debug::Enum<Game::BLUR_ACHIEVEMENT>	g_AchievementType("Achievements.Achievement", Game::BLUR_ACHIEVEMENT_GETTING_STARTED );
Debug::Enum<Game::BLUR_STICKER>	g_StickerType("Achievements.Sticker", Game::BLUR_STICKER_WRECK_YOUR_FIRST);

Debug::Int g_UserIndexToGrantAchievement("Achievements.UserIndexToGrantAchievementTo", 0);
Debug::Float g_StickerProgressToAdd("Achievements.StickerProgressToAdd", 0.f);

static void DebugCompleteSticker(void *) { //yeah yeah, but what sort of sticker needs more than ten million progress?!
	Game::C_StickerManager::Instance().GetSticker(g_StickerType)->AddProgress(10000000);
}
static void DebugGiveAchievement(void *) {
	Game::C_AchievementsManager::Instance().GiveAchievement(g_UserIndexToGrantAchievement, g_AchievementType);
}
static void DebugAddStickerProgress(void *) {
	Game::C_StickerManager::Instance().GetSticker(g_StickerType)->AddProgress(g_StickerProgressToAdd);
}

static void DebugOutputProgress(void *) {
	Game::C_StickerManager::Instance().DebugDumpSticker(g_StickerType);
}

static void DebugOutputAllProgress(void *) {
	Game::C_StickerManager::Instance().DebugDumpAllStickers();
}

static void DebugActivateUiMessage(void *) {
	Game::C_AchievementsManager::Instance().StickerProgressed(Game::C_AchievementsManager::Instance().GetAchievementContainingSticker(g_StickerType),
																	Game::C_StickerManager::Instance().GetSticker(g_StickerType) );
}

Debug::Function g_CompleteAchievement("Achievements.CompleteAchievement", DebugGiveAchievement, 0);
Debug::Function g_CompleteSticker("Achievements.CompleteSticker", DebugCompleteSticker, 0);
Debug::Function g_AddStickerProgress("Achievements.AddStickerProgress", DebugAddStickerProgress, 0);
Debug::Function g_DumpStickerProgressToDebugOutput("Achievements.DumpStickerProgressToDebugOutput", DebugOutputProgress, 0);
Debug::Function g_DumpAllStickerProgressToDebugOutput("Achievements.DumpAllStickerProgressToDebugOutput", DebugOutputAllProgress, 0);
Debug::Function g_TestStickerProgressMessage("Achievements.TestStickerProgressMessage", DebugActivateUiMessage, 0);

namespace Game
{
	C_AchievementsManager C_AchievementsManager::s_instance;

	void C_AchievementsManager::Init()
	{
		SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );

		// We populate our containers with data from sorcery, note that this function also
		// populates StickerManager to avoid multiple passes through the data
		LoadFromSorceryData();
	}

	void C_AchievementsManager::LoadFromSorceryData()
	{
		AchievementsList* pAchievements = C_AchievementData::Instance().mAchievementsList[0];
		BIZ_ASSERT_MSG( pAchievements, "C_AchievementManager::LoadFromSorceryData() > Couldn't find Sorcery data (not loaded?)" );

		int numStickers = 0;
		//	Find the number of valid achievements/trophies and stickers
		Btl::Vector<U8> validAchievementList;
		for(int i = 0; i < pAchievements->mAchievements.count; i++)
		{
#ifdef PLATFORM_PS3
			if(pAchievements->mAchievements[i]->mDisableOnPS3)
				continue;
#endif	//	PLATFORM_PS3
			BIZ_ASSERT_MSG( pAchievements->mAchievements[i], "C_AchievementManager::LoadFromSorceryData() > Found null achievement (number %d)", i+1 );
			validAchievementList.PushBack(i);
			numStickers += pAchievements->mAchievements[i]->mStickers.count;
		}

		mAchievements.Reserve(validAchievementList.Size());
		mStickerBookLocations.Resize(validAchievementList.Size()); // yes, I do mean resize!
		Btl::Vector<C_Sticker> stickers;
		
		// StickerManager initialisation resizes its member vector accordingly
		C_StickerManager::Instance().Init(numStickers);

		//initialise our local vector and push our data to it
		stickers.Clear();

		bool stickersInAchievement = false;
		for( int i = 0; i < validAchievementList.Size(); i++ )
		{
			U8 achievement = validAchievementList[i];
			// todo: no need to make another copy of the sorcery data! use handles instead
			Achievement a;
			a.mName = pAchievements->mAchievements[achievement]->mName;
			a.mType = pAchievements->mAchievements[achievement]->mType;
			a.mCategory = pAchievements->mAchievements[achievement]->mCategory;
			a.mTextureIndex = pAchievements->mAchievements[achievement]->mTextureIndex;
			a.mSinglePlayer = pAchievements->mAchievements[achievement]->mSinglePlayer;
			a.mMultiplayer = pAchievements->mAchievements[achievement]->mMultiplayer;
			a.mSplitScreen = pAchievements->mAchievements[achievement]->mSplitScreen;
			a.mStickerBookLocation = (U8)i;
			mAchievements.PushBack(a);

			// for each achievement data we have, push its Stickers to the StickerManager and receive pointers to them
			for( int j = 0; j < pAchievements->mAchievements[achievement]->mStickers.count; j++ )
			{
				stickers.PushBack( C_Sticker(pAchievements->mAchievements[achievement]->mStickers[j], &(mAchievements.Back())) );
				stickersInAchievement = true;
			}
			if(stickersInAchievement)
			{
				mAchievements.Back().mStickers = C_StickerManager::Instance().AddStickersFromSorcery(stickers);
				stickers.Clear();
				stickersInAchievement = false;
			}
			else
			{
				mAchievements.Back().mStickers.Clear();
			}

			// cache the location of this achievement in the sticker book by index so we may easily access it later
			if( mStickerBookLocations.Size() > a.mStickerBookLocation )
			{
				mStickerBookLocations[a.mStickerBookLocation] = &(mAchievements.Back());
			}
			else
			{
				LOGWARN( g_Log, "C_AchievementManager::LoadFromSorceryData() > Can't place achievement %d at location %d in the sticker book, max size is %d",
								(int)(a.mType), a.mStickerBookLocation, mStickerBookLocations.Size() - 1 );
			}
		}
	}

	int C_AchievementsManager::GetControllerIndex(int idx) const
	{
		return C_StickerManager::Instance().GetControllerIndex(idx);
	}

	bool C_AchievementsManager::IsSignedIn() const
	{
		S32 sPadNo = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		return ( UI::C_ProfileInterface::Instance().PadProfileOnline( sPadNo ) || UI::C_ProfileInterface::Instance().PadProfileOffline( sPadNo ) );
	}

	Achievement* C_AchievementsManager::GetAchievement(BLUR_ACHIEVEMENT type)
	{
		for( int i = 0; i < mAchievements.Size(); i++ )
		{
			if( mAchievements[i].mType == type )
				return &mAchievements[i];
		}

		LOGWARN(g_Log, "C_AchievementsManager::GetAchievement > You're looking for an achievement I don't have (enum number %d)!", (int)type);
		return NULL;
	}

	Achievement* C_AchievementsManager::GetAchievementContainingSticker(BLUR_STICKER type)
	{
		for( int i = 0; i < mAchievements.Size(); i++ )
		{
			for( int j = 0; j < mAchievements[i].mStickers.Size(); j++ )
			{
				if(mAchievements[i].mStickers[j]->Type() == type)
					return &mAchievements[i];
			}
		}
		LOGWARN(g_Log, "C_AchievementsManager::GetAchievementContainingSticker > You're looking for an achievement I don't have (enum number %d)!", (int)type);
		return NULL;
	}

	// Checks for completion with the AchievementSystem and gifts achievement if not yet complete
	void C_AchievementsManager::GiveAchievement(int user_idx, BLUR_ACHIEVEMENT achievement)
	{
		if( !C_AchievementSystem::Instance().HasAchievement(user_idx, achievement ) )
		{
			C_AchievementSystem::Instance().GiveAchievement(user_idx, achievement);
		}
		if(g_DebugOutputAchievementCompletion)
			LOGINFO(g_Log, "ACHIEVEMENT GRANTED >>>> (achievement number %d for user index %d)", (int)achievement, user_idx);
	}

	void C_AchievementsManager::Update()
	{
	}

	// Checks all Stickers relating to the relevant achievement
	bool C_AchievementsManager::IsAchievementComplete(BLUR_ACHIEVEMENT achievement)
	{
		Achievement* a = GetAchievement(achievement);
		if(a)
			return IsAchievementComplete(a);
		else
			return false;
	}
	bool C_AchievementsManager::IsAchievementComplete(Achievement* achievement)
	{
		for( int i = 0; i < achievement->mStickers.Size(); i++ )
		{
			if( achievement->mStickers[i]->IsComplete() == false )
				return false;
		}

		return true;
	}

	// We're being informed about a completed sticker, check the achievement it is part of.
	// Note that stickers may only be completed on the primary profile! If you wish to give
	// a non-primary player an achievement (e.g. won split screen race), do it via GiveAchievement
	void C_AchievementsManager::StickerComplete(BLUR_STICKER sticker)
	{
		Achievement* a = GetAchievementContainingSticker( sticker );
		if( a && IsAchievementComplete( a ) )
		{
			GiveAchievement( SysProfile::C_Manager::Instance().GetPrimaryPadNo(), a->mType );

			if(g_DebugOutputStickerCompletion)
				LOGINFO(g_Log, "STICKER GRANTED >>>> (sticker number %d for pad number %d)", (int)sticker, SysProfile::C_Manager::Instance().GetPrimaryPadNo());
		}

		StickerProgressed( a, Game::C_StickerManager::Instance().GetSticker( sticker ) );
		Game::C_StickerManager::Instance().GetStickerBlackboard().mHasUnlockedNewStickers = true;
	}

	void C_AchievementsManager::StickerProgressed( Achievement* a, C_Sticker* sticker )
	{
		UI::C_WideString128 strName;
		UI::C_WideString128 strDesc;

		strName.Clear();
		strDesc.Clear();

		strName.Format( UI::C_TextStringID( a->mName.id ).String() );
		strDesc.Format( UI::C_TextStringID(sticker->ShortDescription().id).String(), sticker->Goal() );

		// find which sticker this is
		S8 sStickerPart = -1;

		for ( S32 i = 0; i < a->mStickers.Size(); i++ )
		{
			if ( a->mStickers[ i ]->Type() == sticker->Type() )
			{
				sStickerPart = i;
				break;
			}
		}

		UI::C_UiProgressMessage uiMsg( strName, strDesc, sticker->Progress(), sticker->Goal(), 0, sticker->IsOneShot(), a->mCategory, sStickerPart + 1 );
		_UI_MESSAGE_FULL( uiMsg );
	}

	bool C_AchievementsManager::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		/*
		//warning C4065: switch statement contains 'default' but no 'case' labels
		switch( pMessage->Type() )
		{
		default:
			return false;
		}
		*/
		return false;
	}

	//-------------------------------------------------------------------------------------------------------
	//			helper functions for the UI to move between All/SP/MP achievements filtering
	//-------------------------------------------------------------------------------------------------------

	Btl::Vector<Achievement*> C_AchievementsManager::GetFilteredStickerLocations(bool singlePlayer)
	{
		Btl::Vector<Achievement*> toReturn;

		for( int i=0; i<mStickerBookLocations.Size(); i++ )
		{
			if( singlePlayer ? mStickerBookLocations[i]->mSinglePlayer : mStickerBookLocations[i]->mMultiplayer )
			{
				toReturn.PushBack( mStickerBookLocations[i] );
			}
		}

		return toReturn;
	}

	C_Sticker* C_AchievementsManager::GetStickerInQuadrant(Achievement* a, int quadrant)
	{
		for( int i = 0; i < a->mStickers.Size(); i++)
		{
			C_Sticker* sticker = a->mStickers[i];
			switch( quadrant )
			{
				case 1: // top left
				{
					if( sticker->InTopLeftQuad() )
					{
						return sticker;
					}
					break;
				}
				case 2: // top right
				{
					if( sticker->InTopRightQuad() )
					{
						return sticker;
					}
					break;
				}
				case 3: // bottom right
				{
					if( sticker->InBottomRightQuad() )
					{
						return sticker;
					}
					break;
				}
				case 4: // bottom left
				{
					if( sticker->InBottomLeftQuad() )
					{
						return sticker;
					}
					break;
				}
				default:
				{
					LOGWARN(g_Log, "C_AchievementsManager::GetStickerInQuadrant > Invalid quadrant arg (%d), ignored!", quadrant);
					return NULL;
				}
			}
		}
		return NULL;
	}
}