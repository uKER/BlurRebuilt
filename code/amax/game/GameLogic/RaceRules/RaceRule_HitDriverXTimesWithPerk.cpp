#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic\RaceRules\RaceRule_HitDriverXTimesWithPerk.h>
#include <ui/Game/UILocaliser.h>
#include "ui/UiXtConfig.h"

namespace Game
{

RaceModeRule_DriverHitDriverXTimesWithPerk::RaceModeRule_DriverHitDriverXTimesWithPerk() : mDriverID(0), mDriverOverrideID(0), mNumHits(0), mIsNitro(false)
{
	if( UI::C_UiXtConfig::Instance() )
	{
		mPulseFreq		= UI::C_UiXtConfig::Instance()->m_PulseDriverHitDriverXFreq;				// Pulse frequency when not earning fans
		mPulseWaitTime	= UI::C_UiXtConfig::Instance()->m_PulseDriverHitDriverXWaitTime;			// Time not earning fans until we start pulsing
		mPulseTimer		= mPulseWaitTime;
	}
}

bool RaceModeRule_DriverHitDriverXTimesWithPerk::IsTriggered(int vehicleIndex)
{
	C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	int vehIdx = vehicleIndex;

	if( !mDriverOverrideID )
	{
		for(unsigned int a=0;a<C_Game::GetInstance()->GetLevelState().mNumCars;a++)
		{
			const LevelStateCar& car = levelState.mCars[a];
			if(car.mCharacterID == mDriverOverrideID)
			{
				vehIdx = a;
				break;
			}
		}
	}

	bool triggered = false;

	const Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehIdx);

