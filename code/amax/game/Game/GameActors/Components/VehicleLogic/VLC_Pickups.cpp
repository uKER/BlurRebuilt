#include "stdafx.h"

#include <Game/Game.h>

#include <Game\GameActors\Components\GameComponents.h>
#include "VLC_Pickups.h"

#include <Game/Messaging/GameMessages.h>
#include <Game\Messaging\GameflowMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <Game/Messaging/PickupActorMessages.h>

#include <GameLogic\Messaging\GameLogicMessages.h>

#include <GameLogic\Pickups\Normal\PickupNitrous.h>
#include <GameLogic\Pickups\Normal\PickupShock.h>
#include <GameLogic\Pickups\Normal\PickupShunt.h>
#include <GameLogic\Pickups\Normal\PickupBlindfire.h>
#include <GameLogic\Pickups\Normal\PickupBarge.h>
#include <GameLogic\Pickups\Normal\PickupShield.h>
#include <GameLogic\Pickups\Normal\PickupSuperShield.h>

#include <GameLogic\Pickups\Upgraded\PickupBomb.h>
#include <GameLogic\Pickups\Double\PickupDoubleNitrous.h>
#include <GameLogic\Pickups\Reverse\PickupNitrousReverse.h>

#include <Game\GameActors\Vehicles\VehicleActor.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <actor_system/ActorSys.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <game/gamecontrols/GameControls.h>

#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#include <GameLogic\InGameDIalogueManager\InGameDialogueManager.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

#include <GameLogic/Structures/LevelState.h>

#include "GameAudio/GameAudioLayer.h"

#include "GameAudio/PickupSubsystem.h"

#include <Ui/UiInterface.h>
#include <Ui/SoundEffects.h>

#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

// All these are need to access the racelogiccore... Must be a better way?
#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <GameLogic/Layers/RaceLogicLayer.h>

#include "Automation\GameDataLogging.h"

#include <RaceBook\RaceBookManager.h>

#include "Game/GameActors/Components/Messages/AudioMessages.h"

#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game\Achievements\AchievementSystem.h>
#include <GameLogic\StickerManager.h>
#include <GameLogic\Pickups\PickupsManager.h>

Debug::Logger &g_debugPickupMpSyncLogger(Debug::GetLogger("Amax.Multiplayer.PickupMpSync"));

Debug::Float g_debugCheckCutOffLength("Perks.CheckCutOffLength",150,Debug::Persist());
Debug::Float g_debugDropPickupBackMultiplier("Multiplayer.DropPickupBackMultiplier", 5.0f, Debug::Range(0.0f, 1000.0f));
Debug::Bool g_debugPerksAutoShuffle("Perks.AutoShuffle",false,Debug::Persist());

Debug::Float g_debugForwardFirePitch("Perks.ForwardFirePitch", 0.8f);
Debug::Float g_debugBackwardFirePitch("Perks.BackwardFirePitch", 0.2f);
Debug::Float g_debugPerksChargeTime("Perks.ChargeTime", 0.5f);

Debug::Float g_debugPickupsDisableTime("Perks.DisableTime", 0.75f);
Debug::Float g_debugPickupsDisableTimeBlindfireToBlindfire("Perks.DisableTimeBlindfireToBlindfire", 0.05f);
Debug::Float g_debugPickupsDisableTimeBlindfireToOther("Perks.DisableTimeBlindfireToOther", 0.75f);

Debug::Int g_SafetyNetMaxPickups("Multiplayer.Upgrades.SafetyNet.MaxPickupsAbsorbable", 2, Debug::Persist());

Debug::Bool g_BribeMeNow("Multiplayer.Upgrades.Bribe.BribeMeNow", false);

Debug::Bool g_debugPerksForceShotgunBlindfire("Perks.BlindfireShotgun.ForceShotgunBlindfire", false);
Debug::Bool g_debugPerksForceDoubleShield("Perks.DoubleShield.ForceDoubleShield", false);
Debug::Bool g_debugPerksForceDoubleNitrous("Perks.DoubleNitrous.ForceDoubleNitrous", false);

Debug::Float g_debugPerksDisableWhenHitByShunt("Perks.DisableWhenHitBy.Shunt", 0.9f);
Debug::Float g_debugPerksDisableWhenHitByBomb("Perks.DisableWhenHitBy.Bomb", 0.5f);

Debug::Bool g_ShieldedBoostersUseExistingEffect("Multiplayer.Upgrades.ShieldedBoosters.UseExistingEffect", false, Debug::Persist());

Debug::Int	g_debugVehicleAnyIndex("Perks.Gift.Any.VehicleIndex",0,Debug::Range(0,19),Debug::Persist());
Debug::Bool	g_debugVehicleAnyReplenish("Perks.Gift.Any.Replenish",false);

Debug::Float g_debugPerksDoubleShieldCooldown("Perks.DoubleShield.Cooldown", 1.0f);

Debug::Float g_debugGiftPickupDelayTime("Multiplayer.Upgrades.GiftPickupDelayTime", 1.1f);

extern Debug::Float g_debugShockManagerMinSpeedScrub;

extern Debug::Enum<UI::E_MultiplayerVehicleAbilities> g_AbilitySlot1;
extern Debug::Enum<UI::E_MultiplayerVehicleAbilities> g_AbilitySlot2;
extern Debug::Enum<UI::E_MultiplayerVehicleAbilities> g_AbilitySlot3;

static void DisplayAbilities(void *)
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	bool isRanked = !pLayer || !pLayer->GetGameModePtr() || pLayer->GetGameModePtr()->IsRanked();

	const UI::E_MULTIPLAYER_LOADOUT_TYPE type = pMultiplayerProfile->GetLoadouts(isRanked).GetCurrentLoadOutType();
	Game::MultiplayerLoadOut& loadOut = const_cast<Game::MultiplayerLoadOut&>(pMultiplayerProfile->GetLoadouts(isRanked).GetLoadOut( type ));
	
	g_AbilitySlot1 = loadOut.GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT1);
	g_AbilitySlot2 = loadOut.GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT2);
	g_AbilitySlot3 = loadOut.GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT3);
}

static void GrantAbility(void *)
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

	Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
	bool isRanked = !pLayer || !pLayer->GetGameModePtr() || pLayer->GetGameModePtr()->IsRanked();

	const UI::E_MULTIPLAYER_LOADOUT_TYPE type = pMultiplayerProfile->GetLoadouts(isRanked).GetCurrentLoadOutType();
	Game::MultiplayerLoadOut& loadOut = const_cast<Game::MultiplayerLoadOut&>(pMultiplayerProfile->GetLoadouts(isRanked).GetLoadOut( type ));
	
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	Game::C_LevelStateMultiplayerVehicleAbilities *mpMultiplayerAbilities = levelState.mCars[0].mUpgrades.GetMultiplayerAbilities();


	for(int slot = 0; slot < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; slot++)
	{
		//Disable current ability in thats lot
		if(loadOut.slots[slot]!= UI::E_MultiplayerVehicleAbilities_None)
		{
			mpMultiplayerAbilities->GetAbility(loadOut.slots[slot])->SetActive(false);
		}

		switch(slot)
		{
		case 0:
			loadOut.SetAbilityInSlot((UI::E_MULTIPLAYER_LOADOUT_SLOT)slot, g_AbilitySlot1);
			break;
		case 1:
			loadOut.SetAbilityInSlot((UI::E_MULTIPLAYER_LOADOUT_SLOT)slot, g_AbilitySlot2);
			break;
		case 2:
			loadOut.SetAbilityInSlot((UI::E_MULTIPLAYER_LOADOUT_SLOT)slot, g_AbilitySlot3);
			break;
		}

		if(loadOut.slots[slot]!= UI::E_MultiplayerVehicleAbilities_None)
		{
			//And set the new one
			mpMultiplayerAbilities->GetAbility(loadOut.slots[slot])->SetActive(true);
		}
	}

	DisplayAbilities(0);
}

Debug::Function	g_debugGrantAbility("Multiplayer.Upgrades.GrantAbility", GrantAbility, 0);
Debug::Function	g_debugShowCurrentLoadout("Multiplayer.Upgrades.ShowCurrentLoadOut", DisplayAbilities, 0);
Debug::Float	g_debugTitaniumArmourIntensity("Multiplayer.Upgrades.TitaniumArmour.Intensity", 0.4f);

extern Debug::Float g_debugPerkStartDelay;
extern Debug::Bool g_ForceShieldsTiedToPickups;
extern Debug::Float g_debugShieldScaleX;
extern Debug::Float g_debugShieldScaleY;
extern Debug::Float g_debugShieldScaleZ;

