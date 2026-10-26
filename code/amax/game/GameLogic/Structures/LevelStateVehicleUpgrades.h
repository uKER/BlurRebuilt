#ifndef LEVEL_STATE_VEHICLE_UPGRADES_H
#define LEVEL_STATE_VEHICLE_UPGRADES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Structures/LevelStateMultiplayerAbilities.h>

namespace RacerNet
{
	struct NetPlayerVehicle;
}

namespace Game
{
	extern Debug::Int g_debugPerkSlotOveride;

struct LevelStateVehicleUpgrades
{
	//	Clear all values to zero or false. A vehicle with only these settings won't be able to pick anything up or use anything (it's PGR!)
	void Clear();

	//	Values we use for every vehicle in GameModeDebug
	void DebugValues();

	void OfflineValues();

	void BuildFromNetPlayerVehicle(const RacerNet::NetPlayerVehicle& vehicleData);

	//	Pickup use
public:
	int									NumPickupSlots() const							{ return mNumPickupSlots; }
	void								NumPickupSlots(int val)							{ mNumPickupSlots = val; }
	bool								CollectAnyPerkType() const						{ return mCollectAnyPerkType; }
	void								CollectAnyPerkType(bool val)					{ mCollectAnyPerkType = val; }
	bool								ManualPerkChange() const						{ return mManualPerkChange; }
	void								ManualPerkChange(bool val)						{ mManualPerkChange = val; }

private:
	int									mNumPickupSlots;
	bool								mCollectAnyPerkType;
	bool								mManualPerkChange;

	//	Double-taps
public:
	bool								DoubleNitro() const								{ return mDoubleNitro; }
	void								DoubleNitro(bool val)							{ mDoubleNitro = val; }
	bool								DoubleShunt() const								{ return mDoubleShunt; }
	void								DoubleShunt(bool val)							{ mDoubleShunt = val; }
	bool								DoubleBarge() const								{ return mDoubleBarge; }
	void								DoubleBarge(bool val)							{ mDoubleBarge = val; }
	bool								DoubleShock() const								{ return mDoubleShock; }
	void								DoubleShock(bool val)							{ mDoubleShock = val; }
	bool								DoubleBomb() const								{ return mDoubleBomb; }
	void								DoubleBomb(bool val)							{ mDoubleBomb = val; }
	bool								DoubleShield() const							{ return mDoubleShield; }
	void								DoubleShield(bool val)							{ mDoubleShield = val; }

private:
	bool								mDoubleNitro;
	bool								mDoubleShunt;
	bool								mDoubleBarge;
	bool								mDoubleShock;
	bool								mDoubleBomb;
	bool								mDoubleShield;

	////	"Advanced" pickup functionality
public:
	bool								AdvancedSlipstream() const						{ return mAdvancedSlipstream; }
	void								AdvancedSlipstream(bool val)					{ mAdvancedSlipstream = val; }
	bool								AdvancedConvertToHealth() const					{ return mAdvancedConvertToHealth; }
	void								AdvancedConvertToHealth(bool val)				{ mAdvancedConvertToHealth = val; }
	bool								AdvancedCloneFirst() const 						{ return mAdvancedCloneFirst; }
	void								AdvancedCloneFirst(bool val)					{ mAdvancedCloneFirst = val; }

private:
	bool								mAdvancedSlipstream;
	bool								mAdvancedConvertToHealth;
	bool								mAdvancedCloneFirst;

	//	Shields
public:
	bool								Shield() const									{ return mShield; }
	void								Shield(bool val)								{ mShield = val; }
	int									NumShieldSlots() const							{ return mNumShieldSlots; }
	void								NumShieldSlots(int val)							{ mNumShieldSlots = val; }

private:
	bool								mShield;
	int									mNumShieldSlots;

	//	Health
	public:
	int									NumAdditionalHealthSlots() const				{ return mNumAdditionalHealthSlots; }
	void								NumAdditionalHealthSlots(int val)				{ mNumAdditionalHealthSlots = val; }

private:
	int									mNumAdditionalHealthSlots;

	//	Repair boost
public:
	bool								RepairBoostLevel2() const						{ return mRepairBoostLevel2; }
	void								RepairBoostLevel2(bool val)						{ mRepairBoostLevel2 = val; }
	bool								RepairBoostLevel3() const						{ return mRepairBoostLevel3; }
	void								RepairBoostLevel3(bool val)						{ mRepairBoostLevel3 = val; }

private:
	bool								mRepairBoostLevel2;
	bool								mRepairBoostLevel3;

	//	Fan bonuses
public:
	bool								FansSearcher() const							{ return mFansSearcher; }
	void								FansSearcher(bool val)							{ mFansSearcher = val; }
	bool								FansViral() const								{ return mFansViral; }
	void								FansViral(bool val)								{ mFansViral = val; }
	bool								FansForumite() const							{ return mFansForumite; }
	void								FansForumite(bool val)							{ mFansForumite = val; }
	bool								FansBlogger() const								{ return mFansBlogger; }
	void								FansBlogger(bool val)							{ mFansBlogger = val; }
	bool								FansSabotage() const							{ return mFansSabotage; }
	void								FansSabotage(bool val)							{ mFansSabotage = val; }
	bool								FansBribe() const								{ return mFansBribe; }
	void								FansBribe(bool val)								{ mFansBribe = val; }

private:
	bool								mFansSearcher;
	bool								mFansViral;
	bool								mFansForumite;
	bool								mFansBlogger;
	bool								mFansSabotage;
	bool								mFansBribe;

	// Dont want to interfere with existing SP setup so making seperate MP fan bonuses
public:
	float								MPFansBlogger() const								{ return mMPFansBloggerMultiplier; }
	void								MPFansBlogger(float val)							{ mMPFansBloggerMultiplier = val; }

private:
	float								mMPFansBloggerMultiplier;

	//	Power-up power bonuses - [GavinY] : must be VERY temporary!
public:
	bool								NitroUpgrade() const							{ return mNitroUpgrade; }
	void								NitroUpgrade(bool val)							{ mNitroUpgrade = val; }
	bool								ShuntUpgrade() const							{ return mShuntUpgrade; }
	void								ShuntUpgrade(bool val)							{ mShuntUpgrade = val; }
	bool								BargeUpgrade() const							{ return mBargeUpgrade; }
	void								BargeUpgrade(bool val)							{ mBargeUpgrade = val; }
	bool								ShockUpgrade() const							{ return mShockUpgrade; }
	void								ShockUpgrade(bool val)							{ mShockUpgrade = val; }
	bool								BombUpgrade() const								{ return mBombUpgrade; }
	void								BombUpgrade(bool val)							{ mBombUpgrade = val; }
	bool								AdditionalHP() const							{return mAdditionalHP;}
	void								AdditionalHP(bool val)							{mAdditionalHP=val;}

private:
	bool								mNitroUpgrade;
	bool								mShuntUpgrade;
	bool								mBargeUpgrade;
	bool								mShockUpgrade;
	bool								mBombUpgrade;
	bool								mAdditionalHP;

public:
	//	Multiplayer only stuff beyond this point, upgrades for cars in online play
	Game::C_LevelStateMultiplayerVehicleAbilities* GetMultiplayerAbilities() {return &mMultiplayerAbilities;}

private:
	Game::C_LevelStateMultiplayerVehicleAbilities mMultiplayerAbilities;
};

} // namespace Game


#endif // LEVEL_STATE_VEHICLE_UPGRADES_H
