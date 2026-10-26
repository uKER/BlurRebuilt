#include "stdafx.h"
#include "MultiplayerUnlockHistory.h"
#include "system\serialise\BinarySerialiseAutoEndian.h"
#include "GameData\MultiplayerData.h"
#include "GameData\GameVehicleData.h"

namespace Game
{

C_MultiplayerUnlockHistory::C_MultiplayerUnlockHistory()
{
	
}


void C_MultiplayerUnlockHistory::Serialise( Sys::BinarySerialiserAutoEndian& stream ) const
{
	Sys::BinarySerialiserAutoEndian out( stream );

	// Serialise Car Unlock Recent status
	out << mCars.Size();
	for( CarsMapType::ConstIterator carIter = mCars.Begin(); carIter != mCars.End(); carIter++ )
	{
		out << carIter->GetKey() << (U8)carIter->GetValue();
	}

	// Serialise Car Unlock Recent status
	out << mCarChallenges.Size();
	for( CarsMapType::ConstIterator carIter = mCarChallenges.Begin(); carIter != mCarChallenges.End(); carIter++ )
	{
		out << carIter->GetKey() << (U8)carIter->GetValue();
	}

	// Serialise Challenge Pack Unlock Recent status
	out << mChallengePacks.Size();
	for( ChallengePacksContainerType::ConstIterator packIter = mChallengePacks.Begin(); packIter != mChallengePacks.End(); packIter++ )
	{
		out << (U8)(*packIter);
	}

	// Serialise Ability Unlock Recent Status
	out << mAbilities.Size();
	for( AbilitiesContainerType::ConstIterator abilityIter = mAbilities.Begin(); abilityIter != mAbilities.End(); abilityIter++ )
	{
		out << (U8)(*abilityIter);
	}

	// Serialise Features Unlock Recent Status
	out << mFeatures.Size();
	for( FeaturesContainerType::ConstIterator featuresIter = mFeatures.Begin(); featuresIter != mFeatures.End(); featuresIter++ )
	{
		out << (U8)(*featuresIter);
	}

	// Serialise Events Unlock Recent Status
	out << mEvents.Size();
	for( EventsContainerType::ConstIterator eventsIter = mEvents.Begin(); eventsIter != mEvents.End(); eventsIter++ )
	{
		out << (U32)(*eventsIter);
	}

}

void C_MultiplayerUnlockHistory::Deserialise(const Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );

	// Deserialise Car Unlock Recent status
	mCars.Clear();

	int sizeOfCarArray = 0;
	XtId carKey = 0;
	U8 carValue = false;
	in >> sizeOfCarArray;

	for(int i = 0; i < sizeOfCarArray; i++)
	{
		in >> carKey;
		in >> carValue;
		mCars.Insert(carKey,carValue != 0 );
	}

	mCarChallenges.Clear();

	int sizeOfCarChallengesArray = 0;
	XtId carChallengeKey = 0;
	U8 carChallengeValue = false;
	in >> sizeOfCarChallengesArray;

	for(int i = 0; i < sizeOfCarChallengesArray; i++)
	{
		in >> carChallengeKey;
		in >> carChallengeValue;
		mCarChallenges.Insert(carChallengeKey,carValue != 0 );
	}


	// Deserialise Challenge Pack Unlock Recent status

	int sizeOfPackArray = 0;
	U8 packValue = false;
	in >> sizeOfPackArray;

	for(int i = 0; i < sizeOfPackArray; i++)
	{
		in >> packValue;
		mChallengePacks[i]=packValue!=0;
	}

	// Deserialise Ability Unlock Recent Status

	int sizeOfAbilityArray = 0;
	U8  abilityValue = false;
	in >> sizeOfAbilityArray;

	for(int i = 0; i < sizeOfAbilityArray; i++)
	{
		in >> abilityValue;
		mAbilities[i]=abilityValue!=0;		
	}

	// Deserialise Features Unlock Recent Status

	int sizeOfFeatureArray = 0;
	U8  featureValue = false;
	in >> sizeOfFeatureArray;

	for(int i = 0; i < sizeOfFeatureArray; i++)
	{
		in >> featureValue;
		mFeatures[i]=featureValue!=0;		
	}

	// Deserialise Events Unlock Recent Status


	mEvents.Clear();

	int sizeOfEventArray = 0;
	U32  eventValue = false;
	in >> sizeOfEventArray;

	for(int i = 0; i < sizeOfEventArray; i++)
	{
		in >> eventValue;

		if (GetEventValid(eventValue))
		{
			mEvents.PushBack(eventValue);
		}
	}
}