namespace Components
{

bool C_VehicleComponent_Pickups::CreateImpl()
{
	const I_VehicleComponent_Pickup_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Pickup_CreateInfo *>(GetCreator().GetData());

	if(creation_data)
	{
		mVehicleIndex=creation_data->mVehicleIndex;
		mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
	}

	//AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_PickupMessage_PickupPickedUp );
	AddSubscriber( Game::E_GameLogicMessage_Scoring_Event );
	AddSubscriber( Game::E_VehicleMessage_FirePickup );
	AddSubscriber( Game::E_VehicleMessage_ResetAfterWrecked );
	AddSubscriber( Game::E_PickupMessage_GrantPickup );
	AddSubscriber( Game::E_PickupMessage_ClearHitUpdaters );
	AddSubscriber( Game::E_GameflowMessage_RaceStarted );
	AddSubscriber( Game::E_VehicleMessage_DebugRender );
	AddSubscriber( Game::E_GameLogicMessage_LapIncremented );
	AddSubscriber( Game::E_GameLogicMessage_SplitCrossed );
	
	
	AddSubscriber( Game::E_GameflowMessage_QuitRace );
	AddSubscriber( Game::E_GameflowMessage_RaceFinished );
	AddSubscriber( Game::E_GameflowMessage_QuitAndWinRace );
	AddSubscriber( Game::E_GameflowMessage_RestartRace );

	AddSubscriber( E_VehicleLogicMsg_FiredPerk );
	AddSubscriber( E_VehicleLogicMsg_EnableDisablePickupSystem );
	AddSubscriber( E_VehicleLogicMsg_DropPickupOnTrack );
	AddSubscriber( E_VehicleLogicMsg_ShieldedPerk ); 

	Initialise();

	return true;
}

void C_VehicleComponent_Pickups::Initialise()
{
	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;
	info.mCurrentlySelectedType=info.mCurrentlyUsingType=E_PICKUP_TYPE_NONE;
	info.mCharging = false;
	info.mPickupStore.Clear();
	info.mPickupStoreIndex=-1;
	info.mCurrentlyUsingLevel=-1;
	mBlindfireHitLeft = true;

	mCanCharge = true;
	mChargingTime = 0.f;

	mLevelStatePtr = &Game::C_Game::GetInstance()->GetLevelState();
	mRLCPtr = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	BIZ_ASSERT(mLevelStatePtr);
	BIZ_ASSERT(mRLCPtr);

	if(mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.FansBribe())
	{
		AmaxEvent *ev=C_GameEventData::Instance().mEvents[(GAMEDATA_ID)mLevelStatePtr->mEvent];
		if(ev)
		{
			float probs[E_PICKUP_TYPE_MAX]={1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f};

			for(int a=0;a<ev->pickupProbabilities->pickupProbabilities.count;a++)
			{
				probs[ev->pickupProbabilities->pickupProbabilities[a].pickupType]=ev->pickupProbabilities->pickupProbabilities[a].probability;
			}

			Btl::Vector<E_PICKUP_TYPE> types;
			for(int a=E_PICKUP_TYPE_NITROUS;a<=E_PICKUP_TYPE_SHIELD;a++)
			{
				if(probs[a])
				{
					types.PushBack((E_PICKUP_TYPE)a);
				}
			}

			// Setup num shots awarded
			int randIndex = rand()%types.Size(); 
			E_PICKUP_TYPE type = types[randIndex];
			int numShotsAwarded = GetNumShotsAwarded(type);
			HitPickUp(type,-1,numShotsAwarded);
		}
		else
		{
			E_PICKUP_TYPE types[5] = {E_PICKUP_TYPE_NITROUS,E_PICKUP_TYPE_SHUNT,E_PICKUP_TYPE_SHOCK,E_PICKUP_TYPE_BARGE,E_PICKUP_TYPE_BOMB};
			E_PICKUP_TYPE randType = types[rand()%5];

			if(randType == E_PICKUP_TYPE_SHOCK && !AIAmax::AmaxAIR::Instance().IsDataInitialised())
				randType = E_PICKUP_TYPE_SHUNT;

			int numShotsAwarded = GetNumShotsAwarded(randType);
			HitPickUp(randType,-1,numShotsAwarded);
		}
	}

	mRaceStarted=false;

	mDeltaTime=0.f;
	mLastFiredPickupId = 0;

	mTimer_PerkGifting = mLevelStatePtr->mCars[mVehicleIndex].mPerkGrantingInitialTimer;
	mUsingPerkLevel=0;

	mVehicleTargettedByShockTimer=0.f;
	mVehicleTargettedByShock=-1;

	mLastFiredPerk=E_PICKUP_TYPE_NONE;

	mDisablePickups=0.f;
	mDisableSlowFiringPickups=0.f;
	mHitByDoubleShieldCooldown = 0.f;

	mSafetyNetNumTimesAbsorbed = 0;
	mSafetyNetNumBlindfiresAbsorbed = 0;

	mIsMultiplayer = false;
	mDropPickup = false;

	mGroupPickupId=1;	//	Start this at 1 so we can check against 0 for group IDs we don't care about (and don't need to send around the network)

	// Cache pointer to owner vehicle's abilities
	mpMultiplayerAbilities = mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
	BIZ_ASSERT_MSG(mpMultiplayerAbilities,"Error.. VLC_MP_Local_Pickups CreateImpl() unable to obtain pointer to Multiplayer Abilities");

	//Keep track of the last pickup to hit us
	mLastShockPickupIDToHit = -1;

	//[CWright_Integrate] Knock this out?
	//PopulatePickupProbabilities();
	mSafetyNetFXCooldownTimer = 0.f;
	mRoadsweeperPickupFXCooldownTimer = 0.f;
	mBribeDelayTimer = 0.0f;

	Gravity::C_Scene* physicsScene = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetPhysicsScene();
	mLasersight.Init(mVehicleIndex, mpMultiplayerAbilities, physicsScene, &C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings, mRLCPtr->GetViewportIndex(mVehicleIndex));
	mLastLapAwardedFrontRunner = -1; 
	mStartingLogicPosition = 0.0f;
}

bool C_VehicleComponent_Pickups::ShufflePerks(bool left)
{
	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
	VehiclePickupInfo& info = vehActor->mPickupInfo;

	E_PICKUP_TYPE prevType = vehActor->GetCurrentlySelectedPickup();

	if(info.mPickupStore.Size()==0)
		return false;

	info.mPickupStoreIndex += (left ? -1 : 1);
	if(info.mPickupStoreIndex == -1)
		info.mPickupStoreIndex=info.mPickupStore.Size()-1;

	if(info.mPickupStoreIndex >= info.mPickupStore.Size())
		info.mPickupStoreIndex=0;

	info.mCurrentlySelectedType=info.mPickupStore[info.mPickupStoreIndex].mType;

#ifndef CFG_GOLD
	Game::C_ReplayDirector::Instance().AddPickupStore(mVehicleIndex);
#endif

	HandlePickupSlotChange();

	return true;
}
	
void C_VehicleComponent_Pickups::PlaySelectedPerkSfx()
{
	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;
	if( info.mPickupStore.Size() < 2 )
		return;

	UI::E_UiSoundEffect eSound = UI::E_UiSoundEffect_PerkSwitchActionIn;

	if ( eSound != UI::E_UiSoundEffect_Unknown )
		UI::UiInterface()->PlaySound( eSound );
}

E_PICKUP_TYPE C_VehicleComponent_Pickups::HandleDropPickupOnTrack(SpawnInformation& information)
{
	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
	
	GetSpawnInformation(information);

	if(information.mType == E_PICKUP_TYPE_NONE)
	{
		return information.mType;
	}

	//	Drop this pickup on the track (If we have the max shots left)
	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;
	S8 power = info.mPickupStore[info.mPickupStoreIndex].mNumShots;
	if(power == GetNumShotsAwarded(information.mType))
	{
		U32 uniqueID = mRaceLogicLayerPtr->DropPickup(-1, mVehicleIndex, information.mType, information.mPosition);
		information.mUniqueId = uniqueID;
	}
	else
	{
		information.mUniqueId = 0;
	}

	// [mtaylor] let's also do a catch-all sync if using the "shields are pickups" system
	if(mLevelStatePtr->mShieldsTiedToPickups || g_ForceShieldsTiedToPickups)
	{
		vehActor->mFanPower = (float)vehActor->mPickupInfo.mPickupStore.Size();
	}

	return information.mType;
}

E_PICKUP_TYPE C_VehicleComponent_Pickups::HandleDropDecoyOnTrack(SpawnInformation& information)
{	
	GetSpawnInformation(information);

	if(information.mType == E_PICKUP_TYPE_NONE)
	{
		return information.mType;
	}

	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;
	S8 power = info.mPickupStore[info.mPickupStoreIndex].mNumShots;
	if(power == GetNumShotsAwarded(information.mType))
	{
		// Before we drop this lets check for adjustments
		if(Game::VehicleIsLocal(mVehicleIndex))
		{
			AdjustPosToAvoidExistingPickups(information); 
		}

		U32 uniqueID = mRaceLogicLayerPtr->DropDecoyPickup(-1, mVehicleIndex, information.mType, information.mPosition);
		information.mUniqueId = uniqueID;
	}
	else
	{
		information.mUniqueId = 0;
	}

	return information.mType;
}

void C_VehicleComponent_Pickups::AdjustPosToAvoidExistingPickups(SpawnInformation& information)
{
	// Dont allow users to drop a decoy within another existing pickup. Usability indicates this is frustrating for victims of the decoy!.

	// 1) Determine if we lie too close to an existing pickup. 

	// For this we simply ask our dropper 
	const Game::Actors::C_VehicleLogicActor* pLogicActor = (const Game::Actors::C_VehicleLogicActor*)GetActor();
	if(!pLogicActor || !mRaceLogicLayerPtr)
	{
		return;
	}

	// for the ID of the last pickup it collected. (this may be none!)
	int lastPickupCollectedID = pLogicActor->GetLastPickupIdCollided(); 
	::Actors::PickupPositionActor* pPickup = mRaceLogicLayerPtr->GetPickupPositionActor(lastPickupCollectedID);
	if(!pPickup)
	{
		// may not have collected a pickup yet
		return;
	}
	
	// Get our distance from that pickup
	Maths::HVec3 existingPickupPos = pPickup->GetMatrix().AW();
	Maths::HVec3 decoyDesiredPos   = information.mPosition.AW();
	float distSqrd = Maths::DistanceSquared(existingPickupPos,decoyDesiredPos);

	// Compare our distance against the tolerance value
	bool requiresPositionAdjust = false; 
	MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
	if(!pConfig)
	{
		return;
	}

	float tolerance = pConfig->m_MiscModSettings.m_DecoyDropToleranceDistance;
	float toleranceSqrd = tolerance * tolerance;
	if(distSqrd < toleranceSqrd )
	{
		requiresPositionAdjust = true;
	}

	// 2) If we are positioned too close! 
	if(requiresPositionAdjust)
	{
		// Get Vector from car to pickup
		Maths::HVec3 carToPickupDir =  existingPickupPos - pLogicActor->GetWorldMatrix().AW();

		// Knock out the Y component
		carToPickupDir.Y(0.0f);

		// Normalize
		carToPickupDir.Normalise(); 

		// Set the position of the new decoy to be existing pickup pos + (offsetDist * awayfromPickupVector).
		Maths::HVec3 adjustedPos = existingPickupPos + (tolerance * carToPickupDir);
		information.mPosition.AW(adjustedPos);
	}
}

void C_VehicleComponent_Pickups::GetSpawnInformation(SpawnInformation& information)
{
	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

	//	Check that we don't already have too many pickups on track - mostly used for MP but a general safety net.
	if(vehActor->mPickupInfo.mPickupStoreIndex == -1 || !mRaceLogicLayerPtr->CanSpawnPickupOnTrack())
	{
		information.mType = E_PICKUP_TYPE_NONE;
		return;
	}

	information.mType = vehActor->mPickupInfo.mPickupStore[vehActor->mPickupInfo.mPickupStoreIndex].mType;

	Maths::HMat43 posMat = mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mMatrix;
	//perk clips through the floor when spawned bang on the vehicle position, shift it up a bit
	Maths::HVec3 up=mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mMatrix.AY();
	up = Maths::Normalise(up);
	Maths::HVec3 forwards = mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mMatrix.AZ();
	forwards = Maths::Normalise(forwards);
	posMat.AW(posMat.AW()+up*0.7f);
	posMat.AW(posMat.AW() - (forwards * g_debugDropPickupBackMultiplier));

	information.mPosition = posMat;
}

// Handles the Bride mod
void C_VehicleComponent_Pickups::CheckBribeAbility()
{
	// Bribe Upgrade
	Game::C_MultiplayerVehicleAbility* bribeAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Bribe);
	if(bribeAbility->GetActive() || g_BribeMeNow)
	{
		E_PICKUP_TYPE eRandomPickupType = EvaluateBribeChances();

		// Bribe ability activated
		mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Bribe, true);
		mpMultiplayerAbilities->SetPickupSlotGiftedInto(((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo.mPickupStore.Size() + 1);

		if(g_DebugShowMpAbilityActivateSpam)
		{
			LOGDEBUG(g_MultiplayerLog, "Vehicle %d's Bribe ability has been activated!", mVehicleIndex );
		}

		DelayGiftPickUp(eRandomPickupType, g_debugGiftPickupDelayTime, GetNumShotsAwarded(eRandomPickupType));
	}
}

void C_VehicleComponent_Pickups::PopulatePickupProbabilities(PickupPercentages& percentages)
{
	if(m_PickupProbabilities.Size() != 0)
	{
		m_PickupProbabilities.Clear();
	}

	//Make sure this is kept in the order of E_PICKUP_TYPE or everything will go screwy
	m_PickupProbabilities.PushBack(percentages.m_ChanceOfNitro);
	m_PickupProbabilities.PushBack(percentages.m_ChanceOfShock);
	m_PickupProbabilities.PushBack(percentages.m_ChanceOfShunt);
	m_PickupProbabilities.PushBack(percentages.m_ChanceOfBlindfire);
		
	m_PickupProbabilities.PushBack(percentages.m_ChanceOfBarge);
	m_PickupProbabilities.PushBack(percentages.m_ChanceOfMine);
	m_PickupProbabilities.PushBack(percentages.m_ChanceOfRepair);
	m_PickupProbabilities.PushBack(percentages.m_ChanceOfShield);
}

const E_PICKUP_TYPE C_VehicleComponent_Pickups::EvaluateBribeChances() const
{
	//See if we've got AI data for this track
	const bool aiDataValid = AIAmax::AmaxAIR::Instance().AllOK();

	Btl::Vector<int> pickupProbabilities;
	int value = 0, total = 0;
	for(int i = 0; i < m_PickupProbabilities.Size(); i++)
	{
		if(!aiDataValid && i == E_PICKUP_TYPE_SHOCK)
		{
			value = 0;
		}
		else 
		{
			//Make sure we're not trying to get any dodgy values
			value = (int)(m_PickupProbabilities[i] * BRIBE_PICKUP_PROBABILITY_MULTIPLAYER);
		}

		total += value;
		pickupProbabilities.PushBack( total );
	}	

	//Now get a random value and mod it with the total
	int randomValue = rand() % total;
	for(int i = 0; i < pickupProbabilities.Size()-1; i++)
	{
		if(randomValue < pickupProbabilities[i])
			return (const E_PICKUP_TYPE)i;
	}

	//If we're here then it must be the last one in the list...mustn't it?
	return E_PICKUP_TYPE_SHIELD;
}

void C_VehicleComponent_Pickups::CheckFrontRunner(bool wasRespawn)
{
	Game::C_MultiplayerVehicleAbility* frontRunnerAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Front_Runner);
	if(frontRunnerAbility->GetActive())
	{
		
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		if(pActor)
		{
			// Check that (if not motormash) player has progressed since last receiving a front runner gift, and isnt trying to sneakily cheat us!
			if(wasRespawn || pActor->GetLapInfo().mCurrentLap > mLastLapAwardedFrontRunner || mLevelStatePtr->mRaceMode == Game::E_RaceMode_Multiplayer_Destruction) 
			{
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Front_Runner, true);
				mpMultiplayerAbilities->SetPickupSlotGiftedInto(pActor->mPickupInfo.mPickupStore.Size() + 1);

				if(g_DebugShowMpAbilityActivateSpam)
				{
					LOGDEBUG(g_MultiplayerLog, "Vehicle %d's Front Runner ability has been activated!", mVehicleIndex);
				}

				// Award Nitro
				DelayGiftPickUp(E_PICKUP_TYPE_NITROUS, g_debugGiftPickupDelayTime);

				// Record last lap awarded front runner if necessary so palyewr cant reverse over finish line to get more nitros!
				mLastLapAwardedFrontRunner = pActor->GetLapInfo().mCurrentLap;
			}
		}
	}
}

void C_VehicleComponent_Pickups::AttemptDropPickup()
{
	Game::C_MultiplayerVehicleAbility* decoyDrop = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Decoy_Drop);
	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;
	
	// If we have the decoy-drop ability activated then drop a decoy...
	if( decoyDrop->GetActive() )
	{
		SpawnInformation information;
		if( HandleDropDecoyOnTrack(information) != E_PICKUP_TYPE_NONE )
		{
			if(info.mPickupStoreIndex != -1)
			{
				S8 power = info.mPickupStore[info.mPickupStoreIndex].mNumShots;
				for(S8 i = 0; i < power; i++)
				{
					RemovePower(1);
				}

				//play audio
				UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_DiscardDecoyPickupActionIn );
			}
		}

		mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Decoy_Drop, true);
		
	}
	else // otherwise drop the currently selected perk 
	{
		DropCurrentPickup();		
	}
}

void C_VehicleComponent_Pickups::Update()
{
	PERF_EVENT("C_VehicleComponent.Pickups.Update");

	if(!mRaceStarted)
	{
		UpdateHitUpdaters();
		return;
	}

#ifndef CFG_GOLD
	ProcessDebugUpdate();
#endif //CFG_GOLD

	if(mRaceLogicLayerPtr->GetPerksActivatedIn())
		return;

	mDeltaTime = C_GameApp::GetInstance()->GetTimeStep();

	GiftPickups();
	
	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;

	Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
	if(mDisablePickups > 0.f)
	{
		mDisablePickups-=mDeltaTime;
		if(mDisablePickups < 0.f)
		{
			mDisablePickups=0.f;
		}
	}

	if(mHitByDoubleShieldCooldown > 0.f)
		mHitByDoubleShieldCooldown -= mDeltaTime;

	if(mDisableSlowFiringPickups > 0.f)
	{
		mDisableSlowFiringPickups-=mDeltaTime;
		if(mDisableSlowFiringPickups < 0.f)
		{
			mDisableSlowFiringPickups=0.f;
		}
	}
	
	// Check if we need to gift any pickups
	if(mBribeDelayTimer > 0.0f)
	{
		mBribeDelayTimer -= C_GameApp::GetInstance()->GetTimeStep();
		if(mBribeDelayTimer <= 0.0f)
		{
			// Actually do the gifting
			CheckBribeAbility(); 

			// For now use the same delay for any front runners set to award on respawn
			if(C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings.m_FrontrunnerNitroOnRespawn)
			{
				CheckFrontRunner(true); 
			}

			mBribeDelayTimer = 0.0f;
		}

	}

	mLasersight.Activate(info.mCurrentlySelectedType == E_PICKUP_TYPE_BLINDFIRE);
	
	if(pActor->mCurrentHealthLevel != E_HEALTH_LEVEL_WRECKED)
	{
		if(info.mPickupStoreIndex != -1)
		{
			//Timer in destruction for Frontrunner (Nitro every X seconds)
			if(mLevelStatePtr->mRaceMode == Game::E_RaceMode_Multiplayer_Destruction)
			{
				mFrontRunnerTimer -= mDeltaTime;
				if(mFrontRunnerTimer <= 0)
				{
					mFrontRunnerTimer = C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings.m_FrontrunnerMotormashTimer;
					CheckFrontRunner();
				}
			}
		}

		if(mLevelStatePtr->mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER && mLevelStatePtr->mCars[mVehicleIndex].mHuman && 
			!mRLCPtr->IsVehicleAIControlled(mVehicleIndex) && !mRLCPtr->IsPaused())
		{
			PERF_EVENT("VLC.Pickups.CheckFiring");
			int controller=mLevelStatePtr->mCars[mVehicleIndex].mController;

			// Double taps are surprisingly rubbish, so we'll disable them for now
			bool do_charging=false;//Input::GetGameInput(controller)->GetButtonPressed(Game::E_Button_UsePickup);

			// Note: GetButtonReleasedDebounced needed for charged shots to work properly
			bool do_pickup=Input::GetGameInput(controller)->GetButtonPressedDebounced(Game::E_Button_UsePickup);
#ifdef PLATFORM_PC
			// [Blur2PC] retail Blur PC (0x0047be80): the forward and backward fire buttons fire too
			bool do_pickup_forward=Input::GetGameInput(controller)->GetButtonPressedDebounced(Game::E_Button_UsePickupForward);
			bool do_pickup_backward=Input::GetGameInput(controller)->GetButtonPressedDebounced(Game::E_Button_UsePickupBackward);
			do_pickup = do_pickup || do_pickup_forward || do_pickup_backward;
#endif
			bool do_discard=Input::GetGameInput(controller)->GetButtonPressedDebounced(Game::E_Button_DiscardPickup);

			bool do_fireslot1=Input::GetGameInput(controller)->GetButtonPressedDebounced(Game::E_Button_FireSlot1);
			bool do_fireslot2=Input::GetGameInput(controller)->GetButtonPressedDebounced(Game::E_Button_FireSlot2);
			bool do_fireslot3=Input::GetGameInput(controller)->GetButtonPressedDebounced(Game::E_Button_FireSlot3);

			bool do_switch=Input::GetGameInput(controller)->GetButtonPressedDebounced( Game::E_Button_SwitchPickup );
			bool do_switch_alt=Input::GetGameInput(controller)->GetButtonPressedDebounced( Game::E_Button_SwitchPickupAlt );
			bool do_switch_alt1=Input::GetGameInput(controller)->GetButtonPressedDebounced( Game::E_Button_UseSecondTierPickup );
			bool do_switch_left=Input::GetGameInput(controller)->GetButtonPressedDebounced( Game::E_Button_SwitchPickupLeft );

			bool do_auto_shuffle=Input::GetGameInput(controller)->GetButtonPressedDebounced( Game::E_Button_Auto_Shuffle_Perks );
			bool do_convert_to_health=Input::GetGameInput(controller)->GetButtonPressedDebounced( Game::E_Button_Convert_To_Health );
			bool do_clone_selected=Input::GetGameInput(controller)->GetButtonPressedDebounced( Game::E_Button_Clone_Selected_Perk );

#ifdef PLATFORM_PC
			// [Blur2PC] retail Blur PC: the fire buttons force the direction; the d-pad reaches the pitch axis instead
			// of the menu pad buttons (SetupAmaxUserControlsPC)
			bool force_backwardFire = Input::GetGameInput(controller)->GetAxisValue(Game::E_Axis_Pitch) <= g_debugBackwardFirePitch || do_pickup_backward;
			bool force_forwardFire	= Input::GetGameInput(controller)->GetAxisValue(Game::E_Axis_Pitch) >= g_debugForwardFirePitch || do_pickup_forward;
#else
			bool force_backwardFire = Input::GetGameInput(controller)->GetAxisValue(Game::E_Axis_Pitch) <= g_debugBackwardFirePitch || Input::GetGameInput(controller)->GetButtonPressed(Game::E_UiButton_Pad_Down);
			bool force_forwardFire	= Input::GetGameInput(controller)->GetAxisValue(Game::E_Axis_Pitch) >= g_debugForwardFirePitch || Input::GetGameInput(controller)->GetButtonPressed(Game::E_UiButton_Pad_Up);
#endif

			// Note if we are in last gasp mode.. the UI would like to know about do_pickup so it can detonate the car when the user presses use pickup
			if(pActor->GetIsInLastGaspMode() && do_pickup)
			{
				_UI_MESSAGE( UI::E_UiGameMsg_UsePickup );
			}


			if( mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.ManualPerkChange() )
			{
				if(do_switch || do_switch_left || do_switch_alt || do_switch_alt1)
				{
					if( ShufflePerks(do_switch_left) )
					{
						if(info.mPickupStore.Size())
							PlaySelectedPerkSfx();

						UI::C_UiGameMsg_ShufflePerks uimsg(mVehicleIndex,false);
						_UI_MESSAGE_FULL( uimsg );
					}
				}
			}

			if(do_fireslot1)
				info.mPickupStoreIndex = 0;
			else if(do_fireslot2)
				info.mPickupStoreIndex = 1;
			else if(do_fireslot3)
				info.mPickupStoreIndex = 2;

			bool chargedShot = false;
			PickupStore* pPickupStore = NULL;
			if(info.mPickupStoreIndex < info.mPickupStore.Size() && info.mPickupStoreIndex >= 0)
				pPickupStore = &info.mPickupStore[info.mPickupStoreIndex];

			if(do_charging && mCanCharge && pPickupStore && CanUseDouble(pPickupStore->mType))
			{
				mChargingTime += mDeltaTime;
				if(mChargingTime > g_debugPerksChargeTime)
				{
					mChargingTime = 0.f;
					chargedShot = true;
					mCanCharge = false;
				}
			}
			else if(mChargingTime > 0.f)
				mChargingTime = 0.f;

			if(pPickupStore)
			{
				bool canUseInstantly = ( mDisablePickups <= 0.5f && (pPickupStore->mType == E_PICKUP_TYPE_REPAIR || pPickupStore->mType == E_PICKUP_TYPE_SHIELD) );
				bool canFire = do_pickup || do_fireslot1 || do_fireslot2 || do_fireslot3;

				if(((canFire && mCanCharge) || chargedShot) && (mDisablePickups == 0.f || canUseInstantly) && !pActor->IsGhosted() && !pActor->IsAwayFromBody())
				{
					ActivatePerk(pPickupStore->mType, pPickupStore->mDroppedByIndex, force_backwardFire, force_forwardFire, chargedShot);
				}
				//else
				//if(((canFire && mCanCharge) || chargedShot) && (mDisablePickups > 0.f) && !pActor->IsGhosted())
				//{
				//		//check all viewports to see if this car has focus
				//		int numViewports = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
				//		for(int i = 0; i < (int)numViewports; i++)
				//		{
				//			if(mVehicleIndex == mRLCPtr->GetViewportFocusVehicleIndex(i))
				//				UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_PickupsFireErrorActionIn );
				//		}
				//}
				else if(do_discard) 
				{
					AttemptDropPickup();
				}

				// Need to do this after we've fired
				if(!do_charging && !mCanCharge)
					mCanCharge = true;
			}
		}
	}

	UpdateHitUpdaters();

	if(info.mPickupStoreIndex == -1 && info.mPickupStore.Size())
	{
		BIZ_ASSERT_FAIL("Pickup Store has got out of sync");
	}

	if(info.mPickupStoreIndex != -1 && info.mPickupStore.Size() == 0)
	{
		BIZ_ASSERT_FAIL("Pickup Store has got out of sync");
	}

	if(mDropPickup)
	{
		DropCurrentPickup();
		mDropPickup = false;
	}

	if(mSafetyNetFXCooldownTimer)
	{
		mSafetyNetFXCooldownTimer-=mDeltaTime;
		if(mSafetyNetFXCooldownTimer <= 0.f)
		{
			mSafetyNetFXCooldownTimer = 0.f;
		}
	}

	if(mRoadsweeperPickupFXCooldownTimer)
	{
		mRoadsweeperPickupFXCooldownTimer-=mDeltaTime;
		if(mRoadsweeperPickupFXCooldownTimer <= 0.f)
		{
			mRoadsweeperPickupFXCooldownTimer = 0.f;
		}
	}

	if(mShieldedBoostersAbsorbFXCooldownTimer)
	{
		mShieldedBoostersAbsorbFXCooldownTimer-=mDeltaTime;
		if(mShieldedBoostersAbsorbFXCooldownTimer <= 0.f)
		{
			mShieldedBoostersAbsorbFXCooldownTimer = 0.f;
		}
	}
}

