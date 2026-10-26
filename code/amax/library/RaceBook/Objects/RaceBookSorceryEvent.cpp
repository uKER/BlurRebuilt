#include "stdafx.h"

#include <RaceBook/Objects/RaceBookEvent.h>
#include <RaceBook/RaceBookManager.h>
#include <GameLogic/RaceRules/RaceRulesManager.h>
#include <GameLogic/Structures/LevelState.h>
#include <GameLogic/Structures/LevelResult.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Profile/GameProfileManager.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

Debug::Bool g_RemoveMinorCharactersFromRaces("Game.RemoveMinorCharactersFromRaces", false, Debug::Persist());

Debug::Logger &g_RaceBookSorceryEventLogger( Debug::GetLogger("RaceBook.SorceryEvent") );

extern Debug::Int g_debugShieldsEarnedRepairSinglePlayer;
extern Debug::Int g_debugShieldsEarnedHitByPerkSinglePlayer;
extern Debug::Int g_debugShieldsEarnedFansSinglePlayer;
extern Debug::Int g_debugShieldsEarnedFansRequiredSinglePlayer;
extern Debug::Int g_debugShieldsInitialSinglePlayer;

#ifndef CFG_GOLD
extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;
#endif

Debug::Int g_debugForceVehicleTextureDetail("Vehicle.LOD.SinglePlayerForceTextureDetail", 0, Debug::Range(0, 10), Debug::Step(1));
Debug::Int g_debugSPNonFocusDetail("Vehicle.LOD.SinglePlayerNonFocusDetail", 7, Debug::Range(1, 10), Debug::Step(1));