	if(!mDriverID)
	{
		// No specific target
		int numHits = 0;
		for( unsigned int i = 0; i < levelState.mNumCars; i++ )
		{
			if( !mPickupTypes.Size() )
			{
				if( mRequireDoubleHits )
					numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfTimesHitWithDoubleAggressivePerk;
				else
					numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfTimesHitWithAggressivePerk;
			}
			else
			{
				for( int j = 0; j < mPickupTypes.Size(); j++ )
				{
					switch(mPickupTypes[j])
					{
					case E_PICKUP_TYPE_SHOCK:
						{
							numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfDoubleShocksAgainstDriver;
							if(!mRequireDoubleHits)
								numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfShocksAgainstDriver;
							break;
						}

					case E_PICKUP_TYPE_SHUNT:
						{
							numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfDoubleShuntsAgainstDriver;
							if(!mRequireDoubleHits)
								numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfShuntsAgainstDriver;
							break;
						}

					case E_PICKUP_TYPE_BARGE:
						{
							numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfDoubleBargesAgainstDriver;
							if(!mRequireDoubleHits)
								numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfBargesAgainstDriver;
							break;
						}

					case E_PICKUP_TYPE_NITROUS:
						{
							numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumTimesOvertookWithDoubleNitro;
							if(!mRequireDoubleHits)
								numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumTimesOvertookWithNitro;
							break;
						}

					case E_PICKUP_TYPE_BOMB:
						{
							numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfDoubleBombsAgainstDriver;
							if(!mRequireDoubleHits)
								numHits += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfBombsAgainstDriver;
							break;
						}
					}
				}
			}
		}

		// Err whaaat? Only tracking player0 for this hackery
		if( vehIdx == 0 && mDriverHits[ 0 ] != numHits )
			mPulseTimer = mPulseWaitTime;

		mDriverHits[vehicleIndex] = numHits;

		triggered = ( numHits >= mNumHits );
	}
	else
	{
		// Specific target
		S32 i = -1;

		for( unsigned int j = 0; j < levelState.mNumCars; j++ )
		{
			if ( levelState.mCars[j].mCharacterID == mDriverID )
			{
				i = j;
				break;
			}
		}

		if ( i != -1 )
		{
			if( !mPickupTypes.Size() )
			{
				if( mRequireDoubleHits )
					triggered = ( pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfTimesHitWithDoubleAggressivePerk >= mNumHits );
				else
					triggered = ( pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfTimesHitWithAggressivePerk >= mNumHits );
			}
			else
			{
				triggered = true;

				for( int j = 0; j < mPickupTypes.Size(); j++ )
				{
					switch(mPickupTypes[j])
					{
					case E_PICKUP_TYPE_SHOCK:
						{
							mDriverHits[vehicleIndex] = pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfDoubleShocksAgainstDriver;

							if(!mRequireDoubleHits)
								mDriverHits[vehicleIndex] += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfShocksAgainstDriver;

							if( mDriverHits[vehicleIndex] < mNumHits )
								triggered = false;

							break;
						}


					case E_PICKUP_TYPE_SHUNT:
						{
							mDriverHits[vehicleIndex] = pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfDoubleShuntsAgainstDriver;

							if(!mRequireDoubleHits)
								mDriverHits[vehicleIndex] += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfShuntsAgainstDriver;

							if( mDriverHits[vehicleIndex] < mNumHits )
								triggered = false;

							break;
						}

					case E_PICKUP_TYPE_BARGE:
						{
							mDriverHits[vehicleIndex] = pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfDoubleBargesAgainstDriver;

							if(!mRequireDoubleHits)
								mDriverHits[vehicleIndex] += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfBargesAgainstDriver;

							if( mDriverHits[vehicleIndex] < mNumHits )
								triggered = false;

							break;
						}

					case E_PICKUP_TYPE_BOMB:
						{
							mDriverHits[vehicleIndex] = pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfDoubleBombsAgainstDriver;

							if(!mRequireDoubleHits)
								mDriverHits[vehicleIndex] += pLogicActor->mStatistics.mDualVehicleStats[i].mNumberOfBombsAgainstDriver;

							if( mDriverHits[vehicleIndex] < mNumHits )
								triggered = false;

							break;
						}

					case E_PICKUP_TYPE_NITROUS:
						{
							mDriverHits[vehicleIndex] = pLogicActor->mStatistics.mDualVehicleStats[i].mNumTimesOvertookWithDoubleNitro;

							if(!mRequireDoubleHits)
								mDriverHits[vehicleIndex] += pLogicActor->mStatistics.mDualVehicleStats[i].mNumTimesOvertookWithNitro;

							if( mDriverHits[vehicleIndex] < mNumHits )
								triggered = false;

							break;
						}
					}
				}
			}
		}
	}

	if(triggered)
		mDontShowOnHUD[vehicleIndex] = true;

	return triggered;
}