bool C_VehicleComponent_Pickups::CanUseDouble(E_PICKUP_TYPE p_PickupType)
{
	Game::Actors::C_VehicleLogicActor* pActor	= (Game::Actors::C_VehicleLogicActor*)GetActor();
	bool canUseDouble							= false;

	if((mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.DoubleNitro()) && p_PickupType == E_PICKUP_TYPE_NITROUS)
		canUseDouble = true;
	else if((mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.DoubleShunt()) && p_PickupType == E_PICKUP_TYPE_SHUNT)
		canUseDouble = true;
	else if((mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.DoubleBarge()) && p_PickupType == E_PICKUP_TYPE_BARGE)
		canUseDouble = true;
	else if((mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.DoubleShock()) && p_PickupType == E_PICKUP_TYPE_SHOCK)
		canUseDouble = true;
	else if((mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.DoubleBomb()) && p_PickupType == E_PICKUP_TYPE_BOMB)
		canUseDouble = true;
	else if((mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.DoubleShield()) && p_PickupType == E_PICKUP_TYPE_SHIELD)
		canUseDouble = true;

	if(canUseDouble)
	{
		int numOfType = 0;
		for(int i = 0; i < pActor->mPickupInfo.mPickupStore.Size(); i++)
		{
			if(pActor->mPickupInfo.mPickupStore[i].mType == p_PickupType)
				numOfType++;
		}

		if(numOfType > 1)
			return true;
	}

	return false;
}

#ifndef CFG_GOLD
void C_VehicleComponent_Pickups::ProcessDebugUpdate()
{
	if(g_BribeMeNow)
	{
		//Give a bribe if we've got it equipped
		CheckBribeAbility();

		g_BribeMeNow = false;
	}
}
#endif //CFG_GOLD

C_VehicleComponent_Pickups::~C_VehicleComponent_Pickups()
{
	// Tidy up all the remaining HitUpdaters
	for (int a=0; a < mHitUpdaters.Size(); a++)
	{		
		SAFE_DELETE_PTR(mHitUpdaters[a]);
	}
	mHitUpdaters.Clear();
}

void C_VehicleComponent_Pickups::GiftPickups()
{
	PERF_EVENT("C_VehicleComponent.Pickups.GiftPickups");
	if(mLevelStatePtr->mCars[mVehicleIndex].mPerkGrantingTimer && mTimer_PerkGifting != -1)
	{
		mTimer_PerkGifting -= mDeltaTime;

		if(mTimer_PerkGifting <= 0.0f)
		{
			ForceGift();
			mTimer_PerkGifting = mLevelStatePtr->mCars[mVehicleIndex].mPerkGrantingTimer;
		}
	}
}

void C_VehicleComponent_Pickups::ForceGift()
{
	PERF_EVENT("C_VehicleComponent.Pickups.ForceGift");
	for( int i = 0; i < mLevelStatePtr->mCars[mVehicleIndex].mPerksToGrant.Size(); i++ )
	{
		E_PICKUP_TYPE type = mLevelStatePtr->mCars[mVehicleIndex].mPerksToGrant[i]; 
		int numShotsAwarded = GetNumShotsAwarded(type);
		HitPickUp(type,-1,numShotsAwarded);
	}
}

void C_VehicleComponent_Pickups::DropCurrentPickup()
{
	PERF_EVENT("C_VehicleComponent.Pickups.DropPickup");
	SpawnInformation information;
	if(HandleDropPickupOnTrack(information) != E_PICKUP_TYPE_NONE)
	{
		VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;

		if(info.mPickupStoreIndex != -1)
		{
			S8 power = info.mPickupStore[info.mPickupStoreIndex].mNumShots;
			for(S8 i = 0; i < power; i++)
			{
				RemovePower(1);
			}

			// play audio
			UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_DiscardPickupActionIn );

			// Record this stat
			Game::Actors::C_VehicleLogicActor* pLogicActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
			if( pLogicActor )
			{
				pLogicActor->mStatistics.mNumPowerupsDropped++;
			}
		}
	}
}

void C_VehicleComponent_Pickups::RemovePower(int amount, bool sameType, bool force)
{
	PERF_EVENT("C_VehicleComponent.Pickups.RemovePower");
	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	VehiclePickupInfo& info = pActor->mPickupInfo;

	if(info.mPickupStore.Size() == 0)
		return;

	BIZ_ASSERT_MSG(info.mPickupStoreIndex != -1, "Trying to remove perk power and the index is -1");

	if(info.mPickupStoreIndex+amount > info.mPickupStore.Size())
	{
		BIZ_ASSERT_FAIL("Trying to remove more perk power than we've actually got");
	}

	if(g_debugVehicleAnyIndex == mVehicleIndex && g_debugVehicleAnyReplenish)
		return;

	if(sameType)
	{
		E_PICKUP_TYPE currentType = E_PICKUP_TYPE_NONE;
		if(info.mPickupStoreIndex < info.mPickupStore.Size())
		{
			currentType = info.mPickupStore[info.mPickupStoreIndex].mType;
			info.mPickupStore.EraseByIndex(info.mPickupStoreIndex);
		}

		for(int a = 0; a < info.mPickupStore.Size(); a++)
		{
			if(!force && info.mPickupStore[a].mInfiniteAmmo)
				continue;

			if(currentType == info.mPickupStore[a].mType)
			{
				info.mPickupStore.EraseByIndex(a);
				break;
			}
		}

		mDisablePickups = g_debugPickupsDisableTime;
	}
	else
	{
		for(int a=info.mPickupStoreIndex+(amount-1); a >= info.mPickupStoreIndex; a--)
		{
			info.mPickupStore[a].mNumShots--;
			bool hasShotgun = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_ShotgunBlindfire)->GetActive();

			if (!force && !hasShotgun && (info.mPickupStore[a].mNumShots > 0 || info.mPickupStore[a].mInfiniteAmmo))
				continue;

			info.mPickupStore.EraseByIndex(a);
			mDisablePickups = g_debugPickupsDisableTime;
		}
	}

	if(info.mPickupStore.Size() == 0)
	{
		info.mPickupStoreIndex=-1;
		info.mCurrentlySelectedType = E_PICKUP_TYPE_NONE;
	}

	if(info.mPickupStoreIndex >= info.mPickupStore.Size())
		info.mPickupStoreIndex=info.mPickupStore.Size()-1;

	if(info.mPickupStore.Size() > 0)
		info.mCurrentlySelectedType = info.mPickupStore[info.mPickupStoreIndex].mType;

	HandlePickupSlotChange();

	LOGDEBUG(g_debugPickupMpSyncLogger, "Vehicle index %d has %d pickups with a storeIndex of %d", mVehicleIndex, info.mPickupStore.Size(), info.mPickupStoreIndex);
}

void C_VehicleComponent_Pickups::HandlePickupSlotChange()
{
	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	VehiclePickupInfo& info = pActor->mPickupInfo;


	Game::C_MultiplayerVehicleAbility* pAdditionalBlindfireAbility	= mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots);
	Game::C_MultiplayerVehicleAbility* pBlindfireShotgunAbility		= mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_ShotgunBlindfire);
	Game::C_MultiplayerVehicleAbility* pShockDomesAbility			= mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalShockDomes);

	bool hasBlindfireShotsMod = pAdditionalBlindfireAbility && pAdditionalBlindfireAbility->GetActive();
	bool hasBlindfireShotgunMod = pBlindfireShotgunAbility && pBlindfireShotgunAbility->GetActive();
	bool hasShockDomesMod = pShockDomesAbility && pShockDomesAbility->GetActive();

	if(info.mPickupStoreIndex >= 0 && info.mPickupStoreIndex < info.mPickupStore.Size())
	{
		if(info.mPickupStore[info.mPickupStoreIndex].mType == E_PICKUP_TYPE_SHUNT || info.mPickupStore[info.mPickupStoreIndex].mType == E_PICKUP_TYPE_SHOCK)
			pActor->ForceTargetVehicleEvaluation();

		if(hasBlindfireShotsMod)
		{
			if(info.mPickupStore[info.mPickupStoreIndex].mType == E_PICKUP_TYPE_BLINDFIRE)
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots, true, false, mVehicleIndex);
			else
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots, false, false, mVehicleIndex);
		}

		if(hasBlindfireShotgunMod)
		{
			if(info.mPickupStore[info.mPickupStoreIndex].mType == E_PICKUP_TYPE_BLINDFIRE)
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_ShotgunBlindfire, true, false, mVehicleIndex);
			else
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_ShotgunBlindfire, false, false, mVehicleIndex);
		}

		if(hasShockDomesMod)
		{
			if(info.mPickupStore[info.mPickupStoreIndex].mType == E_PICKUP_TYPE_SHOCK)
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalShockDomes, true, false, mVehicleIndex);
			else
				mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalShockDomes, false, false, mVehicleIndex);
		}
	}
	else
	{
		if(hasBlindfireShotsMod)
			mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots, false, false, mVehicleIndex);

		if(hasBlindfireShotgunMod)
			mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_ShotgunBlindfire, false, false, mVehicleIndex);

		if(hasShockDomesMod)
			mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalShockDomes, false, false, mVehicleIndex);
	}
}

void C_VehicleComponent_Pickups::ActivatePerk(E_PICKUP_TYPE pickup, char dropped_by_index, bool forceBackwardFire, bool forceForwardFire, bool forceDoubleFire)
{
	PERF_EVENT("C_VehicleComponent.Pickups.ActivatePerk");
	if(pickup == E_PICKUP_TYPE_NONE)
		return;

	if(!StartPerk(pickup, forceBackwardFire, forceForwardFire, forceDoubleFire))
		return;

	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;

	//[mtaylor] - putting this here for now to get it working, is there a better place for this?
	if( mLevelStatePtr->mPerkDroppingWhenUsed )
	{
		DropAndRemoveUsedPickup(pickup);
	}
	else
	{
		int numToRemove = (forceDoubleFire ? 2 : 1);
		RemovePower(numToRemove, forceDoubleFire);

		// [mtaylor] let's also do a catch-all sync if using the "shields are pickups" system
		if(mLevelStatePtr->mShieldsTiedToPickups || g_ForceShieldsTiedToPickups)
		{
			((Game::Actors::C_VehicleLogicActor*)GetActor())->mFanPower = (float)info.mPickupStore.Size();
		}
	}

	// Reset these counters for the Safety Net mod
	if(pickup == E_PICKUP_TYPE_SHIELD)
	{
		mSafetyNetNumBlindfiresAbsorbed = 0;
		mSafetyNetNumTimesAbsorbed = 0;
	}

#ifndef CFG_GOLD
	Game::C_ReplayDirector::Instance().AddPickupStore(mVehicleIndex);
#endif
}

void C_VehicleComponent_Pickups::DropAndRemoveUsedPickup(E_PICKUP_TYPE pickup)
{
	SpawnInformation information;
	HandleDropPickupOnTrack(information);
}

