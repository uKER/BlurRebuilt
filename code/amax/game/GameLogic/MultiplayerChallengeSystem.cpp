
#include "stdafx.h"
#include "GameLogic/MultiplayerChallengeSystem.h"

#include <system/serialise/BinarySerialiseAutoEndian.h>

#include "GameLogic/GameData/MultiplayerData.h"
#include "GameLogic/GameData/GameVehicleData.h"
#include <UI/UIMessages.h>
#include <ui/Game/UILocaliser.h>

namespace Game
{

Debug::Logger &g_MultiplayerLog(Debug::GetLogger("Amax.Multiplayer"));
Debug::Logger& g_MultiplayerUnlocksLog = Debug::GetLogger("Amax.Multiplayer.Unlocks");
Debug::Bool g_ForceNotifyAllChallengeProgress("Amax.VLC.MultiplayerChallenges.ForceNotifyAllChallengeProgress", false );


C_MultiplayerChallengeSystem::C_MultiplayerChallengeSystem()
:	m_PackViewingType( UI::E_ChallengePack_Count )
{
	for ( S32 i = 0; i < UI::E_ChallengePack_Count; i++ )									
		m_PackCounterCache[i] = 0;
}

void C_MultiplayerChallengeSystem::Initialise()
{
	// Just incase we manage to get this far and sorcery hasnt been loaded
	if ( !C_MultiplayerData::Instance().m_pConfig )
		return;

	m_CarMap.Clear();

	S32 carPackIndex = -1;
	m_LastChallengeCompleted = UI::E_MULTIPLAYER_CHALLENGE_TYPE_NONE;
	m_LastChallengeCompletedLevel = 0;
	m_LastChallengeCarId = 0;

	// For each pack
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++ )
	{
		MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];
		BIZ_ASSERT_MSG_SPEED( pPack, "Invalid challenge pack in config (NULL)" );

		S32 packUnlockLevel=GetPackUnlockLevel(pPack);

		// Ignore the car challenges
		if ( pPack->mType == UI::E_ChallengePack_Car )
			carPackIndex = i;

		// For each challenge
		for ( S32 j = 0; j < pPack->mChallenges.count; j++ )
		{
			BIZ_ASSERT_MSG_SPEED( pPack->mChallenges[j].Object(), "Invalid challenge(%i) in pack %i(NULL)", j, i );
			BIZ_ASSERT_MSG_SPEED( pPack->mChallenges[j]->mTiers.count, "Challenge(%i) in pack %i has no valid tiers", j, i );

			m_Challenges[pPack->mChallenges[j]->mType] = C_MultiplayerChallenge( pPack->mChallenges[j], packUnlockLevel, pPack->mType );
		}
	}

	// Build up the car challenges
	if ( carPackIndex != -1 )
	{
		MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[carPackIndex];
		BIZ_ASSERT(pPack!=0);
		InitCarChallenges(*pPack);
	}

// Enable this if you want to catch any challenge data that is not coming through from sorcery.
#ifndef CFG_GOLD
	// Validate the challenge data.
	for ( S32 i = 0; i < m_Challenges.Size(); i++ )
	{
		// Upgrading this to an assert since everyone is ignoring the debug output!
		BIZ_ASSERT_MSG_SPEED( m_Challenges[i].GetType() != UI::E_MULTIPLAYER_CHALLENGE_TYPE_NONE, "C_MultiplayerChallengeSystem::Initialise(): challenge %i sorcery data missing.", i );
	}
#endif

	// Cache the data we use to govern wether or not we display a UI prompt when incrementing challenge progress
	MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
	m_MinChallengeSize		   = pConfig->m_minChallengeSize;	
	m_IntroZone				   = pConfig->m_introZone;			
	m_OutroZone				   = pConfig->m_outroZone;		
	m_PercentInterval		   = pConfig->m_percentInterval;	
}

