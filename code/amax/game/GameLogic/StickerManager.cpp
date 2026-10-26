#include "stdafx.h"

#include <GameApp.h>

#include "StickerManager.h"
#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game\Achievements\AchievementEnums.h>
#include <Game\Achievements\AchievementSystem.h>

#include <Game/Messaging/GameflowMessages.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <GameLogic/Structures/MultiplayerAwards.h>
#include <GameLogic\Structures\LevelState.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic\Structures\MpResults.h>
#include <GameLogic/SinglePlayer/SinglePlayer.h>
#include <SystemProfile\SystemProfileManager.h>

#include <GameLogic\Profile\GameProfileManager.h>

#include <GameLogic\Structures\MultiplayerChallenge.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <SinglePlayer/SinglePlayer.h>

#include <Game\Achievements\AvatarAwardSystem.h>

#include <AI/AI_Amax.h>

namespace Game
{
	C_StickerManager C_StickerManager::s_instance;
	C_StickerBlackboardData C_StickerManager::s_blackboard;

	static U8 s_allOffensivePowerups = (1 << E_PICKUP_TYPE_BOMB) + (1 << E_PICKUP_TYPE_BARGE) 
				+ (1 << E_PICKUP_TYPE_BLINDFIRE) + (1 << E_PICKUP_TYPE_SHUNT) + (1 << E_PICKUP_TYPE_SHOCK);

	void C_StickerManager::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		stream << mStickers.Size();
		for(int i = 0; i < mStickers.Size(); i++)
		{
			stream << (int)(mStickers[i].Type());
			stream << mStickers[i];
		}

