
#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/RaceModes/RaceModeDestruction.h>

#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/StickerManager.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic\RaceRules\RaceRulesManager.h>

#include <Game\Messaging\GameflowMessages.h>
#include "GameLogic/Messaging/GameLogicMessages.h"

#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>
#include <Network/Player/NetPlayer.h>
#include <Network/Player/NetPlayerManager.h>
#include <Network/Message/NetMessage.h>
#include <Network/Misc/NetDatagramPacketTypes.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <RaceBook/RaceBookManager.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <../library/kanzen/camera/Utility.h>

#include <ui/UiHelpers.h>
#include <ui/UiMessages.h>
#include <Ui/UiInterface.h>
#include <Ui/SoundEffects.h>

#include <GameLogic\ContentServer\KeyArchiveManager.h>

#include <GameLogic\profile\GameProfileManager.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>

#include <SinglePlayer\SinglePlayer.h>

#include <Mode/Destruction/iModeDestruction.h>

#include "Automation\GameDataLogging.h"

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD



//#pragma optimize("", off)					// TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP



Debug::Bool gDestructionUnlimitedTime("Amax.GameMode.Destruction.UnlimitedTime", false, Debug::Persist() );
Debug::Float gDestructModeDistanceEffectDamping("Amax.GameMode.Destruction.DistanceEffectDamping", 0.75f, Debug::Range(0.0f,1.0f));
Debug::Float gDestructModeDistanceEffectBuffer("Amax.GameMode.Destruction.DistanceEffectBuffer", 5.0f, Debug::Range(0.0f,20.0f));
Debug::Float gDestructModeDistanceEffectFullIntensity("Amax.GameMode.Destruction.DistanceEffectFullIntensity", 1.0f, Debug::Range(0.0f,1.0f));
Debug::Float gWreckedImpulseScale("Amax.GameMode.Destruction.WreckedImpulseScale", 1.5f, Debug::Range(0.0f,10.0f));
Debug::Float gDestructModeMinDriftDistance("Amax.GameMode.Destruction.MinDriftDistance", 2.0f, Debug::Range(1.0f,20.0f));
Debug::Float gDestructModeBonusHitDuration("Amax.GameMode.Destruction.BonusHitDuration", 2.0f, Debug::Range(1.0f,10.0f));
Debug::Float gDestructModeWreckedTimer("Amax.GameMode.Destruction.WreckedTimer", 4.5f, Debug::Range(0.0f,10.0f));


extern Debug::Float g_debugNetworkLeftRaceGhostTime;
struct distanceInfo
{
	F32 mDistance;
	int mIndex;
};

namespace Game
{

	Util::C_Random gDestructionModeRandom;
int C_RaceModeDestruction::sortByDistanceFunc(const void *a, const void *b)
{
	distanceInfo* distanceDataA = (distanceInfo*)a;
	distanceInfo* distanceDataB = (distanceInfo*)b;

	if(distanceDataA->mDistance > distanceDataB->mDistance )
	{
		return 1;
	}
	else if( distanceDataA->mDistance > distanceDataB->mDistance)
	{
		return -1;
	}

	return 0;
}

void C_RaceModeDestruction::Initialise(Game::C_LevelState *pLevelState)
{
	SetInternalNameAndID("Destruction",E_RaceMode_Destruction);

	SetLevelState(pLevelState);

	mTimeLeft=0.0f;
	mRaceInProgress=false;
	mPassed=false;
	mStarted=false;

	mEventState = kSP_Incomplete;
	mNumPoints = 0;
	mDiffToMedal = 0;

	mLastPoints = 0;
	mLastTime = 0.0f;

	mLastBonusTimer = 0.0f;
	mLastBonusPoints = 0;

	mTotalCarsWrecked = 0;
	mEasyCarsWrecked = 0;
	mMediumCarsWrecked = 0;
	mHardCarsWrecked = 0;
	mVHardCarsWrecked = 0;

	mNormalShots = 0;
	mLongShots = 0;
	mSkillShots = 0;
	mWreckedTimer = -1.0f;


	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( E_GameLogicMessage_Scoring_Event );
	AddSubscriber( E_GameLogicMessage_Failed_Rule );
	AddSubscriber( E_GameLogicMessage_Passed_All_Rule );

	AddSubscriber( E_GameflowMessage_IntroStarted );
	AddSubscriber( E_GameflowMessage_OutroStarted );
	AddSubscriber( E_GameflowMessage_RaceStarted );
	AddSubscriber( E_GameflowMessage_RaceFinished );
	AddSubscriber( E_GameLogicMessage_LapIncremented );

	AddSubscriber( E_VehicleMessage_FatallyDamaged );	
//	AddSubscriber( E_VehicleLogicMsg_Wrecked );					// TEST TEST

	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( airacingtech::E_DestructionModeMessage_WaveCompleted );

	mpDestructionParams = &RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction;
	BIZ_ASSERT_MSG(mpDestructionParams, "\nC_RaceModeDestruction::Initialise(Game::C_LevelState *pLevelState) < Could notcache RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction");
}

SP_DestructionEvent* C_RaceModeDestruction::GetEvent() const
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	SP_Event* pBaseEvent = SP_GetEventById( SP_GetCurrentSaveGame(), levelState.mSpEventId );
	
	BIZ_ASSERT(pBaseEvent);

	if (!pBaseEvent)		
		return NULL;
	
	if (pBaseEvent->kind != kSP_DestructionEvent)
		return NULL;
	
	return (SP_DestructionEvent*)pBaseEvent;
}


void C_RaceModeDestruction::SetupRace()
{
	SP_DestructionEvent* pEvent = GetEvent();

	if (!pEvent || !pEvent->event || !pEvent->event->raceMode)
		return;

	mRaceModeDestructionData = (AmaxRaceModeDestruction*)pEvent->event->raceMode;
	BIZ_ASSERT(mRaceModeDestructionData);

	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	levelState.mDamageMode = E_DAMAGE_MODE_POWERUPS_ONLY;
	levelState.mWreckedBehaviour = E_AMAXWRECKED_DO_NOTHING;
	mRaceInProgress=false;
	mPassed=false;
	mStarted=false;
	mTimeLeft=0.0f;

	mTotalCarsWrecked = 0;
	mEasyCarsWrecked = 0;
	mMediumCarsWrecked = 0;
	mHardCarsWrecked = 0;
	mVHardCarsWrecked = 0;

	mEventState = kSP_Incomplete;
	mNumPoints = 0;
	mWreckedTimer = -1.0f;
//	if (AmaxRaceModeDestructionVars* diffVars = GetDifficultyVars())		// Level state not setup at this point!
//		mDiffToMedal = diffVars->bronzePoints;

	mLastPoints = 0;

	mNormalShots = 0;
	mLongShots = 0;
	mSkillShots = 0;
	mBonusPoints = 0;
}