bool C_VehicleComponent_Pickups::HitPickUp(E_PICKUP_TYPE type, char dropped_by_index, int numShots, bool infinite_shots, bool checkReservedSlots)
{
	PERF_EVENT("C_VehicleComponent.Pickups.HitPickUp");

	if(type == E_PICKUP_TYPE_FAN || type == E_PICKUP_TYPE_STOPWATCH)
		return false;

	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;

	if(!checkReservedSlots && info.mPickupStore.Size() < mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.NumPickupSlots())
	{
		//[mtaylor] - also, for Craig's new shield system, add a shield when we pick up a perk
		if( mLevelStatePtr->mShieldsTiedToPickups || g_ForceShieldsTiedToPickups )
		{
			((Game::Actors::C_VehicleLogicActor*)GetActor())->mFanPower += 1.f;
		}

		if(mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.CollectAnyPerkType())
		{
			AddPickUp(type, dropped_by_index, numShots, infinite_shots);
			return true;
		}
		else
		{
			if(info.mPickupStore.Size() == 0 || type == info.mPickupStore[0].mType)
			{
				AddPickUp(type, dropped_by_index, numShots, infinite_shots);
				return true;
			}
		}
	}
	//When we are gifted them we reserve a slot in advance
	else if(checkReservedSlots)
	{
		for(U8 i = 0; i < info.mPickupStore.Size(); i++)
		{
			if(info.mPickupStore[i].mType == E_PICKUP_TYPE_NONE)
			{
				info.mPickupStore[i].mType = type;
				info.mPickupStore[i].mInfiniteAmmo = false;
				break;
			}
		}
	}

	return false;
}

void C_VehicleComponent_Pickups::AddPickUp(E_PICKUP_TYPE type, char dropped_by_index, int numShots, bool infinite_shots)
{
	PERF_EVENT("C_VehicleComponent.Pickups.AddPickUp");

	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;
	if(info.mPickupStore.Size() == 0)
	{
		info.mPickupStoreIndex=0;
		info.mCurrentlySelectedType=type;
	}

	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

	PickupStore pus;
	pus.mType=type;
	pus.mDroppedByIndex=dropped_by_index;
	pus.mNumShots = numShots;
	pus.mInfiniteAmmo = infinite_shots;
	pus.mPickupGroupId = mGroupPickupId++;
	pus.mDistancePickedUp = pActor->GetPositionInfo().mLogicPositionCombined;
	
	if(g_debugPerksAutoShuffle)
	{
		bool inserted=false;
		for(int a=0;a<info.mPickupStore.Size();a++)
		{
			if(info.mPickupStore[a].mType == type)
			{
				// re-size the array
				info.mPickupStore.Resize(info.mPickupStore.Size()+1);

				//shuffle everything up one
				for(int b=info.mPickupStore.Size()-1;b>a;b--)
				{
					info.mPickupStore[b+1].mDroppedByIndex=info.mPickupStore[b].mDroppedByIndex;
					info.mPickupStore[b+1].mType=info.mPickupStore[b].mType;
					info.mPickupStore[b+1].mInfiniteAmmo=info.mPickupStore[b].mInfiniteAmmo;
				}

				//put it the new one in place
				info.mPickupStore[a+1].mDroppedByIndex=pus.mDroppedByIndex;
				info.mPickupStore[a+1].mType=pus.mType;
				info.mPickupStore[a+1].mInfiniteAmmo=infinite_shots;
				
				inserted=true;
				
				break;
			}
		}

		if(!inserted)
		{
			info.mPickupStore.PushBack(pus);
		}
	}
	else
	{
		info.mPickupStore.PushBack(pus);
	}

	HandlePickupSlotChange();

#ifndef CFG_GOLD
	Game::C_ReplayDirector::Instance().AddPickupStore(mVehicleIndex);
#endif

	UI::C_UiGameMsg_PickedupAPerk uimsg(mVehicleIndex,type);
	_UI_MESSAGE_FULL( uimsg );
}

void C_VehicleComponent_Pickups::DelayGiftPickUp(E_PICKUP_TYPE type, F32 secondsToDelay, U8 numShots)
{
	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	if(pActor)
	{
		HitPickUp(E_PICKUP_TYPE_NONE,-1,numShots,true);
		Game::Actors::C_GrantPickupMsg msg(mVehicleIndex, type, false, true);
		pActor->GetActorList()->DelayMessageToActorBySeconds(secondsToDelay, pActor->GetHandle(), &msg);
	}
}

bool C_VehicleComponent_Pickups::StartPerk(E_PICKUP_TYPE p_PickupType, bool forceBackwardFire, bool forceForwardFire, bool forceDoubleFire)
{
	PERF_EVENT("C_VehicleComponent.Pickups.StartPerk");

	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;
	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

	// If we are firing something other than a blindfire after a blindfire or one of our insta-fires, check our longer delay
	bool canAlwaysFire = (p_PickupType == E_PICKUP_TYPE_SHIELD || p_PickupType == E_PICKUP_TYPE_REPAIR);
	bool canFireBlindfire = p_PickupType == E_PICKUP_TYPE_BLINDFIRE && mLastFiredPerk == E_PICKUP_TYPE_BLINDFIRE;

	if( !canAlwaysFire && !canFireBlindfire && mDisableSlowFiringPickups > 0.f )
	{
		return false;
	}

	mLastFiredPerk	= p_PickupType;
	mUsingPerkLevel = 0;

	if(forceDoubleFire)
	{
		mUsingPerkLevel = 1;
	}

	if(p_PickupType == E_PICKUP_TYPE_BLINDFIRE)
	{
		mDisablePickups = g_debugPickupsDisableTimeBlindfireToBlindfire;
		mDisableSlowFiringPickups = g_debugPickupsDisableTimeBlindfireToOther;
	}
	else
		mDisablePickups = g_debugPickupsDisableTime;

	info.mCurrentlyUsingType=p_PickupType;
	info.mCurrentlyUsingLevel=mUsingPerkLevel;

	RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(p_PickupType);
	int targetIdx						=info.mCurrentTargetIdx;

	E_PICKUP_SUB_TYPE pickupSubType = E_PICKUP_SUB_TYPE_NORMAL;

	if(forceForwardFire && p_PickupType == E_PICKUP_TYPE_BOMB)
		pickupSubType = mUsingPerkLevel ? E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP : E_PICKUP_SUB_TYPE_REVERSED;
	else if(forceBackwardFire && p_PickupType != E_PICKUP_TYPE_BOMB)
		pickupSubType = mUsingPerkLevel ? E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP : E_PICKUP_SUB_TYPE_REVERSED;
	else
		pickupSubType = mUsingPerkLevel ? E_PICKUP_SUB_TYPE_DOUBLE_TAP : E_PICKUP_SUB_TYPE_NORMAL;	

	RLPickupLevel* pLevel = GetRLSubTypeLevel(p_PickupType, pickupSubType);
	if(!pLevel)
		pLevel = pPickup->mNormal; // We're assuming this always exists - if not, our data is rather broken.

	Game::PickupCreationStruct pickupInfo(p_PickupType, pickupSubType, mVehicleIndex);
	pickupInfo.mDelay		= pLevel->mBalancingData->mDelay; 
	pickupInfo.mTargetIdx	= targetIdx;

	//	We only use the group ID for a couple of Bolt challenges/stickers - so leave it as zero otherwise
	if(info.mPickupStoreIndex >= 0 && info.mPickupStoreIndex < info.mPickupStore.Size() && p_PickupType == E_PICKUP_TYPE_BLINDFIRE)
		pickupInfo.mGroupPickupId = info.mPickupStore[info.mPickupStoreIndex].mPickupGroupId;
	else
		pickupInfo.mGroupPickupId = 0;

	switch(p_PickupType)
	{
		case E_PICKUP_TYPE_REPAIR:
		{
			//	Repair is an instant effect on the firing vehicle, so rather than create a pickup we'll just go straight to the "hit" effect
			Game::PickupHitInfo hitInfo;
			RegisterHitByPickup(E_PICKUP_TYPE_REPAIR, E_PICKUP_SUB_TYPE_NORMAL, E_PICKUP_HIT_TYPE_MINOR, mVehicleIndex, 0, E_TARGETTING_DIRECTION_NONE, false, hitInfo, 0);
			break;
		}

		case E_PICKUP_TYPE_SHIELD:
		{
			// We don't want a double to just become an extended single if a single is running.
			//bool isDoubleTap = pickupInfo.mPickupSubType == E_PICKUP_SUB_TYPE_DOUBLE_TAP || pickupInfo.mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP;
			//bool overrideDoubleAttempt = (!pActor->mDoubleShield && isDoubleTap);

			if(pActor->mShieldTime > 0.f /*&& !overrideDoubleAttempt*/)
			{
				Game::C_PickupShield* pShield = (Game::C_PickupShield*)Game::C_PickupsManager::Instance().GetPickup(pActor->mLastShieldId);

				if(pShield)
				{
					pShield->Restart();
				}
			}
			else
			{
				Game::C_MultiplayerVehicleAbility* pAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_DoubleShield);
				if(pAbility->GetActive() || g_debugPerksForceDoubleShield)
				{
					pickupInfo.mPickupSubType = E_PICKUP_SUB_TYPE_DOUBLE_TAP;
				}
				Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
			}

			CheckShieldedBoostersActivated();
			break;
		}

		case E_PICKUP_TYPE_SUPER_SHIELD:
		{
			Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
			break;
		}


		case E_PICKUP_TYPE_NITROUS:
			{
				if(mUsingPerkLevel || g_debugPerksForceDoubleNitrous || mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_NitroRift)->GetActive())
				{
					if(pickupSubType == E_PICKUP_SUB_TYPE_NORMAL)
					{
						pickupInfo.mPickupSubType = E_PICKUP_SUB_TYPE_DOUBLE_TAP;
						U32 id = Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
					}
				}

				if(pickupSubType == E_PICKUP_SUB_TYPE_NORMAL)
				{
					Game::PickupHitInfo hitInfo;

					E_PICKUP_SUB_TYPE nitrousSubType = pickupSubType;
					if(nitrousSubType == E_PICKUP_SUB_TYPE_DOUBLE_TAP)
						nitrousSubType = E_PICKUP_SUB_TYPE_NORMAL;

					RegisterHitByPickup(E_PICKUP_TYPE_NITROUS, nitrousSubType, E_PICKUP_HIT_TYPE_MINOR, mVehicleIndex, 0, E_TARGETTING_DIRECTION_NONE, false, hitInfo, 0);
				}
				else if(pickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
				{
					U32 id = Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
					mUsingPerkLevel ? mLastFiredPickupId = 0 : mLastFiredPickupId = id;
				}
				
				break;
			}

		case E_PICKUP_TYPE_BOMB:
			{
				if(mUsingPerkLevel)
				{
					pickupInfo.mTargetIdx		= targetIdx;
					pickupInfo.mParentPickupId	= mLastFiredPickupId;
					Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);
				}
				else
				{
					pickupInfo.mVal2 = -1.f;
					U32 id = Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);

					mUsingPerkLevel ? mLastFiredPickupId = 0 : mLastFiredPickupId = id;
				}

				break;
			}

		case E_PICKUP_TYPE_SHOCK:
			{
				pickupInfo.mPickupSubType = E_PICKUP_SUB_TYPE_DOUBLE_TAP; // TEMP

				U32 id = Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);

				mUsingPerkLevel ? mLastFiredPickupId = 0 : mLastFiredPickupId = id;

				break;
			}

		case E_PICKUP_TYPE_BLINDFIRE:
			{
				Game::C_MultiplayerVehicleAbility* pAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_ShotgunBlindfire);

				if(pAbility->GetActive() || g_debugPerksForceShotgunBlindfire)
				{
					// This doesn't work in SP yet it seems
					//mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_ShotgunBlindfire, true);

					if(pickupInfo.mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
						pickupInfo.mPickupSubType = E_PICKUP_SUB_TYPE_MOD_REVERSED;
					else
						pickupInfo.mPickupSubType = E_PICKUP_SUB_TYPE_MOD;
				}

				U32 id = Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);

				mUsingPerkLevel ? mLastFiredPickupId = 0 : mLastFiredPickupId = id;
				break;
			}

		default:
			{
				U32 id = Game::C_PickupsManager::Instance().CreatePickup(pickupInfo);

				mUsingPerkLevel ? mLastFiredPickupId = 0 : mLastFiredPickupId = id;
				break;
			}
	}

	UI::C_UiGameMsg_StartedAPerk uimsg(mVehicleIndex,p_PickupType);
	_UI_MESSAGE_FULL( uimsg );

	return true;
}


void C_VehicleComponent_Pickups::RegisterShieldedPickup( E_PICKUP_TYPE pickupType, int firingIdx, U32 pickupId )
{
	PERF_EVENT("C_VehicleComponent.Pickups.RegisterShieldedPickup");

	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

	C_VehicleLogicMsg_ShieldedPerkMsg shieldPerkMsg(pickupType, firingIdx, mVehicleIndex, 0, pickupId, pActor->mShieldTime);

	pActor->GetActorList()->SendMessageToActor(mRaceLogicLayerPtr->GetLogicVehicleHandle(mVehicleIndex), &shieldPerkMsg);
	// If we hit our own pickup and shielded.. dont send the shielded message to ourselves twice!
	if(mVehicleIndex != firingIdx)
	{
		pActor->GetActorList()->SendMessageToActor(mRaceLogicLayerPtr->GetLogicVehicleHandle(firingIdx), &shieldPerkMsg);
	}


	Game::C_StickerManager::Instance().MessageHandler(&shieldPerkMsg);
	//Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, ActorMsg::E_AudioMsg_Pickups_Blocked,0 , &shieldPerkMsg);

	RLPickupType* pPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(pickupType);
	if(pPickup)
	{
		Game::Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mVehicleIndex);
		AmaxVehicle* pXtVehicle					= C_GameVehicleData::Instance().mVehicles[(GAMEDATA_ID)pVehActor->GetXtIndex()];

		Maths::HVec3 extents = pVehActor->GetExtents();
		Maths::HVec3 scaleTest(	g_debugShieldScaleX * extents.X() * (pXtVehicle->m_ShieldSizeModifier.vx ? pXtVehicle->m_ShieldSizeModifier.vx : 1.f), 
								g_debugShieldScaleY * extents.Y() * (pXtVehicle->m_ShieldSizeModifier.vy ? pXtVehicle->m_ShieldSizeModifier.vy : 1.f),
								g_debugShieldScaleZ * extents.Z() * (pXtVehicle->m_ShieldSizeModifier.vz ? pXtVehicle->m_ShieldSizeModifier.vz : 1.f) );

		C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(pickupType, 0, mVehicleIndex, firingIdx, pickupId, pickupId, -1, 0);		
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Hit, pActor->mShieldHealth, &hit_by_msg);

		if(!CheckShieldedBoostersAbsorbDamage(pickupType, firingIdx) || g_ShieldedBoostersUseExistingEffect)
		{
			Game::C_MultiplayerVehicleAbility* safetyNetAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Safety_Net);
			XtArray<RLPickupSpecialFX>& sfx = pPickup->mNormal->mSpecialFX->mShieldedSpecialFX;
			if(safetyNetAbility->GetActive() && mSafetyNetFXCooldownTimer <= 0.001f)
			{
				// Firstly check if we can absorb this pickup type
				if( CanSafetyNetAbsorbPickupType( pickupType ) )
				{	
					switch(pickupType)
					{
						case E_PICKUP_TYPE_BARGE:
							sfx = ((RLBargeSpecialFX*)pPickup->mNormal->mSpecialFX)->mSafetyNetShieldedSpecialFX;
							break;
						case E_PICKUP_TYPE_BLINDFIRE:
							sfx = ((RLBlindfireSpecialFX*)pPickup->mNormal->mSpecialFX)->mSafetyNetShieldedSpecialFX;
							break;
						case E_PICKUP_TYPE_BOMB:
							sfx = ((RLBombSpecialFX*)pPickup->mNormal->mSpecialFX)->mSafetyNetShieldedSpecialFX;
							break;
						case E_PICKUP_TYPE_SHOCK:
							sfx = ((RLShockSpecialFX*)pPickup->mNormal->mSpecialFX)->mSafetyNetShieldedSpecialFX;
							break;
						case E_PICKUP_TYPE_SHUNT:
							sfx = ((RLShuntSpecialFX*)pPickup->mNormal->mSpecialFX)->mSafetyNetShieldedSpecialFX;
							break;
					}	

					mSafetyNetFXCooldownTimer = 2.0f;
				}
			}

			Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(sfx, pVehActor->GetMatrix(), pVehActor->GetVelocity(), pVehActor->GetGroundPosition(), pVehActor->GetGroundNormal(), mVehicleIndex, mVehicleIndex, -1, Game::E_InGameSpecialFX_Trigger_HitByPerk, 
				-1.f, scaleTest);
		}
	}

	Game::C_PickupBase* pPickupBase = Game::C_PickupsManager::Instance().GetPickup(pickupId);

	if(pPickupBase)
		pPickupBase->RegisterVehicleShielded(mVehicleIndex);

	// Shield no longer takes damage.
	if(pickupType == E_PICKUP_TYPE_SHOCK)
		pActor->mShieldHealth -= pPickup->mNormal->mBalancingData->GetDamageInflicted();

	//pActor->mShieldTime   -= pPickup->mNormal->mBalancingData->GetDamageInflicted() * g_debugShieldDamageTimeScale;