		stream << s_blackboard;
	}

	void C_StickerManager::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		ClearStickerProgress();
		int numSavedStickers = 0;
		F32 progress = 0;
		int type = -1;

		stream >> numSavedStickers;
		for(int i = 0; i < numSavedStickers; i++)
		{
			stream >> type;
			C_Sticker* s = GetSticker((BLUR_STICKER)type);
			if(s)
				stream >> *s;
			else
				stream >> progress; // pointer is invalid, sticker not found, bypass this value

		}

		stream >> s_blackboard;
	}

	void C_StickerManager::Init(int numStickers)
	{
		SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );

		s_blackboard.Initialise();

		BIZ_ASSERT(numStickers <= BLUR_STICKER_MAX);
		
		for( int i = 0; i < mStickers.Size(); i++ )
			mStickers[i].Reset();

		AddSubscriber(Game::E_GameflowMessage_RaceStarted);
		AddSubscriber(Game::E_GameflowMessage_RaceFinished);
		AddSubscriber(Game::E_GameflowMessage_QuitRace);
		AddSubscriber(Game::E_GameflowMessage_QuitAndWinRace);
		AddSubscriber(Game::E_GameflowMessage_RestartRace);
		AddSubscriber(Game::E_PickupMessage_CollectStopwatch);
		AddSubscriber(Game::E_GameLogicMessage_LapIncremented);
		AddSubscriber( E_VehicleLogicMsg_ShieldedPerk );
		AddSubscriber( E_VehicleLogicMsg_Repair );
		AddSubscriber( Game::E_GameflowMessage_ResultsCalculated );
		AddSubscriber(E_VehicleLogicMsg_HitWithAllDomes);
		AddSubscriber(E_VehicleLogicMsg_AvoidedAllDomes);
		AddSubscriber(E_VehicleLogicMsg_CrossedCheckpoint);
		AddSubscriber(E_GameLogicMessage_Passed_All_Rule);

		// Cache levelstate
		mpLevelState = &C_Game::GetInstance()->GetLevelState();
		BIZ_ASSERT_MSG(mpLevelState,"C_StickerManager::Init() - could not cache pointer to level state...");

		ClearCachedPtrs();
	}


	void C_StickerManager::InitForNewRace()
	{
		GetStickerBlackboard().InitForNextRace();
		mTimerVariables.Clear();
		GetStickerBlackboard().mCurrentCityId = C_Game::GetInstance()->GetLevelState().mCity;
		mpRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		if( mpRaceLogicLayer )
		{
			mpLogicActor = mpRaceLogicLayer->GetLogicVehicleActor( GetPrimaryProfileVehicleIndex() );
		}

		// Cache event info
		mCurrentTierIdx = mCurrentEventIdx = -1;
		SP_Event* pSpEvent = SP_GetEventById(SP_GetCurrentSaveGame(), mpLevelState->mSpEventId);
		if(pSpEvent)
		{
			mCurrentEventIdx = pSpEvent->index;
			if(pSpEvent->tier)
				mCurrentTierIdx = pSpEvent->tier->index;
		}
	}
	
	void C_StickerManager::Shutdown()
	{
		ClearCachedPtrs();
	}

	void C_StickerManager::Update(float frameTime)
	{
		PERF_EVENT("Stickers.Update");

		//let's incremement all the timers we currently have running
		for( int i = 0; i < mTimerVariables.Size(); i++ )
		{
			*(mTimerVariables[i]) += frameTime;
		}

		// if there's a better way of checking for this other than every frame, please tell me :(
		if( frameTime && GetStickerBlackboard().mInRace )
		{
			CheckHighSpeed();
			CheckBrightonFireworks();
		}
	}

	void C_StickerManager::StartTimer(float *pTimer, float startTime)
	{
		*pTimer = startTime;
		for(int i=0; i<mTimerVariables.Size(); i++)
		{
			// if we're already tracking this variable, merely zeroing it is enough
			if( pTimer == mTimerVariables[i] )
			{
				return;
			}
		}

		//not found, track this variable
		mTimerVariables.PushBack(pTimer);
	}
	
	void C_StickerManager::StopTimer(float *pTimer)
	{
		for( int i = 0; i < mTimerVariables.Size(); i++ )
		{
			if(mTimerVariables[i] == pTimer)
			{
				mTimerVariables.EraseByIndex(i);
			}
		}
	}

	void C_StickerManager::ClearStickerProgress()
	{
		for( int i = 0; i < mStickers.Size(); i++ )
		{
			mStickers[i].Reset();
		}

		GetStickerBlackboard().Initialise();
	}

	// No need to bother with this ever, called from AchievementsManager to keep passes through the data to a minimum
	Btl::Vector<C_Sticker*> C_StickerManager::AddStickersFromSorcery(Btl::Vector<C_Sticker> stickers)
	{
		Btl::Vector<C_Sticker*> toReturn;
		BIZ_ASSERT(stickers.Size() <= BLUR_STICKER_MAX);
		for(int i = 0; i < stickers.Size(); i++)
		{
			if (stickers[i].Type() == BLUR_STICKER_INVALID)
			{
				LOGERROR(g_Log, "AddStickersFromSorcery - INVALID Sticker type!");
				continue;
			}

			mStickers[stickers[i].Type()] = stickers[i];
			toReturn.PushBack(&mStickers[stickers[i].Type()]);
		}

		return toReturn;
	}

	// Given a vehicle index, this will return the pad index controlling it
	int C_StickerManager::GetControllerIndex(int idx)
	{
		if((int)(mpLevelState->mNumCars) > idx && mpLevelState->mCars[idx].mHuman && mpLevelState->mCars[idx].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
			return mpLevelState->mCars[idx].mController;

		return -1;
	}

	U8 C_StickerManager::GetPrimaryProfileVehicleIndex()
	{
		int primaryPad = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		for( U32 i=0; i<mpLevelState->mNumCars; i++ )
		{
			if( mpLevelState->mCars[i].mHuman && mpLevelState->mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER &&
				mpLevelState->mCars[i].mController == primaryPad )
			{
				return i;
			}
		}

		return 255;
	}

	C_Sticker* C_StickerManager::GetSticker(BLUR_STICKER sticker)
	{
		if (sticker <= BLUR_STICKER_INVALID || sticker >= BLUR_STICKER_MAX)
		{
			LOGWARN(g_Log, "C_StickerManager::GetSticker > You're looking for a sticker I don't have (enum number %d)!", (int)sticker);
			return NULL;
		}

		return &mStickers[sticker];		
	}

	bool C_StickerManager::IsStickerComplete(BLUR_STICKER sticker)
	{
		C_Sticker* pSticker = GetSticker(sticker);

		if (!pSticker)
			return false;

		return pSticker->IsComplete();
	}

	void C_StickerManager::DebugDumpSticker(BLUR_STICKER sticker)
	{
		C_Sticker* s = GetSticker(sticker);
		if(s)
			LOGINFO(g_Log, "Progress on sticker %d is %f out of %f", sticker, s->Progress(), s->Goal() );
	}

	void C_StickerManager::DebugDumpAllStickers()
	{
		LOGINFO(g_Log, "C_StickerManager >>>> Beginning dump of all stickers we have" );
		for(int i=0; i<mStickers.Size(); i++)
		{
			LOGINFO(g_Log, "Progress on sticker %d is %f out of %f", mStickers[i].Type(), mStickers[i].Progress(), mStickers[i].Goal() );
		}
		LOGINFO(g_Log, "C_StickerManager >>>> Sticker dump complete" );
	}

	void C_StickerManager::HandleDamageByPickup( U8 victimIndex, U8 firingIndex, int damage )
	{
		// if the primary pad vehicle took damage from a pickup
		if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == victimIndex )
		{
			GetStickerBlackboard().mDamageTakenThisRace += damage;
		}
		else if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == firingIndex )
		{
			GetStickerBlackboard().mDamageDoneThisRace += damage;
		}
	}

	//---------------------------------------------------------------------------------------------

	void C_StickerManager::CheckSinglePlayerStickers()
	{
		if (SP_SaveGame* sg = SP_GetCurrentSaveGame())
		{
			int tierCount = SP_GetTierCount(sg);

			int						fanPars						= 0;
			int						fanRuns						= 0;
			int						stars						= 0;
			int						primaryStars				= 0;
			int						bossDemands					= 0;
			Game::GAME_DIFFICULTY	worstDiff					= Game::GAME_DIFFICULTY_HARDCORE;
			int						maxPrimaryStars				= 0;
			int						maxStars					= 0;
			bool					completedDriftDemand		= false;
			bool					completedEvadeShocksDemand	= false;
			int						goldRaces					= 0;

			for(int tierNo = 0; tierNo < tierCount; tierNo++)
			{
				if (SP_Tier* tier = SP_GetTier(sg, tierNo))
				{
					for(int eventNo = 0; eventNo < tier->events.count; eventNo++)
					{
						if (SP_Event* event = tier->events[eventNo])
						{
							SP_EventState			state	= SP_GetEventState(sg, event);
							Game::GAME_DIFFICULTY	diff	= SP_GetEventDifficulty(sg, event);

							if (event->kind == kSP_BossEvent)
							{
								if (state == kSP_Gold)
									primaryStars += 8;
							}
							else
							{
								if (state == kSP_Gold)
								{
									primaryStars += 5;

									if (event->kind == kSP_RaceEvent)
										goldRaces++;
								}
								else if(state == kSP_Silver)
									primaryStars += 4;
								else if(state == kSP_Bronze)
									primaryStars += 3;
							}

							if (diff < worstDiff)
								worstDiff = diff;

							if (SP_IsEventFanParComplete(sg, event))
								fanPars++;

							if (SP_IsEventFanRunComplete(sg, event))
								fanRuns++;

							stars		+= SP_CountTotalEventStars(sg, event);
							maxPrimaryStars	+= SP_CountMaxPrimaryEventStars(sg, event);
							maxStars	+= SP_CountMaxEventStars(sg, event);
						}
					}

					if (SP_Boss* boss = tier->boss)
					{
						for(int bossNo = 0; bossNo < boss->demands.count; bossNo++)
						{
							if (SP_BossDemand* demand = boss->demands[bossNo])
							{
								if (SP_IsBossDemandComplete(sg, demand))
								{
									bossDemands++;

									if (demand->kind == kSP_BossDemand_Tier_Drift)
										completedDriftDemand = true;

									if (demand->kind == kSP_BossDemand_Tier_Evade_Full_Shocks)
										completedEvadeShocksDemand = true;
								}
							}
						}
					}
				}
			}

			// Fan pars

			if (fanPars > 0)
			{
				if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_FIRST_FAN_PAR))
					pSticker->AddProgress();
			}

			if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_EVERY_FAN_PAR))
				pSticker->SetProgress(fanPars);

			// Fan runs
			if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_COMPLETE_ALL_FAN_RUNS))
				pSticker->SetProgress(fanRuns);

			// Boss demands

			if (bossDemands > 0)
			{
				if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_FIRST_BOSS_DEMAND))
					pSticker->AddProgress();
			}

			if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_MEET_BOSS_DEMANDS))
				pSticker->SetProgress(bossDemands);

			if (completedDriftDemand)
			{
				if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_COMPLETE_FIRST_LEONARD_DEMAND))
					pSticker->AddProgress();
			}

			if (completedEvadeShocksDemand)
			{
				if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_COMPLETE_THIRD_SHOWDOWN_DEMAND))
					pSticker->AddProgress();
			}

			// Stars

			if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_EVERY_PRIMARY_LIGHT))
				pSticker->SetProgress(primaryStars);

			if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_EVERY_LIGHT))
				pSticker->SetProgress(stars);

			// Complete on hard

			if ((primaryStars >= maxPrimaryStars) && (worstDiff >= Game::GAME_DIFFICULTY_HARD))
			{
				if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_COMPLETE_GAME_HARD))
					pSticker->AddProgress();
			}

			// Win 40 races

			if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_WIN_SINGLE_PLAYER_RACES))
				pSticker->SetProgress(goldRaces);
		}
	}

	void C_StickerManager::CheckVisitedCities()
	{
		int numVisited = 0;
		for( int i=0; i<C_AchievementData::Instance().GetMiscStickerData().mAllCitiesInTheGame.count; i++)
		{
			if( GetStickerBlackboard().mCurrentCityId == C_AchievementData::Instance().GetMiscStickerData().mAllCitiesInTheGame[i].Id() )
			{
				GetStickerBlackboard().mVisitedCitiesBitset |= 1 << i;
			}

			if( ( GetStickerBlackboard().mVisitedCitiesBitset & (1 << i) ) )
			{
				numVisited++;
			}
		}

		C_Sticker* pTouristSticker = GetSticker(BLUR_STICKER_RACE_EACH_LOCATION);
		if( pTouristSticker )
		{
			pTouristSticker->SetProgress( (float)numVisited );
		}
	}

	// we know here the results have been calculated and so are safe to grab
	void C_StickerManager::CheckTopThreeInPinkCar()
	{
		if( Game::C_Game::GetInstance()->GetLevelResult().mCars[GetPrimaryProfileVehicleIndex()].mPosition <= 3 )
		{
			for( int i=0; i < C_AchievementData::Instance().GetMiscStickerData().mAllPinkPaintJobs.count; i++ )
			{
				// only award this sticker if we're in a shade of pre-approved pink
				if( mpLevelState->mCars[GetPrimaryProfileVehicleIndex()].mResprayID == 
						C_AchievementData::Instance().GetMiscStickerData().mAllPinkPaintJobs[i].Id() )
				{
					C_Sticker* pPinkSticker = GetSticker(BLUR_STICKER_TOP_THREE_IN_PINK);
					if( pPinkSticker )
					{
						pPinkSticker->AddProgress();
					}
				}
			}
		}
	}

	void C_StickerManager::CheckPickupStopwatch()
	{
		C_Sticker* pStopwatchSticker = GetSticker(BLUR_STICKER_STOPWATCH_POWERUPS);

		// Since this is Single Player only, we know that (as long as primary profile isn't -1) it was the primary profile who picked it up
		if( pStopwatchSticker && SysProfile::C_Manager::Instance().GetPrimaryPadNo() != -1 )
		{
			pStopwatchSticker->AddProgress();
		}
	}

	void C_StickerManager::CheckWreckYourFirst(U8 wreckerIndex)
	{
		C_Sticker* stickerWreck = GetSticker(BLUR_STICKER_WRECK_YOUR_FIRST);

		// If we are the primary profile, award the sticker
		if( stickerWreck && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(wreckerIndex) )
		{
			stickerWreck->AddProgress();
		}

	}

	void C_StickerManager::CheckWreckedBizarre( const U8 wreckerId, const U8 wreckedId )
	{
#ifndef PLATFORM_PS3
		if(mpLevelState->mCars[wreckedId].mBeatenBizarre)
		{
			mpLevelState->mCars[wreckerId].mBeatenBizarre = true;
			C_Sticker* pSticker = GetSticker(BLUR_STICKER_BEAT_BIZ_DEV);
			if( pSticker && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(wreckerId))
			{
				pSticker->AddProgress();
			}
		}
#endif // !PLATFORM_PS3
	}
	
	void C_StickerManager::CheckWreckFirstBoss(U8 wreckerIndex)
	{
		Game::C_RaceBookLayer* pRaceBookLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

		if ( !pRaceBookLayer )
		{
			LOGERROR(g_Log, "C_StickerManager::CheckWreckFirstBoss > Problem getting RaceBookLayer, returning early...");
			return;
		}
		
		// If this is a boss "one on one" and we have wrecked someone, we know it's a boss! Elementary, my dear Watson!
		if( pRaceBookLayer->GetRaceMode() == E_RaceMode_BossBattle && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(wreckerIndex) )
		{
			C_Sticker* stickerWreck = GetSticker(BLUR_STICKER_WRECK_FIRST_BOSS);
			if( stickerWreck )
			{
				stickerWreck->AddProgress();
			}
		}
	}

	void C_StickerManager::CheckWreckCarsInDestruction(U8 wreckerIdx)
	{
		Game::C_RaceBookLayer* pRaceBookLayer = C_GameApp::GetInstance()->GetRaceBookLayer();

		if ( !pRaceBookLayer )
		{
			LOGERROR(g_Log, "C_StickerManager::CheckWreckCarsInDestruction > Problem getting RaceBookLayer, returning early...");
			return;
		}
		
		// If this is a boss "one on one" and we have wrecked someone, we know it's a boss! Elementary, my dear Watson!
		if( pRaceBookLayer->GetRaceMode() == E_RaceMode_Destruction && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(wreckerIdx) )
		{
			C_Sticker* stickerWreck = GetSticker(BLUR_STICKER_WRECK_CARS_IN_DESTRUCTION);
			if( stickerWreck )
			{
				stickerWreck->AddProgress();
			}
		}
	}

	void C_StickerManager::CheckWreckByThreeBolts(U8 wreckerIdx)
	{
		C_Sticker* pBoltThreeSticker = GetSticker(BLUR_STICKER_THREE_BOLT_WRECK);

		// If we are the primary profile, award the sticker
		if( pBoltThreeSticker && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(wreckerIdx)
			&& GetStickerBlackboard().mNumTimesHitSameCarWithBolt >= pBoltThreeSticker->Goal())
		{
			pBoltThreeSticker->AddProgress(); //one shot
		}
	}

	void C_StickerManager::CheckCustomizeLoadout()
	{
		C_Sticker* sticker = GetSticker(BLUR_STICKER_CUSTOMISE_A_LOADOUT);

		// If we are the primary profile, award the sticker
		if( sticker )
		{
			sticker->AddProgress();
		}
	}

	void C_StickerManager::CheckWinWithCustomLoadout()
	{
		C_GameProfile* pGameProfile = C_GameProfileManager::Instance().GetPrimaryProfile();
		C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
		bool isRanked = true; //Challenge only available in ranked matches

		const UI::E_MULTIPLAYER_LOADOUT_TYPE type = pMultiplayerProfile->GetLoadouts(isRanked).GetCurrentLoadOutType();
		//No modshop selection. Just a precaution.
		if(type == UI::E_MULTIPLAYER_LOADOUT_TYPE_NONE)
		{
			return;
		}

		const Game::MultiplayerLoadOut& loadOut = pMultiplayerProfile->GetLoadouts(isRanked).GetLoadOut( type );

		MultiplayerAbilityLoadOut& originalLoadout = C_MultiplayerData::Instance().m_pConfig->m_LoadOuts[type];

		//Because of default null values in sorcery (By design) we need an additional check to avoid death
		if(originalLoadout.mAbilitySlot1.Object() && originalLoadout.mAbilitySlot2.Object() && originalLoadout.mAbilitySlot3.Object())
		{
			if(loadOut.GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT1) == originalLoadout.mAbilitySlot1.Object()->mAbilityId
				&& loadOut.GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT2) == originalLoadout.mAbilitySlot2.Object()->mAbilityId
				&& loadOut.GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT3) == originalLoadout.mAbilitySlot3.Object()->mAbilityId)
			{
				return;
			}
		}
		
		//If they haven't unlocked the modshop then we catch this here and avoid giving out the sticker
		if(loadOut.GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT1) == UI::E_MultiplayerVehicleAbilities_None
			&& loadOut.GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT2) == UI::E_MultiplayerVehicleAbilities_None
			&& loadOut.GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT3) == UI::E_MultiplayerVehicleAbilities_None)
		{
			return;
		}

		C_Sticker* sticker = GetSticker(BLUR_STICKER_WIN_WITH_CUSTOM_LOADOUT);

		// If we are the primary profile, award the sticker
		if( sticker )
		{
			sticker->AddProgress();
		}
	}

	void C_StickerManager::CheckWinInPlaylist()
	{
		C_Sticker* pWinPlaylistSticker = GetSticker(BLUR_STICKER_WIN_ANY_PLAYLIST);
		if( pWinPlaylistSticker )
		{
			pWinPlaylistSticker->AddProgress();
		}

		//	Since "Hardcore" isn't a true race mode, we need to grab the UI enum from the level state.
		if( mpLevelState->mUiRaceType == UI::E_RaceType_Hardcore )
		{
			C_Sticker* pWinHardcoreSticker = GetSticker(BLUR_STICKER_WIN_HARDCORE_RACING);
			if( pWinHardcoreSticker )
			{
				pWinHardcoreSticker->AddProgress();
			}
		}
	}

	void C_StickerManager::CheckMpRaceFinishedStickers()
	{
		S8 primaryProfileResultsIndex = C_MpResults::Instance().m_PlayerSortedIndices[SysProfile::C_Manager::Instance().GetPrimaryPadNo()];
		if(primaryProfileResultsIndex < 0)
			return;
		S8 primaryProfileVehicleIndex = C_MpResults::Instance().m_SortedVehicleResults[primaryProfileResultsIndex].m_LocalIndex;
		if(primaryProfileVehicleIndex < 0)	//	Safety catch
			return;

		CheckBeatRank50(primaryProfileResultsIndex);
		CheckPhotoFinish(primaryProfileResultsIndex);
		CheckTeamWinner(primaryProfileResultsIndex);

		//For winners only
		if(primaryProfileResultsIndex == 0)
		{
			CheckWinWithCustomLoadout();
			CheckWinInPlaylist();
		}
	}

	//	Did we place ahead of a Rank 50 player?
	void C_StickerManager::CheckBeatRank50( S8 resultsIndex )
	{
		if(C_AchievementSystem::Instance().HasAchievement(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), BLUR_ACHIEVEMENT_GIANT_KILLER))
			return;

		int maxRank = C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count - 1; //	Ranks go from 0 to 49

		if(resultsIndex + 1 >= (S8)mpLevelState->mNumCars)	//	If you're last then you're out of luck
			return;

		for(U32 i = resultsIndex + 1; i < mpLevelState->mNumCars; i++)	// position is 1 for winner, so this starts at the next element in the results vector
		{
			if(mpLevelState->mCars[C_MpResults::Instance().m_SortedVehicleResults[i].m_LocalIndex].mMultiplayerRank == maxRank)
			{
				C_Sticker* pSticker = GetSticker(BLUR_STICKER_BEAT_A_RANK_FIFTY);
				if(pSticker)
				{
					pSticker->AddProgress();
				}
				break;
			}
		}
	}

	void C_StickerManager::CheckMpChallenges(UI::E_MULTIPLAYER_CHALLENGE_TYPE type, S8 tier)
	{
		//Check the complete challenge X and Y stickers
		if(!IsStickerComplete(BLUR_STICKER_CHALLENGE_X_DONE))
		{
			MultiplayerChallenge* challenge = C_AchievementData::Instance().mAchievementsList[0]->mMiscStickerData.Object()->mChallengeX.Object();
			if(challenge->mType == type && challenge->mTiers.count-1 == tier)
			{
				C_Sticker* pSticker = GetSticker(BLUR_STICKER_CHALLENGE_X_DONE);
				if( pSticker )
				{
					pSticker->AddProgress();
				}
			}
		}
		
		if(!IsStickerComplete(BLUR_STICKER_CHALLENGE_Y_DONE))
		{
			MultiplayerChallenge* challenge = C_AchievementData::Instance().mAchievementsList[0]->mMiscStickerData.Object()->mChallengeY.Object();
			if(challenge->mType == type && challenge->mTiers.count-1 == tier)
			{
				C_Sticker* pSticker = GetSticker(BLUR_STICKER_CHALLENGE_Y_DONE);
				if( pSticker )
				{
					pSticker->AddProgress();
				}
			}
		}

		//Then also check to see if we have completed 1 challenge pack or half completed 3 of them
		if(!IsStickerComplete(BLUR_STICKER_CHALLENGE_PACK_FULL_DONE) || !IsStickerComplete(BLUR_STICKER_CHALLENGE_PACK_THREE_HALF_DONE))
		{
			C_GameProfile* pGameProfile = C_GameProfileManager::Instance().GetPrimaryProfile();
			C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

			Game::C_MultiplayerChallengeSystem& challengeSystem = pMultiplayerProfile->GetChallengeSystem();
			//If we dont do this our challenge progress will be out of date...
			challengeSystem.CachePackProgress();

			XtArray<XtHandle <MultiplayerChallengePack> >& challengePacks = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks;
			F32 packCount = 0;
			F32 completedCount = 0;
			U8 challengePacksHalfComplete = 0;
			for(int i = 0; i < challengePacks.count; i++)
			{
				if(challengePacks[i]->mType == UI::E_ChallengePack_Car)
				{
					packCount = (F32)challengeSystem.GetCarPackTotal();
					completedCount = (F32)challengeSystem.GetCarPackCounter();
				}
				else
				{
					packCount = (F32)challengePacks[i].Object()->mChallenges.count;
					completedCount = (F32)challengeSystem.GetPackProgressCounter(challengePacks[i].Object()->mType);
				}

				F32 progress = completedCount / packCount;
				if(progress >= 0.5f)
				{
					challengePacksHalfComplete++;
					if(progress >= 1.0f)
					{
						C_Sticker* pSticker = GetSticker(BLUR_STICKER_CHALLENGE_PACK_FULL_DONE);
						pSticker->AddProgress();
					}
				}

			}

			C_Sticker* pSticker = GetSticker(BLUR_STICKER_CHALLENGE_PACK_THREE_HALF_DONE);
			if(pSticker && pSticker->Progress() < challengePacksHalfComplete)
			{
				pSticker->AddProgress(challengePacksHalfComplete-pSticker->Progress());
			}
		}
	}

	void  C_StickerManager::CheckDominator(const U8 numAwardsLocalPlayerWon)
	{
		// If player has won something, add to consecutively won awards
		if(numAwardsLocalPlayerWon > 0)
		{
			GetStickerBlackboard().mNumAwardsLocalPlayerWonConsecutively += 1; // Player must win 'at least' one award in consecutive races to get this sticker. 
		}
		else
		{
			// Player hasn't won anything.. reset their progress
			GetStickerBlackboard().mNumAwardsLocalPlayerWonConsecutively = 0;
		}

		// Early out if havent won anything
		if(GetStickerBlackboard().mNumAwardsLocalPlayerWonConsecutively <= 0)
		{
			return;
		}
		else
		{
			// PART 1 / 3
			if(!IsStickerComplete(BLUR_STICKER_FIRST_AWARD)) // requires winning one award so if here.. can ++ progress
			{
				GetSticker(BLUR_STICKER_FIRST_AWARD)->AddProgress(); 
			}

			// PART 2 / 3
			if( !IsStickerComplete(BLUR_STICKER_WIN_TWO_AWARDS))
			{
				Game::C_Sticker* sticker = GetSticker(BLUR_STICKER_WIN_TWO_AWARDS);

				if( GetStickerBlackboard().mNumAwardsLocalPlayerWonConsecutively >= sticker->Goal())
				{
					sticker->AddProgress(); 
				}
			}
			
			// PART 3 / 3
			if( !IsStickerComplete(BLUR_STICKER_WIN_THREE_AWARDS))
			{
				Game::C_Sticker* sticker = GetSticker(BLUR_STICKER_WIN_THREE_AWARDS);

				if( GetStickerBlackboard().mNumAwardsLocalPlayerWonConsecutively >= sticker->Goal() )
				{	
					sticker->AddProgress(); 
				}
			}
		}
	}


	void C_StickerManager::CheckWreckYourRival(const U8 wreckerID)
	{
		if(!GetSticker(BLUR_STICKER_WRECK_RIVAL)->IsComplete())
		{
			// Motorman Sticker 2/3
			// If this was us,
			if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(wreckerID) )
			{
				GetSticker(BLUR_STICKER_WRECK_RIVAL)->AddProgress(); 
			}
		}
	}

	void C_StickerManager::CheckMotorman(const bool won, const S32 points)
	{
		// Motorman Sticker 1/3
		// Did we win, 
		if( won &&
			!GetSticker(BLUR_STICKER_WIN_MOTOR_MASH)->IsComplete())
		{
			GetSticker(BLUR_STICKER_WIN_MOTOR_MASH)->AddProgress();
		}

		// Motorman Sticker 3/3
		// Did we achieve required points?
		C_Sticker* stickerMotorMashPoints = GetSticker(BLUR_STICKER_POINTS_IN_ONE_MASH);

		if( stickerMotorMashPoints &&
			points >= stickerMotorMashPoints->Goal() &&
			!stickerMotorMashPoints->IsComplete() )
		{
			stickerMotorMashPoints->AddProgress();
		}
	}

	void C_StickerManager::CheckRankFifty()
	{
		// These are the stats we require
		bool unlockedFinalRankedPlaylist = false;
		bool unlockedAllChallengePacks   = false;
		bool unlockedAllVehicleMods	     = false;
		bool reachedRankFifty			 = false;

		// Calculate them!
		Game::C_GameProfile* pGameProfile    = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		Game::C_MultiplayerProfile* pProfile = pGameProfile->GetMultiplayerProfile();
		reachedRankFifty		         = ( pProfile->GetCurrentLevel() == 49 ); // since [0,49]

		CalculateIfFinalUnlocksAchieved(unlockedFinalRankedPlaylist,unlockedAllChallengePacks,unlockedAllVehicleMods, pProfile); 

		// Hand out awards based on this.
		DealOutRankFiftyStickers(unlockedFinalRankedPlaylist, unlockedAllChallengePacks,unlockedAllVehicleMods, reachedRankFifty);
	}

	// Called from RaceModeDestruction upon the wrecking of a red car
	void C_StickerManager::WreckedRedCarInDestruction()
	{
		// Since this is a single player achievement, as long as the primary profile exists, we can award progress (no one else could have done the wrecking)
		C_Sticker* stickerRedWreck = GetSticker(BLUR_STICKER_WRECK_RED_CARS);
		if( stickerRedWreck && SysProfile::C_Manager::Instance().GetPrimaryPadNo() != -1 )
		{
			stickerRedWreck->AddProgress();
		} 
	}

	// Called from VLC_FanDemands upon going through a Fan Gate
	void C_StickerManager::HitFanGate()
	{
		C_Sticker* pFanGateSticker = GetSticker(BLUR_STICKER_THROUGH_FAN_GATES);
		if( pFanGateSticker )
		{
			pFanGateSticker->AddProgress();
		}
	}

	// Called from VLC_FanDemands upon completing a Fan Demands
	void C_StickerManager::CompletedFanDemand()
	{
		GetStickerBlackboard().mFanDemandsCompleteThisEvent++;
		if( C_Sticker* pFanDemandSticker = GetSticker(BLUR_STICKER_COMPLETE_FAN_DEMANDS) )
		{
			pFanDemandSticker->AddProgress();
		}

		if( C_Sticker* pAllFdSticker = GetSticker(BLUR_STICKER_ALL_FAN_DEMANDS_IN_EVENT ) )
		{
			if( GetStickerBlackboard().mFanDemandsCompleteThisEvent >= pAllFdSticker->Goal() )
				pAllFdSticker->AddProgress(); // one-shot
		}
	}

	// Called from VLC_FanDemands upon completing a Fan Gate
	void C_StickerManager::CompletedFanGate()
	{
		if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_FIRST_FAN_RUN))
			pSticker->AddProgress();

		if (C_Sticker* pSticker = GetSticker(BLUR_STICKER_COMPLETE_FAN_RUNS))
			pSticker->AddProgress();
	}

	// Called from lua_Amax on the completion of a boss race
	void C_StickerManager::CheckBossBeaten(int tier)
	{
		C_Sticker* pSticker = NULL;
		switch( tier ) //tiers running from 0 -> 8
		{
			case 0:
				pSticker = GetSticker(BLUR_STICKER_BEAT_SHANNON);
				break;
			case 1:
				pSticker = GetSticker(BLUR_STICKER_BEAT_KHAN);
				break;
			case 2:
				pSticker = GetSticker(BLUR_STICKER_BEAT_RHYMER);
				break;
			case 3:
				pSticker = GetSticker(BLUR_STICKER_BEAT_DRAKE );
				break;
			case 4:
				pSticker = GetSticker(BLUR_STICKER_BEAT_NATALYA);
				break;
			case 5:
				pSticker = GetSticker(BLUR_STICKER_BEAT_TALON);
				break;
			case 6:
				pSticker = GetSticker(BLUR_STICKER_BEAT_AYUMI_HARUMI);
				break;
			case 7:
				pSticker = GetSticker(BLUR_STICKER_BEAT_CARVER);
				break;
			case 8:
				pSticker = GetSticker(BLUR_STICKER_BEAT_SHANNON_AGAIN);
				break;
		}

		if(pSticker)
		{
			pSticker->AddProgress();
		}
	}

	// Called from Playlist.cpp on completion of a championship
	void C_StickerManager::CheckTeamWinner( S8 resultsIndex )
	{
		if(mpLevelState->mTeams && (C_MpResults::Instance().GetWinningTeam() == C_MpResults::Instance().m_SortedVehicleResults[resultsIndex].m_Team))
		{
			C_Sticker* pTeamSticker = GetSticker(BLUR_STICKER_WIN_TEAM);
			if( pTeamSticker )
			{
				pTeamSticker->AddProgress();
			} 
		}
	}

	// called from vehicleActor where the trigger message is received
	void C_StickerManager::CheckAmboyOilfieldsTriggerRegion(U8 triggeringVehicleIndex)
	{
		if( GetControllerIndex(triggeringVehicleIndex) != SysProfile::C_Manager::Instance().GetPrimaryPadNo() )
			return;

		GetStickerBlackboard().mIsInAmboyOilfield = !(GetStickerBlackboard().mIsInAmboyOilfield);

		if( mpLevelState && GetStickerBlackboard().mCurrentCityId == C_AchievementData::Instance().GetMiscStickerData().mSmokingOilfieldsCity.Id()
			&& mpLogicActor && mpLogicActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_LOW )
		{
			// we've just gone into the amboy oilfield on fire!
			if( GetStickerBlackboard().mIsInAmboyOilfield )
			{
				GetStickerBlackboard().mCanGetSmokingOilfieldAchievement = true;
			}
			// we've just exited the oilfield on fire, plus we were on fire when we entered!
			else if( GetStickerBlackboard().mCanGetSmokingOilfieldAchievement )
			{
				C_Sticker* pSmokingSticker = GetSticker(BLUR_STICKER_AMBOY_OILFIELDS_SMOKING_CAR);
				if( pSmokingSticker )
				{
					pSmokingSticker->AddProgress();
				}
			}
		}
	}

	// called from vehicleActor where the trigger message is received
	void C_StickerManager::CheckGasStationTriggerRegion(U8 triggeringVehicleIndex)
	{
		if( GetControllerIndex(triggeringVehicleIndex) != SysProfile::C_Manager::Instance().GetPrimaryPadNo() )
			return;

		GetStickerBlackboard().mIsInNitroThroughTriggerBox = !(GetStickerBlackboard().mIsInNitroThroughTriggerBox);

		// we've just gone into the trigger box!
		if( GetStickerBlackboard().mIsInNitroThroughTriggerBox )
		{
			if( GetStickerBlackboard().mIsUsingNitro )
			{
				GetStickerBlackboard().mCanGetNitroTriggerBoxAchievement = true;
			}
		}
		// we've just come out of the trigger box!
		else
		{
			if( GetStickerBlackboard().mIsUsingNitro && GetStickerBlackboard().mCanGetNitroTriggerBoxAchievement )
			{
				if( mpLevelState && GetStickerBlackboard().mCurrentCityId == C_AchievementData::Instance().GetMiscStickerData().mHollywoodHillsSignCity.Id() )
				{
					C_Sticker* pWoodSmashSticker = GetSticker(BLUR_STICKER_HOLLYWOOD_NITRO_NO_DAMAGE);
					if( pWoodSmashSticker )
					{
						pWoodSmashSticker->AddProgress();
					}
				}
				else if( mpLevelState && GetStickerBlackboard().mCurrentCityId == C_AchievementData::Instance().GetMiscStickerData().mGasStationCity.Id() )
				{
					C_Sticker* pGasSticker = GetSticker(BLUR_STICKER_NITRO_THROUGH_GAS_STATION);
					if( pGasSticker )
					{
						pGasSticker->AddProgress();
					}
				}
			}

			GetStickerBlackboard().mCanGetNitroTriggerBoxAchievement = false;
		}

	}

	void C_StickerManager::CheckBrightonFireworks()
	{
		// are we on Brighton? are we in the Hummer?
		if( GetStickerBlackboard().mCurrentCityId == C_AchievementData::Instance().GetMiscStickerData().mFireworksCity.Id() && mpLogicActor 
			&& mpLevelState->mCars[mpLogicActor->GetVehicleIndex()].mCarID == C_AchievementData::Instance().GetMiscStickerData().mFireworksViewingVehicle.Id() )
		{
			// let's avoid doing all this calculation if we've done the sticker already!
			C_Sticker* pFireworksSticker = GetSticker(BLUR_STICKER_VIEW_BRIGHTON_FIREWORKS);
			if( pFireworksSticker && !(pFireworksSticker->IsComplete()) && mpLogicActor->GetPositionInfo().mSpeedInMph < 2.0f )
			{
				Maths::HMat43 vehicleMat = mpLogicActor->GetGraphicalActor(mpLogicActor->GetVehicleIndex())->GetMatrix();
				Maths::HVec3 vehicleFacing = vehicleMat.AZ();
				Maths::HVec3 fireworksDirection = Maths::HVec3( -685.0f, vehicleMat.AW().Y(), 40.0f ) - vehicleMat.AW();

				float theta = Maths::ArcCos( Maths::DotProduct( vehicleFacing, fireworksDirection ) / ( vehicleFacing.Length() * fireworksDirection.Length() ) );

				if( theta <= C_AchievementData::Instance().GetMiscStickerData().mMaxFireworksViewingAngle )
				{
					pFireworksSticker->AddProgress();
				}
			}
		}
	}

	// Helper func
	void C_StickerManager::CalculateIfFinalUnlocksAchieved(bool& unlockedFinalRankedPlaylist, bool& unlockedAllChallengePacks, bool& unlockedAllVehicleMods,Game::C_MultiplayerProfile* pProfile)
	{
		// Init vars
		U8 finalPlaylistUnlockRank   = C_MultiplayerData::Instance().GetFinalPlaylistUnlockRank();
		U8 finalChallengeUnlockRank  = C_MultiplayerData::Instance().GetFinalChallengePackUnlockRank();
		U8 finalModUnlockRank		 = C_MultiplayerData::Instance().GetFinalModUnlockRank();

		// Calc player's current rank
		U8 currentPlayerRank = pProfile->GetCurrentLevel(); 

		// Now compare the required ranks with the player's current rank
		if( finalPlaylistUnlockRank != -1 &&
			currentPlayerRank >= finalPlaylistUnlockRank)
		{
			unlockedFinalRankedPlaylist = true; 
		}
		else
		{
			unlockedFinalRankedPlaylist = false; 
		}

	    if( finalChallengeUnlockRank != -1 &&
			currentPlayerRank >= finalChallengeUnlockRank)
	    {
			unlockedAllChallengePacks = true;
	    }
		else
		{
			unlockedAllChallengePacks = false;
		}

	    if( finalModUnlockRank != -1 &&
			currentPlayerRank >= finalModUnlockRank)
	    {
			unlockedAllVehicleMods = true;
	    }
		else
		{
			unlockedAllVehicleMods = false;
		}
	}

	void C_StickerManager::DealOutRankFiftyStickers(bool unlockedFinalRankedPlaylist, bool unlockedAllChallengePacks,bool unlockedAllVehicleMods, bool reachedRankFifty)
	{
		// Part 1 / 4 - Unlock all ranked playlists
		if(!IsStickerComplete(BLUR_STICKER_UNLOCK_ALL_PLAYLISTS) && 
			unlockedFinalRankedPlaylist)
		{
			GetSticker(BLUR_STICKER_UNLOCK_ALL_PLAYLISTS)->AddProgress();
		}

		// Part 2 / 4 - Unlock all challenge packs
		if(!IsStickerComplete(BLUR_STICKER_UNLOCK_ALL_CHALLENGE_PACKS) && 
			unlockedAllChallengePacks)
		{
			GetSticker(BLUR_STICKER_UNLOCK_ALL_CHALLENGE_PACKS)->AddProgress();
		}

		// Part 3 / 4 - Unlock all vehicle mods
		if(!IsStickerComplete(BLUR_STICKER_UNLOCK_ALL_MODS) && 
			unlockedAllVehicleMods)
		{
			GetSticker(BLUR_STICKER_UNLOCK_ALL_MODS)->AddProgress();
		}

		// Part 4 / 4 - reached rank 50
		if(!IsStickerComplete(BLUR_STICKER_REACH_RANK_FIFTY) && 
			reachedRankFifty)
		{
			GetSticker(BLUR_STICKER_REACH_RANK_FIFTY)->AddProgress();
		}
	}

	void C_StickerManager::CheckPhotoFinish( U8 resultsIndex )
	{
		if( mpLevelState->mRaceMode == E_RaceMode_Multiplayer_Racing && mpLevelState->mNumCars > (resultsIndex + 1))
		{
			U8 beatenIndex = resultsIndex + 1;
			F32 photoFinishTime = C_AchievementData::Instance().GetMiscStickerData().mMaxPhotoFinishWinTimeDifference;
			F32 theirTime = C_MpResults::Instance().m_SortedVehicleResults[beatenIndex].m_Statistics[UI::E_MultiplayerStat_RaceTime];
			F32 myTime = C_MpResults::Instance().m_SortedVehicleResults[resultsIndex].m_Statistics[UI::E_MultiplayerStat_RaceTime];

			//	If both ourselves and the player below us in the results finished the race, and within the specified time of each other
			if(C_MpResults::Instance().m_SortedVehicleResults[resultsIndex].m_State == E_VEHICLE_STATE_FINISHED &&
				C_MpResults::Instance().m_SortedVehicleResults[beatenIndex].m_State == E_VEHICLE_STATE_FINISHED &&
				(theirTime - myTime) <= photoFinishTime)
			{
				C_Sticker* pSticker = GetSticker(BLUR_STICKER_PHOTO_FINISH);
				if(pSticker)
				{
					pSticker->AddProgress();
				}
			}
		}
	}

	void C_StickerManager::CheckEvadedOrDestroyedFirstPowerup(U8 vehicleIndex)
	{
		C_Sticker* stickerEvaded = GetSticker(BLUR_STICKER_EVADE_FIRST_POWERUP);

		if( stickerEvaded && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(vehicleIndex) )
		{
			stickerEvaded->AddProgress();
		}
	}

	void C_StickerManager::CheckAvoidShuntsByDriving(const C_VehicleLogicMsg_EvadedPowerUp* pMsg)
	{
		C_Sticker* shuntEvaded = GetSticker(BLUR_STICKER_AVOID_SHUNTS_BY_DRIVING);
		if( pMsg->mPickupType == E_PICKUP_TYPE_SHUNT && !GetStickerBlackboard().mShuntWasEvadedByDestroying && shuntEvaded 
			&& SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex) )
		{
			shuntEvaded->AddProgress();
		}
	}

	void C_StickerManager::CheckDestroyMines(const C_VehicleLogicMsg_PowerUpDestroyedByPowerUp* pMsg)
	{
		C_Sticker* stickerDestroyed = GetSticker(BLUR_STICKER_DESTROY_MINES);

		if( stickerDestroyed && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex)
				&& pMsg->mPickupType == E_PICKUP_TYPE_BOMB && pMsg->mPickupSubType != E_PICKUP_SUB_TYPE_MINI && pMsg->mPickupSubType != E_PICKUP_SUB_TYPE_MOD)
		{
			stickerDestroyed->AddProgress();
		}
	}

	void C_StickerManager::CheckDestroyShunts(const C_VehicleLogicMsg_PowerUpDestroyedByPowerUp* pMsg)
	{
		C_Sticker* stickerDestroyed = GetSticker(BLUR_STICKER_DESTROY_INCOMING_SHUNTS);

		if( stickerDestroyed && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex)
				&& pMsg->mPickupType == E_PICKUP_TYPE_SHUNT)
		{
			stickerDestroyed->AddProgress();
		}
	}

	// we know here that the primary pad owns the vehicle that did the defending (pMsg->mDefendingVehicleIndex)
	void C_StickerManager::CheckShieldedAttack(const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg)
	{
		C_Sticker* pShieldSticker = GetSticker(BLUR_STICKER_SHIELD_ATTACKS);
		if( pShieldSticker )
		{
			pShieldSticker->AddProgress();
		}

		GetStickerBlackboard().mAttacksShieldedThisShield++;

		C_Sticker* pShieldMultipleSticker = GetSticker(BLUR_STICKER_SHIELD_ATTACKS_SINGLE_SHIELD);
		if( GetStickerBlackboard().mAttacksShieldedThisShield >= pShieldMultipleSticker->Goal() )
		{
			pShieldMultipleSticker->AddProgress();
		}

		C_Sticker* pShieldCloseSticker = GetSticker(BLUR_STICKER_SHIELD_CLOSE_RANGE);
		if( pShieldCloseSticker 
			&& GetStickerBlackboard().mShieldElapsedTime < C_AchievementData::Instance().GetMiscStickerData().mMaxTimeForShieldPowerupToBeCloseRange )
		{
			pShieldCloseSticker->AddProgress();
		}
	}

	// the primary profile has repaired some health
	void C_StickerManager::CheckRepairHealth(const C_VehicleLogicRepairMsg* pMsg)
	{
		C_Sticker* pRepairSticker = GetSticker(BLUR_STICKER_REPAIR_HEALTH);
		if(mpLogicActor)
		{
			int amountHealed = Maths::Min( mpLogicActor->GetMaxHealth() - mpLogicActor->GetCurrentHealth(), pMsg->mHealAmount );
			if( pRepairSticker && amountHealed )
			{
				// add the number of "health pips" the player has repaired
				pRepairSticker->AddProgress( (float)(amountHealed) / 
											(float)(mpLogicActor->GetRLC()->GetRLSystem()->mDamageSystem->mParameters->mHealthIncrement) );
			}

			//sneakily, in here we can also update the amount we've repaired this race
			GetStickerBlackboard().mAmountRepairedThisRace += amountHealed;
		}
	}

	// Check all stickers which require us to be the aggressive vehicle (we know here that we were == pMsg->mFiringVehicle
	// and also that that vehicle is on the primary pad)
	void C_StickerManager::CheckPickupAggressiveHits(const C_VehicleLogicMsg_HitByPerkMsg* pMsg)
	{
		if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_DECOY)
		{
			// Then we are done. 
			return;
		}

		C_Sticker* pSticker = NULL;
		switch(pMsg->mPickupType)
		{
			case E_PICKUP_TYPE_BARGE:
			{
				// Dont count last gasp
				if(pMsg->mPickupSubType != E_PICKUP_SUB_TYPE_MOD)
				{
					GetStickerBlackboard().mNumVehiclesHitByOneBarge++; //keep track of the number of people we hit with a barge (this is zeroed when we fire one)

					pSticker = GetSticker(BLUR_STICKER_BARGE_CARS);

					StartTimer(&(GetStickerBlackboard().mBargedSomeGuyOffCliff)); // if we soon encounter a "drown" or "off cliff" score, it was probably this one!
				}
				break;
			}
			case E_PICKUP_TYPE_SHOCK:
			{
				pSticker = GetSticker(BLUR_STICKER_SHOCK_CARS);

				// we've hit the same car twice with shock, this is a sticker criteria!
				if( (GetStickerBlackboard().mVehiclesHitByShockBitset & (1 << pMsg->mVehicleIndex)) != 0 )
				{
					C_Sticker* pShockTwice = GetSticker(BLUR_STICKER_SHOCK_CAR_TWICE);
					if(pShockTwice)
					{
						pShockTwice->AddProgress();
					}

					// zero this bit again
					GetStickerBlackboard().mVehiclesHitByShockBitset &= ~(1 << pMsg->mVehicleIndex);
				}
				else // we've hit a car we haven't hit before with shock, record it
				{
					// We dont care about hitting our own car
					if(pMsg->mVehicleIndex != mpLogicActor->GetVehicleIndex())
					{
						GetStickerBlackboard().mVehiclesHitByShockBitset |= 1 << pMsg->mVehicleIndex;
					}
				}

				// there's a sticker to "shock Leonard" (Talon), we know the event here and if we've shocked someone, we know it was him (1v1 race)
				if( mpLevelState->mEvent == C_AchievementData::Instance().GetMiscStickerData().mEventForShockLeonard.Id() )
				{
					C_Sticker* pShockLeonardSticker = GetSticker(BLUR_STICKER_SHOCK_LEONARD);
					if( pShockLeonardSticker )
					{
						pShockLeonardSticker->AddProgress();
					}
				}

				break;
			}
			case E_PICKUP_TYPE_BOMB:
			{
				pSticker = GetSticker(BLUR_STICKER_MINE_CARS);
				break;
			}
			case E_PICKUP_TYPE_SHUNT:
			{
				pSticker = GetSticker(BLUR_STICKER_SHUNT_CARS);
				break;
			}
			case E_PICKUP_TYPE_NITROUS:
			{
				if(pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
				{
					GetStickerBlackboard().mNumVehiclesHitByOneAirbrake++;
					pSticker = GetSticker(BLUR_STICKER_AIRBRAKE_RIFT_CARS);
					if(GetStickerBlackboard().mNumVehiclesHitByOneAirbrake < pSticker->Goal())
					{
						// if we're not quite there yet, nullify the sticker so we don't add to it (it's a one shot)
						pSticker = NULL;
					}
				}
				break;
			}
			case E_PICKUP_TYPE_BLINDFIRE:
			{
				GetStickerBlackboard().mNumBoltsHitThisRace++;
				// hit the same car as last time
				if(GetStickerBlackboard().mVehicleIndexYouHitWithBolt == pMsg->mVehicleIndex)
				{
					// with a bolt from the same group
					if(GetStickerBlackboard().mGroupIdOfLastBoltHit == pMsg->mGroupID)
					{
						GetStickerBlackboard().mNumTimesHitSameCarWithBolt++;

						C_Sticker* pBoltMoreSticker = GetSticker(BLUR_STICKER_FOUR_BOLT_HIT);
						if(pBoltMoreSticker && pBoltMoreSticker->Goal() <= GetStickerBlackboard().mNumTimesHitSameCarWithBolt)
						{
							pBoltMoreSticker->AddProgress(); // one shot
						}
					}
					else
					{
						GetStickerBlackboard().mGroupIdOfLastBoltHit = pMsg->mGroupID;
						GetStickerBlackboard().mNumDifferentCarsHitWithSingleBolt = 0;
						GetStickerBlackboard().mNumTimesHitSameCarWithBolt = 1; // must be from same group!
					}
				}
				// hit a different car than last time
				else
				{
					// with a bolt from the same group
					if(GetStickerBlackboard().mGroupIdOfLastBoltHit == pMsg->mGroupID)
					{
						GetStickerBlackboard().mNumDifferentCarsHitWithSingleBolt++;
						C_Sticker* pBoltDifferentSticker = GetSticker(BLUR_STICKER_THREE_BOLT_DIFFERENT);
						if(pBoltDifferentSticker && pBoltDifferentSticker->Goal() <= GetStickerBlackboard().mNumDifferentCarsHitWithSingleBolt)
						{
							pBoltDifferentSticker->AddProgress(); //one shot
						}
					}
					else // with a bolt from a different group
					{
						GetStickerBlackboard().mGroupIdOfLastBoltHit = pMsg->mGroupID;
						GetStickerBlackboard().mNumDifferentCarsHitWithSingleBolt = 1;
					}

					GetStickerBlackboard().mNumTimesHitSameCarWithBolt = 1;
					GetStickerBlackboard().mVehicleIndexYouHitWithBolt = pMsg->mVehicleIndex;
				}

				// additionally, there is a sticker for bolting someone while airbraking
				if( GetStickerBlackboard().mIsUsingNitro && pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED )
				{
					GetStickerBlackboard().mNumBoltsHitDuringThisAirbrake++;
					if( GetStickerBlackboard().mNumBoltsHitDuringThisAirbrake >= C_AchievementData::Instance().GetMiscStickerData().mNumBoltsNeededToHitWhileAirbraking )
					{
						C_Sticker* pBoltWhenAirbrakeSticker = GetSticker(BLUR_STICKER_AIRBRAKE_AND_BOLT);
						if( pBoltWhenAirbrakeSticker )
						{
							pBoltWhenAirbrakeSticker->AddProgress();
						}
					}
				}
				
				break;
			}
		}

		// if we have populated pSticker by this point, we know we want to increment the sticker in it
		// [JBUNNER] : BUG 38362 - Dont award aggressive sticker progress (e.g. mine / shock X cars) for hitting yourself!
		if(pMsg->mVehicleIndex != mpLogicActor->GetVehicleIndex() && pSticker)
		{
			pSticker->AddProgress();
		}

		// let's check some other data and see if we have any more stickers to add progress to

		pSticker = GetSticker(BLUR_STICKER_BARGE_THREE_AT_ONCE);
		if( pSticker && GetStickerBlackboard().mNumVehiclesHitByOneBarge >= pSticker->Goal() )
		{
			pSticker->AddProgress();
		}

		// we also wish to record having aggresively hit with certain powerups for "hit with all in one race"
		if((int)pMsg->mPickupType <= E_PICKUP_TYPE_BOMB && pMsg->mPickupType != E_PICKUP_TYPE_NITROUS )
		{
			GetStickerBlackboard().mPowerupTypesHitThisRace |= 1 << pMsg->mPickupType;

			// have we hit with at least one of every (offensive) powerup in this race (note nitro is set on initialisation)?
			if( GetStickerBlackboard().mPowerupTypesHitThisRace == s_allOffensivePowerups )
			{
				C_Sticker* pUsedAllSticker = GetSticker(BLUR_STICKER_USE_EACH_POWERUP);
				if( pUsedAllSticker )
				{
					pUsedAllSticker->AddProgress();
				}
			}
		}
	}

	void C_StickerManager::CheckPickupHitVehicles(const C_VehicleLogicMsg_HitByPerkMsg* pMsg)
	{
		int tierIdx		= C_AchievementData::Instance().GetMiscStickerData().mSuccessfulPowerUpHitOnEveryCarTier - 1;
		int eventIdx	= C_AchievementData::Instance().GetMiscStickerData().mSuccessfulPowerUpHitOnEveryCarEvent - 1;

		if(mCurrentTierIdx == tierIdx && mCurrentEventIdx == eventIdx)
		{
			if(pMsg->mVehicleIndex >= 0)
			{
				GetStickerBlackboard().mVehiclesHitThisRace.SetBit(pMsg->mVehicleIndex);

				C_Sticker* pHitCarsSticker = GetSticker(BLUR_STICKER_HIT_EVERY_CAR);
				if(pHitCarsSticker && GetStickerBlackboard().mVehiclesHitThisRace.CountBitsSet() >= pHitCarsSticker->Goal() )
				{
					pHitCarsSticker->AddProgress();
				}
			}	
		}
	}

	void C_StickerManager::CheckKhansRide()
	{
		// if we've smashed up Khan's ride to low health or less
		if( mpLevelState->mEvent == C_AchievementData::Instance().GetMiscStickerData().mEventForSmashKhansRide.Id() )
		{
			// find Khan's ride!
			if( mpRaceLogicLayer )
			{
				Game::Actors::C_VehicleLogicActor* pKhanLogicActor = mpRaceLogicLayer->GetLogicVehicleActor( 1 ); //cough

				if( pKhanLogicActor && pKhanLogicActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_LOW )
				{
					C_Sticker* pSmashKhanSticker = GetSticker(BLUR_STICKER_SMASH_KHAN);
					if( pSmashKhanSticker )
					{
						pSmashKhanSticker->AddProgress();
					}
				}
			}
		}
	}

	void C_StickerManager::CheckAvoidNatalyasPowerups()
	{	
		if( mpLevelState->mEvent == C_AchievementData::Instance().GetMiscStickerData().mEventForNatalyaAvoidance.Id() )
		{
			// if we're running natalya's boss 1v1 and we haven't been hit at all this race, we know we avoided all her powerups
			C_Sticker* pAvoidNatalyaSticker = GetSticker(BLUR_STICKER_AVOID_NATALYAS_POWERUPS);
			if( pAvoidNatalyaSticker && GetStickerBlackboard().mHasBeenHitThisRace == false )
			{
				pAvoidNatalyaSticker->AddProgress();
			}
		}
	}

	void C_StickerManager::CheckDoMoreDamageThanCarver()
	{
		if( mpLevelState->mEvent == C_AchievementData::Instance().GetMiscStickerData().mEventForCarverDamage.Id() )
		{
			// if we're doing carver's 1v1, he's the only one who can damage us so if we have more damage done than taken, we fulfil the criteria
			C_Sticker* pMoreDamageSticker = GetSticker(BLUR_STICKER_DAMAGE_CARVER_MORE);
			if( pMoreDamageSticker && GetStickerBlackboard().mDamageTakenThisRace < GetStickerBlackboard().mDamageDoneThisRace )
			{
				pMoreDamageSticker->AddProgress();
			}
		}
	}

	// The primary pad vehicle here was the one hit by the pickup
	void C_StickerManager::CheckPickupVictimHits(const C_VehicleLogicMsg_HitByPerkMsg* pMsg)
	{
		switch(pMsg->mPickupType)
		{
			case E_PICKUP_TYPE_NITROUS:
			{
				// we've fired a new nitro
				if( pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_NORMAL )
				{
					GetStickerBlackboard().mIsUsingNitro = true;
					GetStickerBlackboard().mHasUsedNitroThisRace = true;
				}
				break;
			}
		}

		// if they were hit at all, this disqualifies them from the "lap without getting hit" sticker
		if( pMsg->mPickupType != E_PICKUP_TYPE_NITROUS && pMsg->mPickupType != E_PICKUP_TYPE_REPAIR )
		{
			CheckCloseRepair();
			GetStickerBlackboard().mHasBeenHitThisLap = true;
			GetStickerBlackboard().mHasBeenHitThisRace = true;
		}
	}

	// we've been hit by something, check if we just repaired
	void C_StickerManager::CheckCloseRepair()
	{
		C_Sticker* pCloseRepairSticker = GetSticker(BLUR_STICKER_REPAIR_JUST_IN_TIME);
		if( pCloseRepairSticker 
			&& GetStickerBlackboard().mTimeSinceLastRepair <= C_AchievementData::Instance().GetMiscStickerData().mMaxTimeForPowerupHitAfterCriticalRepair
			&& GetStickerBlackboard().mTimeSinceLastRepair != 0.0f )
		{
			pCloseRepairSticker->AddProgress();
		}
	}

	// Check all stickers and data which require us merely to fire a pickup (note primary pad is guaranteed to be the firer in this case)
	void C_StickerManager::CheckPickupFired(const C_VehicleLogicMsg_FiredPerkMsg* pMsg)
	{
		switch(pMsg->mPickupType)
		{
			case E_PICKUP_TYPE_BARGE:
			{
				GetStickerBlackboard().mNumVehiclesHitByOneBarge = 0; //we've fired a new barge so start counting again
				break;
			}
			case E_PICKUP_TYPE_NITROUS:
			{
				// we've fired a new nitro
				if( pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED )
				{
					GetStickerBlackboard().mNumVehiclesHitByOneAirbrake = 0;
					GetStickerBlackboard().mIsAirbraking = true;
					if( mpLogicActor && mpLogicActor->GetPositionInfo().mRacePosition == 1 )
					{
						GetStickerBlackboard().mNumTimesAirbrakedInFirstPosition++;
					}
				}
				break;
			}
			case E_PICKUP_TYPE_BOMB:
			{
				GetStickerBlackboard().mOurRecentlyFiredMineId = pMsg->mUniqueId;
				StartTimer( &(GetStickerBlackboard().mTimeSinceWeDroppedOurLastMine) );
				break;
			}
			case E_PICKUP_TYPE_BLINDFIRE:
			{
				GetStickerBlackboard().mNumBoltsFiredThisRace++;
				break;
			}
		}
	}

	// we know here that the primary pad was the one who did the destroying (i.e. mPickupKillerOwnerID)
	void C_StickerManager::CheckDestroyedPickupWithPickup(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg)
	{
		switch(pMsg->mPickupKillerType)
		{
			case E_PICKUP_TYPE_BARGE:
			{
				// Dont count last gasp
				if(pMsg->mPickupKillerSubType != E_PICKUP_SUB_TYPE_MOD && !(pMsg->mPickupVictimType == E_PICKUP_TYPE_BOMB && pMsg->mPickupVictimSubType == E_PICKUP_SUB_TYPE_NORMAL))
				{
					C_Sticker* pBargeIncomingSticker = GetSticker(BLUR_STICKER_BARGE_INCOMING_ATTACK);
					if( pBargeIncomingSticker )
					{
						pBargeIncomingSticker->AddProgress();
					}
				}
				break;
			}
			case E_PICKUP_TYPE_BLINDFIRE:
			{
				C_Sticker* pBoltIncomingSticker = GetSticker(BLUR_STICKER_BOLT_INCOMING_POWERUPS);
				if( pBoltIncomingSticker )
				{
					pBoltIncomingSticker->AddProgress();
				}
				break;
			}
			//note these are projectiles only, so this covers the case of lobbed mines which have not yet come to rest
			case E_PICKUP_TYPE_BOMB:
			{
				if( pMsg->mPickupVictimType != E_PICKUP_TYPE_BOMB ) //not really an "incoming" attack if you're driving towards a stationary bomb...
				{
					C_Sticker* pMineIncomingSticker = GetSticker(BLUR_STICKER_MINE_INCOMING_ATTACKS);
					if( pMineIncomingSticker )
					{
						pMineIncomingSticker->AddProgress();
					}
				}
				break;
			}
		}
	}

	// we know here that the primary pad was the one who had his/her pickup destroyed (i.e. mPickupVictimOwnerID)
	void C_StickerManager::CheckHadPickupDestroyedByPickup(const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg)
	{
		switch(pMsg->mPickupVictimType)
		{
			case E_PICKUP_TYPE_BOMB:
			{
				// dropped mines are always the "victims" in any collision
				if( GetStickerBlackboard().mOurRecentlyFiredMineId == pMsg->mPickupVictimActorID &&
					GetStickerBlackboard().mTimeSinceWeDroppedOurLastMine <= C_AchievementData::Instance().GetMiscStickerData().mMaxTimeForBombToStopIncomingAttack )
				{
					// if our recently dropped mine has been hit by a projectile, we know that attack was probably "incoming" to us. skills!
					C_Sticker* pMineIncomingSticker = GetSticker(BLUR_STICKER_MINE_INCOMING_ATTACKS);
					if(pMineIncomingSticker)
					{
						pMineIncomingSticker->AddProgress();
					}
				}
				break;
			}
		}
	}

	// Called from RaceModeDestruction.cpp
	void C_StickerManager::CheckDestructionLongshot()
	{
		if(C_Sticker* longshotSticker = GetSticker(BLUR_STICKER_LONGSHOTS_IN_DESTRUCTION))
		{
			longshotSticker->AddProgress();
		}
	}

	// we know here that the primary pad was the one who did the scoring (i.e. mVehicleIndex)
	void C_StickerManager::CheckSkillMoves(E_BLUR_SCORE_MOVE_BASE base, Btl::Vector<E_BLUR_SCORE_MOVE_SKILL> skills)
	{
		for(Btl::Vector<E_BLUR_SCORE_MOVE_SKILL>::ConstIterator it = skills.Begin(); it != skills.End(); it++)
		{
			switch(*it)
			{
				case E_BLUR_SCORE_MOVE_SKILL_ALT_FIRE:
				{
					if( base == E_BLUR_SCORE_MOVE_BASE_SHUNT )
					{
						C_Sticker* pReverseShunt = GetSticker(BLUR_STICKER_REVERSE_SHUNT_CARS);

						if( pReverseShunt )
						{
							pReverseShunt->AddProgress();
						}
					}
					break;
				}
			}
		}
	}

	// we know here that the primary pad was the one who did the scoring (i.e. mVehicleIndex)
	void C_StickerManager::CheckBaseMoves(E_BLUR_SCORE_MOVE_BASE base, Btl::Vector<E_BLUR_SCORE_MOVE_SKILL> skills)
	{
		switch(base)
		{
			case E_BLUR_SCORE_MOVE_BASE_OVERTAKE:
			{
				for(int i=0; i<skills.Size(); i++)
				{
					if( skills[i] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1 ||   // skill level 1-5 are now different levels of nitro.. go figure.. ? (:S) ?
						skills[i] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2 ||
						skills[i] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3 ||
						skills[i] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_4 ||
						skills[i] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_5 )
					{
						C_Sticker* pNitroOvertakeSticker = GetSticker(BLUR_STICKER_NITRO_OVERTAKE_CARS);
						if( pNitroOvertakeSticker )
						{
							pNitroOvertakeSticker->AddProgress();
						}
					}
				}
				break;
			}
		}
	}

	// we know here that the primary pad was the one who did the scoring (i.e. mVehicleIndex)
	void C_StickerManager::CheckBonusMoves(Btl::Vector<E_BLUR_SCORE_MOVE_BONUS> bonus)
	{
		for(Btl::Vector<E_BLUR_SCORE_MOVE_BONUS>::ConstIterator it = bonus.Begin(); it != bonus.End(); it++)
		{
			//switch(*it)
			//{
			//}
		}
	}

	// we know here that the primary pad was the one who did the scoring (i.e. mVehicleIndex)
	void C_StickerManager::CheckAggressiveScoringMoves(const C_VehicleLogicMsg_AggressiveScore* pMsg)
	{
		switch(pMsg->mBonusScoringEvent)
			{
				case E_BLUR_SCORE_MOVE_BONUS_CLIFF: // handle both of these the same
				case E_BLUR_SCORE_MOVE_BONUS_DROWN:
				{
					C_Sticker* cliffSticker = GetSticker(BLUR_STICKER_BARGE_OFF_WATER_OR_CLIFF);
					if(GetStickerBlackboard().mBargedSomeGuyOffCliff < 
						C_AchievementData::Instance().GetMiscStickerData().mMaxTimeBetweenBargingGuyAndOffCliffScoringMessage)
					{
						// if we barged them off
						cliffSticker->AddProgress();
						StopTimer(&(GetStickerBlackboard().mBargedSomeGuyOffCliff));
					}
				}
				break;

			default:
				break;
			}
	}

	// we already know we've won
	void C_StickerManager::CheckAirbrakeInFirstThenWin()
	{
		C_Sticker* pAirbrakeFirstSticker = GetSticker(BLUR_STICKER_WIN_AFTER_AIRBRAKES);
		if( pAirbrakeFirstSticker && pAirbrakeFirstSticker->Goal() <= GetStickerBlackboard().mNumTimesAirbrakedInFirstPosition )
		{
			pAirbrakeFirstSticker->AddProgress();
		}
	}

	// we know here that the primary profile has won the race
	void C_StickerManager::CheckWinWithRepair()
	{
		// do we have half a pip of health
		if( mpLogicActor && mpLogicActor->GetCurrentHealth() <= mpLogicActor->GetRLC()->GetRLSystem()->mDamageSystem->mParameters->mHealthIncrement / 2 )
		{
			// do we have a repair in our inventory?
			for( int i=0; i < mpLogicActor->GetPickupInfo().mPickupStore.Size(); i++ )
			{
				if( mpLogicActor->GetPickupInfo().mPickupStore[i].mType == E_PICKUP_TYPE_REPAIR )
				{
					C_Sticker* pWithRepairSticker = GetSticker(BLUR_STICKER_WIN_WITH_REPAIR_AND_HALF_PIP);
					if( pWithRepairSticker )
					{
						pWithRepairSticker->AddProgress();
						return;
					}
				}
			}
		}
	}

	// we know that the primary profile has won the race
	void C_StickerManager::CheckWinWithoutWrecked()
	{
		C_Sticker* pNoWreckSticker = GetSticker(BLUR_STICKER_REPAIR_MORE_THAN_MAX_HEALTH);
		if( mpLogicActor && pNoWreckSticker && !(GetStickerBlackboard().mHasBeenWreckedThisRace)
			&& GetStickerBlackboard().mAmountRepairedThisRace > mpLogicActor->GetMaxHealth() )
		{
			pNoWreckSticker->AddProgress();
		}
	}

	void C_StickerManager::CheckResults()
	{
		SP_EventState medal = (SP_EventState)(Game::C_Game::GetInstance()->GetLevelResult().mRaceStatistics.mMedal);

		if( medal == kSP_Gold )
		{
			CheckGoldMedalStickers();
			CheckAvoidNatalyasPowerups();
			CheckDoMoreDamageThanCarver();
		}
	}

	// we know here that the primary pad has got gold in their single player race!
	void C_StickerManager::CheckGoldMedalStickers()
	{
		Game::C_RaceBookLayer* pRaceBookLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if ( !pRaceBookLayer )
		{
			LOGERROR(g_Log, "C_StickerManager::CheckGoldMedalStickers > Problem getting RaceBookLayer, returning early...");
			return;
		}
		
		// did we hit every bolt in that destruction game?
		if( pRaceBookLayer->GetRaceMode() == E_RaceMode_Destruction
			&& GetStickerBlackboard().mNumBoltsFiredThisRace <= GetStickerBlackboard().mNumBoltsHitThisRace )
		{
			C_Sticker* pHitEveryBoltSticker = GetSticker(BLUR_STICKER_DESTRUCTION_EVERY_BOLT_GOLD);
			if( pHitEveryBoltSticker )
			{
				pHitEveryBoltSticker->AddProgress();
			}
		}

		if( pRaceBookLayer->GetRaceMode() == E_RaceMode_Checkpoint )
		{
			// did we complete that checkpoint mode without using a stopwatch?
			if( !( GetStickerBlackboard().mHasUsedAStopwatchThisRace) )
			{
				C_Sticker* pStopwatchSticker = GetSticker(BLUR_STICKER_GOLD_WITHOUT_STOPWATCH);
				if( pStopwatchSticker )
				{
					pStopwatchSticker->AddProgress();
				}
			}

			// did we complete that checkpoint event without using a single nitro?
			if( !( GetStickerBlackboard().mHasUsedNitroThisRace) )
			{
				C_Sticker* pNitroSticker = GetSticker(BLUR_STICKER_GOLD_WITHOUT_NITRO);
				if( pNitroSticker )
				{
					pNitroSticker->AddProgress();
				}
			}

			// did we complete that checkpoint on that level without taking damage?
			if( mpLevelState->mRoute == C_AchievementData::Instance().GetMiscStickerData().mRouteForCheckpointGoldWithoutDamage.Id()
				&& GetStickerBlackboard().mDamageTakenThisRace == 0 )
			{
				C_Sticker* pGoldWithoutDamageSticker = GetSticker(BLUR_STICKER_HARUNA_GOLD_NO_DAMAGE);
				if( pGoldWithoutDamageSticker )
				{
					pGoldWithoutDamageSticker->AddProgress();
				}
			}
		}
	}

	void C_StickerManager::CheckShieldMultipleLap()
	{
		for( int i = 0; i < mpLogicActor->GetPickupInfo().mPickupStore.Size(); i++ )
		{
			PickupStore& pickupStore	= mpLogicActor->GetPickupInfo().mPickupStore[i];

			if(pickupStore.mType == E_PICKUP_TYPE_SHIELD)
			{
				float trackLength			= AIAmax::AmaxAIR::Instance().TrackLength();
				float totalDist				= mpLevelState->mLaps * trackLength;
				float totalDistInPercent	= totalDist / trackLength;

				// Distance always seems to start at 1.0f
				if( pickupStore.mDistancePickedUp <= totalDistInPercent )
				{
					C_Sticker* pShieldMultipleLap = GetSticker(BLUR_STICKER_HOLD_SHIELD_FOR_LAP);
					if( pShieldMultipleLap )
					{
						pShieldMultipleLap->AddProgress();
					}	
					break;
				}
			}
		}
	}

	// the primary pad vehicle has completed a lap
	void C_StickerManager::CheckCleanLap()
	{
		C_Sticker* pCleanLapSticker = GetSticker(BLUR_STICKER_LAP_WITHOUT_GETTING_HIT);
		Game::C_RaceBookLayer* pRaceBookLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
		if ( !pRaceBookLayer )
		{
			LOGERROR(g_Log, "C_StickerManager::CheckCleanLap > Problem getting RaceBookLayer, returning early...");
			return;
		}
		if( pCleanLapSticker && GetStickerBlackboard().mHasBeenHitThisLap == false && pRaceBookLayer->GetRaceMode() == E_RaceMode_StreetRace )
		{
			pCleanLapSticker->AddProgress();
		}
	}

	// this is called every frame while racing :(
	void C_StickerManager::CheckHighSpeed()
	{
		if( mpLevelState->mRaceMode == E_RaceMode_Nothing || mpLevelState->mNumCars == 0 )
		{	
			GetStickerBlackboard().mInRace = false;
			return;
		}

		LevelStateCar& levelCar = mpLevelState->mCars[GetPrimaryProfileVehicleIndex()];
		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)levelCar.mCarID];

		if( mpLogicActor && pVehicle)
		{
			U32 ourSpeed = (U32)Maths::Abs<F32>( mpLogicActor->GetPositionInfo().mSpeedInMph );
			switch( pVehicle->carClass )
			{
				case AmaxCarClass_A:
				{
					if( GetStickerBlackboard().mCurrentCityId == C_AchievementData::Instance().GetMiscStickerData().mClassAHighSpeedCity.Id())
					{
						C_Sticker* pHighSpeedSticker = GetSticker(BLUR_STICKER_HIGH_SPEED_CLASS_A);
						if( pHighSpeedSticker && ourSpeed >= pHighSpeedSticker->Goal() && GetStickerBlackboard().mHasUsedNitroThisRace == false )
						{
							pHighSpeedSticker->AddProgress();
						}
					}
					break;
				}

				case AmaxCarClass_B:
				{
					if( GetStickerBlackboard().mCurrentCityId == C_AchievementData::Instance().GetMiscStickerData().mClassBHighSpeedCity.Id())
					{
						C_Sticker* pHighSpeedSticker = GetSticker(BLUR_STICKER_HIGH_SPEED_CLASS_B);
						if( pHighSpeedSticker && ourSpeed >= pHighSpeedSticker->Goal() && GetStickerBlackboard().mHasUsedNitroThisRace == false )
						{
							pHighSpeedSticker->AddProgress();
						}
					}
					break;
				}

				case AmaxCarClass_C:
				{
					if( GetStickerBlackboard().mCurrentCityId == C_AchievementData::Instance().GetMiscStickerData().mClassCHighSpeedCity.Id())
					{
						C_Sticker* pHighSpeedSticker = GetSticker(BLUR_STICKER_HIGH_SPEED_CLASS_C);
						if( pHighSpeedSticker && ourSpeed >= pHighSpeedSticker->Goal() && GetStickerBlackboard().mHasUsedNitroThisRace == false )
						{
							pHighSpeedSticker->AddProgress();
						}
					}
					break;
				}
		
				case AmaxCarClass_D:
				{
					if( GetStickerBlackboard().mCurrentCityId == C_AchievementData::Instance().GetMiscStickerData().mClassDHighSpeedCity.Id())
					{
						C_Sticker* pHighSpeedSticker = GetSticker(BLUR_STICKER_HIGH_SPEED_CLASS_D);
						if( pHighSpeedSticker && ourSpeed >= pHighSpeedSticker->Goal() && GetStickerBlackboard().mHasUsedNitroThisRace == false )
						{
							pHighSpeedSticker->AddProgress();
						}
					}
					break;
				}
			}
		}
	}

	void C_StickerManager::CheckCheckpointNitrous()
	{
		bool isCheckpoint = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceMode() == E_RaceMode_Checkpoint ||
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceMode() == E_RaceMode_FriendDemand_Checkpoint;

		// Make sure we used nitrous over the finish line
		if(isCheckpoint && !GetStickerBlackboard().mCrossedCheckpointWithoutNitrous && mpLogicActor->GetScoring().mUsingNitroTimer > 0.f)
		{
			C_Sticker* pCheckpointSticker = GetSticker(BLUR_STICKER_NITRO_EVERY_CHECKPOINT);
			if( pCheckpointSticker )
			{
				pCheckpointSticker->AddProgress();
			}
		}
	}

	const bool C_StickerManager::CheckStickers(const Messaging::C_MessageBase *pMessage )
	{
		if(GetStickerBlackboard().mInRace != true)
			return true;

		switch( pMessage->Type() )
		{
			case E_VehicleLogicMsg_Wrecked:
			{
				const C_VehicleLogicMsg_WreckedMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_WreckedMsg>(pMessage);

				if(VehicleIsLocal(pMsg->mWreckerIdx))
				{
					CheckWreckYourFirst(pMsg->mWreckerIdx);
					CheckWreckFirstBoss(pMsg->mWreckerIdx);
					CheckWreckCarsInDestruction(pMsg->mWreckerIdx);
					CheckWreckByThreeBolts(pMsg->mWreckerIdx);
				}
				return true;
			}

			case E_VehicleLogicMsg_PowerUpDestroyedByPowerUp:
			{
				const C_VehicleLogicMsg_PowerUpDestroyedByPowerUp* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_PowerUpDestroyedByPowerUp>(pMessage);
				if( VehicleIsLocal(pMsg->mVehicleIndex) )
				{
					CheckEvadedOrDestroyedFirstPowerup(pMsg->mVehicleIndex);
					CheckDestroyMines(pMsg);
					CheckDestroyShunts(pMsg);
					GetStickerBlackboard().mShuntWasEvadedByDestroying = true;
				}
				return true;
			}

			case E_VehicleLogicMsg_EvadedPowerUp:
			{
				const C_VehicleLogicMsg_EvadedPowerUp* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_EvadedPowerUp>(pMessage);
				if( VehicleIsLocal(pMsg->mVehicleIndex) )
				{
					// [JBUNNER] : BUG 38362 - Dont award evasion sticker progress for evading your own shock!
					if( mpLogicActor &&
						mpLogicActor->GetVehicleIndex() != pMsg->mFirerIndex )
					{
						CheckEvadedOrDestroyedFirstPowerup(pMsg->mVehicleIndex);
					}
					CheckAvoidShuntsByDriving(pMsg);
				}

				// if we did evade the incoming shunt by blasting it into oblivion, we've just handled the evade message pertaining to it, so reset this
				if( pMsg->mPickupType == E_PICKUP_TYPE_SHUNT )
				{
					GetStickerBlackboard().mShuntWasEvadedByDestroying = false;
				}
				return true;
			}

			case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);
				if( VehicleIsLocal(pMsg->mFiringVehicleIndex) && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mFiringVehicleIndex) )
				{
					CheckPickupHitVehicles(pMsg);
					CheckPickupAggressiveHits(pMsg);
					CheckKhansRide();
				}
				if( VehicleIsLocal(pMsg->mVehicleIndex) && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex) )
				{
					CheckPickupVictimHits(pMsg);
				}

				return true;
			}

			case E_VehicleLogicMsg_FiredPerk:
			{		
				const C_VehicleLogicMsg_FiredPerkMsg* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_FiredPerkMsg >(pMessage);
				if( VehicleIsLocal(pMsg->mVehicleIndex) && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex) )
				{
					if( pMsg->mPickupType == E_PICKUP_TYPE_SHIELD )
					{
						StartTimer( &(GetStickerBlackboard().mShieldElapsedTime) );
					}
					CheckPickupFired(pMsg); 
				}
				return true;
			}

			case E_VehicleLogicMsg_Scored:
			{
				const C_VehicleLogicMsg_Scored* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_Scored >(pMessage);
				//Check that this scoring event relates to us
				if(SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex))
				{
					CheckBaseMoves(pMsg->mBaseMove, pMsg->mSkillMoves);
					CheckSkillMoves(pMsg->mBaseMove, pMsg->mSkillMoves);
					CheckBonusMoves(pMsg->mBonusMoves);
				}
				break;
			}

			case E_VehicleLogicMsg_AggressiveScore:
			{
				const C_VehicleLogicMsg_AggressiveScore* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_AggressiveScore >(pMessage);
				if(SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex))
				{
					CheckAggressiveScoringMoves(pMsg);
				}
				if(pMsg->mBonusScoringEvent == E_BLUR_SCORE_MOVE_BONUS_WRECKED)
				{
					//	We actually want to check this even if neither vehicle is local, so that we know who has gained the achievement mid-race
					//	and then potentially get it from them before it ends.
					CheckWreckedBizarre(pMsg->mVehicleIndex, pMsg->mAboutVehicleIndex);
					// if it was us who got wrecked
					if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mAboutVehicleIndex) )
					{
						GetStickerBlackboard().mHasBeenWreckedThisRace = true;
					}
				}
				break;
			}

			case E_VehicleLogicMsg_DestroyedPickupWithPickup:
			{
				const C_VehicleLogicMsg_DestroyedPickupWithPickup* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_DestroyedPickupWithPickup >(pMessage);
				if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mPickupKillerOwnerID))
				{
					CheckDestroyedPickupWithPickup(pMsg);
				}
				else if(SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mPickupVictimOwnerID))
				{
					CheckHadPickupDestroyedByPickup(pMsg);
				}
				break; 
			}
			case Game::E_PickupMessage_CollectStopwatch:
			{
				CheckPickupStopwatch();
				GetStickerBlackboard().mHasUsedAStopwatchThisRace = true;
				break;
			}
			case E_VehicleLogicMsg_PerkEnded:
			{
				const C_VehicleLogicMsg_PerkEnded* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_PerkEnded >(pMessage);
				if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex) )
				{
					switch( pMsg->mPickupType )
					{
						case E_PICKUP_TYPE_NITROUS:
						{
							if( pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED )
							{
								GetStickerBlackboard().mIsAirbraking = false;
								GetStickerBlackboard().mNumBoltsHitDuringThisAirbrake = 0;
							}
							else if( pMsg->mPickupSubType == E_PICKUP_SUB_TYPE_NORMAL )
							{
								GetStickerBlackboard().mIsUsingNitro = false;
							}
							break;
						}
						case E_PICKUP_TYPE_SHIELD:
						{
							GetStickerBlackboard().mAttacksShieldedThisShield = 0;
							StopTimer( &(GetStickerBlackboard().mShieldElapsedTime) );
							break;
						}
					}
				}
				break;
			}
			case E_VehicleLogicMsg_ShieldedPerk:
			{
				const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_ShieldedPerkMsg>(pMessage);
				S32 primaryPadNo = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
				if( primaryPadNo == GetControllerIndex(pMsg->mDefendingVehicleIndex) &&
					primaryPadNo != GetControllerIndex(pMsg->mFiringVehicleIndex) )	// Dont give progress for shielding own pickups
				{
					CheckShieldedAttack(pMsg);
				}
				break;
			}
			case E_VehicleLogicMsg_Repair:
			{
				const C_VehicleLogicRepairMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicRepairMsg>(pMessage);
				if(SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex))
				{
					if(GetStickerBlackboard().mInRace)
					{
						CheckRepairHealth(pMsg);
					}

					if( (mpLogicActor && mpLogicActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_LOW ) )
					{
						StartTimer(&(GetStickerBlackboard().mTimeSinceLastRepair) );
					}
				}
				break;
			}
			case Game::E_GameLogicMessage_LapIncremented:
			{
				const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);
				if( pMsg->mLap != 1 && SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex) )
				{
					CheckCleanLap();
				}
				break;
			}
			case Game::E_GameLogicMessage_GameplayImpact:
			{
				const Game::C_GameplayImpactMsg* pMsg = Messaging::MessageCast<Game::C_GameplayImpactMsg>(pMessage);
				if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIdx) )
				{
					// if we were damaged, update our total damage taken for the race
					GetStickerBlackboard().mDamageTakenThisRace += pMsg->mDamage;
				}
				else if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mAggressiveVehicleIdx) )
				{
					// if it was us who did the damaging, update total damage done
					GetStickerBlackboard().mDamageDoneThisRace += pMsg->mDamage;
				}
				break;
			}
			case E_VehicleLogicMsg_HitWithAllDomes:
			{
				const C_VehicleLogicMsg_HitWithAllDomes* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_HitWithAllDomes>(pMessage);
				if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex) )
				{
					C_Sticker* pShieldSticker = GetSticker(BLUR_STICKER_SHOCK_EVERY_DOME);
					if( pShieldSticker )
					{
						pShieldSticker->AddProgress();
					}
				}
				break;
			}
			case E_VehicleLogicMsg_AvoidedAllDomes:
				{
					const C_VehicleLogicMsg_AvoidedAllDomes* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_AvoidedAllDomes>(pMessage);
					if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mFirerIdx) && pMsg->mFirerIdx == pMsg->mTargetIdx)
					{
						int tierIdx		= C_AchievementData::Instance().GetMiscStickerData().mAvoidOwnShockTier - 1;
						int eventIdx	= C_AchievementData::Instance().GetMiscStickerData().mAvoidOwnShockEvent - 1;

						if(tierIdx == mCurrentTierIdx && eventIdx == mCurrentEventIdx)
						{
							C_Sticker* pShieldSticker = GetSticker(BLUR_STICKER_SHOCK_YOURSELF_AND_AVOID);
							if( pShieldSticker )
							{
								pShieldSticker->AddProgress();
							}
						}
					}
					break;
				}
			case E_VehicleLogicMsg_CrossedCheckpoint:
			{
				const C_VehicleLogicMsg_CrossedCheckpoint* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_CrossedCheckpoint>(pMessage);
				if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex) )
				{
					if( mpLogicActor->GetScoring().mUsingNitroTimer <= 0.f )
					{
						GetStickerBlackboard().mCrossedCheckpointWithoutNitrous = true;
					}
				}
				break;
			}
			case E_GameLogicMessage_Passed_All_Rule:
			{
				const C_PassedAllRuleMsg* pMsg = Messaging::MessageCast<C_PassedAllRuleMsg>(pMessage);
				if( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == GetControllerIndex(pMsg->mVehicleIndex) )
				{
					CheckCheckpointNitrous();
				}
				break;
			}
		}
		return false;
	}

	bool C_StickerManager::MessageHandler(const Messaging::C_MessageBase* pMessage)
	{
		switch( pMessage->Type() )
		{
			case Game::E_GameflowMessage_RaceStarted:
			{
				GetStickerBlackboard().mInRace = true;
				return true;
			}
			case E_GameflowMessage_QuitRace:		//	Fall through
			case E_GameflowMessage_QuitAndWinRace:	//	Fall through
			case E_GameflowMessage_RestartRace:		//	Fall through
				{
					ClearCachedPtrs();
				}
			case E_GameflowMessage_RaceFinished:	// Fall through
			case Game::E_GameflowMessage_VehicleFinished:
			{
				GetStickerBlackboard().mInRace = false;
				return true;
			}
			case Game::E_GameflowMessage_ResultsCalculated:
			{
				// These might not be set if we Q&W before we cross the start line
				if(mpRaceLogicLayer && mpLogicActor)
				{
					CheckResults();
				
					CheckTopThreeInPinkCar();
					CheckVisitedCities();

					// we can check anything that involves us winning a race here
					if( Game::C_Game::GetInstance()->GetLevelResult().mCars[GetPrimaryProfileVehicleIndex()].mPosition == 1 )
					{
						CheckAirbrakeInFirstThenWin();
						CheckWinWithRepair();
						CheckWinWithoutWrecked();
						CheckShieldMultipleLap();
					}
				}

				bool complete=true;
				for(int sticker=0; sticker < BLUR_STICKER_MAX; sticker++)
				{
					if(!IsStickerComplete((BLUR_STICKER)sticker))
					{
						complete=false;
						break;
					}
				}

				if(complete)
				{
					Game::C_AvatarAwardSystem::Instance().GiveAvatarAward(SysProfile::C_Manager::Instance().GetPrimaryPadNo(), Game::BLUR_AVATAR_AWARD_ALL_STICKERS_TEE);
				}

				return true;
			}
			default:
				return CheckStickers(pMessage);;
		}
		return false;
	}

	void C_StickerManager::CheckBlurParty()
	{
		//	We already know there were 4 human players.
		const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		const Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
		U8 winnerIndex = 0;
		for(int i = 0; i < results.mNumCars; i++)
		{
			if(results.mCars[i].mPosition == 1)
			{
				winnerIndex = i;
				break;
			}
		}
		if(winnerIndex == GetPrimaryProfileVehicleIndex())	//	If the primary player won then they get the sticker (and in turn, the achievement)
		{
			C_Sticker* pShieldSticker = GetSticker(BLUR_STICKER_WIN_SPLIT_SCREEN);
			if( pShieldSticker )
			{
				pShieldSticker->AddProgress();
			}
		}
		else if(levelState.mCars[winnerIndex].mController != INVALID_CONTROLLER)	//	Otherwise, just give the achievement itself to the winner(if it's a player)
		{
			C_AchievementSystem::Instance().GiveAchievement(levelState.mCars[winnerIndex].mController, BLUR_ACHIEVEMENT_BLUR_PARTY);
		}
	}

	void C_StickerManager::ClearCachedPtrs()
	{
		mpLogicActor = NULL;
		mTimerVariables.Clear();
		mpRaceLogicLayer = NULL;
		mCurrentTierIdx = -1;
		mCurrentEventIdx = -1;
	}
}

void CheckSinglePlayerStickers()
{
	Game::C_StickerManager::Instance().CheckSinglePlayerStickers();
}