void C_RaceModeDestruction::StartLevel()
{
	if (AmaxRaceModeDestructionVars* diffVars = GetDifficultyVars())
		mDiffToMedal = diffVars->bronzePoints;

	C_RaceModeBase::StartLevel();
}


bool C_RaceModeDestruction::SetupOpponents()
{
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	AmaxEvent* ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];

	// Ensure that opponent character IDs are the same when restarting an event,
	// otherwise the game may run out of memory as it tries to load new driver models.
	gDestructionModeRandom.Seed(levelState.mSpEventId);

	DifficultySettings* difficultySettings = NULL;
	switch(levelState.m_difficulty)
	{
	case Game::GAME_DIFFICULTY_VERY_EASY:
	case Game::GAME_DIFFICULTY_EASY:
		difficultySettings = &ev->easyVersion;
		break;

	case Game::GAME_DIFFICULTY_NORMAL:
		difficultySettings = &ev->normalVersion;
		break;

	case Game::GAME_DIFFICULTY_HARD:
	case Game::GAME_DIFFICULTY_HARDCORE:
		difficultySettings = &ev->hardVersion;
		break;
	}
	
	int playersCreated = 0;
	int opponentsCreated = 0;
	
	

	for(int i=0;i<ev->opponents.count;i++)
	{
		RaceBook::C_RaceBookProfile* prof = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)ev->opponents[i].opponent->ID );
		BIZ_ASSERT_MSG(prof, "\nC_RaceModeDestruction::SetupOpponents() Could not find RaceBook character to add to race");

		if(prof->mType == E_CHARACTER_TYPE_PLAYER)
		{
			AmaxOpponentSetup opponentSetupXt = ev->opponents[i];

			LevelSetupOpponentData opponentSetup;

			opponentSetup.startPositionIndex	= playersCreated;
			opponentSetup.ev					= ev;
			opponentSetup.difficultySettings	= difficultySettings;

			opponentSetup.opponent				= opponentSetupXt.opponent.Object();
			opponentSetup.vehicleOverride		= opponentSetupXt.vehicleOverride.Object();
			opponentSetup.detailOverride		= opponentSetupXt.detailOverride;
			opponentSetup.alwaysShields			= opponentSetupXt.alwaysShields;			
			opponentSetup.onlyAttacksPlayer		= opponentSetupXt.onlyAttacksPlayer;		
			opponentSetup.healthOverride		= opponentSetupXt.healthOverride;		
			opponentSetup.perkSlotsOverride		= opponentSetupXt.perkSlotsOverride;	
			opponentSetup.perkGiftTimer			= opponentSetupXt.perkGiftTimer;			
			opponentSetup.perkGiftTimerInitial	= opponentSetupXt.perkGiftTimerInitial;	
			opponentSetup.perkGiftLaps			= opponentSetupXt.perkGiftLaps;			
			opponentSetup.perkGiftTypes			= opponentSetupXt.perkGiftTypes;			
			opponentSetup.startDisabled			= false;

			if(levelState.SetupOpponent( playersCreated, opponentSetup))
			{
				playersCreated++;
			}
			else
			{
				BIZ_ASSERT_FAIL("\nC_RaceModeDestruction::SetupOpponents() - Failed to create player %d", i);
			}
		}
	}

	int opponentsMax = MAX_VEHICLES-playersCreated;
	int idx = playersCreated;
	if (mRaceModeDestructionData)
	{
		for(int i=0;i<mRaceModeDestructionData->waves.count;i++)
		{
			DestructionMode_Wave* wave = &mRaceModeDestructionData->waves[i];
		
			
			BIZ_ASSERT_MSG( wave->waveAI.slot1 == DM_AI__None || opponentsCreated<opponentsMax , "\nC_RaceModeDestruction::SetupOpponents() Trying to create too many opponents. Ensure that the total number of vehicles specified in all the waves is %d or less", opponentsMax);
			if(SetupOpponent( wave->waveAI.slot1, idx, difficultySettings, mRaceModeDestructionData ))
			{
				opponentsCreated++;
				idx++;
			}

			BIZ_ASSERT_MSG( wave->waveAI.slot2 == DM_AI__None || opponentsCreated<opponentsMax , "\nC_RaceModeDestruction::SetupOpponents() Trying to create too many opponents. Ensure that the total number of vehicles specified in all the waves is %d or less", opponentsMax);
			if(SetupOpponent( wave->waveAI.slot2, idx, difficultySettings, mRaceModeDestructionData ))
			{
				opponentsCreated++;
				idx++;
			}

			BIZ_ASSERT_MSG( wave->waveAI.slot3 == DM_AI__None || opponentsCreated<opponentsMax , "\nC_RaceModeDestruction::SetupOpponents() Trying to create too many opponents. Ensure that the total number of vehicles specified in all the waves is %d or less", opponentsMax);
			if(SetupOpponent( wave->waveAI.slot3, idx, difficultySettings, mRaceModeDestructionData ))
			{
				opponentsCreated++;
				idx++;
			}

			BIZ_ASSERT_MSG( wave->waveAI.slot4 == DM_AI__None || opponentsCreated<opponentsMax , "\nC_RaceModeDestruction::SetupOpponents() Trying to create too many opponents. Ensure that the total number of vehicles specified in all the waves is %d or less", opponentsMax);
			if(SetupOpponent( wave->waveAI.slot4, idx, difficultySettings, mRaceModeDestructionData ))
			{
				opponentsCreated++;
				idx++;
			}
			
		}
	}

	BIZ_ASSERT_MSG(playersCreated>0, "\nC_RaceModeDestruction::SetupOpponents() Failed to create a player");

	

	levelState.mNumCars = playersCreated+opponentsCreated;

	InitialiseSpotlights();

	// We have handled the opponent setup so 
	// return true to inform the caller that
	// we do not require the default opponent
	// setup.
	return true;
}