#ifndef CFG_GOLD
	Game::C_ReplayDirector::Instance().AddShieldedPickup(mVehicleIndex, pickupType);
#endif // ifndef CFG_GOLD
}

//	A pickup object has collided/triggered/whatever with this vehicle, so we need to check if it's shielded or has some other immunity
void C_VehicleComponent_Pickups::AttemptHitByPickup( E_PICKUP_TYPE pickupType,E_PICKUP_SUB_TYPE pickupSubType, E_PICKUP_HIT_TYPE hitType, int firingIdx, E_TARGETTING_DIRECTION hitDir, U32 pickupId, bool noPhysical, Game::PickupHitInfo hitInfo, U32 groupId )
{
	PERF_EVENT("C_VehicleComponent.Pickups.AttemptHitByPickup");

	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

	float timeShielded = pActor->mShieldTime;

	if( timeShielded > 0.f && pickupType != E_PICKUP_TYPE_NITROUS)
		RegisterShieldedPickup(pickupType, firingIdx, pickupId);
	else
		RegisterHitByPickup(pickupType, pickupSubType, hitType, firingIdx, pickupId, hitDir, noPhysical, hitInfo, groupId);
}

bool C_VehicleComponent_Pickups::RegisterHitByPickup( E_PICKUP_TYPE pickupType, E_PICKUP_SUB_TYPE subType, E_PICKUP_HIT_TYPE hitType, int firingIdx, U32 pickupId, E_TARGETTING_DIRECTION hitDir, bool noPhysical, Game::PickupHitInfo hitInfo, U32 groupId )
{
	PERF_EVENT("C_VehicleComponent.Pickups.RegisterHitByPickup");

	C_VehicleLogicMsg_HitByPerkMsg* hit_by_msg = NULL;
	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

	pActor->mPickupHitPosition	= hitInfo.mPosition;
	pActor->mPickupHitVector	= hitInfo.mDirection;

	bool createHitByMsg = false;
	bool ignoreSFX = false;
	bool teammates = Game::VehiclesAreTeammates(mVehicleIndex, firingIdx);
	XtArray<RLPickupSpecialFX>* sfxOverride = NULL;
	RLPickupLevel* pPickupLevel = GetRLSubTypeLevel(pickupType, subType);

	U32 id = -1;

	// Always check for decoy collision first. We ignore the E_PICKUP_TYPE for deyoy collisions. 
	if( subType == E_PICKUP_SUB_TYPE_DECOY )
	{
		if(!teammates)
		{
			pActor->ResetHitBy(firingIdx);

			if(Game::VehicleIsLocal(mVehicleIndex))
			{
				Game::Actors::HitDecoyMsg decoyMsg(mVehicleIndex, firingIdx);
				pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &decoyMsg);
			}

			if(!noPhysical)
			{
				MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;

				pActor->GetGraphicalActor(mVehicleIndex)->GetFakeSpeed()->Start(Game::Actors::E_FAKE_SPEED_BRAKE, pConfig->m_MiscModSettings.m_DecoyDropbrakeTarget);
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_360_LEFT);
			}

			createHitByMsg = true;
		}
	}
	else
	{
		switch(pickupType)
		{
		case E_PICKUP_TYPE_SHUNT:
			{
				DisableActiveNitrous(false);

				if(!teammates)
				{
					if(!noPhysical)
					{
						if(hitType == E_PICKUP_HIT_TYPE_MAJOR) // Normal strength
						{
							if(mDisablePickups < g_debugPerksDisableWhenHitByShunt)
								mDisablePickups = g_debugPerksDisableWhenHitByShunt;

							pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_FLIP_FORWARD);
						}
						else // Weak strength
						{
							pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_TIP_FORWARD);
						}
					}

					createHitByMsg = true;
				}
				break;
			}

		case E_PICKUP_TYPE_SHOCK:
			{
				createHitByMsg = true;
				if(!noPhysical)
				{
					RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHOCK);
					RLBalancingGraph* pGraph			= &pPickup->mNormal->mBalancingGraph;

					float speed		= pActor->GetPositionInfo().mSpeedInMph;
					float graphVal	= 200.f; // 1.0f on the graph == 200mph
					float speedVal	= (pActor->GetPositionInfo().mSpeedInMph / graphVal);

					Game::Actors::C_VehicleActor* pGraphicalActor		= pActor->GetGraphicalActor(mVehicleIndex);
					Game::Actors::FakeVehiclePhysicsState* fakePhysics	= pGraphicalActor->GetFakePhysics();

					float newSpeed = pGraph->mEffectCurve.Evaluate(speedVal) * graphVal;

					fakePhysics->IncrementConsecutiveHits();


					float recoveryTimeSpeedMod=0.1f;
					const RLPickupParametersStoredOnline* ppso=RLDatabase::Instance().GetPickupParametersStoredOnline();
					if (ppso)
					{
						recoveryTimeSpeedMod=ppso->mRecoveryTimeSpeedMod;
					}

					for( int i = 1; i <= fakePhysics->_numConsecutiveHits; i++ )
					{
						newSpeed += recoveryTimeSpeedMod * speed;
					}
					
					Maths::Clamp(newSpeed, newSpeed, speed - g_debugShockManagerMinSpeedScrub);
					pGraphicalActor->GetFakeSpeed()->Start(Game::Actors::E_FAKE_SPEED_BRAKE, newSpeed);
				}

				break;
			}

		case E_PICKUP_TYPE_BLINDFIRE:
			{
				if(!teammates)
				{
					if(!noPhysical)
					{
						ApplyBlindfireSpin(false);
					}

					createHitByMsg = true;
				}

				break;
			}

		case E_PICKUP_TYPE_BARGE:
			{
				if(!teammates)
				{
					if(!noPhysical)
					{
						switch(hitDir)
						{
						case E_TARGETTING_DIRECTION_LEFT:
							{
								if(subType == E_PICKUP_SUB_TYPE_MOD)
								{
									pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_ROLL_RIGHT);
								}
								else
								{
									if(hitType == E_PICKUP_HIT_TYPE_MAJOR)
										pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_RIGHT);
									else
										pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_MINOR_RIGHT);
								}
								break;
							}

						case E_TARGETTING_DIRECTION_RIGHT:
							{
								if(subType == E_PICKUP_SUB_TYPE_MOD)
								{
									pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_ROLL_LEFT);
								}
								else
								{
									if(hitType == E_PICKUP_HIT_TYPE_MAJOR)
										pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_LEFT);
									else
										pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_BARGE_MINOR_LEFT);
								}

								break;
							}
						}
					}

					createHitByMsg = true;
				}

				break;
			}

		case E_PICKUP_TYPE_NITROUS:
			{
				if(!noPhysical)
				{
					switch(subType)
					{
					case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
						{
							switch(hitDir)
							{
							case E_TARGETTING_DIRECTION_LEFT:
								pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_NITROUS_RIGHT);
								break;

							case E_TARGETTING_DIRECTION_RIGHT:
								pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_NITROUS_LEFT);
								break;
							}

							pickupId = Game::C_PickupsManager::Instance().AllocateUniqueId(firingIdx);
							//create uniqueID for audio
							U32 vehicleUniqueId = Game::C_PickupsManager::Instance().AllocateUniqueId(mVehicleIndex);
							hit_by_msg = new C_VehicleLogicMsg_HitByPerkMsg(pickupType, 0, mVehicleIndex, firingIdx, pickupId, -1, mVehicleIndex, groupId,subType,hitType);

							Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Hit, 0, hit_by_msg);
							Game::C_StickerManager::Instance().MessageHandler(hit_by_msg); // also needed for stickers

							SAFE_DELETE_PTR(hit_by_msg);
							break;
						}

					case E_PICKUP_SUB_TYPE_NORMAL:
						{
							DisableActiveNitrous(true);

							Game::C_PickupEffectsNitrous* pNitrous = new Game::C_PickupEffectsNitrous(E_PICKUP_TYPE_NITROUS, mVehicleIndex);

							id = Game::C_PickupsManager::Instance().AllocateUniqueId(mVehicleIndex);
							pNitrous->SetUniqueID(id);

							hit_by_msg = new C_VehicleLogicMsg_HitByPerkMsg(pickupType, 0, mVehicleIndex, firingIdx, id, -1, mVehicleIndex, groupId);

							Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Hit, 0, hit_by_msg);
							Game::C_StickerManager::Instance().MessageHandler(hit_by_msg); // also needed for stickers

							SAFE_DELETE_PTR(hit_by_msg);

							mHitUpdaters.PushBack(pNitrous);
							break;
						}

					case E_PICKUP_SUB_TYPE_REVERSED:
						{
							if(!teammates)
							{
								if(hitType == E_PICKUP_HIT_TYPE_MAJOR)
								{
									DisableActiveNitrous(true);

									Game::C_PickupEffectsNitrousReverse* pRevNitrousEffects = new Game::C_PickupEffectsNitrousReverse(E_PICKUP_TYPE_NITROUS, mVehicleIndex);
									mHitUpdaters.PushBack(pRevNitrousEffects);
									ignoreSFX = true;

									//create uniqueID for audio
									U32 vehicleUniqueId = Game::C_PickupsManager::Instance().AllocateUniqueId(mVehicleIndex);
									hit_by_msg = new C_VehicleLogicMsg_HitByPerkMsg(pickupType, 0, mVehicleIndex, firingIdx, vehicleUniqueId, -1, mVehicleIndex, groupId,subType);

									Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Hit, 0, hit_by_msg);
									Game::C_StickerManager::Instance().MessageHandler(hit_by_msg); // also needed for stickers

									SAFE_DELETE_PTR(hit_by_msg);

									pRevNitrousEffects->SetUniqueID(vehicleUniqueId);
								}
								else
								{
									createHitByMsg = true;

									switch(hitDir)
									{
									case E_TARGETTING_DIRECTION_LEFT:
										pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_NITROUS_RIGHT);
										pickupId = Game::C_PickupsManager::Instance().AllocateUniqueId(firingIdx);
										break;

									case E_TARGETTING_DIRECTION_RIGHT:
										pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_NITROUS_LEFT);
										pickupId = Game::C_PickupsManager::Instance().AllocateUniqueId(firingIdx);
										break;
									}
								}
							}

							break;
						}

					}
				}

				break;
			}

		case E_PICKUP_TYPE_BOMB:
			{
				if(!teammates)
				{
					if(!noPhysical)
					{
						switch(subType)
						{
						case E_PICKUP_SUB_TYPE_MINI:
							{
								ApplyBlindfireSpin(true);
								break;
							}

						default:
							{
								if(mDisablePickups < g_debugPerksDisableWhenHitByBomb)
									mDisablePickups = g_debugPerksDisableWhenHitByBomb;

								DisableActiveNitrous(false);
								pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_360_LEFT);
								break;
							}
						}
					}

					createHitByMsg = true;
				}
				break;
			}

		case E_PICKUP_TYPE_SHIELD:
			{
				break;
			}

		case E_PICKUP_TYPE_REPAIR:
			{
				RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_REPAIR);
				RLRepairBalancingData* pRepairData	= (RLRepairBalancingData*)pPickup->mNormal->mBalancingData;
				const Actor::C_ActorHandle& handle	= GetActor()->GetHandle();
				Actor::C_ActorList* pActorList		= GetActorList();

				BIZ_ASSERT(pRepairData);

				C_VehicleLogicRepairMsg repairVehicleMsg(mVehicleIndex, pRepairData->mAmountToRepair);

				pActorList->SendMessageToActorComponent(handle, E_IF_VehicleComponent_Damage, &repairVehicleMsg);	
				pActorList->SendMessageToActorComponent(handle, E_IF_VehicleComponent_NewScoring, &repairVehicleMsg);

				//create msg for audio
				U32 vehicleUniqueId = Game::C_PickupsManager::Instance().AllocateUniqueId(mVehicleIndex);
				hit_by_msg = new C_VehicleLogicMsg_HitByPerkMsg(pickupType, 0, mVehicleIndex, firingIdx, vehicleUniqueId, -1, mVehicleIndex, groupId);
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Hit, 0, hit_by_msg);
				Game::C_StickerManager::Instance().MessageHandler(hit_by_msg); // also needed for stickers
				
				if(GetActor()->QueryInterface(E_IF_VehicleComponent_Challenges))
				{
					pActorList->SendMessageToActorComponent(handle, E_IF_VehicleComponent_Challenges, hit_by_msg);	
				}

				SAFE_DELETE_PTR(hit_by_msg);
				break;
			}
		}

	}

	if(createHitByMsg)
		hit_by_msg = new C_VehicleLogicMsg_HitByPerkMsg(pickupType, 0, mVehicleIndex, firingIdx, pickupId, pickupId, -1, groupId, subType, hitType);

	if(!ignoreSFX)
	{
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		Game::Actors::C_VehicleActor* pVehicleActor = pActor->GetGraphicalActor(mVehicleIndex);

		bool titaniumArmour = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Titanium_Armour)->GetActive() &&
								Game::C_Game::GetInstance()->GetMultiplayerAbilities().TitaniumArmourActivatedOnPickupType(pickupType);

		if(titaniumArmour && id == -1)
		{
			id = Game::C_PickupsManager::Instance().AllocateUniqueId(mVehicleIndex);
		}

		if(sfxOverride)
		{
			Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(*sfxOverride, 
				pVehicleActor->GetMatrix(), pVehicleActor->GetVelocity(), pVehicleActor->GetGroundPosition(), pVehicleActor->GetGroundNormal(), firingIdx, mVehicleIndex, id, Game::E_InGameSpecialFX_Trigger_HitByPerk);
		}
		else if(pPickupLevel && pPickupLevel->mSpecialFX)
		{
			if(subType == E_PICKUP_SUB_TYPE_DECOY)
			{
				Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(pPickupLevel->mSpecialFX->mHitVehicleSpecialFXDecoy, 
					pVehicleActor->GetMatrix(), pVehicleActor->GetVelocity(), pVehicleActor->GetGroundPosition(), pVehicleActor->GetGroundNormal(), firingIdx, mVehicleIndex, id, Game::E_InGameSpecialFX_Trigger_HitByPerk);
			}
			else
			{
				//Check for being hit by a splash damage barge
				if(pickupType == E_PICKUP_TYPE_BARGE && levelState.mCars[firingIdx].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Splash_Damage)->GetActive())
				{
					Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(((RLBargeSpecialFX*)pPickupLevel->mSpecialFX)->mSplashDamageVehicleHitSpecialFX, 
						pVehicleActor->GetMatrix(), pVehicleActor->GetVelocity(), pVehicleActor->GetGroundPosition(), pVehicleActor->GetGroundNormal(), firingIdx, mVehicleIndex, id, Game::E_InGameSpecialFX_Trigger_HitByPerk);
				}
				else
				{
					Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(pPickupLevel->mSpecialFX->mHitVehicleSpecialFX, 
						pVehicleActor->GetMatrix(), pVehicleActor->GetVelocity(), pVehicleActor->GetGroundPosition(), pVehicleActor->GetGroundNormal(), firingIdx, mVehicleIndex, id, Game::E_InGameSpecialFX_Trigger_HitByPerk);
				}
			}
		}

		if(titaniumArmour && id != -1)
		{	
			Game::C_InGameSpecialFXManager::Instance().SetIntensity(id, g_debugTitaniumArmourIntensity);
		}
	}


#ifndef CFG_GOLD
	//	The Replay system should know about this
	Game::C_ReplayDirector::Instance().AddPickupHit(mVehicleIndex, pickupType);
