#include "stdafx.h"
#include "LevelStateVehicleUpgrades.h"
#include <Network\Player\Data\NetPlayerRaceData.h>

void Game::LevelStateVehicleUpgrades::Clear()
{
	// Clear status of all abilities
	mMultiplayerAbilities.Clear(); 

	//	bools = false
	mCollectAnyPerkType = false;
	mManualPerkChange = false;
	mDoubleNitro = false; 
	mDoubleShunt = false; 
	mDoubleBarge = false; 
	mDoubleShock = false; 
	mDoubleBomb = false; 
	mDoubleShield = false;
	mAdvancedSlipstream = false;
	mAdvancedConvertToHealth = false; 
	mAdvancedCloneFirst = false; 
	mShield = false; 
	mRepairBoostLevel2 = false; 
	mRepairBoostLevel3 = false; 
	mFansSearcher = false; 
	mFansViral = false;
	mFansForumite = false; 
	mFansBlogger = false; 
	mFansSabotage = false; 
	mFansBribe = false; 
	mNitroUpgrade = false; 
	mShuntUpgrade = false; 
	mBargeUpgrade = false; 
	mShockUpgrade = false; 
	mBombUpgrade = false;
	mAdditionalHP = false;
	mMPFansBloggerMultiplier = 1.0f; 

	//	memset(&mShop[0], 0, sizeof(mShop));
}

void Game::LevelStateVehicleUpgrades::DebugValues()
{
	Clear();

	mNumPickupSlots		= g_debugPerkSlotOveride;
	mNumAdditionalHealthSlots	= 0;
	mAdditionalHP				= false;
	mManualPerkChange	= true;
	mCollectAnyPerkType	= true;
	mShield				= true;
	mNumShieldSlots		= 5;

	mDoubleNitro		= true;
	mDoubleShunt		= true;
	mDoubleBomb			= true;
	mDoubleShield		= true;
}

void Game::LevelStateVehicleUpgrades::OfflineValues()
{
	Clear();

	mNumPickupSlots		= 3;
	mNumShieldSlots		= 3;
	mNumAdditionalHealthSlots	= 0;
	mAdditionalHP				= false;


	mManualPerkChange	= true;
	mCollectAnyPerkType	= true;
	mShield				= true;

	mDoubleNitro		= true;
	mDoubleShunt		= true;
	mDoubleBomb			= true;
	mDoubleShield		= true;
}

void Game::LevelStateVehicleUpgrades::BuildFromNetPlayerVehicle( const RacerNet::NetPlayerVehicle& vehicleData )
{
	mNumPickupSlots				= vehicleData.m_PickupSlots;
	mNumShieldSlots				= vehicleData.m_ShieldSlots;
	mNumAdditionalHealthSlots	= vehicleData.m_AdditionalHealthPips;
	mAdditionalHP				= vehicleData.m_AdditionalHP;

	//	These are always true for all vehicles in Multiplayer, so we don't need to grab the settings from anywhere.
	mCollectAnyPerkType			= true;
	mManualPerkChange			= true;
	mShield						= true;
	mDoubleNitro				= true;
	mDoubleShunt				= true;
	mDoubleBomb					= true;
	mDoubleShield				= true;
}