bool C_RaceModeDestruction::SetupOpponent( E_DestructionMode_AI_Level level, int idx, DifficultySettings* difficultySettings, AmaxRaceModeDestruction* pRaceMode )
{
	if( level != DM_AI__None)
	{
		int numProfiles = RaceBook::C_RaceBookManager::Instance().NumProfiles();
		Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
		AmaxEvent* ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)levelState.mEvent];

		
		int attempts=0;
		int randomCharacterIdx = gDestructionModeRandom.Rand(numProfiles-1);
		RaceBook::C_RaceBookProfile* randomCharacter = NULL;

		//Starting at a random index loop around until
		//we find a NORMAL character type or we do a full loop
		while( !randomCharacter && attempts < numProfiles )
		{
			randomCharacter = RaceBook::C_RaceBookManager::Instance().GetProfileByIndex(randomCharacterIdx);
			if(randomCharacter->mType != E_CHARACTER_TYPE_NORMAL)
			{
				attempts++;
				randomCharacter = NULL;
				randomCharacterIdx=randomCharacterIdx==(numProfiles-1)?0:(randomCharacterIdx+1);
			}
		}
		BIZ_ASSERT_MSG(randomCharacter, "\nC_RaceModeDestruction::SetupOpponent() - Failed to find a character of type E_CHARACTER_TYPE_NORMAL");

		LevelSetupOpponentData opponentSetup;

		const int damageInflicted = RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BLINDFIRE)->mNormal->mBalancingData->GetDamageInflicted();

		switch(level)
		{
		case	DM_AI__Easy:
			opponentSetup.vehicleOverride = pRaceMode->vehicleEasy.Object();
			opponentSetup.healthOverride  = damageInflicted;							// Hard Coded :(
			break;
		case	DM_AI__Medium:
			opponentSetup.vehicleOverride = pRaceMode->vehicleMedium.Object();
			opponentSetup.healthOverride  = 2 * damageInflicted;
			break;
		case	DM_AI__Hard:
			opponentSetup.vehicleOverride = pRaceMode->vehicleHard.Object();
			opponentSetup.healthOverride  = 3 * damageInflicted;
			break;
		case	DM_AI__VHard:
			opponentSetup.vehicleOverride = pRaceMode->vehicleVHard.Object();
			opponentSetup.healthOverride  = 1 * damageInflicted;
			break;
		default:
			BIZ_ASSERT_FAIL("C_RaceModeDestruction::SetupOpponent() - Unhandled AI Level");
		};

		

		opponentSetup.startPositionIndex	= idx;
		opponentSetup.ev					= ev;
		opponentSetup.difficultySettings	= difficultySettings;


		opponentSetup.opponent				= C_RaceBookData::Instance().mCharacters[randomCharacter->repIdx];
		opponentSetup.alwaysShields = false;	
		opponentSetup.detailOverride = false;
		opponentSetup.onlyAttacksPlayer = true;
		opponentSetup.perkSlotsOverride = 1;
		opponentSetup.perkGiftTimer = 0.0f;
		opponentSetup.perkGiftTimerInitial = 0.0f;
		opponentSetup.perkGiftLaps = false;
		opponentSetup.perkGiftTypes;
		opponentSetup.startDisabled = true;
		if( !levelState.SetupOpponent( idx, opponentSetup))
		{
			BIZ_ASSERT_FAIL("\nC_RaceModeDestruction::SetupOpponents() - Failed to create opponent %d", idx);
			return false;
		}
		levelState.mCars[idx].mPickupsAllowed = false;
		levelState.mCars[idx].mPickupInfo.InitDestruction();
		return true;
	}
	return false;
}

void C_RaceModeDestruction::Update()
{
	if(!mRaceInProgress || !mStarted)
		return;

	SP_DestructionEvent* pEvent = GetEvent();

	if (!pEvent || !pEvent->event || !pEvent->event->raceMode)
		return;

	AmaxRaceModeDestruction* pRaceMode = (AmaxRaceModeDestruction*)pEvent->event->raceMode;
	
	if (AmaxRaceModeDestructionVars* diffVars = GetDifficultyVars())
		mPassed = mNumPoints >= diffVars->bronzePoints;
	
	float frameTime = C_GameApp::GetInstance()->GetTimeStep();

	if (mLastBonusTimer > 0.0f)
	{
		mLastBonusTimer -= frameTime;

		if ( mLastBonusTimer <= 0.0f )
			_UI_MESSAGE( UI::E_UiGameMsg_SpDestructionBonusHitEnd );
	}

	CheckRaceEnd();

	//store these away so the Friend Demands can use them
	Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);
	if (pLogicActor)
	{
		pLogicActor->GetScoring().mDestructionPoints = mNumPoints;
		pLogicActor->GetScoring().mDestructionWrecks = mTotalCarsWrecked;
		pLogicActor->GetScoring().mDestructionBlueWrecks = mVHardCarsWrecked;
		pLogicActor->GetScoring().mDestructionRedWrecks = mHardCarsWrecked;
		pLogicActor->GetScoring().mDestructionYellowWrecks = mMediumCarsWrecked;
		pLogicActor->GetScoring().mDestructionGreenWrecks = mEasyCarsWrecked;
	}

	if (AmaxRaceModeDestructionVars* diffVars = GetDifficultyVars())
	{
		if (mNumPoints >= diffVars->goldPoints)
		{
			mEventState = kSP_Gold;
			mDiffToMedal = 0;
		}
		else if (mNumPoints >= diffVars->silverPoints)
		{
			mEventState = kSP_Silver;
			mDiffToMedal = diffVars->goldPoints;
		}
		else if (mNumPoints >= diffVars->bronzePoints)
		{
			mEventState = kSP_Bronze;
			mDiffToMedal = diffVars->silverPoints;
		}
		else
		{
			mEventState = kSP_Incomplete;
			mDiffToMedal = diffVars->bronzePoints;
		}
	}

	
	Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	Game::Actors::C_VehicleLogicActor* pPlayerLogicActor = pLayer->GetLogicVehicleActor(0);
	Game::Actors::C_VehicleActor* pPlayerActor = pPlayerLogicActor->GetGraphicalActor(0);

	VehiclePositionInfo& playerPositionInfo = pPlayerLogicActor->GetPositionInfo();

	distanceInfo distanceInFrontOfPlayerArray[MAX_VEHICLES];
	// Calculate and cache the distance between each 'ai' and the player
	distanceInFrontOfPlayerArray[0].mDistance = -1.0f;
	distanceInFrontOfPlayerArray[0].mIndex = 0;
	
	for(int i=1;i<MAX_VEHICLES;i++ )
	{
		distanceInFrontOfPlayerArray[i].mDistance = -1.0f;
		distanceInFrontOfPlayerArray[i].mIndex = i;

		Game::Actors::C_VehicleLogicActor* pLogicActor = pLayer->GetLogicVehicleActor(i);
		if(pLogicActor)
		{
		
			Game::Actors::C_VehicleActor* pActor = pLogicActor->GetGraphicalActor(i);

			VehiclePositionInfo& positionInfo = pLogicActor->GetPositionInfo();

			
			if(pActor->IsVisible() && !pLogicActor->IsWreckedOrEliminated())
			{
				distanceInFrontOfPlayerArray[i].mDistance = AIAmax::AmaxAIR::Instance().DistanceInfrontBehindLogicCalculationParamatised(playerPositionInfo.mLogicPosition, positionInfo.mLogicPosition);
			}
		}
	}

	qsort(distanceInFrontOfPlayerArray, MAX_VEHICLES, sizeof(distanceInfo), sortByDistanceFunc);

	InitialiseSpotlights();

	int inRangeCount = 0;
	// update the vehicles which are shown with effects to highlight their positions
	// start from 1 as we are not interested in the player
	for(int j=0;j<MAX_VEHICLES;j++ )
	{
		
		F32 desiredIntensity = 0.0f;
		F32 distanceInFrontOfPlayer = distanceInFrontOfPlayerArray[j].mDistance;
		int vehicleIdx = distanceInFrontOfPlayerArray[j].mIndex;

		Game::Actors::C_VehicleLogicActor* pLogicActor = pLayer->GetLogicVehicleActor(vehicleIdx);
		Game::Actors::C_VehicleActor* pActor = NULL;

		if(pLogicActor)
		{
			 pActor = pLogicActor->GetGraphicalActor(vehicleIdx);
		}

		if( distanceInFrontOfPlayer >= 0.0f && 
			pActor &&
			inRangeCount < mpDestructionParams->mMaxDistanceEffects )
		{
			if( distanceInFrontOfPlayer > mpDestructionParams->mMinRangeForDistanceEffect &&
				distanceInFrontOfPlayer < mpDestructionParams->mMaxRangeForDistanceEffect )
			{
				desiredIntensity = gDestructModeDistanceEffectFullIntensity;
				inRangeCount++; 

				if(!mPillarEffects[vehicleIdx].active())
				{
					const SpecialFX::C_SpecialFX* p_pEffect = NULL;

					U32 vehicleID = pActor->GetXtIndex();
					if( vehicleID == mRaceModeDestructionData->vehicleEasy->vehicle->ID )
					{
						p_pEffect = mpDestructionParams->mEasyCar.mDistanceSpecialFX;
					}
					else if( vehicleID == mRaceModeDestructionData->vehicleMedium->vehicle->ID )
					{
						p_pEffect = mpDestructionParams->mMediumCar.mDistanceSpecialFX;
					}
					else if( vehicleID == mRaceModeDestructionData->vehicleHard->vehicle->ID )
					{
						p_pEffect = mpDestructionParams->mHardCar.mDistanceSpecialFX;
					}
					else if( vehicleID == mRaceModeDestructionData->vehicleVHard->vehicle->ID )
					{
						p_pEffect = mpDestructionParams->mVHardCar.mDistanceSpecialFX;
					}

					mPillarEffects[vehicleIdx].setEffect(p_pEffect);
					mPillarEffects[vehicleIdx].start();
				}
			}
			else if( mPillarEffects[vehicleIdx].active() )
			{
				if( (distanceInFrontOfPlayer < (mpDestructionParams->mMinRangeForDistanceEffect - gDestructModeDistanceEffectBuffer)) ||
					(distanceInFrontOfPlayer > (mpDestructionParams->mMaxRangeForDistanceEffect + gDestructModeDistanceEffectBuffer)))
				{
					if( !mPillarEffects[vehicleIdx].visible())
					{
						mPillarEffects[vehicleIdx].stop();
					}
				}
			}

		}
		else
		{
			if( !mPillarEffects[vehicleIdx].visible())
			{
				mPillarEffects[vehicleIdx].stop();
			}
		}

		// Update the distance effect
		mPillarEffects[vehicleIdx].update(frameTime, desiredIntensity ,pActor?&pActor->GetMatrix():NULL);
	}
}