#endif

	if(hit_by_msg)
	{
		//	We've been hit by an offensive pickup here, so we set the index of the vehicle which hit us and start the various aggressive scoring timers
		pActor->ResetHitBy(firingIdx);
		Components::C_VehicleComponent_NewScoring* pScoring = (Components::C_VehicleComponent_NewScoring*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_NewScoring);
		if(pScoring)
		{
			pScoring->ResetHitByPerkRecentlyTimer(firingIdx);
		}

		pActor->GetActorList()->BroadcastMessage(hit_by_msg);
		Game::C_StickerManager::Instance().MessageHandler(hit_by_msg);

		Game::C_RaceModeBase* raceMode = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceModePtr();
		if(raceMode->GetID() == Game::E_RaceMode_Multiplayer_Destruction ||
			raceMode->GetID() == Game::E_RaceMode_Destruction ||
			raceMode->GetID() == Game::E_RaceMode_FriendDemand_Destruction )
		{
			raceMode->MessageHandler(hit_by_msg);
		}

		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Hit, 0, hit_by_msg);


		SAFE_DELETE_PTR(hit_by_msg);
	}

	return true;
}

void C_VehicleComponent_Pickups::DisableActiveNitrous(bool slowdownOnly)
{
	for( int i = 0; i < mHitUpdaters.Size(); i++ )
	{
		if(mHitUpdaters[i] && mHitUpdaters[i]->GetType() == E_PICKUP_TYPE_NITROUS)
		{
			switch(mHitUpdaters[i]->GetSubType())
			{
			case E_PICKUP_SUB_TYPE_NORMAL:
				{
					Game::C_PickupEffectsNitrous* pNitroUpdater = (Game::C_PickupEffectsNitrous*)mHitUpdaters[i];
					pNitroUpdater->DisableSlowdown();

					if(!slowdownOnly)
						pNitroUpdater->KillEffect();

					break;
				}

			case E_PICKUP_SUB_TYPE_REVERSED:
				{
					Game::C_PickupEffectsNitrousReverse* pNitroUpdater = (Game::C_PickupEffectsNitrousReverse*)mHitUpdaters[i];
					pNitroUpdater->KillEffect();

					break;
				}
			}
		}
	}
}

void C_VehicleComponent_Pickups::ApplyBlindfireSpin(bool fromMine)
{
	Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	Game::Actors::FakeVehiclePhysicsState* pFakePhysics = pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics();
	if(mBlindfireHitLeft)
	{
		if( pFakePhysics->IsActive() && pFakePhysics->IsInterruptable() )
		{
			if(fromMine)
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT_MINIMINE);
			else
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT_WEAK);
		}
		else
		{
			if(fromMine)
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT_MINIMINE);
			else
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_45_LEFT);		
		}
		mBlindfireHitLeft = false;
	}
	else
	{
		if( pFakePhysics->IsActive() && pFakePhysics->IsInterruptable() )
		{
			if(fromMine)
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT_MINIMINE);
			else
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT_WEAK);
		}
		else
		{
			if(fromMine)
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT_MINIMINE);
			else
				pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_SPIN_45_RIGHT);
		}
		mBlindfireHitLeft = true;
	}
}

RLPickupLevel* C_VehicleComponent_Pickups::GetRLSubTypeLevel(E_PICKUP_TYPE type, E_PICKUP_SUB_TYPE subType)
{
	RLPickupType* pPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(type);

	switch(subType)
	{
	case E_PICKUP_SUB_TYPE_NORMAL:
	case E_PICKUP_SUB_TYPE_DECOY:
		return pPickup->mNormal.Object();

	case E_PICKUP_SUB_TYPE_REVERSED:
		return pPickup->mReversed.Object();

	case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
		return pPickup->mDoubleTap.Object();

	case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
		return pPickup->mReversedDoubleTap.Object();

	case E_PICKUP_SUB_TYPE_SUB_MOD:
	case E_PICKUP_SUB_TYPE_MOD:
		return pPickup->mModified.Object();

	case E_PICKUP_SUB_TYPE_MOD_REVERSED:
		return pPickup->mReverseModified.Object();
	}

	return NULL;
}

void C_VehicleComponent_Pickups::UpdateHitUpdaters()
{
	PERF_EVENT("C_VehicleComponent.Pickups.UpdateHitUpdaters");
	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;
	info.mUsingNitrousReverse = false;

	// Run through list to delete unwanted items - increment 'a' only when we don't delete (as rest get shifted down)
	Game::C_PickupLocalEffectsBase* pHitUpdater = NULL;
	for (int a=0; a < mHitUpdaters.Size(); )
	{		
		pHitUpdater = mHitUpdaters[a];
		if(pHitUpdater)
		{
			bool erase = pHitUpdater->Update();

			if(erase)
			{
				SAFE_DELETE_PTR(pHitUpdater);
				mHitUpdaters.EraseByIndex(a);

				info.mCurrentlyUsingType=E_PICKUP_TYPE_NONE;
				info.mCurrentlyUsingLevel=-1;
			}
			else
			{
				if(pHitUpdater->GetType() == E_PICKUP_TYPE_NITROUS && pHitUpdater->GetSubType() == E_PICKUP_SUB_TYPE_REVERSED)
				{
					info.mUsingNitrousReverse = true;
				}

				a++;
			}
		}	
	}
}

void C_VehicleComponent_Pickups::CheckShieldedBoostersActivated()
{
	//Shielded Boosters mod - shields give player some health
	Game::C_MultiplayerVehicleAbility* shieldedBoostersAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Shielded_Boosters);
	if(shieldedBoostersAbility->GetActive())
	{
		//Give some health
		Components::C_VehicleComponent_Damage* pDamage = (Components::C_VehicleComponent_Damage*)GetActor()->QueryInterface(Components::E_IF_VehicleComponent_Damage);
		if(pDamage && shieldedBoostersAbility->GetValue() > 0.001f)
		{
			mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Shielded_Boosters, true);

			if(g_DebugShowMpAbilityActivateSpam)
			{
				LOGDEBUG(g_MultiplayerLog, "Vehicle %d's Shielded Boosters ability has been activated!", mVehicleIndex);
			}

			int healthPip = mRLCPtr->GetRLSystem()->mDamageSystem->mParameters->mHealthIncrement;
			C_VehicleLogicRepairMsg repairMsg(mVehicleIndex, healthPip * shieldedBoostersAbility->GetValue());
			pDamage->OnMessage(&repairMsg);
		}
	}
}

bool C_VehicleComponent_Pickups::CheckShieldedBoostersAbsorbDamage(E_PICKUP_TYPE pickupType, int firingIdx)
{
	Game::C_MultiplayerVehicleAbility* shieldedBoostersAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Shielded_Boosters);
	if(shieldedBoostersAbility->GetActive())
	{
		RLPickupType* pPickup					= (RLPickupType*)::RLDatabase::Instance().GetPickupType(pickupType);
		int damage = pPickup->mNormal->mBalancingData->GetDamageInflicted();

		C_VehicleLogicRepairMsg repairMsg(mVehicleIndex, damage);
		GetActorList()->SendMessageToActorComponent(GetActor()->GetHandle(), E_IF_VehicleComponent_Damage, &repairMsg);

		// Flash the UI Icon
		mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Shielded_Boosters, true);

		//Start the special fx if not in the cooldown state
		if(mShieldedBoostersAbsorbFXCooldownTimer <= 0.001f)
		{
			Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();
	
			RLPickupType* pShield = (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHIELD);

			Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(((RLShieldSpecialFX*)pShield->mNormal->mSpecialFX)->mShieldDomeShieldedBoostersSpecialFX, 
				pActor->GetWorldMatrix(), mVehicleIndex, mVehicleIndex, -1, Game::E_InGameSpecialFX_Trigger_Repairing);

			mShieldedBoostersAbsorbFXCooldownTimer = ((RLShieldSpecialFX*)pShield->mNormal->mSpecialFX)->mShieldDomeShieldedBoostersSpecialFX[0].mDurationIntro.mEnd;
		}
	
		return true;
	}

	return false;
}