S32 C_MultiplayerChallengeSystem::GetPackUnlockLevel(const MultiplayerChallengePack* pPack) const
{
	XtArray< MultiplayerLevel >& levellingData=C_MultiplayerData::Instance().m_pConfig->m_LevelingData;

	for(int i=0; i<levellingData.count; i++)
	{
		XtArray< XtHandle<MultiplayerChallengePack> >& challengeUnlocks=levellingData[i].mChallengeUnlocks;

		for(int j=0; j<challengeUnlocks.count; j++)
		{
			if (challengeUnlocks[j]==pPack)
				return i;
		}
	}

	LOGERROR(g_MultiplayerUnlocksLog, "C_MultiplayerChallengeSystem::GetPackUnlockLevel: Pack is never unlocked in the levelling data. Either delete the pack or associate it with a level. The pack is going to be unlocked in default.");

	return 0;
}


void C_MultiplayerChallengeSystem::Reset()
{
	// Reset "normal" challenges
	for(int i=0; i<UI::E_MULTIPLAYER_CHALLENGE_TYPE_MAX; i++)
	{
		m_Challenges[i].Reset();
	}

	// Reset our per-vehicle challenges
	for(Btl::Map<U32, C_MultiplayerCarChallenges>::Iterator it = m_CarMap.Begin(); it != m_CarMap.End(); it++ )
	{
		it->GetValue().Reset();
	}

	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++ )
	{
		MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];
		if(pPack->mType == UI::E_ChallengePack_Car)
		{
			InitCarChallenges(*pPack);
		}
	}
}

void C_MultiplayerChallengeSystem::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	// Dont need to write the size out as its a fixed size array.
	for ( S32 i = 0; i < m_Challenges.Size(); i++ )
	{
		stream << m_Challenges[i];
	}

	// Not included in the above are per-vehicle challenges for which we DO need to write out the size
	stream << m_CarMap.Size();

	for( Btl::Map<U32, C_MultiplayerCarChallenges>::Iterator it = m_CarMap.Begin(); it != m_CarMap.End(); it++ )
	{
		stream << it->GetKey() << it->GetValue();
	}

}

void C_MultiplayerChallengeSystem::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	// Dont need to read the size in as its a fixed size array.
	for ( S32 i = 0; i < m_Challenges.Size(); i++ )
	{
		stream >> m_Challenges[i];
	}

	int numChallengeVehicles;
	stream >> numChallengeVehicles;

	U32 key;
	for(int i=0; i < numChallengeVehicles; i++ )
	{
		stream >> key;
		m_CarMap.Find( key )->GetValue().Deserialise( stream );
	}
}

const MultiplayerChallengePack* C_MultiplayerChallengeSystem::GetViewingPack() const
{
	// Return the challenge pack. We could cache the index but sorcery can reload at runtime.
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++ )
	{
		if ( C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i]->mType == m_PackViewingType )
		{
			return C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];
		}
	}
	return NULL;
}


void C_MultiplayerChallengeSystem::CachePackProgress()
{
	// For each pack
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++ )
	{
		MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];

		m_PackCounterCache[pPack->mType] = 0; // reset to zero

		// For each challenge
		for ( S32 j = 0; j < pPack->mChallenges.count; j++ )
		{
			if ( m_Challenges[pPack->mChallenges[j]->mType].IsCompleted() )
				m_PackCounterCache[pPack->mType]++;
		}	
	}

	// Build up the car challenges data
	for ( S32 i = 0; i < UI::E_VehicleClass_Max; i++ )
	{
		m_ClassCounterCache[i] = 0; // clear them
		m_ClassTotalCache[i] = 0;
	}
	m_CarPackCounter = 0;
	m_CarPackTotal = 0;

	for ( S32 level = 0; level < C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count; level++ )
	{
		MultiplayerLevel& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[level];

		// For each car
		for ( S32 i = 0; i < levelData.mVehicleUnlocks.count; i++ )
		{
			if(!levelData.mVehicleUnlocks[i]->m_ParentVehicle) //Ignore boss cars
			{
				CacheCarProgress( levelData.mVehicleUnlocks[i] );
			}
		}
	}
}