void RaceModeRule_DriverHitDriverXTimesWithPerk::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
{
	int hits = 0;

	switch(stringType)
	{
		/*
	case RACE_MODE_RULE_STRING_TYPE_RESULTS:
		{
			if( isPassRule )
			{
				if(mDriverID)
					str.Format(mRuleStr.Chars(), mDriverLocString.Chars(), mNumHits, mDoubleStr.Chars(), mPickupStr.Chars());
				else
					str.Format(mRuleStr.Chars(), mNumHits, mDoubleStr.Chars(), mPickupStr.Chars());
			}
			else
			{
				if(mDriverID)
					str.Format(mRuleStrNot.Chars(), mNumHits, mDoubleStr.Chars(), mPickupStr.Chars());
				else
					str.Format(mRuleStrNot.Chars(), mNumHits, mDoubleStr.Chars(), mPickupStr.Chars());
			}			
		}
		break;

	case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
		{
			if(mDriverID)
				str.Format(mRuleStrTarget.Chars(), mDriverLocString.Chars(), mNumHits, mDoubleStr.Chars(), mPickupStr.Chars());
			else
				str.Format(mRuleStrTarget.Chars(), mNumHits, mDoubleStr.Chars(), mPickupStr.Chars());
		}
		break;
		*/

	case RACE_MODE_RULE_STRING_TYPE_RESULTS:
	case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
		hits = mNumHits;
		break;

	case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
		{
			hits = mNumHits - mDriverHits[idx];

			if(hits < 0)
				hits = 0;
		}
		break;
	}

	bool bMore = (hits != mNumHits);
	bool bSingle = (hits == 1);

	static UI::C_TextStringID stringDouble("RR_HitWithPerks_Double_HUD_Current");
	static UI::C_TextStringID stringDoubleS("RR_HitWithPerks_Double_HUD_Current_S");
	static UI::C_TextStringID string("RR_HitWithPerks_HUD_Current");
	static UI::C_TextStringID stringS("RR_HitWithPerks_HUD_Current_S");
	static UI::C_TextStringID stringNoDriver("RR_HitWithPerks_NoDriver_HUD_Current");
	static UI::C_TextStringID stringNoDriverS("RR_HitWithPerks_NoDriver_HUD_Current_S");
	static UI::C_TextStringID stringDoubleNoDriver("RR_HitWithPerks_Double_NoDriver_HUD_Current");
	static UI::C_TextStringID stringDoubleNoDriverS("RR_HitWithPerks_Double_NoDriver_HUD_Current_S");

	static UI::C_TextStringID stringDoubleM("RR_HitWithPerks_Double_HUD_Current_M");
	static UI::C_TextStringID stringDoubleMS("RR_HitWithPerks_Double_HUD_Current_M_S");
	static UI::C_TextStringID stringM("RR_HitWithPerks_HUD_Current_M");
	static UI::C_TextStringID stringMS("RR_HitWithPerks_HUD_Current_M_S");
	static UI::C_TextStringID stringNoDriverM("RR_HitWithPerks_NoDriver_HUD_Current_M");
	static UI::C_TextStringID stringNoDriverMS("RR_HitWithPerks_NoDriver_HUD_Current_M_S");
	static UI::C_TextStringID stringDoubleNoDriverM("RR_HitWithPerks_Double_NoDriver_HUD_Current_M");
	static UI::C_TextStringID stringDoubleNoDriverMS("RR_HitWithPerks_Double_NoDriver_HUD_Current_M_S");
		
	if(mDriverID == 0)
	{
		UI::C_TextStringID& id = 
			mRequireDoubleHits 
				? ( bMore 
					? ( bSingle ? stringDoubleNoDriverMS : stringDoubleNoDriverM ) 
					: ( bSingle ? stringDoubleNoDriverS  : stringDoubleNoDriver ) 
				)
				: ( bMore 
					? ( bSingle ? stringNoDriverMS : stringNoDriverM ) 
					: ( bSingle ? stringNoDriverS  : stringNoDriver ) 
				)
		;

		str.Format(id.String(), mPickupId.String(), hits);
	}
	else
	{
		UI::C_TextStringID& id = 
			mRequireDoubleHits 
				? ( bMore 
					? ( bSingle ? stringDoubleMS : stringDoubleM ) 
					: ( bSingle ? stringDoubleS  : stringDouble ) 
				)
				: ( bMore 
					? ( bSingle ? stringMS : stringM ) 
					: ( bSingle ? stringS  : string ) 
				)
		;

		str.Format(id.String(), mPickupId.String(), mDriverLocString.Chars(), hits);
	}
}