// Message Handlers
bool C_VehicleComponent_Pickups::OnMessage(const Messaging::C_MessageBase *pMessage) 
{
	PERF_EVENT("C_VehicleComponent.Pickups.MessageHandler");
	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;

	MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
	BIZ_ASSERT_MSG(pConfig, "C_VehicleComponent_Pickups::OnMessage() > Couldn't get a pointer to the MP config :¬{");

	switch(pMessage->Type())
	{
		case Game::E_GameLogicMessage_GameplayImpact:
			{
				const Game::C_GameplayImpactMsg* msg = Messaging::MessageCast< Game::C_GameplayImpactMsg >(pMessage);

				if(msg->mVehicleIdx == mVehicleIndex && msg->mOtherVehicleIdx != INVALID_VEHICLE_IDX)
				{
					Game::Actors::C_VehicleLogicActor* pActor		= (Game::Actors::C_VehicleLogicActor*)GetActor();
					Game::Actors::C_VehicleLogicActor* pOtherActor	= mRaceLogicLayerPtr->GetLogicVehicleActor(msg->mOtherVehicleIdx);

					bool myShieldActive = false;
					bool hisShieldActive = false;

					if(pActor && pActor->mShieldTime > 0.f)
					{
						myShieldActive = (pActor->mDoubleShield ||pActor->mSuperShield);
					}
					if(pOtherActor && pOtherActor->mShieldTime > 0.f)
					{
						hisShieldActive	= (pOtherActor->mDoubleShield || pOtherActor->mSuperShield);
					}

					if(hisShieldActive && !myShieldActive && mHitByDoubleShieldCooldown <= 0.f)
					{
						RLPickupType* pShieldPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHIELD);
						RLPickupType* pSuperShieldPickup = (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SUPER_SHIELD);
						if(pShieldPickup && pSuperShieldPickup)
						{
							RLPickupLevel* pLevel = 0;
							if(pOtherActor->mSuperShield)
							{
								pLevel = pSuperShieldPickup->mNormal;
							}
							else 
							{
								pLevel = pShieldPickup->mDoubleTap;
							}

							RLSpecialFXBase* pSFXBase = pLevel->mSpecialFX;
							if(pSFXBase)
							{
								Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(pSFXBase->mHitVehicleSpecialFX, pActor->GetWorldMatrix(), 
									pOtherActor->GetVehicleIndex(), mVehicleIndex, -1, Game::E_InGameSpecialFX_Trigger_HitByPerk);

								Maths::HVec3 dir = Maths::Normalise(pActor->GetWorldMatrix().AW() - pOtherActor->GetWorldMatrix().AW());
								float dotProdX = Maths::DotProduct(dir, pActor->GetWorldMatrix().AX());
								if( dotProdX < 0 )
								{
									pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_SHIELD_LEFT);
								}
								else
								{
									pActor->GetGraphicalActor(mVehicleIndex)->GetFakePhysics()->Start(Game::Actors::E_FAKE_PHYSICS_JINK_SHIELD_RIGHT);
								}								
							}

							mHitByDoubleShieldCooldown = g_debugPerksDoubleShieldCooldown;

							if(pOtherActor->mSuperShield && pLevel)
							{
								C_VehicleLogicForceDamageMsg forceDamageMsg(pActor->GetVehicleIndex(), pLevel->mBalancingData->GetDamageInflicted());
								pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Damage, &forceDamageMsg);
							}
						}	
					}	
				}
				return true;
			}
			break;

		case Game::E_PickupMessage_PickupPickedUp:
			{
				const Game::Actors::PickupPickedUpMsg* msg = Messaging::MessageCast< Game::Actors::PickupPickedUpMsg >(pMessage);

				if(msg->mVehicleIndex == mVehicleIndex)
				{
					PERF_EVENT("C_VehicleComponent.Pickups.HitPickUp");

					// Setup num shots awarded
					bool picked_up = false;
					if(!msg->mFxOnly)
					{
						int numShotsAwarded = GetNumShotsAwarded(msg->mePickupType);
						picked_up = HitPickUp(msg->mePickupType, msg->mDroppedFromIndex, numShotsAwarded);
					}

					if(msg->mFxOnly || picked_up)
					{
						RLPickupType* pPickup		= (RLPickupType*)::RLDatabase::Instance().GetPickupType(msg->mePickupType);
						RLSpecialFXBase* pSFXBase	= pPickup->mNormal->mSpecialFX;
						BIZ_ASSERT(pSFXBase);

						Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

						if(mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Road_Sweeper)->GetActive())
						{
							if(mRoadsweeperPickupFXCooldownTimer <= 0.001f)
							{
								Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(pSFXBase->mRoadsweeperedSpecialFX, Maths::HMat43(Maths::HMat33(), msg->mPickupPosition), mVehicleIndex, 
																								-1, -1, Game::E_InGameSpecialFX_Trigger_PickedUp);
								mRoadsweeperPickupFXCooldownTimer = 1.0f;
							}
							else
							{
								Game::C_InGameSpecialFXManager::Instance().SetUpVehicleToMatrixFX(pSFXBase->mRoadsweeperedSpecialFX[0], Maths::HMat43(), mVehicleIndex, 
																								-1, Game::E_InGameSpecialFX_Trigger_PickedUp, -1, Maths::HVec3(1,1,1), Maths::HVec3());
							}
							mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Road_Sweeper, true, false, mVehicleIndex);
						}
						else
						{
							Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(pSFXBase->mPickedUpSpecialFX, Maths::HMat43(Maths::HMat33(), msg->mPickupPosition), mVehicleIndex, 
																							-1, -1, Game::E_InGameSpecialFX_Trigger_PickedUp);
						}
#ifndef CFG_GOLD
						AmaxDataLogging::LogPickedUpPickup(msg->mVehicleIndex, mRaceLogicLayerPtr->ConvertPickupTypeToString(msg->mePickupType), 
															msg->mPickupPosition, msg->mDroppedFromIndex, msg->mUniqueId);

						Game::C_ReplayDirector::Instance().AddPickupAdded(mVehicleIndex, msg->mePickupType);
#endif
					}
				}

				return true;
			}
			break;

		case Game::E_PickupMessage_GrantPickup:
			{
				const Game::Actors::C_GrantPickupMsg* msg = Messaging::MessageCast<Game::Actors::C_GrantPickupMsg>(pMessage);

				if(msg->mVehicleIndex == mVehicleIndex)
				{
					PERF_EVENT("C_VehicleComponent.Pickups.HitPickUp");
					// Setup num shots awarded
					int numShotsAwarded = GetNumShotsAwarded(msg->mPickupType);
					HitPickUp(msg->mPickupType, -1, numShotsAwarded, msg->mRefill, msg->mReserved);
				}

				return true;
			}
			break;

		case Game::E_VehicleMessage_FirePickup:
			{
				const Game::Actors::C_VehicleFirePickupMsg* msg = Messaging::MessageCast<Game::Actors::C_VehicleFirePickupMsg>(pMessage);


				if(msg->mVehicleIndex == mVehicleIndex)
				{
					PERF_EVENT("C_VehicleComponent.Pickups.ActivatePerk");
					ActivatePerk(msg->mPickupOverride,-1,msg->mBackwards,msg->mForwards,false);
				}
				return true;
			}
		case Game::E_GameLogicMessage_GiftDelayedPickupWithAnim:
			{
				const Game::C_GiftDelayedPickupWithAnimMsg* msg = Messaging::MessageCast< Game::C_GiftDelayedPickupWithAnimMsg >(pMessage);
				if(msg->mVehicleIndex == mVehicleIndex)
				{
					int numShotsAwarded = msg->mNumShotsAwarded; // by default always award 1 shot (single use)
					DelayGiftPickUp(msg->mPickupType, g_debugGiftPickupDelayTime, numShotsAwarded);
				}
			}
		case Game::E_PickupMessage_SetPickup:
			{
				const Game::Actors::PickupSetMsg* msg = Messaging::MessageCast<Game::Actors::PickupSetMsg>(pMessage);

				if(msg->mVehicleIndex == mVehicleIndex)
				{
					//if( msg->mReset )
					//	info.mCurrentPickupPowerBar.Clear();

					//info.mCurrentPickupType = msg->mPickupType;
					//info.mCurrentPickupPowerBar.PushBack(1.0f);

					//C_VehicleLogicPickUpInfoMsg info_msg(mVehicleIndex, info.mCurrentPickupType, info.mCurrentPickupPowerBar.Size(), info.mCurrentPickupPowerBar, info.mPickupBarType);
					//GetActorList()->BroadcastMessage(&info_msg);
					//Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&info_msg);
				}

				return true;
			}

		case Game::E_PickupMessage_RemovePower:
			{
				const Game::Actors::PickupRemovePowerMsg* msg = Messaging::MessageCast<Game::Actors::PickupRemovePowerMsg>(pMessage);

				if(msg->mVehicleIndex == mVehicleIndex)
				{
					PERF_EVENT("C_VehicleComponent.Pickups.RemovePower");
					RemovePower(msg->mPower, msg->mForceRemoval);
				}

				return true;
			}

		case Game::E_PickupMessage_ClearHitUpdaters:
			{
				const Game::Actors::PickupClearHitUpdatersMsg* msg = Messaging::MessageCast<Game::Actors::PickupClearHitUpdatersMsg>(pMessage);

				if(msg->mVehicleIndex == mVehicleIndex)
				{
					// Clear the physical effects
					for (int a=0; a < mHitUpdaters.Size(); a++)	
						mHitUpdaters[a]->KillEffect();
				}

				return true;
			}

		case Game::E_PickupMessage_Shuffle:
			{
				const Game::Actors::PickupShuffleMsg* msg = Messaging::MessageCast<Game::Actors::PickupShuffleMsg>(pMessage);

				// This is different to the MP element - the AI use this to "shuffle", but they just directly
				// choose their index. 
				if(msg->mVehicleIndex == mVehicleIndex)
				{
					PERF_EVENT("C_VehicleComponent.Pickups.Shuffle");
					Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

					// Always shuffle the AI choice to the front.
					pActor->mPickupInfo.mPickupStoreIndex = 0;

					PickupStore oldStore = pActor->mPickupInfo.mPickupStore[0];
					pActor->mPickupInfo.mPickupStore[0] = pActor->mPickupInfo.mPickupStore[msg->mPlaces];
					pActor->mPickupInfo.mPickupStore[msg->mPlaces] = oldStore;

					pActor->mPickupInfo.mCurrentlySelectedType = pActor->mPickupInfo.mPickupStore[0].mType;
				}
				return true;
			}

		case E_VehicleLogicMsg_FiredPerk:
			{
				const C_VehicleLogicMsg_FiredPerkMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_FiredPerkMsg>(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					mVehicleTargettedByShock = -1; // Reset shock
#ifndef CFG_GOLD
					Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

					AmaxDataLogging::LogPickupUse(pMsg->mMatrix.AW(), pMsg->mVehicleIndex, mRaceLogicLayerPtr->ConvertPickupTypeToString(pMsg->mPickupType), pMsg->mLevel,
						mLevelStatePtr->mCars[mVehicleIndex].mUpgrades.NumPickupSlots(), pActor->mPickupInfo.mPickupStore.Size(), pActor->GetRLC()->GetRaceTime());
#endif
				}

				return true;
			}

		case E_VehicleLogicMsg_DropPickupOnTrack:
			{
				const C_VehicleLogicDropPickupOnTrackMsg* pMsg = Messaging::MessageCast<C_VehicleLogicDropPickupOnTrackMsg>(pMessage);

				if(pMsg->m_VehicleIndex == mVehicleIndex)
				{
					mDropPickup = true;
				}

				return true;
			}
		case Game::E_VehicleMessage_ResetAfterWrecked:
			{
				const Game::Actors::C_VehicleResetAfterWreckedMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleResetAfterWreckedMsg>(pMessage);

				//Check that its us who's been wrecked
				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					if(mLevelStatePtr->mRaceMode == Game::E_RaceMode_Destruction || mLevelStatePtr->mRaceMode == Game::E_RaceMode_FriendDemand_Destruction)
					{
						//MotorMash 
						PopulatePickupProbabilities(pConfig->m_BribeModSettings.m_MotorMashRespawn);
					}
					else
					{
						//Other race modes
						PopulatePickupProbabilities(pConfig->m_BribeModSettings.m_RaceRespawn);
					}

					// indicate that we want to give a pickup
					mBribeDelayTimer = pConfig->m_BribeModSettings.m_GiftDelayTime;
				}
				return true;
			}

		case Game::E_GameLogicMessage_SplitCrossed:
			{
				if(mIsMultiplayer && pConfig->m_MiscModSettings.m_FrontrunnerNitroOnSplit)
				{
					CheckFrontRunner();
				}
				return true;
			}

		case Game::E_GameLogicMessage_LapIncremented:
			{
				const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					if(mLevelStatePtr->mCars[mVehicleIndex].mPerkGrantingLaps)
					{
						PERF_EVENT("C_VehicleComponent.Pickups.ForceGift");
						ForceGift();
					}

					// Front Runner Upgrade - dont activate going from lap 0 --> lap 1
					if(mIsMultiplayer)
					{
						// Get Actor
						Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());

						// On Final lap
						if( pConfig->m_MiscModSettings.m_FrontrunnerNitroOnFinalLap &&
							mLevelStatePtr && mLevelStatePtr->mLaps-1 == pActor->GetLapInfo().mCurrentLap )
						{
							CheckFrontRunner();
						}
						else
						{	
							// On lap completed 
							if( pConfig->m_MiscModSettings.m_FrontrunnerNitroOnLapCompleted &&
								pActor->GetPositionInfo().mLogicPositionCombined - mStartingLogicPosition >= 0.99f &&	// And we have travelled at least a lap since starting the race
								mLevelStatePtr && mLevelStatePtr->mLaps != pActor->GetLapInfo().mCurrentLap &&			// But not 'finish'
								mLevelStatePtr && mLevelStatePtr->mLaps-1 != pActor->GetLapInfo().mCurrentLap)			// and not final lap
							{
								CheckFrontRunner();
							}
						}
					}
				}
				
				return true;
			}

		case E_VehicleLogicMsg_EnableDisablePickupSystem:
			{
				const C_VehicleLogicMsg_EnableDisablePickupSystem* msg = Messaging::MessageCast<C_VehicleLogicMsg_EnableDisablePickupSystem>(pMessage);

				if(msg->mVehicleIndex == mVehicleIndex)
				{
					if(mLevelStatePtr->mRaceMode == Game::E_RaceMode_Destruction || mLevelStatePtr->mRaceMode == Game::E_RaceMode_FriendDemand_Destruction)
					{
						//MotorMash 
						PopulatePickupProbabilities(pConfig->m_BribeModSettings.m_MotorMashStart);
					}
					else
					{
						//Other race modes
						PopulatePickupProbabilities(pConfig->m_BribeModSettings.m_RaceStart);
					}

					if(msg->mIncremental)
					{
						mDisablePickups += msg->mDisablePickupTime;
					}
					else
					{
						mDisablePickups=msg->mDisablePickupTime;
					}
				}

				return true;
			}

		case Game::E_PickupMessage_ActivatePickups:
			{
				if(pConfig->m_MiscModSettings.m_BribeOnActivatePickups)
				{
					if(mLevelStatePtr->mRaceMode == Game::E_RaceMode_Destruction || mLevelStatePtr->mRaceMode == Game::E_RaceMode_FriendDemand_Destruction)
					{
						//MotorMash 
						PopulatePickupProbabilities(pConfig->m_BribeModSettings.m_MotorMashStart);
					}
					else
					{
						//Other race modes
						PopulatePickupProbabilities(pConfig->m_BribeModSettings.m_RaceStart);
					}

					CheckBribeAbility();
				}
				return true;
			}

		case Game::E_GameflowMessage_RaceStarted:
			{
				if(!mRaceStarted && mIsMultiplayer && pConfig->m_MiscModSettings.m_FrontrunnerNitroOnRaceStart)
				{
					CheckFrontRunner();
				}
				mFrontRunnerTimer = pConfig->m_MiscModSettings.m_FrontrunnerNitroOnRaceStart;
				mRaceStarted=true;
			    Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
				if(pActor)
				{
					mStartingLogicPosition = pActor->mPositionInfo.mLogicPositionCombined;

					// calculate our starting lap (rounding up or down where necessary)
					mStartingLogicPosition = Maths::Floor(mStartingLogicPosition + 0.5f);
				}

				return true;
			}
		case E_VehicleLogicMsg_ShieldedPerk:
			{
				const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_ShieldedPerkMsg>(pMessage);
				if(pMsg->mDefendingVehicleIndex != mVehicleIndex)
				{
					return true; 
				}

				// If have Safety Net Mod, grant the user the pickup that they shielded from
				if(Game::VehicleIsLocal(mVehicleIndex))
				{
					Game::C_MultiplayerVehicleAbility* ability = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Safety_Net);
					if(ability->GetActive())
					{
						// Award any pickups
						HandleSafetyNetPickupAward(pMsg); 
					}
				}
				return true;
			}
		case Game::E_GameflowMessage_RestartRace:		//	Mass fall-through
		case Game::E_GameflowMessage_QuitAndWinRace:
		case Game::E_GameflowMessage_QuitRace:
		case Game::E_GameflowMessage_RaceFinished:
		case Game::E_GameflowMessage_VehicleFinished:
			{
				if(mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Laser_Sight))
				{
					mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Laser_Sight, false);
				}
				mRaceStarted=false;
				return true;
			}
	}

	return I_GameActorComponent::OnMessage(pMessage);
}

void C_VehicleComponent_Pickups::HandleSafetyNetPickupAward(const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg)
{
	// First of all.. check we can actually absorb this pickup type
	if( !CanSafetyNetAbsorbPickupType( pMsg->mPickupType ) )
	{
		return;
	}


	//maximum on number of pickups absorbable
	if(mSafetyNetNumTimesAbsorbed > g_SafetyNetMaxPickups)
		return;

	// May be cases we dont actaully activate the mod (i.e. hit by both spheres of a shock only activates once)
	bool showEffect = true; 

	// Blindfire and shock cause problems.. handle differently to everything else...
	switch(pMsg->mPickupType)
	{
		case E_PICKUP_TYPE_BLINDFIRE:
		{
			HandleSafetyNetBlindFire(pMsg);
			mSafetyNetNumBlindfiresAbsorbed++;
			if(mSafetyNetNumBlindfiresAbsorbed >= 3)
			{
				mSafetyNetNumBlindfiresAbsorbed = 0;
				mSafetyNetNumTimesAbsorbed++;
			}
		}
		break;

		case E_PICKUP_TYPE_SHOCK:
		{
			HandleSafetyNetShock(pMsg,showEffect);
		}
		break;

		default:
		{
			int numShotsAwarded = 1; // by default always award 1 shot (single use)
			mSafetyNetNumTimesAbsorbed++;
			DelayGiftPickUp(pMsg->mPickupType, g_debugGiftPickupDelayTime, numShotsAwarded);
		}
		break;

	}

	if(showEffect)
	{	
		// Show some flashy UI effect
		mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Safety_Net, true, false, mVehicleIndex);
		mpMultiplayerAbilities->SetPickupSlotGiftedInto(((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo.mPickupStore.Size());

		// Record this stat
		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
		if(pActor)
		{
			pActor->mStatistics.mPowerupsAbsorbedBySafetyNet++;
		}
	}
}

void C_VehicleComponent_Pickups::HandleSafetyNetBlindFire(const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg)
{
	// Blind fire is a bit messier.. Award 1 blindfire 'shot' for each blindfire pellet that hits.. (1 blindfire contains 3 shots normally which isnt what we want)
	VehiclePickupInfo& info=((Game::Actors::C_VehicleLogicActor*)GetActor())->mPickupInfo;

	int indexOfExistingBlindFire = -1; 
	bool hasNoPickups = false; 
	if(info.mPickupStoreIndex == -1)
	{	
		 hasNoPickups = true; 
	}
	else
	{
		// If have pickups.. then
		// Do we have any blindfire pickups with < full power?
		for(int i = 0; i < info.mPickupStore.Size(); i++)
		{
			if(info.mPickupStore[i].mType == E_PICKUP_TYPE_BLINDFIRE)
			{
				// If < full power.. then we have found our candidate
				if(info.mPickupStore[i].mNumShots < 3) // TODO: why is numshots hard coded as 3?
				{
					indexOfExistingBlindFire = i; 

					// done
					i = info.mPickupStore.Size();
				}
			}
		}
	}

	// If have no blind fires.. or have no blindfires with < max num shots...
	if(hasNoPickups || indexOfExistingBlindFire == -1)
	{
		// Gift new blindfire
		// Reduce new blindfire numshots to 1 shot
		const int numShotsOverride = 1; 
		DelayGiftPickUp(pMsg->mPickupType, g_debugGiftPickupDelayTime, numShotsOverride);

	}
	else
	{
		// top-up blindfire's num shots by 1 
		info.mPickupStore[indexOfExistingBlindFire].mNumShots++; 
		BIZ_ASSERT_MSG(info.mPickupStore[indexOfExistingBlindFire].mNumShots <= 3,"VLC_Pickups::OnMessage E_VehicleLogicMsg_ShieldedPerk < Safety Net Mod somehow incremented num blindfire shots above max...");
	}

}

void C_VehicleComponent_Pickups::HandleSafetyNetShock(const C_VehicleLogicMsg_ShieldedPerkMsg* pMsg, bool& showEffect)
{
	// Only activate safety net if have NOT been hit by this shock before. Shocks cause multiple hits as they are structured
	// as multiple spheres, therefore technically possible to receive 2 * shielded messages. Only want to award ONE shock pickup.
	S32 newID = (S32)(pMsg->mUniqueID);
	if(newID != mLastShockPickupIDToHit)
	{
		int numShotsAwarded = 1;  // Shock is always 1 shot (single use) awarded
		mSafetyNetNumTimesAbsorbed++;
		DelayGiftPickUp(pMsg->mPickupType, g_debugGiftPickupDelayTime, numShotsAwarded);
	}
	else
	{
		// Else.. ignore it. 
		showEffect = false; 
	}

	// Record the ID of this shock
	S32 mLastShockPickupIDToHit = newID; 

}

int C_VehicleComponent_Pickups::GetNumShotsAwarded(E_PICKUP_TYPE type)
{
	int numShots = 1;

	if(type == E_PICKUP_TYPE_BLINDFIRE)
	{
		numShots = 3;

		Game::C_MultiplayerVehicleAbility* pMPAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots);

		if(pMPAbility->GetActive())
			numShots += pMPAbility->GetValue();
	}

	return numShots;
}

bool C_VehicleComponent_Pickups::CanSafetyNetAbsorbPickupType(const E_PICKUP_TYPE pickupType) const
{
	// Lookup sorcery data here, check against allowed types
	MiscModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings;

	switch(pickupType)
	{
		case E_PICKUP_TYPE_SHOCK:
		{
			return modSettings.m_SafetyNetAbsorbEnabledShock;
		}
		case E_PICKUP_TYPE_SHUNT:
		{
			return modSettings.m_SafetyNetAbsorbEnabledShunt;
		}
		case E_PICKUP_TYPE_BLINDFIRE:
		{
			return modSettings.m_SafetyNetAbsorbEnabledBolt;
		}
		case E_PICKUP_TYPE_BARGE:
		{
			return modSettings.m_SafetyNetAbsorbEnabledBarge;
		}
		case E_PICKUP_TYPE_BOMB:
		{
			return modSettings.m_SafetyNetAbsorbEnabledBomb;
		}
		default:
		{
			break;
		}
	}

	// If reach here then cant absorb / unknown type
	return false;
}

} // namespace Components







////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//DEBUG (if somebody can tell me how to do all this better then please write to Paul Kerby, Amax Room, Bizarre Creations, Liverpool, UK)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

DEBUG_ENUM(E_PICKUP_TYPE)
	DE_OPTION(E_PICKUP_TYPE_NITROUS,			"Nitrous")
	DE_OPTION(E_PICKUP_TYPE_SHOCK,				"Shock")
	DE_OPTION(E_PICKUP_TYPE_SHUNT,				"Shunt")
	DE_OPTION(E_PICKUP_TYPE_BLINDFIRE,			"Blindfire")
	DE_OPTION(E_PICKUP_TYPE_BARGE,				"Barge")
	DE_OPTION(E_PICKUP_TYPE_BOMB,				"Bomb")

	DE_OPTION(E_PICKUP_TYPE_REPAIR,				"Repair")
	DE_OPTION(E_PICKUP_TYPE_SHIELD,				"Shield")
	DE_OPTION(E_PICKUP_TYPE_SUPER_SHIELD,		"SuperShield")