void C_RaceModeDestruction::CheckRaceEnd()
{
	C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	bool trigger = false;

	const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);
	
	float frameTime = C_GameApp::GetInstance()->GetTimeStep();
	

	if( gDestructionUnlimitedTime == false)
	{
		mTimeLeft -= frameTime;
	}
	
	if(mWreckedTimer < 0.0f )
	{
		if(pLogicActor->GetCurrentHealthLevel() == E_HEALTH_LEVEL_WRECKED)
		{
			mWreckedTimer = gDestructModeWreckedTimer;
		}
	}
	else
	{
		mWreckedTimer -= frameTime;
		if( mWreckedTimer < 0.0f)
		{
			mTimeLeft = 0.0f;
		}
	}


	if (mTimeLeft <= 0.0f)
	{
		if (mPassed)
			C_RaceRulesManager::Instance().SendPassMessage(0);
		else
			C_RaceRulesManager::Instance().SendFailMessage(0);

		F32 accuracy = 0.f;
		if(pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE] > 0)
		{
			accuracy = (F32)pLogicActor->GetStatistics().mNumPerksHit[E_PICKUP_TYPE_BLINDFIRE] / (F32)pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE];
		}

		pLogicActor->GetGraphicalActor(0)->SetHealthEffect(NULL);
#ifndef CFG_GOLD
		AmaxDataLogging::LogDestructionInfo(mNormalShots, mLongShots, mSkillShots, mTotalCarsWrecked, mNumPoints, mEasyCarsWrecked, mMediumCarsWrecked, mHardCarsWrecked, 
			mVHardCarsWrecked, accuracy, pLogicActor->GetStatistics().mNumPerksFired[E_PICKUP_TYPE_BLINDFIRE], mBonusPoints);
#endif
		mRaceInProgress = false;
	}
}


void C_RaceModeDestruction::StartRace()
{
	C_RaceModeBase::StartRace();
}

void C_RaceModeDestruction::UpdateHUD()
{
	if(!mRaceInProgress)
		return;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	if(mScoreTimer)
	{
		mScoreTimer=Maths::Clamped(mScoreTimer-pRLC->GetFrameTime(),0.f,3.0f);

		if(mScoreTimer == 0.f)
		{
			UI::C_UiTimelineMessage message( "earning_power", false );
			UI::gButtons.BroadcastMessageNow( 0xffffffff, &message );
		}
	}
	if(mRulePassedTimer)
	{
		mRulePassedTimer=Maths::Clamped(mRulePassedTimer-pRLC->GetFrameTime(),0.f,3.0f);
	}
}

const UI::C_WideString512 C_RaceModeDestruction::GetScoreString()
{
	UI::C_WideString512 str;
	UI::C_WideString64 str2;

	str.Clear();

	return str;
}

const UI::C_WideString512 C_RaceModeDestruction::GetRulePassedString(U32 idx)
{
	UI::C_WideString512 str;

	if(mRulePassedTimer)
		C_RaceRulesManager::Instance().GetPassRuleString(idx, str, mRuleIndex, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);
	else
		str.Clear();

	return str;
}

const UI::C_WideString512 C_RaceModeDestruction::GetRuleFailedString(U32 idx)
{
	UI::C_WideString512 str;

	if(mRulePassedTimer)
		C_RaceRulesManager::Instance().GetFailRuleString(idx, str, mRuleIndex, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS);
	else
		str.Clear();

	return str;
}