void C_MultiplayerChallengeSystem::CacheCarProgress( const AmaxVehicle* pCar )
{
	if ( GetCarChallenge( pCar->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES ).IsCompleted() )
	{
		m_ClassCounterCache[pCar->carClass]++;
		m_CarPackCounter++;
	}

	if ( GetCarChallenge( pCar->ID, UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER ).IsCompleted() )
	{
		m_ClassCounterCache[pCar->carClass]++;
		m_CarPackCounter++;
	}

	// increase totals
	m_ClassTotalCache[pCar->carClass] = m_ClassTotalCache[pCar->carClass] +2;
	m_CarPackTotal = m_CarPackTotal + 2;
}


C_MultiplayerChallenge& C_MultiplayerChallengeSystem::GetCarChallenge( const U32 uCarID, const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType )
{
	return m_CarMap[uCarID].GetChallenge(eType);
}

const Btl::BitArray16& C_MultiplayerChallengeSystem::GetCarUpgradesEnabled( const U32 uCarID ) 
{
	return m_CarMap[uCarID].GetUpgradesEnabled();
}

void C_MultiplayerChallengeSystem::ForceCarUpgrades(const U32 uCarID)
{
	Btl::BitArray16 newUpgrades;
	newUpgrades.SetBit(UI::E_MultiplayerVehicleUpgrades_HealthSlot);
	newUpgrades.SetBit(UI::E_MultiplayerVehicleUpgrades_FanBonus);
	newUpgrades.SetBit(UI::E_MultiplayerVehicleUpgrades_SpeedBoost);

	m_CarMap[uCarID].SetUpgradesEnabled( newUpgrades );
}


void C_MultiplayerChallengeSystem::IncreaseCarChallengeAmount( const U32 uCarID,  const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType, const S32 sAmount )
{
	C_MultiplayerChallenge& challenge = m_CarMap[uCarID].GetChallenge(eType);
	if ( !challenge.IsCompleted() )
	{
		Btl::BitArray16 newUpgrades;
		newUpgrades.ClearBits();

		S32 tier = m_Challenges[eType].GetTier(); 
		S32 tiersCompleted = challenge.IncreaseAmount( sAmount );

		// If we just completed a tier
		for( S32 i = 0; i < tiersCompleted; i++ )
		{
			m_Tracker.HandleCompletedChallenge( challenge, tier);
			m_LastChallengeCompleted = eType;
			m_LastChallengeCompletedLevel = tier + i;
			m_LastChallengeCarId = uCarID;
		}

		int lastTierCompleted = challenge.IsCompleted() ? challenge.GetTier() : challenge.GetTier() - 1;
		for(int i=0; i<tiersCompleted; i++)
		{
			newUpgrades.SetBit( challenge.GetReward(lastTierCompleted));
			lastTierCompleted--;
		}
		
		m_CarMap[uCarID].SetUpgradesEnabled( newUpgrades );
	}
}

void C_MultiplayerChallengeSystem::IncreaseChallengeAmount( const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType, const S32 sAmount)
{
	C_MultiplayerChallenge& challenge = m_Challenges[eType];
	if ( !challenge.IsCompleted() )
	{
		float oldProgress = challenge.GetAmount(); 

		S32 tier = challenge.GetTier(); 
		S32 tiersComplete = challenge.IncreaseAmount( sAmount );

		// If we just completed a tier
		// 	[JBunner]BUG: 37523: Do this for Each tier completed
		for ( S32 i = 0; i < tiersComplete; i++ )
		{
			S8 tierComplete = tier + i;
			m_Tracker.HandleCompletedChallenge( challenge, tierComplete);
			m_LastChallengeCompleted = eType;
			m_LastChallengeCompletedLevel = tier + i;
		}

		if(tiersComplete <= 0)
		{
			// Send off a UI message incase we need to show a progress update
			CheckForUIChallengeProgress(eType,oldProgress);
		}
	}
}

