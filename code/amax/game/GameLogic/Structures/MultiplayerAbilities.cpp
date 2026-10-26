#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include "MultiplayerAbilities.h"

Debug::Bool g_DebugShowMpAbilityActivateSpam("Multiplayer.ShowUpgradeAbilityActivationSpam", false);

DEBUG_ENUM(UI::E_MultiplayerVehicleAbilities)
DE_OPTION(UI::E_MultiplayerVehicleAbilities_None, 					"None")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Battering_Ram, 			"BatteringRam")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Bribe, 					"Bribe")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Decoy_Drop, 			"DecoyDrop")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Drifter, 				"Drifter")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_ECM, 						"ECM")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Fan_Favourite, 			"FanFavourite")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Front_Runner, 			"FrontRunner")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Iron_Fist, 				"IronFist")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Jump_The_Gun, 			"JumpTheGun")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Laser_Sight, 			"LaserSight")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Last_Gasp, 				"LastGasp")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Mastermine, 			"Mastermine")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Road_Sweeper, 			"Roadsweeper")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Safety_Net, 			"SafetyNet")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Scrambler, 				"Scrambler")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Shielded_Bay, 			"ShieldedBay")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Shielded_Boosters, 		"ShieldedBoosters")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Shielding_Efficiency, 	"ShieldingEfficiency")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Showy_Flourish, 		"ShowyFlourish")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Silent_Running, 		"SilentRunning")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Splash_Damage, 			"SplashDamage")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Stable_Frame, 			"StableFrame")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Titanium_Armour, 		"TitaniumArmour")
DE_OPTION(UI::E_MultiplayerVehicleAbilities_Vampiric_Wreck, 		"VampiricWreck")
END_DEBUG_ENUM


Debug::Int g_DebugFiringIdx("Multiplayer.Upgrades.FiringVehicle", 0, Debug::Range(0,19) );
Debug::Int g_DebugTargetIdx("Multiplayer.Upgrades.TargetVehicle", -1, Debug::Range(-1,19) );

Debug::Enum<UI::E_MultiplayerVehicleAbilities> g_AbilitySlot1("Multiplayer.Upgrades.AbilitySlot1", UI::E_MultiplayerVehicleAbilities_None);
Debug::Enum<UI::E_MultiplayerVehicleAbilities> g_AbilitySlot2("Multiplayer.Upgrades.AbilitySlot2", UI::E_MultiplayerVehicleAbilities_None);
Debug::Enum<UI::E_MultiplayerVehicleAbilities> g_AbilitySlot3("Multiplayer.Upgrades.AbilitySlot3", UI::E_MultiplayerVehicleAbilities_None);