bool C_RaceModeDestruction::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	if(!ShouldHandleMessages())
		return false;

	bool handledMessage = true;

	switch( pMessage->Type() )
	{
			case Game::E_GameLogicMessage_LapIncremented:
			{
				const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);
				if (pMsg->mLap == 1 && pMsg->mVehicleIndex == 0)
				{
					SP_DestructionEvent* pEvent = GetEvent();
					if (pEvent && pEvent->event && pEvent->event->raceMode)
					{
						AmaxRaceModeDestruction* pRaceMode = (AmaxRaceModeDestruction*)pEvent->event->raceMode;

						if (AmaxRaceModeDestructionVars* diffVars = GetDifficultyVars())
							mTimeLeft = diffVars->initialTime;
//						mTimeLeft = pRaceMode->initialTime;
						mStarted = true;
					}
				}
			}
			break;

		//temp until we have a proper gadget
		case Game::E_GameLogicMessage_Scoring_Event:
			{
				const Game::C_ScoringEventMsg* pMsg = Messaging::MessageCast<Game::C_ScoringEventMsg>(pMessage);

				if(pMsg->mVehicleIndex == 0)
				{
					UI::C_UiTimelineMessage message( "earning_power", true );
					UI::gButtons.BroadcastMessageNow( 0xffffffff, &message );

					mScoreTimer=3.0f;
					mScore=pMsg->mScore;
					mScoreString=pMsg->mString;
				
				}
			}
			break;

		case airacingtech::E_DestructionModeMessage_WaveCompleted:
			
			{
				C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				Game::Actors::C_VehicleLogicActor* playerVehActor	= pLayer->GetLogicVehicleActor(0);

				playerVehActor->GetScoring().mDestruction_WreckedAll = true;
			}
			break;

		case Game::E_GameLogicMessage_Passed_All_Rule:
			{
				const Game::C_PassedAllRuleMsg* pMsg	= Messaging::MessageCast<Game::C_PassedAllRuleMsg>(pMessage);
				Game::Actors::RaceLogicCore* pRLC		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

				if( !pRLC->IsVehicleAIControlled(pMsg->mVehicleIndex) )
				{
					_UI_MESSAGE(E_GameflowMessage_RaceFinished);
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished);
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
				}
				else
				{
					// Should hopefully never be hit, but just in case, we remove them as the player cannot target them.
					Game::C_RaceLogicLayer* pLayer				= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
					pLayer->GetLogicVehicleActor(pMsg->mVehicleIndex)->PreRemoveFromRace(g_debugNetworkLeftRaceGhostTime);
				}
			}
			break;


		case Game::E_GameLogicMessage_Failed_Rule:
			{
				const Game::C_FailedRuleMsg* pMsg = Messaging::MessageCast<Game::C_FailedRuleMsg>(pMessage);
				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

				if( !pRLC->IsVehicleAIControlled(pMsg->mVehicleIndex) )
				{
					_UI_MESSAGE(E_GameflowMessage_Failed);
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
				}
			}
			break;

		case Game::E_GameflowMessage_IntroStarted:
		case Game::E_GameflowMessage_OutroStarted:
			{
				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(0);
				if(pActor)
					pActor->ForceAIControl();
			}
			break;

		case Game::E_GameflowMessage_RaceStarted:
			{
#ifndef CFG_GOLD
				//	Very quick and dirty fix for player control behing forced in replays!
				if(C_ReplayDirector::Instance().GetReplayMode() == E_ReplayMode_Play ||
					C_ReplayDirector::Instance().GetReplayMode() == E_ReplayMode_PrePlay)
					break;
#endif
				Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				Game::Actors::C_VehicleActor *pActor = (Game::Actors::C_VehicleActor *) pRLC->GetVehicleActor(0);
				const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
				
				if (pActor && levelState.mCars[pActor->GetVehicleIndex()].mControlType != E_VEHICLE_CONTROL_TYPE_AI)
				{
					pActor->ForcePlayerControl();
				}
			}
			break;

		case Game::E_GameflowMessage_RaceFinished:
			{
				Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

				for( int i = 0; i < pLayer->GetNumVehicles(); i++)
				{
					Game::Actors::C_VehicleActor* pActor = (Game::Actors::C_VehicleActor*) pLayer->GetLogicVehicleActor(i)->GetGraphicalActor(i);
					if(pActor && !pRLC->IsVehicleAIControlled(i))
						pActor->ForceAIControl();
				}

			}
			break;


		case E_VehicleMessage_FatallyDamaged:
			{
				const Game::Actors::C_VehicleFatallyDamagedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleFatallyDamagedMsg>(pMessage);

				Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
				if (pMsg->m_VehicleIndex != 0)
				{
					levelState.mCars[pMsg->m_VehicleIndex].mState = E_VEHICLE_STATE_FATALLY_DAMAGED;
				}
				else
				{
					_UI_MESSAGE1(E_GameflowMessage_Wrecked, UI::GetLocalPlayerIndex(0));
					_UI_MESSAGE(E_GameflowMessage_RaceFinished);
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
					C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
				}
			}
			break;



		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);

				C_LevelState&						levelState			= C_Game::GetInstance()->GetLevelState();
				S32									hit_veh_index		= pMsg->mVehicleIndex;
				S32									firing_veh_index	= pMsg->mFiringVehicleIndex;
				bool								isWrecked			= ( levelState.mCars[ hit_veh_index ].mState == E_VEHICLE_STATE_FATALLY_DAMAGED );


				if (hit_veh_index == 0)				// Player was hit
				{
/*
					if (isWrecked)
					{
						_UI_MESSAGE1(E_GameflowMessage_Wrecked, UI::GetLocalPlayerIndex(hit_veh_index));
						_UI_MESSAGE(E_GameflowMessage_RaceFinished);
						C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
						C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
					}
*/
					break;
				}


				// AI car has been hit

				C_RaceLogicLayer*					pLayer				= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				RLGameMode_Destruction*				mpDestructionParams	= &RLDatabase::Instance().mRLSystems[0]->mGameModeParameters->mDestruction;
				AmaxScoringConfig*					pScoringConfig		= C_GameData::Instance().mScoringConfig[0];

				Game::Actors::C_VehicleLogicActor*	playerLogActor		= pLayer->GetLogicVehicleActor(0);

				Game::Actors::RaceLogicCore*		pRLC				= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();


				S32		bonus_points = 0;
				S32		numPointsToAdd = 0;
				S32		string_index = 0;				// Index of string for this shot type 0 - 7
				bool	longshot = false;
				bool	skillshot = false;

				if (firing_veh_index == 0)				// Check for special shots
				{
					longshot = (playerLogActor->mVehicleProximities[hit_veh_index].mDistanceSquared > (pScoringConfig->long_distance * pScoringConfig->long_distance));
					if(longshot)
					{
						bonus_points++;
						string_index += 2;
						mLongShots++;

						// [mtaylor] (bug 37649) -- We wish to know about this for the stickers, since normal scoring moves do not accurately inform us of this
						Game::C_StickerManager::Instance().CheckDestructionLongshot();
					}
					skillshot = (playerLogActor->GetScoring().mValidDriftDistance > gDestructModeMinDriftDistance);
					if (skillshot)
					{
						bonus_points++;
						string_index += 4;
						mSkillShots++;
					}
					if (bonus_points == 0)
						mNormalShots++;
				}

				if (isWrecked)
				{
					string_index += 1;
					mTotalCarsWrecked++;
					if (longshot)
						UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_DestructionWreckLongshotActionIn );
					else
						UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_DestructionWreckedActionIn );

					pRLC->GetVehicleActor(hit_veh_index)->ApplyWreckedImpulse(gWreckedImpulseScale);
				}



				numPointsToAdd = bonus_points;
				U32 vehicleID = pRLC->GetVehicleActor(hit_veh_index)->GetXtIndex();
				
				// We have wrecked a vehicle.. lets get the appropriate number of points based on how difficult they were!
				numPointsToAdd += ProcessVehicleDamagedScore(vehicleID, isWrecked, mpDestructionParams);

				mLastTime = 0.0f;						// Increment time
				if (isWrecked)
				{
					mLastTime = CalculateAdditionalTimeReward(vehicleID);
					mTimeLeft += mLastTime;
				}


				if(mRaceInProgress)
					mNumPoints += numPointsToAdd;					// Increment current total


				if (firing_veh_index == 0)						// If player did the damage
				{
					mLastPointsName = levelState.mCars[ hit_veh_index ].mNameAboveCar;
					mLastPoints = numPointsToAdd;

					// Setup the last points string with the correct text
					SetupPointsString(string_index); 

					if ( numPointsToAdd > 0 )
						_UI_MESSAGE1( UI::E_UiGameMsg_SpDestructionPointsAdded, (bonus_points > 0) );		// Make HUD display message
				}
				else												// If AI did the damage
				{
					mLastBonusPointsName = levelState.mCars[ hit_veh_index ].mNameAboveCar;
					if (mLastBonusTimer > 0)
						mLastBonusPoints += numPointsToAdd;
					else
						mLastBonusPoints = numPointsToAdd;

					mBonusPoints += numPointsToAdd;

					if ( numPointsToAdd > 0 )
					{
						if ( mLastBonusTimer > 0 )
							_UI_MESSAGE( UI::E_UiGameMsg_SpDestructionBonusHitAdded );
						else
							_UI_MESSAGE( UI::E_UiGameMsg_SpDestructionBonusHitStart );
					}

					mLastBonusTimer = gDestructModeBonusHitDuration;

					mLastBonusPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_DESTRUCT_BONUS_HIT" ).String();		// It's always this string but it could change and probably will

					SP_BossDemands::Instance().RegisterBossDemandAction(E_BOSSDEMAND_ACTION_DESTRUCTION_BONUS_HIT);
				}


			}
			break;

			case E_VehicleLogicMsg_AggressiveScore:
			{
				const C_VehicleLogicMsg_AggressiveScore* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_AggressiveScore>(pMessage);
				HandleAggressiveScoreMessage(pMsg); 
			}
			break;

	}

	return handledMessage;
}