void RaceModeRule_DriverHitDriverXTimesWithPerk::CacheStrings()
{
	mDriverLocString.Format(L"%s", UI::C_TextStringID(mDriverLocId.id).String());

	if(mPickupTypes.Size() == 1)
	{
		switch(mPickupTypes[0])
		{
		case E_PICKUP_TYPE_SHOCK:
			//mPickupStr.Add(UI::C_TextStringID("CMN_SHOCK_C").String());
			mPickupId = UI::C_TextStringID("CMN_SHOCK_C");
			break;

		case E_PICKUP_TYPE_SHUNT:
			//mPickupStr.Add(UI::C_TextStringID("CMN_SHUNT_C").String());
			mPickupId = UI::C_TextStringID("CMN_SHUNT_C");
			break;

		case E_PICKUP_TYPE_BARGE:
			//mPickupStr.Add(UI::C_TextStringID("CMN_BARGE_C").String());
			mPickupId = UI::C_TextStringID("CMN_BARGE_C");
			break;

		case E_PICKUP_TYPE_BOMB:
			//mPickupStr.Add(UI::C_TextStringID("CMN_BOMB_C").String());
			mPickupId = UI::C_TextStringID("CMN_BOMB_C");
			break;

		case E_PICKUP_TYPE_NITROUS:
			//mPickupStr.Add(UI::C_TextStringID("CMN_NITRO_OVERTAKE_C").String());
			mPickupId = UI::C_TextStringID("CMN_NITRO_OVERTAKE_C");
			break;
		}
	} 

	/*
	if(mRequireDoubleHits)
	{
		UI::C_TextStringID doubleStr("RR_HitWithPerks_Double");
		mDoubleStr.Format(L"%s", doubleStr.String());
	}
	*/

	/*
	if( mPickupTypes.Size() == 1 )
	{
		if( mDriverID )
		{
			if( mPickupTypes[0] == E_PICKUP_TYPE_NITROUS )
			{
				mRuleStr		= UI::C_TextStringID("RR_HitWithPerks_Nitro").String();
				mRuleStrNot		= UI::C_TextStringID("RR_HitWithPerks_Nitro_Not").String();
				mRuleStrCurrent = UI::C_TextStringID("RR_HitWithPerks_Nitro_HUD_Current").String();
				mRuleStrTarget	= UI::C_TextStringID("RR_HitWithPerks_Nitro_HUD_Target").String();
				mIsNitro		= true;
			}
			else
			{
				mRuleStr		= UI::C_TextStringID("RR_HitWithPerks").String();
				mRuleStrNot		= UI::C_TextStringID("RR_HitWithPerks_Not").String();
				mRuleStrCurrent = UI::C_TextStringID("RR_HitWithPerks_HUD_Current").String();
				mRuleStrTarget	= UI::C_TextStringID("RR_HitWithPerks_HUD_Target").String();
			}
		}
		else
		{
			if( mPickupTypes[0] == E_PICKUP_TYPE_NITROUS )
			{
				mRuleStr		= UI::C_TextStringID("RR_HitWithPerks_Nitro_NoDriver").String();
				mRuleStrNot		= UI::C_TextStringID("RR_HitWithPerks_Nitro_NoDriver_Not").String();
				mRuleStrCurrent = UI::C_TextStringID("RR_HitWithPerks_Nitro_NoDriver_HUD_Current").String();
				mRuleStrTarget	= UI::C_TextStringID("RR_HitWithPerks_Nitro_NoDriver_HUD_Target").String();
				mIsNitro		= true;
			}
			else
			{
				mRuleStr		= UI::C_TextStringID("RR_HitWithPerks_NoDriver").String();
				mRuleStrNot		= UI::C_TextStringID("RR_HitWithPerks_NoDriver_Not").String();
				mRuleStrCurrent = UI::C_TextStringID("RR_HitWithPerks_NoDriver_HUD_Current").String();
				mRuleStrTarget	= UI::C_TextStringID("RR_HitWithPerks_NoDriver_HUD_Target").String();
			}
		}
	}
	*/
}

void RaceModeRule_DriverHitDriverXTimesWithPerk::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );
	out << mDriverID << mDriverOverrideID << mDriverLocId.id << mNumHits;
	RaceModeRuleBase::Serialise(stream);
}

void RaceModeRule_DriverHitDriverXTimesWithPerk::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );
	in >> mDriverID >> mDriverOverrideID >> mDriverLocId.id >> mNumHits;
	RaceModeRuleBase::Deserialise(stream);
}

} // namespace Game