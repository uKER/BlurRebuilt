#include "stdafx.h"

#include <GameLogic/GameModes/GameModeDefines.h>
#include <GameLogic/GameModes/GameModeSinglePlayer.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <Game/Game.h>
#include <GameLogic/Gamedata/GameData.h>

#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <GameLogic\profile\GameProfileManager.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <GameLogic/Structures/LevelResult.h>
#include <GameLogic/AchievementsManager.h>

#include <GameLogic\GameLoggingManager.h>
#include "Automation\GameDataLogging.h"

#include <Gamelogic\SocialNetworking\SocialNetworkManager.h>
#include <RaceBook\RaceBookManager.h>

#include <GameLogic\SocialNetworking\EventOwnerManager.h>

#include <SinglePlayer/SinglePlayer.h>
#include <Game\Achievements\AvatarAwardSystem.h>
#include <Game\Achievements\PictureAwardSystem.h>

Debug::Logger &g_GameModeSinglePlayerLogger( Debug::GetLogger("GameMode.SinglePlayer") );

namespace Game
{

void C_GameModeSinglePlayer::Initialise()
{
	SetInternalNameAndID("SinglePlayer",E_GameMode_SinglePlayer);
}

void C_GameModeSinglePlayer::Update()
{

}

void C_GameModeSinglePlayer::Start()
{

}

void C_GameModeSinglePlayer::SetupRace(void *data)
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	levelState.SetupRaceFromEvent(C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentEventId, 
								  C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentSPEventId);

	RaceBook::C_RaceBookProfile* prof = NULL;
	for( U32 i = 0; i < levelState.mNumCars; i++ )
	{
		prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)levelState.mCars[i].mCharacterID);

		if(prof)
			prof->mLifetimeStatistics.PreRaceUpdate();
	}

	// Now we have the event information, we can initialise the dialogue manager.
	
	// PaulM - This is now done in the AmaxWorldLayer 
	//C_InGameDialogueManager::Instance().InitRace();


	//C_AchievementsManager::Instance().PreRace();
}

void C_GameModeSinglePlayer::StartRace()
{
	EnableChangeViewOnAllViewports();

	C_GameProfileManager::Instance().GetPrimaryProfile()->mRetryingEvent=false;

	C_GameProfile *game_prof=C_GameProfileManager::Instance().GetPrimaryProfile();
	game_prof->mPassedEvent = false;

	C_SocialNetworkManager::Instance().StoreDataPreRace();
}