void C_RaceModeDestruction::SetupPointsString(const S32 string_index)
{
	switch(string_index)
	{
		case 0:
			mLastPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_DESTRUCT_BOLTED" ).String();
			break;

		case 1:
			mLastPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_DESTRUCT_WRECKING_SHOT" ).String();
			break;

		case 2:
			mLastPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_DESTRUCT_LONG_BOLT" ).String();
			break;

		case 3:
			mLastPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_DESTRUCT_LONG_WRECKING_SHOT" ).String();
			break;

		case 4:
			mLastPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_DESTRUCT_SKILLED_BOLT" ).String();
			break;

		case 5:
			mLastPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_DESTRUCT_SKILLED_WRECKING_SHOT" ).String();
			break;

		case 6:
			mLastPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_DESTRUCT_SKILLED_LONG_BOLT" ).String();
			break;

		case 7:
			mLastPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_DESTRUCT_SKILLED_LONG_WRECKING_SHOT" ).String();
			break;
	}
}

F32 C_RaceModeDestruction::CalculateAdditionalTimeReward(const U32 victimCarXtID) const
{
	// grab the RLL
	C_RaceLogicLayer*					pLayer				= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

	// Grab the player logic actor
	Game::Actors::C_VehicleLogicActor*	playerLogActor		= pLayer->GetLogicVehicleActor(0);

	// Init time reward
	F32 timeReward = 0.0f;

	if(mEventState != kSP_Gold)
	{
		SP_DestructionEvent* pEvent = GetEvent();
		if (pEvent && pEvent->event && pEvent->event->raceMode)
		{
			AmaxRaceModeDestruction* pRaceMode = (AmaxRaceModeDestruction*)pEvent->event->raceMode;

			if(mTimeLeft < 0.5f)
			{
				playerLogActor->GetScoring().mDestruction_CloseCallWrecked = true;
			}

			if( victimCarXtID == mRaceModeDestructionData->vehicleEasy->vehicle->ID )
			{
				timeReward = pRaceMode->timeForWreckEasy;
			}
			else if( victimCarXtID == mRaceModeDestructionData->vehicleMedium->vehicle->ID )
			{
				timeReward = pRaceMode->timeForWreckMedium;
			}
			else if( victimCarXtID == mRaceModeDestructionData->vehicleHard->vehicle->ID )
			{
				timeReward = pRaceMode->timeForWreckHard;
			}
			else if( victimCarXtID == mRaceModeDestructionData->vehicleVHard->vehicle->ID )
			{
				timeReward = pRaceMode->timeForWreckVHard;
			}
		}
	}

	return timeReward;
}