END_DEBUG_ENUM

//Vehicle any
Debug::Bool					g_debugVehicleAnyAddToMulti("Perks.Gift.Any.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicleAnyPerkType("Perks.Gift.Any.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicleAnyPerkLevel("Perks.Gift.Any.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicleAnyPerkFunc(void *)
{
	for(int a=0;a<g_debugVehicleAnyPerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(g_debugVehicleAnyIndex, g_debugVehicleAnyPerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);

		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(g_debugVehicleAnyIndex, g_debugVehicleAnyPerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicleAnyFire("Perks.Gift.Any.FirePerk", FireVehicleAnyPerkFunc, 0);

static void GrantVehicleAnyPerkFunc(void *)
{
	for(int a=0;a<g_debugVehicleAnyPerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(g_debugVehicleAnyIndex, g_debugVehicleAnyPerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicleAnyGrant("Perks.Gift.Any.GrantPerk", GrantVehicleAnyPerkFunc, 0);


//Vehicle 1
Debug::Bool					g_debugVehicle1AddToMulti("Perks.Gift.Vehicle1.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle1PerkType("Perks.Gift.Vehicle1.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle1PerkLevel("Perks.Gift.Vehicle1.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle1PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(0, g_debugVehicle1PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(0, g_debugVehicle1PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle1Fire("Perks.Gift.Vehicle1.FirePerk", FireVehicle1PerkFunc, 0);

static void GrantVehicle1PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(0, g_debugVehicle1PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle1Grant("Perks.Gift.Vehicle1.GrantPerk", GrantVehicle1PerkFunc, 0);

//Vehicle 2
Debug::Bool					g_debugVehicle2AddToMulti("Perks.Gift.Vehicle2.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle2PerkType("Perks.Gift.Vehicle2.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle2PerkLevel("Perks.Gift.Vehicle2.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle2PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle2PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(1, g_debugVehicle2PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(1, g_debugVehicle2PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle2Fire("Perks.Gift.Vehicle2.FirePerk", FireVehicle2PerkFunc, 0);

static void GrantVehicle2PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(1, g_debugVehicle2PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle2Grant("Perks.Gift.Vehicle2.GrantPerk", GrantVehicle2PerkFunc, 0);

//Vehicle 3
Debug::Bool					g_debugVehicle3AddToMulti("Perks.Gift.Vehicle3.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle3PerkType("Perks.Gift.Vehicle3.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle3PerkLevel("Perks.Gift.Vehicle3.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle3PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle3PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(2, g_debugVehicle3PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(2, g_debugVehicle3PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle3Fire("Perks.Gift.Vehicle3.FirePerk", FireVehicle3PerkFunc, 0);

static void GrantVehicle3PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(2, g_debugVehicle3PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle3Grant("Perks.Gift.Vehicle3.GrantPerk", GrantVehicle3PerkFunc, 0);

//Vehicle 4
Debug::Bool					g_debugVehicle4AddToMulti("Perks.Gift.Vehicle4.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle4PerkType("Perks.Gift.Vehicle4.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle4PerkLevel("Perks.Gift.Vehicle4.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle4PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle4PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(3, g_debugVehicle4PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(3, g_debugVehicle4PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle4Fire("Perks.Gift.Vehicle4.FirePerk", FireVehicle4PerkFunc, 0);

static void GrantVehicle4PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(3, g_debugVehicle4PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle4Grant("Perks.Gift.Vehicle4.GrantPerk", GrantVehicle4PerkFunc, 0);

//Vehicle 5
Debug::Bool					g_debugVehicle5AddToMulti("Perks.Gift.Vehicle5.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle5PerkType("Perks.Gift.Vehicle5.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle5PerkLevel("Perks.Gift.Vehicle5.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle5PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle5PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(4, g_debugVehicle5PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(4, g_debugVehicle5PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle5Fire("Perks.Gift.Vehicle5.FirePerk", FireVehicle5PerkFunc, 0);

static void GrantVehicle5PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(4, g_debugVehicle5PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle5Grant("Perks.Gift.Vehicle5.GrantPerk", GrantVehicle5PerkFunc, 0);

//Vehicle 6
Debug::Bool					g_debugVehicle6AddToMulti("Perks.Gift.Vehicle6.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle6PerkType("Perks.Gift.Vehicle6.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle6PerkLevel("Perks.Gift.Vehicle6.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle6PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle6PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(5, g_debugVehicle6PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(5, g_debugVehicle6PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle6Fire("Perks.Gift.Vehicle6.FirePerk", FireVehicle6PerkFunc, 0);

static void GrantVehicle6PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(5, g_debugVehicle6PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle6Grant("Perks.Gift.Vehicle6.GrantPerk", GrantVehicle6PerkFunc, 0);

//Vehicle 7
Debug::Bool					g_debugVehicle7AddToMulti("Perks.Gift.Vehicle7.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle7PerkType("Perks.Gift.Vehicle7.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle7PerkLevel("Perks.Gift.Vehicle7.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle7PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle7PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(6, g_debugVehicle7PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(6, g_debugVehicle7PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle7Fire("Perks.Gift.Vehicle7.FirePerk", FireVehicle7PerkFunc, 0);

static void GrantVehicle7PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(6, g_debugVehicle7PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle7Grant("Perks.Gift.Vehicle7.GrantPerk", GrantVehicle7PerkFunc, 0);

//Vehicle 8
Debug::Bool					g_debugVehicle8AddToMulti("Perks.Gift.Vehicle8.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle8PerkType("Perks.Gift.Vehicle8.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle8PerkLevel("Perks.Gift.Vehicle8.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle8PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle8PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(7, g_debugVehicle8PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(7, g_debugVehicle8PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle8Fire("Perks.Gift.Vehicle8.FirePerk", FireVehicle8PerkFunc, 0);

static void GrantVehicle8PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(7, g_debugVehicle8PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle8Grant("Perks.Gift.Vehicle8.GrantPerk", GrantVehicle8PerkFunc, 0);

//Vehicle 9
Debug::Bool					g_debugVehicle9AddToMulti("Perks.Gift.Vehicle9.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle9PerkType("Perks.Gift.Vehicle9.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle9PerkLevel("Perks.Gift.Vehicle9.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle9PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle9PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(8, g_debugVehicle9PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(8, g_debugVehicle9PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle9Fire("Perks.Gift.Vehicle9.FirePerk", FireVehicle9PerkFunc, 0);

static void GrantVehicle9PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(8, g_debugVehicle9PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle9Grant("Perks.Gift.Vehicle9.GrantPerk", GrantVehicle9PerkFunc, 0);

//Vehicle 10
Debug::Bool					g_debugVehicle10AddToMulti("Perks.Gift.Vehicle10.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle10PerkType("Perks.Gift.Vehicle10.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle10PerkLevel("Perks.Gift.Vehicle10.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle10PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle10PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(9, g_debugVehicle10PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(9, g_debugVehicle10PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle10Fire("Perks.Gift.Vehicle10.FirePerk", FireVehicle10PerkFunc, 0);

static void GrantVehicle10PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(9, g_debugVehicle10PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle10Grant("Perks.Gift.Vehicle10.GrantPerk", GrantVehicle10PerkFunc, 0);

//Vehicle 11
Debug::Bool					g_debugVehicle11AddToMulti("Perks.Gift.Vehicle11.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle11PerkType("Perks.Gift.Vehicle11.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle11PerkLevel("Perks.Gift.Vehicle11.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle11PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle11PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(10, g_debugVehicle11PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(10, g_debugVehicle11PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle11Fire("Perks.Gift.Vehicle11.FirePerk", FireVehicle11PerkFunc, 0);

static void GrantVehicle11PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(10, g_debugVehicle11PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle11Grant("Perks.Gift.Vehicle11.GrantPerk", GrantVehicle11PerkFunc, 0);

//Vehicle 12
Debug::Bool					g_debugVehicle12AddToMulti("Perks.Gift.Vehicle12.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle12PerkType("Perks.Gift.Vehicle12.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle12PerkLevel("Perks.Gift.Vehicle12.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle12PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle12PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(11, g_debugVehicle12PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(11, g_debugVehicle12PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle12Fire("Perks.Gift.Vehicle12.FirePerk", FireVehicle12PerkFunc, 0);

static void GrantVehicle12PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(11, g_debugVehicle12PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle12Grant("Perks.Gift.Vehicle12.GrantPerk", GrantVehicle12PerkFunc, 0);

//Vehicle 13
Debug::Bool					g_debugVehicle13AddToMulti("Perks.Gift.Vehicle13.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle13PerkType("Perks.Gift.Vehicle13.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle13PerkLevel("Perks.Gift.Vehicle13.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle13PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle13PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(12, g_debugVehicle13PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(12, g_debugVehicle13PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle13Fire("Perks.Gift.Vehicle13.FirePerk", FireVehicle13PerkFunc, 0);

static void GrantVehicle13PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(12, g_debugVehicle13PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle13Grant("Perks.Gift.Vehicle13.GrantPerk", GrantVehicle13PerkFunc, 0);

//Vehicle 14
Debug::Bool					g_debugVehicle14AddToMulti("Perks.Gift.Vehicle14.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle14PerkType("Perks.Gift.Vehicle14.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle14PerkLevel("Perks.Gift.Vehicle14.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle14PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle14PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(13, g_debugVehicle14PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(13, g_debugVehicle14PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle14Fire("Perks.Gift.Vehicle14.FirePerk", FireVehicle14PerkFunc, 0);

static void GrantVehicle14PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(13, g_debugVehicle14PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle14Grant("Perks.Gift.Vehicle14.GrantPerk", GrantVehicle14PerkFunc, 0);

//Vehicle 15
Debug::Bool					g_debugVehicle15AddToMulti("Perks.Gift.Vehicle15.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle15PerkType("Perks.Gift.Vehicle15.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle15PerkLevel("Perks.Gift.Vehicle15.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle15PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle15PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(14, g_debugVehicle15PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(14, g_debugVehicle15PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle15Fire("Perks.Gift.Vehicle15.FirePerk", FireVehicle15PerkFunc, 0);

static void GrantVehicle15PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(14, g_debugVehicle15PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle15Grant("Perks.Gift.Vehicle15.GrantPerk", GrantVehicle15PerkFunc, 0);

//Vehicle 16
Debug::Bool					g_debugVehicle16AddToMulti("Perks.Gift.Vehicle16.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle16PerkType("Perks.Gift.Vehicle16.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle16PerkLevel("Perks.Gift.Vehicle16.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle16PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle16PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(15, g_debugVehicle16PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(15, g_debugVehicle16PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle16Fire("Perks.Gift.Vehicle16.FirePerk", FireVehicle16PerkFunc, 0);

static void GrantVehicle16PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(15, g_debugVehicle16PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle16Grant("Perks.Gift.Vehicle16.GrantPerk", GrantVehicle16PerkFunc, 0);

//Vehicle 17
Debug::Bool					g_debugVehicle17AddToMulti("Perks.Gift.Vehicle17.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle17PerkType("Perks.Gift.Vehicle17.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle17PerkLevel("Perks.Gift.Vehicle17.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle17PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle17PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(16, g_debugVehicle17PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(16, g_debugVehicle17PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle17Fire("Perks.Gift.Vehicle17.FirePerk", FireVehicle17PerkFunc, 0);

static void GrantVehicle17PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(16, g_debugVehicle17PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle17Grant("Perks.Gift.Vehicle17.GrantPerk", GrantVehicle17PerkFunc, 0);

//Vehicle 18
Debug::Bool					g_debugVehicle18AddToMulti("Perks.Gift.Vehicle18.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle18PerkType("Perks.Gift.Vehicle18.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle18PerkLevel("Perks.Gift.Vehicle18.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle18PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle18PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(17, g_debugVehicle18PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(17, g_debugVehicle18PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle18Fire("Perks.Gift.Vehicle18.FirePerk", FireVehicle18PerkFunc, 0);

static void GrantVehicle18PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(17, g_debugVehicle18PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle18Grant("Perks.Gift.Vehicle18.GrantPerk", GrantVehicle18PerkFunc, 0);

//Vehicle 19
Debug::Bool					g_debugVehicle19AddToMulti("Perks.Gift.Vehicle19.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle19PerkType("Perks.Gift.Vehicle19.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle19PerkLevel("Perks.Gift.Vehicle19.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle19PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle19PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(18, g_debugVehicle19PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(18, g_debugVehicle19PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle19Fire("Perks.Gift.Vehicle19.FirePerk", FireVehicle19PerkFunc, 0);

static void GrantVehicle19PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(18, g_debugVehicle19PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle19Grant("Perks.Gift.Vehicle19.GrantPerk", GrantVehicle19PerkFunc, 0);

//Vehicle 20
Debug::Bool					g_debugVehicle20AddToMulti("Perks.Gift.Vehicle20.AddToMulti",false,Debug::Persist());
Debug::Enum<E_PICKUP_TYPE>	g_debugVehicle20PerkType("Perks.Gift.Vehicle20.Type", E_PICKUP_TYPE_NONE,Debug::Persist());
Debug::Int					g_debugVehicle20PerkLevel("Perks.Gift.Vehicle20.Level",1,Debug::Range(1,3),Debug::Persist());
static void FireVehicle20PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle20PerkLevel;a++)
	{
Game::Actors::C_GrantPickupMsg grantPickupMsg(19, g_debugVehicle20PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(19, g_debugVehicle20PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&pickupFireMsg);
	}
}
Debug::Function				g_debugVehicle20Fire("Perks.Gift.Vehicle20.FirePerk", FireVehicle20PerkFunc, 0);

static void GrantVehicle20PerkFunc(void *)
{
	for(int a=0;a<g_debugVehicle1PerkLevel;a++)
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(19, g_debugVehicle20PerkType);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage(&grantPickupMsg);
	}
}
Debug::Function				g_debugVehicle20Grant("Perks.Gift.Vehicle20.GrantPerk", GrantVehicle20PerkFunc, 0);


static void FireMultiPerkFunc(void *)
{
	if(g_debugVehicle1AddToMulti)
		FireVehicle1PerkFunc(NULL);

	if(g_debugVehicle2AddToMulti)
		FireVehicle2PerkFunc(NULL);

	if(g_debugVehicle3AddToMulti)
		FireVehicle3PerkFunc(NULL);

	if(g_debugVehicle4AddToMulti)
		FireVehicle4PerkFunc(NULL);

	if(g_debugVehicle5AddToMulti)
		FireVehicle5PerkFunc(NULL);

	if(g_debugVehicle6AddToMulti)
		FireVehicle6PerkFunc(NULL);

	if(g_debugVehicle7AddToMulti)
		FireVehicle7PerkFunc(NULL);

	if(g_debugVehicle8AddToMulti)
		FireVehicle8PerkFunc(NULL);

	if(g_debugVehicle9AddToMulti)
		FireVehicle9PerkFunc(NULL);

	if(g_debugVehicle10AddToMulti)
		FireVehicle10PerkFunc(NULL);

	if(g_debugVehicle11AddToMulti)
		FireVehicle11PerkFunc(NULL);

	if(g_debugVehicle12AddToMulti)
		FireVehicle12PerkFunc(NULL);

	if(g_debugVehicle13AddToMulti)
		FireVehicle13PerkFunc(NULL);

	if(g_debugVehicle14AddToMulti)
		FireVehicle14PerkFunc(NULL);

	if(g_debugVehicle15AddToMulti)
		FireVehicle15PerkFunc(NULL);

	if(g_debugVehicle16AddToMulti)
		FireVehicle16PerkFunc(NULL);

	if(g_debugVehicle17AddToMulti)
		FireVehicle17PerkFunc(NULL);

	if(g_debugVehicle18AddToMulti)
		FireVehicle18PerkFunc(NULL);

	if(g_debugVehicle19AddToMulti)
		FireVehicle19PerkFunc(NULL);

	if(g_debugVehicle20AddToMulti)
		FireVehicle20PerkFunc(NULL);
}
Debug::Function				g_debugMultiFire("Perks.MultiFirePerk", FireMultiPerkFunc, 0);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//DEBUG
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