void C_GameModeSinglePlayer::InterpretRaceResults()
{
	const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	const Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	bool has_passed_event = true;
	RaceBook::C_RaceBookProfile *prof = NULL;
	C_GameProfile *game_prof=C_GameProfileManager::Instance().GetPrimaryProfile();

	bool alreadyCompleted = false;
	//if(game_prof->mCurrentEvent->mEventCompleted)
	//	alreadyCompleted = true;

	for( int i = 0; i < results.mCars[0].mPassedRule.Size(); i++ )
	{
		const LevelResultCar* car = &results.mCars[0];
		if(!results.mCars[0].mPassedRule[i])
			has_passed_event = false;
	}

	for( int i = 0; i < results.mCars[0].mFailedRule.Size(); i++ )
	{
		if(results.mCars[0].mFailedRule[i])
			has_passed_event = false;
	}
	
	game_prof->mUnlockedCar=0;
	AmaxEvent *ev = game_prof->GetCurrentEvent();

	game_prof->mPassedEvent=has_passed_event;

	//if(has_passed_event)
	//	game_prof->mCurrentEvent->mEventCompleted = true;	

	if(ev)
	{
//		if(ev->saveFinalPosition)
//			game_prof->mLastSavedRacePosition = results.mCars[0].mPosition;

		if(!results.mFake)
		{
			for(unsigned int a=0;a<levelState.mNumCars;a++)
			{
				prof=RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)levelState.mCars[a].mCharacterID);

				//work out fans from score
				int fans=(U32)results.mCars[a].mScore;

				if(ev->disableFans)
					fans = 0;

				// This will do it for everyone, but it doesn't really matter at this point - the individual
				// and player stats are intermingled beyond comprehension these days anyway.
				if(has_passed_event)
					prof->mLifetimeStatistics.mRacesPassed++;

				prof->mNumPreviousFans=prof->mNumFans;
				prof->mNumFans+=fans;

				//just adding a ceiling in case somebody goes mad and plays the game for ever
				if(prof->mNumFans > 1999999999)
					prof->mNumFans = 1999999999;

				// This gets set back to zero from the profile screen.
				prof->mUiFansEarned += fans;

				prof->mPreviousBankBalance=prof->mBankBalance;

				if(a == 0)
				{
#ifndef CFG_GOLD
					Game::C_GameProfile* gameProf = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
					F32 previousRaceTime = 0;
					F32 newRaceTime = 0;
					if(gameProf)
					{
						previousRaceTime = gameProf->mSpTotalRaceTime;
						gameProf->mSpTotalRaceTime += results.mCars[a].mStatistics.mTotalRaceTime;
						newRaceTime = gameProf->mSpTotalRaceTime;
					}
					AmaxDataLogging::LogSinglePlayerRaceFinished(fans, 0.0f, prof->CalculateProgress(), prof->mNumFans);		// No longer any cash

					AmaxEvent *ev = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentEvent();
					SP_SaveGame* pSave = SP_GetCurrentSaveGame();
					if(ev && pSave)
					{
						U8 starsEarnt = 0;
						SP_Event* pEvent = SP_GetEventById(pSave, levelState.mSpEventId);
						if(pEvent)
						{
							if(results.mRaceStatistics.mMedal >= kSP_Bronze)
							{
								SP_EventState state = SP_GetEventState(pSave, pEvent);
								switch(state)
								{
								case kSP_Incomplete:
									starsEarnt = 2 + results.mRaceStatistics.mMedal; //Base of 2 + 1-3 for medal
									break;
								default:
									starsEarnt = results.mRaceStatistics.mMedal >= state ? results.mRaceStatistics.mMedal - state : 0;
									break;
								}
								
								//Completing golden fan demand for first time
								if(!SP_IsEventFanRunComplete(pSave, pEvent) && results.mRaceStatistics.mGoldenFanDemandComplete)
								{
									starsEarnt++;
								}

								//Completing fan par for first time
								if(!SP_IsEventFanParComplete(pSave, pEvent) && results.mRaceStatistics.mFansEarned >= pEvent->fanPar)
								{
									starsEarnt++;
								}
							}
						}

						U8 lightsEarntThisEvent = 0;
						if(results.mRaceStatistics.mMedal >= kSP_Bronze)
						{
							lightsEarntThisEvent = 2 + results.mRaceStatistics.mMedal;
						}
						if(results.mRaceStatistics.mGoldenFanDemandComplete)
						{
							lightsEarntThisEvent++;
						}
						if(results.mRaceStatistics.mFansEarned >= pEvent->fanPar)
						{
							lightsEarntThisEvent++;
						}

						AmaxDataLogging::LogEventStatus(ev->tag.Text(), starsEarnt, SP_GetFans(pSave), SP_GetFans(pSave)+fans, previousRaceTime, newRaceTime, lightsEarntThisEvent);
					}
#endif
				}
			}
		}
	}

	prof = RaceBook::C_RaceBookManager::Instance().GetPlayerProfile();

	if(has_passed_event)
	{
		//store away what happened in the race
		bool store_away=false;

		Btl::Map<U32, RaceEventHistory>::Iterator it = game_prof->mSinglePlayerRaceHistory.Find(ev->ID);
		if(it == game_prof->mSinglePlayerRaceHistory.End())
		{
			// first time for this event, so just store it away
			store_away=true;
		}
		else
		{
			//if complete at better diff then store
			if(levelState.m_difficulty > it->GetValue().mDifficulty)
			{
				store_away=true;
			}
			
			//if complete at same diff but more fans then store
			if(levelState.m_difficulty == it->GetValue().mDifficulty)
			{
				if(prof->mNumFans - prof->mNumPreviousFans > it->GetValue().mFans)
				{
					store_away=true;
				}
			}
		}

		if(store_away)
		{
			RaceEventHistory rh;
			rh.mDifficulty = levelState.m_difficulty;
			rh.mFanDemandsComplete = results.mCars[0].mStatistics.mFanPickupsCollected;
			rh.mFans = prof->mNumFans - prof->mNumPreviousFans;
			rh.mHighestCombo = results.mCars[0].mStatistics.mLongestChain;
			rh.mVehicleUsed = levelState.mCars[0].mCarID;

			Btl::Map<U32, RaceEventHistory>::InsertionReturnType retType = game_prof->mSinglePlayerRaceHistory.Insert(ev->ID, rh);
			if(!retType.inserted)
			{
				Btl::Map<U32, RaceEventHistory>::Iterator it = retType.iterator;
				it->GetValue() = rh;
			}

		}
	}

	if(has_passed_event)
	{
		if(ev->ID == 0xAFA00028)
		{
			if(results.mCars[0].mPosition >= 16 || results.mCars[0].mStatistics.mWrecked)
			{
				game_prof->SetDifficulty(GAME_DIFFICULTY_EASY);
			}
			else if(results.mCars[0].mPosition <= 3)
			{
				game_prof->SetDifficulty(GAME_DIFFICULTY_HARD);
			}
			else
			{
				game_prof->SetDifficulty(GAME_DIFFICULTY_NORMAL);
			}
		}
	}

	if(prof)
	{
		prof->mLifetimeStatistics.PostRaceUpdate();
	}

	for(int a = 0; a < RaceBook::C_RaceBookManager().Instance().NumProfiles(); a++)
	{
		//update everyones levels/unlocks etc
		RaceBook::C_RaceBookManager().Instance().GetProfileByIndex(a)->PostRaceUpdate();
	}

	//C_AchievementsManager::Instance().PostRace(has_passed_event);

	C_SocialNetworkManager::Instance().PostRace(has_passed_event,false);
}

} // namespace Game