namespace Game
{

C_MultiplayerAbilities::C_MultiplayerAbilities()
{

}

//Pre-cache all of the abilities (Unlocked or otherwise)
void C_MultiplayerAbilities::Init()
{
	BIZ_ASSERT_MSG(C_MultiplayerData::Instance().m_pConfig != 0, "C_MultiplayerAbilities::SetupAbilities - sorcery data hasnt been initiated yet.");

	//For each pack
	for( int slot = 0; slot < C_MultiplayerData::Instance().m_pConfig->m_AbilitySlots.count; slot++ )
	{
		MultiplayerAbilitySlot* pSlot = C_MultiplayerData::Instance().m_pConfig->m_AbilitySlots[slot];

		// add each ability in the slot ( this is a list by category )
		for ( S32 i = 0; i < pSlot->mAbilities.count; i++ )
		{
			XtHandle<MultiplayerAbilityData> hAbility = pSlot->mAbilities[i];
			
			mAbilities[hAbility->mAbilityId] = C_MultiplayerAbility( hAbility );
		}
	}
}

const C_MultiplayerAbility* C_MultiplayerAbilities::GetAbility(const UI::E_MultiplayerVehicleAbilities ability) const
{
	if((U8)ability >= 0 && (U8)ability < mAbilities.Size())
		return &mAbilities[ability];
	else
		return NULL;
}

void C_MultiplayerAbilities::FireAbilityVFX(UI::E_MultiplayerVehicleAbilities ability, Maths::HMat43 matrix, int firingIdx, int targetIdx,
											E_InGameSpecialFX_Trigger priority, float liveTime, Maths::HVec3 scale, Maths::HVec3 offset, bool extraOption)
{
	// If we have some visual effect for this mod
	if(mAbilities[ability].GetSpecialFX())
	{
		C_InGameSpecialFXManager::Instance().CreateSpecialFX(mAbilities[ability].GetSpecialFX(), matrix, firingIdx,
																targetIdx, -1, priority, liveTime, scale, offset, extraOption);
	}
}

const C_MultiplayerAbilities::MultiplayerAbilities* C_MultiplayerAbilities::GetAbilities() const
{
	return &mAbilities;
}

void C_MultiplayerAbilities::ResetLoadOuts(Game::LevelStateCar& car)
{
	car.mLoadOuts.Clear();
}

void C_MultiplayerAbilities::FillOutLevelStateCar(const RacerNet::NetPlayerLoadOut& loadOut, Game::LevelStateCar& car)
{
	ResetLoadOuts(car);

	for(int i = 0; i < loadOut.slots.Size(); i++)
	{			
		//If theres an ability assigned to the slot
		UI::E_MultiplayerVehicleAbilities ability = loadOut.slots[i];
		if(ability != UI::E_MultiplayerVehicleAbilities_None)
		{
			//Get the ability object and apply it
			const C_MultiplayerAbility* pAbility = GetAbility(ability);
			if(pAbility)
			{
				pAbility->ApplyAbility(car);
			}
		}
	}
}

void C_MultiplayerAbilities::ActivateBossSlot(UI::E_MultiplayerVehicleAbilities ability, Game::LevelStateCar& car)
{
	if(ability != UI::E_MultiplayerVehicleAbilities_None)
	{
		//Get the ability object and apply it
		const C_MultiplayerAbility* pAbility = GetAbility(ability);
		if(pAbility)
		{
			pAbility->ApplyAbility(car);
		}
	}
}

UI::E_MultiplayerVehicleAbilities C_MultiplayerAbilities::GetSorceryDefaultAbility(UI::E_MULTIPLAYER_LOADOUT_TYPE type, UI::E_MULTIPLAYER_LOADOUT_SLOT slot, bool ranked) const
{
	if(type == UI::E_MULTIPLAYER_LOADOUT_TYPE_NONE || type >= UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX)
	{
		return UI::E_MultiplayerVehicleAbilities_None;
	}

	MultiplayerAbilityLoadOut* loadOut = 0;

	if(ranked)
	{
		loadOut = &C_MultiplayerData::Instance().m_pConfig->m_LoadOuts[type];
	}
	else
	{
		loadOut = &C_MultiplayerData::Instance().m_pConfig->m_SystemLinkLoadOuts[type];
	}

	if(loadOut)
	{
		switch(slot)
		{
		case UI::E_MULTIPLAYER_LOADOUT_SLOT1:
			if(loadOut->mAbilitySlot1)
				return loadOut->mAbilitySlot1->mAbilityId;
			break;
		case UI::E_MULTIPLAYER_LOADOUT_SLOT2:
			if(loadOut->mAbilitySlot2)
				return loadOut->mAbilitySlot2->mAbilityId;
			break;
		case UI::E_MULTIPLAYER_LOADOUT_SLOT3:
			if(loadOut->mAbilitySlot3)
				return loadOut->mAbilitySlot3->mAbilityId;
			break;
		}
	}

	return UI::E_MultiplayerVehicleAbilities_None;
}

U32 C_MultiplayerAbilities::IronFistActivatedOnPickupType(const E_PICKUP_TYPE pickupType) const
{
	MiscModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings;

	U32 ironFistAdditionalDamagePoints = 0;
	switch(pickupType)
	{
	case E_PICKUP_TYPE_BARGE:
		ironFistAdditionalDamagePoints = (U32)(modSettings.m_IronFistBargeDamage);	
		break;
	case E_PICKUP_TYPE_BLINDFIRE:
		ironFistAdditionalDamagePoints = (U32)(modSettings.m_IronFistBoltDamage);	
		break;
	case E_PICKUP_TYPE_BOMB:
		ironFistAdditionalDamagePoints = (U32)(modSettings.m_IronFistBombDamage);	
		break;
	case E_PICKUP_TYPE_SHOCK:
		ironFistAdditionalDamagePoints = (U32)(modSettings.m_IronFistShockDamage);	
		break;
	case E_PICKUP_TYPE_SHUNT:
		ironFistAdditionalDamagePoints = (U32)(modSettings.m_IronFistShuntDamage);	
		break;
	default:
		return 0;
	}

	return ironFistAdditionalDamagePoints;
}

bool C_MultiplayerAbilities::TitaniumArmourActivatedOnPickupType(const E_PICKUP_TYPE pickupType) const
{
	MiscModSettings& modSettings = C_MultiplayerData::Instance().m_pConfig->m_MiscModSettings;

	U32 ironFistAdditionalDamagePoints = 0;
	switch(pickupType)
	{
	case E_PICKUP_TYPE_BARGE:
		return modSettings.m_TitaniumArmourBlocksBarge;
	case E_PICKUP_TYPE_BLINDFIRE:
		return modSettings.m_TitaniumArmourBlocksBolt;
	case E_PICKUP_TYPE_BOMB:
		return modSettings.m_TitaniumArmourBlocksBomb;
	case E_PICKUP_TYPE_SHOCK:
		return modSettings.m_TitaniumArmourBlocksShock;
	case E_PICKUP_TYPE_SHUNT:
		return modSettings.m_TitaniumArmourBlocksShunt;
	default:
		return false;
	}
}

}