bool C_MultiplayerUnlockHistory::GetEventValid(XtId eventId) const
{

	// Iterate through all the events and cache the valid events
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_OnlineEvents.count; i++ )
	{
		XtHandle< MultiplayerEvent > pEvent = C_MultiplayerData::Instance().m_pConfig->m_OnlineEvents[i];
		BIZ_ASSERT_MSG_SPEED( pEvent, "NULL event in multiplayer config event(%i)", i );

		if ( pEvent->ID==eventId )
			return true;
	}

	return false;
}

void C_MultiplayerUnlockHistory::Init(const MultiplayerConfig* config)
{
	BIZ_ASSERT_MSG(config!=0, "C_MultiplayerUnlockHistory::Init");

	// Clear existing unlock history
	if(!mCars.Empty())
	{
		mCars.Clear();	
	}

	if(!mCarChallenges.Empty())
	{
		mCarChallenges.Clear();	
	}

	mEvents.Clear();

	// Init new unlock history
	InitCars(config);
	InitChallengePacks();
	InitAbilities();
	InitFeatures();
}

void C_MultiplayerUnlockHistory::InitCars(const MultiplayerConfig* config)
{
	int levelingDataCount = config->m_LevelingData.count;

	// For each car that the player can unlock, add an entry (by sorcery ID) and default it to false (i.e. not recently unlocked)
	for(int i=0; i<levelingDataCount; i++)
	{
		for(int j=0;j<config->m_LevelingData[i].mVehicleUnlocks.count;j++)
		{
			const AmaxVehicle* pVehicle = config->m_LevelingData[i].mVehicleUnlocks[j].Object();
			mCars.Insert(pVehicle->ID,false);
			mCarChallenges.Insert(pVehicle->ID,false);
		}
	}

}

void C_MultiplayerUnlockHistory::InitChallengePacks()
{
	for ( S32 i = 0; i < UI::E_ChallengePack_Count; i++ )
	{
		mChallengePacks[i]=false;
	}
}

void C_MultiplayerUnlockHistory::InitAbilities()
{
	// For each ability that the player can unlock, add an entry (by ability enum) and default it to false (i.e. not recently unlocked)
	for(int i=UI::E_MultiplayerVehicleAbilities_None+1; i < UI::E_MultiplayerVehicleAbilities_NumberOfAbilities; i++)
	{
		mAbilities[i]=0;
	}

}

void C_MultiplayerUnlockHistory::SetCarUnlocked(XtId carID,const bool val)
{
	Btl::Map<XtId, bool>::Iterator it = mCars.Find(carID);

	BIZ_ASSERT_MSG(it != mCars.End(),"SetCarUnlocked(XtId carID,const bool val) < carID invalid!" );

	if(it != mCars.End())
	{
		it->GetValue() =  val; 
	}
}

void C_MultiplayerUnlockHistory::SetCarChallengeUnlocked(XtId carID,const bool val)
{
	Btl::Map<XtId, bool>::Iterator it = mCarChallenges.Find(carID);

	BIZ_ASSERT_MSG(it != mCars.End(),"SetCarUnlocked(XtId carID,const bool val) < carID invalid!" );

	if(it != mCars.End())
	{
		it->GetValue() =  val; 
	}
}

void C_MultiplayerUnlockHistory::SetChallengePackUnlocked(const UI::E_ChallengePack ePackCategory ,const bool val)
{
	mChallengePacks[ePackCategory]=val;
}

void C_MultiplayerUnlockHistory::SetAbilityUnlocked(const UI::E_MultiplayerVehicleAbilities eAbilityType ,const bool val)
{
	mAbilities[eAbilityType]=val;
}

const C_MultiplayerUnlockHistory::CarsMapType& C_MultiplayerUnlockHistory::GetCars() const
{
	return mCars;
}

const C_MultiplayerUnlockHistory::CarsMapType& C_MultiplayerUnlockHistory::GetCarChallenges() const
{
	return mCarChallenges;
}

const C_MultiplayerUnlockHistory::AbilitiesContainerType& C_MultiplayerUnlockHistory::GetAbilities() const
{
	return mAbilities;
}

const C_MultiplayerUnlockHistory::ChallengePacksContainerType& C_MultiplayerUnlockHistory::GetChallengePacks() const
{
	return mChallengePacks;
}

void C_MultiplayerUnlockHistory::FlagCars(const MultiplayerLevel& level)
{
	for(int j=0; j < level.mVehicleUnlocks.count;j++)
	{
		const AmaxVehicle* pVehicle = level.mVehicleUnlocks[j].Object();

		// Find this vehicle and flag it as recently unlocked
		SetCarUnlocked(pVehicle->ID,true);

		if(pVehicle->m_ParentVehicle)
		{
			SetCarChallengeUnlocked(pVehicle->ID,false); //No challenges for boss cars
		}
	}
}