namespace RaceBook
{
	void C_RaceBookSorceryEvent::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		Sys::BinarySerialiserAutoEndian out( stream );
		out << mAmaxEventID << mMessageId << mFinalPosition;
		C_RaceBookEvent::Serialise(stream);
	}

	void C_RaceBookSorceryEvent::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		Sys::BinaryDeserialiserAutoEndian in( stream );
		in >> mAmaxEventID >> mMessageId >> mFinalPosition;
		C_RaceBookEvent::Deserialise(stream);
	}

	void C_RaceBookSorceryEvent::SendToLua( lua_State* pState )
	{
		C_RaceBookEvent::SendToLua( pState );

		AmaxEvent* pEvent = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)mAmaxEventID];
		if(!pEvent)
		{
			LOGWARN( g_RaceBookSorceryEventLogger, DBSTR " event %d not found", mAmaxEventID );
			return;
		}

		Lua::AddDataMember( pState, "tag",				pEvent->tag );
		Lua::AddDataMember( pState, "name",				pEvent->loc_string.id );
		Lua::AddDataMember( pState, "has_message",		( mMessageId ) ? true : false );
		Lua::AddDataMember( pState, "icon",				pEvent->route->city->icon_name );
		Lua::AddDataMember( pState, "city_name",		pEvent->route->city->loc_string.id );
		Lua::AddDataMember( pState, "route_name",		pEvent->route->loc_string.id );
		Lua::AddDataMember( pState, "circuit_type",		pEvent->route->circuitType );
		Lua::AddDataMember( pState, "amax_event_id",	(GAMEDATA_ID)mAmaxEventID );
		Lua::AddDataMember( pState, "car_class",		GetCarClass() );
		Lua::AddDataMember( pState, "post_race_msg",	pEvent->postRaceMessage.id );
		Lua::AddDataMember( pState, "disable_fans",		pEvent->disableFans );
	}

	void C_RaceBookSorceryEvent::SetupRace()
	{
		Game::C_LevelState&  levelState  = Game::C_Game::GetInstance()->GetLevelState();
		Game::C_LevelResult& levelResult = Game::C_Game::GetInstance()->GetLevelResult();

		C_RaceBookEvent::SetupRace();

		Game::C_GameProfile *game_profile=Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		game_profile->mNeedsAIUpdate = true;

		U32 eventId = ((C_RaceBookSorceryEvent*)Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentEvent)->mAmaxEventID;
		AmaxEvent *ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)eventId];

		Game::GAME_DIFFICULTY difficulty = (Game::GAME_DIFFICULTY)game_profile->GetDifficulty();
		
		//[PK] sorry for the hard coded number, its used in other places though :)
		if(ev->ID == 0xAFA00028)
		{
			difficulty=Game::GAME_DIFFICULTY_NORMAL;
			//game_profile->mOptions.mSteeringAssist=0;
			//game_profile->mOptions.mBrakingAssist=0;
		}
				
		DifficultySettings* difficultySettings = NULL;

		levelState.m_RetryCount=0;
		if(eventId == game_profile->mCurrentRetryingEvent)
		{
			levelState.m_RetryCount=(float)game_profile->mNumberOfRetrys;
		}

		switch(difficulty)
		{
		case Game::GAME_DIFFICULTY_VERY_EASY:
		case Game::GAME_DIFFICULTY_EASY:
			{
				difficultySettings = &ev->easyVersion;
			}
			break;

		case Game::GAME_DIFFICULTY_NORMAL:
			{
				difficultySettings = &ev->normalVersion;
			}
			break;

		case Game::GAME_DIFFICULTY_HARD:
		case Game::GAME_DIFFICULTY_HARDCORE:
			{
				difficultySettings = &ev->hardVersion;	
			}
			break;

		default:
			BIZ_ASSERT_FAIL("difficulty is INVALID, defaulting to GAME_DIFFICULTY_NORMAL");
			difficultySettings = &ev->normalVersion;
			break;
		}

		BIZ_ASSERT(difficultySettings);

		if (difficultySettings->aiBalancingFileName.Text() != "")
		{
			Util::C_String temp = Util::C_String::Format("%s",difficultySettings->aiBalancingFileName.Text());
			if (temp.Length() < 5)
			{
				levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\Default_Easy.csv");

			}
			else
			{
				levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\%s", difficultySettings->aiBalancingFileName.Text());
			}
		}
		else
		{
			levelState.m_LevelEventAIFilename = Util::C_String::Format(".\\gamedata\\Events\\Default_Easy.csv");
		}
		
		// Retry count needs populating from somewhere, ie stats manager....
		levelState.m_AIRetryInterval = difficultySettings->aiRetryInterval * levelState.m_RetryCount;
		levelState.m_AIRetryMinTrackSpeed = difficultySettings->aiRetryMinTrackSpeed;
		levelState.m_AIDontPickupPickups = ev->aiDontPickupPickups;
		levelState.mSpecificAudioLinesOnly = ev->specificAudioLinesOnly;
		levelState.m_difficulty = difficulty;
		levelState.m_overrideAISurface = ev->overrideAISurface;
		levelState.m_overrideAISurfaceValue = ev->overrideAISurfaceValue;

		levelState.mKillOutOfWorldPlayer	= ev->killOutOfWorldPlayer;
		levelState.mKillOutOfWorldAI		= ev->killOutOfWorldAI;
		levelState.mDisableAssists			= ev->disableAssists;
		levelState.mDisablePickups			= ev->disablePickups;

		if(ev->pickupProbabilities)
		{
			for( int i = 0; i < ev->pickupProbabilities->pickupProbabilities.count; i++ )
			{
				Game::LevelPickupProbability p;
				p.pickupType = ev->pickupProbabilities->pickupProbabilities[i].pickupType;
				p.probability = ev->pickupProbabilities->pickupProbabilities[i].probability;
				levelState.mPickupProbabilities.PushBack(p);
			}
		}

		if (ev->pickupDisableTime > 0.0f)
			levelState.mPickupDisableTime = ev->pickupDisableTime;
		levelState.mPickupSpawnLevel = ev->pickupSpawnLevel;

		levelState.mEvent = ev->ID;
		levelState.mNumCars = ev->opponents.count;
		levelState.mNumAdditionalCars = 0;

		int car_index=0;

		for(unsigned int a=0;a<levelState.mNumCars + levelState.mNumAdditionalCars;a++)
		{
			AmaxOpponentSetup opponentSetup = ev->opponents[a];
			BIZ_ASSERT_MSG(opponentSetup.opponent.Object(), "NULL opponent in event %s", ev->tag.Text());

			RaceBook::C_RaceBookProfile *rbc=RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)opponentSetup.opponent->ID);
			BIZ_ASSERT_MSG(rbc, "Could not find RaceBook character to add to race");

			//	First thing's first - set this vehicle's upgrades/specs from their RB profile.
			rbc->BuildLevelStateVehicleUpgrades(levelState.mCars[car_index].mUpgrades);

			//	Secondly, we override all vehicles' pickup/shield slots and basic abilities to be the same as the player (don't ask...)
			if(opponentSetup.perkSlotsOverride)
				levelState.mCars[car_index].mUpgrades.NumPickupSlots(opponentSetup.perkSlotsOverride);
			else
				levelState.mCars[car_index].mUpgrades.NumPickupSlots(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->NumPickupSlots());
			levelState.mCars[car_index].mUpgrades.CollectAnyPerkType(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->AllowAnyPerkType());
			levelState.mCars[car_index].mUpgrades.ManualPerkChange(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->AllowManualPerkChange());
			levelState.mCars[car_index].mUpgrades.Shield(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->AllowShield());
			levelState.mCars[car_index].mUpgrades.NumShieldSlots(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->NumShieldSlots());
			levelState.mCars[car_index].mUpgrades.NumAdditionalHealthSlots(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentInGameProfile()->NumAdditionalHealthSlots());

			if(g_RemoveMinorCharactersFromRaces && rbc->mType == E_CHARACTER_TYPE_NORMAL)
				continue;

			rbc->mCurrentCarIdx = car_index;

			bool foundClass = false;
			if(rbc->mType != E_CHARACTER_TYPE_PLAYER)
			{
				for(int i = 0; i < rbc->mGarage.Size(); i++ )
				{
					RaceBook::C_RaceBookVehicle* veh = RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)rbc->mGarage[i].mId);
					AmaxVehicle* amaxVeh = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)veh->mBaseModelID];

					if( amaxVeh && amaxVeh->carClass == ev->carClass )
					{
						rbc->mCurrentCarID = rbc->mGarage[i].mId;
						foundClass = true;
						break;
					}
				}
			}

			if(!foundClass)
			{
				LOGWARN(g_RaceBookSorceryEventLogger, "Character %s has no vehicle in the class specified by the event.", UI::C_TextStringID( rbc->GetName().id ).String());
			}

			RaceBook::C_RaceBookVehicle *rbv=RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)rbc->mCurrentCarID);
			BIZ_ASSERT(rbv);

			if(opponentSetup.vehicleOverride)
			{
				rbv=RaceBook::C_RaceBookManager::Instance().GetVehicle((GAMEDATA_ID)opponentSetup.vehicleOverride->ID);
			}
			BIZ_ASSERT(rbv);

			AmaxVehicle *av = C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)rbv->mBaseModelID];
			BIZ_ASSERT(av);

			levelState.mCars[car_index].mManufacturerID=av->manufacturer->ID;
			levelState.mCars[car_index].mCarID=av->ID;

			levelState.mCars[car_index].mCharacterID=rbc->GetID();

			//if(car_index)
			levelState.mCars[car_index].mStartPosition = a;			

			

			for( int i = 0; i < opponentSetup.perkGiftTypes.count; i++ )
				levelState.mCars[car_index].mPerksToGrant.PushBack(opponentSetup.perkGiftTypes[i]);

			levelState.mCars[car_index].mPerkGrantingLaps			= opponentSetup.perkGiftLaps;
			levelState.mCars[car_index].mPerkGrantingTimer			= opponentSetup.perkGiftTimer;
			levelState.mCars[car_index].mPerkGrantingInitialTimer	= opponentSetup.perkGiftTimerInitial;
			levelState.mCars[car_index].mHealthOverride				= opponentSetup.healthOverride;