void C_MultiplayerChallengeSystem::CheckForUIChallengeProgress(const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType, const float oldProgress)
{
	Game::C_MultiplayerChallenge& challenge  = m_Challenges[eType]; 

	// Do we show progress for this challenge? (dont show anything for end of race challenges etc)
	if(!ChallengeProgressVisible(eType))
	{
		return;
	}

	float goal = challenge.GetTargetAmount(); 
	float progress = challenge.GetAmount(); 

	bool notify = false;
	if (challenge.HideProgress() || (goal < m_MinChallengeSize ) || g_ForceNotifyAllChallengeProgress )
	{
		// Always notify when less than m_MinChallengeSize notches

		notify = true;
	}
	else
	{
		float delta = goal  - progress;

		if ( (progress <= m_IntroZone) || (delta < m_OutroZone))
		{
			// Notify when progressing into first X or final X progress of a challenge

			notify = true;
		}
		else
		{
			// TODO: Rather than notifying every X%, calculate a rounded number near that X% and notify using that instead.
			if(goal != 0 && m_PercentInterval != 0)
			{
				// Take last and current progress vals... and roughly 'bucket' them into the nearest multiple of X%
				float val = 1.0f /(float)m_PercentInterval; // e.g. 20% == 0.05
				val *= 100.0f;								// e.g. 0.05 == 5

				int iOld = (int)((oldProgress * (int)val) / goal); // use this to work out last multiple
				int iNew = (int)((progress * (int)val) / goal);    // use this to work out current multiple

				notify = iOld != iNew;					   // if not same... we've hit a new % multiple
			}
		}
	}

	// TODO: logic about when to display progress towards stickers (e.g. in 1/5ths)
	if (notify)
	{
		ShowUIChallengeProgress(eType);
	}
}

bool C_MultiplayerChallengeSystem::ChallengeProgressVisible( const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType )
{
	// we dont show challenge progress for many challenges (e.g. post race challenges etc).. yet... 
	// May want to store and delay these till race results screen?


	// TODO: Move this check to a sorcery flag set on each challenge? (although this list isnt going to change....)
	switch(eType)
	{
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_PARTICIPANT: // deliberate fall through
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_HARD_DAYS_WORK:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_HAT_TRICK:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_PACIFIST:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_ULTIMATE_COMEBACK:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_SHOW_OFF:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_CARNAGE:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAR_CRAZY:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_PIED_PIPER:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_MINE_SPECIALIST:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_BARGE_SPECIALIST:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_AFTERBURNER:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_MASOCHIST:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_THICK_SKIN:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_HEAD_IN_THE_CLOUDS:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_DISTANCE:
	case UI::E_MULTIPLAYER_CHALLENGE_TYPE_BACKWARDS_THINKING:
		{
			return false;	// dont show these.. POST RACE CHALLENGES!
		}
		break;
	default:
			return true;
		break;
	}


	return true;

}

void C_MultiplayerChallengeSystem::ShowUIChallengeProgress( const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType )
{
	UI::C_WideString128 strName;
	UI::C_WideString128 strDesc;

	strName.Clear();
	strDesc.Clear();

	Game::C_MultiplayerChallenge& challenge  = m_Challenges[eType]; 

	strName.Format( UI::C_TextStringID( challenge.GetName().id ).String() );
	strDesc.Format( UI::C_TextStringID( challenge.GetDesc().id ).String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas( challenge.GetTargetAmount( challenge.GetTier() ) ), challenge.GetData( challenge.GetTier() ) );

	UI::C_UiProgressMessage uiMsg( strName, strDesc, challenge.GetAmount(), challenge.GetTargetAmount(), 0, challenge.HideProgress(), challenge.GetPack() );

	_UI_MESSAGE_FULL( uiMsg );
}