void C_MultiplayerUnlockHistory::FlagChallengePacks(const MultiplayerLevel& level)
{
	// For each newly unlocked challenge pack....
	for( int pack = 0; pack < level.mChallengeUnlocks.count; pack++ )
	{
		const MultiplayerChallengePack* pChallengePack = level.mChallengeUnlocks[pack].Object();
		BIZ_ASSERT_MSG(pChallengePack,"FlagNewlyUnlockedChallengePacks(const int startLevel, int endLevel) < A Multiplayer Challenge Pack is set to NULL. This should be fixed!");

		//  Flag the pack..as recently unlocked
		SetChallengePackUnlocked(pChallengePack->mType,true); 
	}
}

void C_MultiplayerUnlockHistory::FlagAbilities(const MultiplayerLevel& level)
{
	for(int j=0; j < level.mAbilityUnlocks.count ;j++)
	{
		const MultiplayerAbilityData* pAbility = level.mAbilityUnlocks[j];

		BIZ_ASSERT_MSG(pAbility,"FlagNewlyUnlockedAbilities(const int startLevel, int endLevel) < A Multiplayer Ability is set to NULL. This should be fixed!");
		if (pAbility)
		{
			SetAbilityUnlocked(pAbility->mAbilityId,true);
		}
	}
}

void C_MultiplayerUnlockHistory::FlagNewlyUnlockedItems(const MultiplayerConfig* config, const int startLevel, int endLevel)
{
	BIZ_ASSERT_MSG(config!=0, "C_MultiplayerUnlockHistory::FlagNewlyUnlockedVehicles");

	TLevelData levelData = config->m_LevelingData;
	int levelingDataCount = levelData.count;

	// clamp - just in case.
	endLevel = levelingDataCount>endLevel?endLevel:(levelingDataCount-1);

	for(int i=startLevel; i<=endLevel; i++)
	{
		MultiplayerLevel& level=levelData[i];
		FlagCars(level);
		FlagChallengePacks(level);
		FlagAbilities(level);
		FlagFeatures(level);
		FlagEvents(*config, i);
	}
}

void C_MultiplayerUnlockHistory::FlagFeatures(const MultiplayerLevel& level)
{
	for(int j=0; j < level.mFeatureUnlocks.count ;j++)
	{
		SetFeatureUnlocked(level.mFeatureUnlocks[j]->mFeatureId,true);
	}
}

void C_MultiplayerUnlockHistory::SetFeatureUnlocked(const UI::E_MultiplayerFeature eFeatureType ,const bool val)
{
	BIZ_ASSERT_MSG(eFeatureType<UI::E_MultiplayerFeature_Count, "Invalid Feature Id.");
	mFeatures[eFeatureType]=val;
}


void C_MultiplayerUnlockHistory::InitFeatures()
{
	for(int i=0; i<UI::E_MultiplayerFeature_Count; i++)
	{
		SetFeatureUnlocked((UI::E_MultiplayerFeature)i, false);
	}
}

const C_MultiplayerUnlockHistory::FeaturesContainerType& C_MultiplayerUnlockHistory::GetFeatures() const
{
	return mFeatures;
}

const C_MultiplayerUnlockHistory::EventsContainerType& C_MultiplayerUnlockHistory::GetEvents() const
{
	return mEvents;
}

void C_MultiplayerUnlockHistory::FlagEvents(const MultiplayerConfig& config, int rank)
{
	const XtArray< XtHandle<MultiplayerEvent > >& onlineEvents=config.m_OnlineEvents;
	
	for(int i=0; i<onlineEvents.count; i++)
	{
		const MultiplayerEvent& event=*(onlineEvents[i]);
		if (event.min_rank==rank)
		{
			SetEventUnlocked(event.ID, true);
		}
	}
}

void C_MultiplayerUnlockHistory::SetEventUnlocked(XtId event ,const bool val)
{
	for(int i=0; i<mEvents.Size(); i++)
	{
		if (mEvents[i]==event)
		{
			if (!val)
			{
				mEvents.EraseByIndex(i);
			}

			return ;
		}
	}

	if (val)
	{
		mEvents.PushBack(event);
	}
}

bool C_MultiplayerUnlockHistory::GetEventUnlocked(XtId event) const
{
	for(int i=0; i<mEvents.Size(); i++)
	{
		if (mEvents[i]==event)
		{
			return true;
		}
	}

	return false;
}


}; // namespace Game