#ifndef CFG_GOLD
			if (rbc->mType == E_CHARACTER_TYPE_PLAYER && mPlayerIsAI)
			{
				levelState.mCars[car_index].mAutomatedHuman=true;
			}
#endif
			if (rbc->mType == E_CHARACTER_TYPE_PLAYER
#ifndef CFG_GOLD
				&& !mPlayerIsAI
#endif
				)
			{
				levelState.mCars[car_index].mController=SysProfile::C_Manager::Instance().GetPrimaryPadNo();
				levelState.mCars[car_index].mControlType=E_VEHICLE_CONTROL_TYPE_PLAYER;
				levelState.mCars[car_index].mHuman=true;
				levelState.mCars[car_index].mAutoGears=true;
				levelState.mCars[car_index].mResprayID=game_profile->ResprayGet(av->ID);
			}
			else
			{
				levelState.mCars[car_index].mController=Game::INVALID_CONTROLLER;
				levelState.mCars[car_index].mControlType=E_VEHICLE_CONTROL_TYPE_AI;
				levelState.mCars[car_index].mHuman=false;
				levelState.mCars[car_index].mAutoGears=true;

				for( int i = 0; i < difficultySettings->aiPerkUse.count; i++ )
				{
					if( difficultySettings->aiPerkUse[i].opponentIdx == car_index )
					{
						if( difficultySettings->aiPerkUse[i].charactersNeverTargetted.count )
						{
							for( int b = 0; b < difficultySettings->aiPerkUse[i].charactersNeverTargetted.count; b++ )
								levelState.mCars[car_index].mCharactersNotTargetted.PushBack(difficultySettings->aiPerkUse[i].charactersNeverTargetted[b]->ID);
						}
						else
						{
							for( int b = 0; b < opponentSetup.opponent->charactersNeverTargetted.count; b++ )
								levelState.mCars[car_index].mCharactersNotTargetted.PushBack(opponentSetup.opponent->charactersNeverTargetted[b]->ID);
						}
					}

					if( difficultySettings->aiPerkUse[i].preferredPickups.count )
					{
						for( int b = 0; b < difficultySettings->aiPerkUse[i].preferredPickups.count; b++ )
							levelState.mCars[car_index].mPreferredPickups.PushBack(difficultySettings->aiPerkUse[i].preferredPickups[b]);
					}
					else
					{
						for( int b = 0; b < opponentSetup.opponent->preferredPickups.count; b++ )
							levelState.mCars[car_index].mPreferredPickups.PushBack(opponentSetup.opponent->preferredPickups[b]);
					}

					if( difficultySettings->aiPerkUse[i].maxPerkLevel )
						levelState.mCars[car_index].maxPerkLevel = difficultySettings->aiPerkUse[i].maxPerkLevel;
					else
						levelState.mCars[car_index].maxPerkLevel = opponentSetup.opponent->maxPerkLevel;
				}
			}