bool C_MultiplayerChallengeSystem::IsPackComplete( const UI::E_ChallengePack ePack ) const
{
	F32 progress = 0.0f;
	if(ePack == UI::E_ChallengePack_Car)
	{
		progress = (F32)GetCarPackCounter(); (F32)GetCarPackTotal();
	}
	else
	{
		progress = (F32)(GetPackProgressCounter(ePack) / C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[ePack].Object()->mChallenges.count);
	}

	return progress >= 1.0f;
}

// TODO_MP_TS - remove me post beta and fix up the multiplayer packs
void C_MultiplayerChallengeSystem::ReloadSorceryData()
{
	// Just incase we manage to get this far and sorcery hasnt been loaded
	if ( !C_MultiplayerData::Instance().m_pConfig )
		return;

	S32 carPackIndex = -1;

	carPackIndex = ReloadChallengesSorcery();

	if (carPackIndex != -1)
	{
		InvalidateCarChallenges();

		// Build up the car challenges
		MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[carPackIndex];
		BIZ_ASSERT(pPack!=0);

		InitCarChallenges(*pPack);

		RemoveInvalidCarChallenges();
	}
}

C_MultiplayerCarChallenges* C_MultiplayerChallengeSystem::ReloadCarChallengesSorcery( XtId vehicleId, MultiplayerChallengePack& pack)
{
	C_MultiplayerCarChallenges* pChallenges=0;
	if (m_CarMap.Find(vehicleId)!=m_CarMap.End())
	{
		pChallenges=&m_CarMap[vehicleId];
		const S32 level = C_MultiplayerData::Instance().m_MultiplayerVehicleMap[vehicleId];
		pChallenges->ReloadSorceryData( pack.mChallenges, level );
		pChallenges->SetValid(true);
	}
	else
	{
		AddCarChallenge(vehicleId, pack);
		pChallenges=&m_CarMap[vehicleId];
	}

	return pChallenges;
}

S32 C_MultiplayerChallengeSystem::ReloadChallengesSorcery()
{
	S32 carPackIndex = -1;

	// For each pack
	for ( S32 i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks.count; i++ )
	{
		MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[i];
		BIZ_ASSERT_MSG_SPEED( pPack, "Invalid challenge pack in config (NULL)" );

		S32 packUnlockLevel=GetPackUnlockLevel(pPack);

		// Ignore the car challenges
		if ( pPack->mType == UI::E_ChallengePack_Car )
			carPackIndex = i;

		// For each challenge
		for ( S32 j = 0; j < pPack->mChallenges.count; j++ )
		{
			BIZ_ASSERT_MSG_SPEED( pPack->mChallenges[j].Object(), "Invalid challenge(%i) in pack %i(NULL)", j, i );
			BIZ_ASSERT_MSG_SPEED( pPack->mChallenges[j]->mTiers.count, "Challenge(%i) in pack %i has no valid tiers", j, i );

			m_Challenges[pPack->mChallenges[j]->mType].ReloadSorceryData( pPack->mChallenges[j], packUnlockLevel );
		}
	}
	return carPackIndex;
}

void C_MultiplayerChallengeSystem::RemoveInvalidCarChallenges()
{
	for(TCarMap::Iterator i=m_CarMap.Begin(); i!=m_CarMap.End();)
	{
		C_MultiplayerCarChallenges& challenges=i->GetValue();
		if (!challenges.GetValid())
		{
			TCarMap::Iterator eraseThis=i;
			++i;
			m_CarMap.Erase(eraseThis);
		}
		else
		{
			++i;
		}
	}

}
void C_MultiplayerChallengeSystem::InvalidateCarChallenges()
{
	for(TCarMap::Iterator i=m_CarMap.Begin(); i!=m_CarMap.End(); i++)
	{

		C_MultiplayerCarChallenges& challenges=i->GetValue();
		challenges.SetValid(false);
	}
}

void C_MultiplayerChallengeSystem::AddCarChallenge( XtId carId, MultiplayerChallengePack& pack )
{
	C_MultiplayerCarChallenges challenges;

	const S32 level = C_MultiplayerData::Instance().m_MultiplayerVehicleMap[carId];

	challenges.BuildChallengesFromSorcery(pack.mChallenges, level, pack.mType );

	m_CarMap.Insert( carId, challenges );
}