U8 C_RaceModeDestruction::ProcessVehicleDamagedScore(const U32 victimCarXtID, const bool isVictimWrecked, const RLGameMode_Destruction* mpDestructionParams)
{
	// default the points
	U8 numPointsToAdd = 0;

	// grab the RLL
	C_RaceLogicLayer*					pLayer				= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

	// Grab the player logic actor
	Game::Actors::C_VehicleLogicActor*	playerLogActor		= pLayer->GetLogicVehicleActor(0);

	// Increase based on what was damaged..
	if( victimCarXtID == mRaceModeDestructionData->vehicleEasy->vehicle->ID )					// Easy
	{
		numPointsToAdd += mpDestructionParams->mEasyCar.mHitPoints;
		if (isVictimWrecked)
		{
			mEasyCarsWrecked++;
			numPointsToAdd += mpDestructionParams->mEasyCar.mWreckedPoints;
			playerLogActor->GetScoring().mDestruction_WreckedEasy = true;
		}
	}
	else if( victimCarXtID == mRaceModeDestructionData->vehicleMedium->vehicle->ID )			// Medium
	{
		numPointsToAdd += mpDestructionParams->mMediumCar.mHitPoints;
		if (isVictimWrecked)
		{
			mMediumCarsWrecked++;
			numPointsToAdd += mpDestructionParams->mMediumCar.mWreckedPoints;
			playerLogActor->GetScoring().mDestruction_WreckedMedium = true;
		}
	}
	else if( victimCarXtID == mRaceModeDestructionData->vehicleHard->vehicle->ID )				// Hard
	{
		numPointsToAdd += mpDestructionParams->mHardCar.mHitPoints;
		if (isVictimWrecked)
		{
			mHardCarsWrecked++;
			numPointsToAdd += mpDestructionParams->mHardCar.mWreckedPoints;
			playerLogActor->GetScoring().mDestruction_WreckedHard = true;
			Game::C_StickerManager::Instance().WreckedRedCarInDestruction();			// there is a sticker for this, check it
		}
	}
	else if( victimCarXtID == mRaceModeDestructionData->vehicleVHard->vehicle->ID )				// VHard
	{
		numPointsToAdd += mpDestructionParams->mVHardCar.mHitPoints;
		if (isVictimWrecked)
		{
			mVHardCarsWrecked++;
			numPointsToAdd += mpDestructionParams->mVHardCar.mWreckedPoints;
			playerLogActor->GetScoring().mDestruction_WreckedVHard = true;
	//		Game::C_StickerManager::Instance().WreckedRedCarInDestruction();			// DOES THIS NEED FIXING UP FOR BLUE CAR?
		}
	}

	// done!
	return numPointsToAdd;
}

// For UI testing only
void C_RaceModeDestruction::SetupFakeResults()
{
	Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState& state=C_Game::GetInstance()->GetLevelState();

	if(!state.mEvent)
		state.mEvent = C_GameEventData::Instance().mEvents[0]->ID;

	if(!state.mCity)
		state.mCity = C_GameData::Instance().mCities[0]->ID;
	
	if(!state.mNumCars)
		state.mNumCars = C_GameEventData::Instance().mEvents[0]->opponents.count;

	results.Clear();
	results.mFake=true;
	results.mNumCars=state.mNumCars;

	int i = 1;

	for(unsigned int a=0;a<state.mNumCars;a++)
	{
		results.mCars[a].mPosition = i++;
		results.mCars[a].mScore = 1000;
		results.mCars[a].mStatistics.Clear();
		results.mCars[a].mStatistics.mFinished = true;

		for( int b = 0; b < C_RaceRulesManager::Instance().GetNumPassRules(); b++ )
		{
			results.mCars[a].mPassedRule.PushBack(true);
		}

		for( int b = 0; b < C_RaceRulesManager::Instance().GetNumFailRules(); b++ )
		{
			results.mCars[a].mFailedRule.PushBack(false);
		}
	}

	C_RaceModeBase::SetupResults();

	results.Finished();

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_ResultsCalculated );

	mRaceInProgress=false;
}

void C_RaceModeDestruction::SetupQuitAndWinResults(lua_State *pState)
{	
	Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState& state=C_Game::GetInstance()->GetLevelState();

	results.Clear();
	results.mNumCars=state.mNumCars;

	for(unsigned int a=0;a<state.mNumCars;a++)
	{
		results.mCars[a].mPosition=a+1;
		results.mCars[a].mStatistics.Clear();
		results.mCars[a].mStatistics.mFinished = true;
	}

	SP_DestructionEvent* pEvent = GetEvent();

	if (pEvent && pEvent->event)
	{
		if (const Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0))
		{
			S8 pos;
			Lua::GetDataMember(pState,"Position",pos,true);

			int score;
			Lua::GetDataMember(pState,"XP",score,true);

			int goldFanDemand;
			Lua::GetDataMember(pState,"GoldFanDemand",goldFanDemand,true);
			int goldFanDemandFans;
			Lua::GetDataMember(pState,"GoldFanDemandFans",goldFanDemandFans,true);

			if (pos <= 1)
				results.mRaceStatistics.mMedal = kSP_Gold;
			else if (pos <= 2)
				results.mRaceStatistics.mMedal = kSP_Silver;
			else if (pos <= 3)
				results.mRaceStatistics.mMedal = kSP_Bronze;
			else
				results.mRaceStatistics.mMedal = kSP_Incomplete;

			results.mRaceStatistics.mFansEarned = score;
			results.mRaceStatistics.mFanDemandsComplete = pLogicActor->GetScoring().mFanDemandsComplete;
			results.mRaceStatistics.mGoldenFanDemandComplete = (goldFanDemand == 1);
			results.mRaceStatistics.mGoldenFanDemandScore = goldFanDemandFans;
		}
	}

	C_RaceModeBase::SetupQuitAndWinResults(pState);

	results.Finished();

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_QuitAndWinRace );
	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_ResultsCalculated );

	mRaceInProgress=false;
}

void C_RaceModeDestruction::SetupResults()
{
	Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState& state=C_Game::GetInstance()->GetLevelState();
	
	results.Clear();
	results.mNumCars=state.mNumCars;

	for(unsigned int a=0;a<state.mNumCars;a++)
	{
		const Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(a);
		if(a==0)
		{
			results.mRaceStatistics.mTotalRaceTime=pLogicActor->GetStatistics().mTotalRaceTime;
		}
	}

	SP_DestructionEvent* pEvent = GetEvent();

	if (pEvent && pEvent->event)
	{
		if (AmaxRaceModeDestructionVars* diffVars = GetDifficultyVars())
		{
			if (mNumPoints >= diffVars->goldPoints)
				results.mRaceStatistics.mMedal = kSP_Gold;
			else if (mNumPoints >= diffVars->silverPoints)
				results.mRaceStatistics.mMedal = kSP_Silver;
			else if (mNumPoints >= diffVars->bronzePoints)
				results.mRaceStatistics.mMedal = kSP_Bronze;
			else
				results.mRaceStatistics.mMedal = kSP_Incomplete;
		}

		const Actors::C_VehicleLogicActor* pLogicActor	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);
		if (pLogicActor)
		{
			results.mRaceStatistics.mFansEarned = pLogicActor->GetScoring().mFinalScore;
			results.mRaceStatistics.mFanDemandsComplete = pLogicActor->GetScoring().mFanDemandsComplete;
			results.mRaceStatistics.mGoldenFanDemandComplete = pLogicActor->GetScoring().mGoldenFanDemandComplete;
			results.mRaceStatistics.mGoldenFanDemandScore = pLogicActor->GetScoring().mGoldenFanDemandScore;
		}

		results.mRaceStatistics.mDestruction_NumPoints=mNumPoints;
		results.mRaceStatistics.mDestruction_TotalCarsWrecked=mTotalCarsWrecked;
		results.mRaceStatistics.mDestruction_EasyCarsWrecked=mEasyCarsWrecked;
		results.mRaceStatistics.mDestruction_MediumCarsWrecked=mMediumCarsWrecked;
		results.mRaceStatistics.mDestruction_HardCarsWrecked=mHardCarsWrecked;
		results.mRaceStatistics.mDestruction_VHardCarsWrecked=mVHardCarsWrecked;
	}

	C_RaceModeBase::SetupResults();

	results.Finished();

	C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_ResultsCalculated );
	mRaceInProgress=false;
}