#ifndef CFG_GOLD
			if (g_debugSuperSpecialCutSceneVideoMode)
				levelState.mCars[car_index].mTextureDetail = INT_MAX;
			else
#endif
			{		
#ifndef CFG_GOLD
				if (g_debugForceVehicleTextureDetail > 0)
					levelState.mCars[car_index].mTextureDetail = g_debugForceVehicleTextureDetail;
				else
#endif
				if (levelState.mCars[car_index].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
					levelState.mCars[car_index].mTextureDetail = INT_MAX;
				else
					levelState.mCars[car_index].mTextureDetail = opponentSetup.detailOverride == 0 ? g_debugSPNonFocusDetail : opponentSetup.detailOverride;			
			}

			car_index++;
		}

		

		for( int i = 0; i < difficultySettings->aiPerkUse.count; i++ )
		{
			int opponentIdx = difficultySettings->aiPerkUse[i].opponentIdx;
			if( opponentIdx >= 0 && opponentIdx <= 20 )
			{
				levelState.mCars[opponentIdx].mGrantTimeMin		= difficultySettings->aiPerkUse[i].grantTimeMin;
				levelState.mCars[opponentIdx].mGrantTimeMax		= difficultySettings->aiPerkUse[i].grantTimeMax;
				levelState.mCars[opponentIdx].mPerkToGrantAI	= difficultySettings->aiPerkUse[i].perkToGrant;
			}
		}

		levelState.Finished();

		Game::C_RaceRulesManager::Instance().InitRulesFromEvent(levelState.mNumCars, levelResult, ev);
	}	

} // namespace RaceBook