void C_MultiplayerChallengeSystem::InitCarChallenges( MultiplayerChallengePack& pack )
{
	InitNormalCarChallenges(pack);
	InitDLCCarChallenges(pack);
	InitLegendCarChallenges(pack);
}

void C_MultiplayerChallengeSystem::InitDLCCarChallenges( MultiplayerChallengePack& pack )
{
	// Build up DLC car challenges

	for ( S32 i = 0; i < C_GameData::Instance().mCheatPacks.Number(); i++ )
	{	
		AmaxCheatBase* pCheatBase = C_GameData::Instance().mCheatPacks[ i ];

		if (pCheatBase)
		{
			AmaxVehicle* pVehicle = NULL;

			switch( pCheatBase->GetType() )
			{
			case E_CHEAT_TYPE_VEHICLE:
				{
					pVehicle = ((AmaxCheatVehicle*)pCheatBase)->mAmaxVehicle.Object();
					break;
				}

			case E_CHEAT_TYPE_CHROME:
				{
					pVehicle = ((AmaxCheatChromeVehicle*)pCheatBase)->mAmaxVehicleUnlock.Object();
					break;
				}
			}

			if(pVehicle)
			{
				C_MultiplayerCarChallenges* pChallenges=0;
				pChallenges = ReloadCarChallengesSorcery(pVehicle->ID, pack);

				pChallenges->GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER ).ForceComplete();
				pChallenges->GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES ).ForceComplete();
				ForceCarUpgrades(pVehicle->ID);
			}
		}
	}
}

void C_MultiplayerChallengeSystem::InitLegendCarChallenges( MultiplayerChallengePack& pack )
{
	// Build up Legend car challenges
	for ( S32 level = 0; level < C_MultiplayerData::Instance().m_pConfig->m_LegendLevelingData.count; level++ )
	{
		MultiplayerLevel& legendLevelData = C_MultiplayerData::Instance().m_pConfig->m_LegendLevelingData[level];

		// For each car
		for ( S32 i = 0; i < legendLevelData.mVehicleUnlocks.count; i++ )
		{

			XtId vehicleId=legendLevelData.mVehicleUnlocks[i]->ID;

			C_MultiplayerCarChallenges* pChallenges=0;
			pChallenges = ReloadCarChallengesSorcery(vehicleId, pack);

			// Legend cars  begin with full progress in these challenges (so they get the all the shiny awards - paint, health slots etc)
			pChallenges->GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER ).ForceComplete();
			pChallenges->GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES ).ForceComplete();
			ForceCarUpgrades(vehicleId);
		}
	}
}

void C_MultiplayerChallengeSystem::InitNormalCarChallenges( MultiplayerChallengePack& pack )
{
	for ( S32 level = 0; level < C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count; level++ )
	{
		MultiplayerLevel& levelData = C_MultiplayerData::Instance().m_pConfig->m_LevelingData[level];

		// For each car
		for ( S32 i = 0; i < levelData.mVehicleUnlocks.count; i++ )
		{
			const U32 carID = levelData.mVehicleUnlocks[i]->ID;
			const S32 level = C_MultiplayerData::Instance().m_MultiplayerVehicleMap[levelData.mVehicleUnlocks[i]->ID];

			C_MultiplayerCarChallenges* pChallenges=0;
			pChallenges = ReloadCarChallengesSorcery(carID, pack);

			//Boss cars
			if(levelData.mVehicleUnlocks[i]->m_ParentVehicle && pChallenges)
			{
				if(levelData.mVehicleUnlocks[i]->m_ParentVehicle)
				{
					pChallenges->GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER ).ForceComplete();
					pChallenges->GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES ).ForceComplete();
					ForceCarUpgrades(carID);
				}
			}
		}
	}
}
}; // namespace Game