bool C_RaceModeDestruction::ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel)
{
	return false;
}

bool C_RaceModeDestruction::ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel)
{
	return false;
}

AmaxRaceModeDestructionVars* C_RaceModeDestruction::GetDifficultyVars()
{
	SP_DestructionEvent* pEvent = GetEvent();
	if (!pEvent || !pEvent->event || !pEvent->event->raceMode)
		return(NULL);

	AmaxRaceModeDestruction* pRaceMode = (AmaxRaceModeDestruction*)pEvent->event->raceMode;

	switch(C_Game::GetInstance()->GetLevelState().m_difficulty)
	{	
		case Game::GAME_DIFFICULTY_VERY_EASY:
		case Game::GAME_DIFFICULTY_EASY:
			return(&pRaceMode->easy);

		case Game::GAME_DIFFICULTY_NORMAL:
			return(&pRaceMode->medium);

		case Game::GAME_DIFFICULTY_HARD:
		case Game::GAME_DIFFICULTY_HARDCORE:
			return(&pRaceMode->hard);
	}

	return(&pRaceMode->easy);					// Should never reach here
}


void C_RaceModeDestruction::HandleAggressiveScoreMessage(const C_VehicleLogicMsg_AggressiveScore* pMsg)
{
	// (sanity check) we didnt do this to ourselves, 
	// and a player did indeed hit an AI
	if(	pMsg->mVehicleIndex != 0 ||			// Ensure a player was the one who scored this aggressive move
	    pMsg->mAboutVehicleIndex == 0   )	// Ensure the victim an AI player	  
	{
		return;
	}

	// So we know now that a player has gained aggressive score over an AI
	bool causedEnvironmentalDeath = false;
	bool wasDrown = false;
	switch(pMsg->mBonusScoringEvent)
	{
	
	case E_BLUR_SCORE_MOVE_BONUS_DROWN:
		{
			wasDrown = true;
		}
	case E_BLUR_SCORE_MOVE_BONUS_CLIFF: // Deliberate fall through
		{
			causedEnvironmentalDeath = true; // If drowned / cliffed AI.. then we are interested in crediting this!
		}
		break;

	default:
		break;
	}

	// If we caused a death by cliff/drowning then lets get some points + time for it!
	if(causedEnvironmentalDeath)
	{
		const Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(pMsg->mAboutVehicleIndex);
		BIZ_ASSERT_MSG(pLogicActor,"C_RaceModeDestruction::HandleAggressiveScoreMessage(const C_VehicleLogicMsg_AggressiveScore* pMsg) < could not obtain pointer to logic actor");

		Game::Actors::C_VehicleActor* pActor = pLogicActor->GetGraphicalActor(pMsg->mAboutVehicleIndex);
		BIZ_ASSERT_MSG(pActor,"C_RaceModeDestruction::HandleAggressiveScoreMessage(const C_VehicleLogicMsg_AggressiveScore* pMsg) < could not obtain pointer to graphical actor");
		
		U32 victimCarXtID = pActor->GetXtIndex(); // Get the sorcery ID for the wrecked vehicle
		ProcessEnvironmentalKill(pMsg->mVehicleIndex,pMsg->mAboutVehicleIndex, victimCarXtID, wasDrown);
	}
}


void C_RaceModeDestruction::ProcessEnvironmentalKill(const U8 killerVehicleIdx, const U8 victimVehicleIdx, const U32 vehicleXtID, const bool wasDrown)
{
	// Give the Killer some points
	AwardEnvironmentalKillPoints(killerVehicleIdx, victimVehicleIdx, vehicleXtID, wasDrown);

	// Increment our stats 
	mTotalCarsWrecked++;

	// Award additional time
	mLastTime = 0.0f;					
	mLastTime = CalculateAdditionalTimeReward(vehicleXtID);
	mTimeLeft += mLastTime;

	// Clean up the victim's remains
	CleanUpEnvironmentalKillVictim(victimVehicleIdx);
}

void C_RaceModeDestruction::AwardEnvironmentalKillPoints(const U8 killerVehicleIdx, const U8 victimVehicleIdx, const U32 victimCarXtID, bool wasDrown)
{	
	// We have wrecked a vehicle.. lets get the appropriate number of points based on how difficult they were!
	U8 numPointsToAdd = ProcessVehicleDamagedScore(victimCarXtID,true,mpDestructionParams);

	// Now we have worked out how many points we get... Give them!
	if(mRaceInProgress)
		mNumPoints += numPointsToAdd;

	// Setup String / UI prompts
	if (killerVehicleIdx == 0 )					
	{
		mLastPointsName  = C_Game::GetInstance()->GetLevelState().mCars[ victimVehicleIdx ].mNameAboveCar;
		mLastPoints		 = numPointsToAdd;

		if( wasDrown )
		{
			mLastPointsString = UI::C_TextStringID( "SCR_NEW_AMAX_SCORING_DROWNING_SHOT_NO_NAME" ).String(); 
		}
		else
		{
			mLastPointsString = UI::C_TextStringID( "SCR_AMAX_SCORING_EVENT_CLIFF_DIVER" ).String();
		}

		_UI_MESSAGE1( UI::E_UiGameMsg_SpDestructionPointsAdded, (numPointsToAdd > 0) );
		UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_DestructionWreckedActionIn );
	}

}

void C_RaceModeDestruction::CleanUpEnvironmentalKillVictim(const U8 victimVehicleIdx)
{
	// (for now..) set victim to wrecked so they get removed...
	const Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(victimVehicleIdx);
	BIZ_ASSERT_MSG(pLogicActor,"C_RaceModeDestruction::CleanUpEnvironmentalKillVictim(const U8 victimVehicleIdx) < unable to obtain pointer to logic actor..");
		
	C_VehicleLogicForceDamageMsg forceDamageMsg(pLogicActor->GetVehicleIndex(),10000); // Ick... Hardcoded :( want to make sure this guy is DEAD. 
	pLogicActor->GetActorList()->SendMessageToActorComponent(pLogicActor->GetHandle(),Components::E_IF_VehicleComponent_Damage,&forceDamageMsg);	
	
}

} // namespace Game
