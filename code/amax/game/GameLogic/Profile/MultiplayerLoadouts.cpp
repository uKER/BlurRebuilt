#include "stdafx.h"
#include "MultiplayerLoadouts.h"
#include "MultiplayerProfile.h"

namespace Game
{

void C_MultiplayerLoadouts::Reset()
{
	mLoadOuts.Empty();
	meCurrentLoadoutType = UI::E_MULTIPLAYER_LOADOUT_TYPE_DEFAULT1;
}

void C_MultiplayerLoadouts::Init()
{
	meCurrentLoadoutType = UI::E_MULTIPLAYER_LOADOUT_TYPE_DEFAULT1;

	//Go through each load out and reset it to its defaults
	for(int i = 0; i < UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX; i++)
	{
		mLoadOuts[i].Reset();
	}
}	

void C_MultiplayerLoadouts::PopulateLoadOutFromSorcery(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType,  const int sorceryLoadOutConfiguration)
{
	MultiplayerAbilityLoadOut& loadOut = C_MultiplayerData::Instance().m_pConfig->m_LoadOuts[sorceryLoadOutConfiguration];

	//Slot 1
	if(loadOut.mAbilitySlot1.Object())
	{
		mLoadOuts[eLoadOutType].SetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT1, loadOut.mAbilitySlot1->mAbilityId);
	}

	//Slot 2
	if(loadOut.mAbilitySlot2.Object())
	{
		mLoadOuts[eLoadOutType].SetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT2, loadOut.mAbilitySlot2->mAbilityId);
	}

	//Slot 3
	if(loadOut.mAbilitySlot3.Object())
	{
		mLoadOuts[eLoadOutType].SetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT3, loadOut.mAbilitySlot3->mAbilityId);
	}
}

const UI::E_MultiplayerVehicleAbilities C_MultiplayerLoadouts::GetAbilityInCurrent(const UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType) const
{
	BIZ_ASSERT_MSG(eAbilitySlotType < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX,"C_MultiplayerProfile::GetAbilityInCurrent() > Invalid slot");
	return mLoadOuts[meCurrentLoadoutType].GetAbilityInSlot(eAbilitySlotType);
}

void C_MultiplayerLoadouts::SetAbilityInCurrent(const UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType, const UI::E_MultiplayerVehicleAbilities ability)
{
	MultiplayerLoadOut& loadOut = mLoadOuts[meCurrentLoadoutType];

	loadOut.SetAbilityInSlot(eAbilitySlotType, ability);
}

const UI::E_MultiplayerVehicleAbilities C_MultiplayerLoadouts::GetAbilityInLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType) const
{
	BIZ_ASSERT_MSG(eLoadOutType < UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX, "C_MultiplayerProfile::GetAbilityInCurrent() > Invalid load out");
	BIZ_ASSERT_MSG(eAbilitySlotType < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX, "C_MultiplayerProfile::GetAbilityInCurrent() > Invalid slot");

	return mLoadOuts[eLoadOutType].GetAbilityInSlot(eAbilitySlotType);
}

void C_MultiplayerLoadouts::SetAbilityInLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType, const UI::E_MultiplayerVehicleAbilities ability)
{
	MultiplayerLoadOut& loadOut = mLoadOuts[eLoadOutType];
	loadOut.SetAbilityInSlot(eAbilitySlotType, ability);
}

const enum UI::E_MULTIPLAYER_LOADOUT_TYPE C_MultiplayerLoadouts::GetCurrentLoadOutType() const
{
	return meCurrentLoadoutType;
}

void C_MultiplayerLoadouts::SetCurrentLoadOutType(const enum UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType)
{
	BIZ_ASSERT_MSG(eLoadOutType < UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX,"C_MultiplayerProfile::SetCurrentLoadOutType() > Value out of range");

	meCurrentLoadoutType = eLoadOutType;
}

void C_MultiplayerLoadouts::Serialise( Sys::BinarySerialiserAutoEndian& stream ) const
{
	Sys::BinarySerialiserAutoEndian out( stream );

	//Number of slots
	out << mLoadOuts.Size();
	//For each slot
	for( LoadOutArray::ConstIterator it = mLoadOuts.Begin(); it != mLoadOuts.End(); it++ )
	{
		(*it).Serialise(stream);
	}

	//Last used slot selection
	out << (U8)meCurrentLoadoutType;
}

void C_MultiplayerLoadouts::Deserialise( const Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );

	int numLoadOuts;
	in >> numLoadOuts;

	for( int i = 0; i < numLoadOuts; i++ )
	{
		mLoadOuts[i].Deserialise(stream);
	}

	//Last used slot selection
	U8 loadOutType = 0;
	in >> loadOutType;
	meCurrentLoadoutType = (UI::E_MULTIPLAYER_LOADOUT_TYPE)loadOutType;
}

void C_MultiplayerLoadouts::BuildNetworkLoadOutData( RacerNet::NetPlayerLoadOut& out, bool ranked, Game::C_MultiplayerAbilities* abilities)
{
	BIZ_ASSERT_MSG(out.slots.Size() == mLoadOuts[meCurrentLoadoutType].slots.Size(), "Number of upgrade slots doesn't match!");

	for(int i = 0; i < out.slots.Size(); i++)
	{
		out.slots[i] = mLoadOuts[meCurrentLoadoutType].slots[i];
		if(out.slots[i] == UI::E_MultiplayerVehicleAbilities_None && abilities)
		{
			out.slots[i] = abilities->GetSorceryDefaultAbility(meCurrentLoadoutType, (UI::E_MULTIPLAYER_LOADOUT_SLOT)i, ranked);
		}
	}
}

const MultiplayerLoadOut& C_MultiplayerLoadouts::GetLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType) const
{
	//Validate load out type
	BIZ_ASSERT_MSG(eLoadOutType < UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX, "C_MultiplayerProfile::GetLoadOut() > Invalid load out");

	return mLoadOuts[eLoadOutType];
}

void C_MultiplayerLoadouts::SetLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const MultiplayerLoadOut loadOut)
{
	//Validate load out type
	BIZ_ASSERT_MSG(eLoadOutType < UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX, "C_MultiplayerProfile::SetLoadOut() > Invalid load out");

	mLoadOuts[eLoadOutType] = loadOut;
}

void C_MultiplayerLoadouts::RenameLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const WCHAR* pName )
{
	//Validate load out type
	BIZ_ASSERT_MSG(eLoadOutType < UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX, "C_MultiplayerProfile::SetLoadOut() > Invalid load out");

	mLoadOuts[eLoadOutType].mLoadOutName = pName;
}


void C_MultiplayerLoadouts::ResetLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType)
{
	//Validate load out type
	BIZ_ASSERT_MSG(eLoadOutType < UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX, "C_MultiplayerProfile::ResetLoadOut() > Invalid load out");

	return mLoadOuts[eLoadOutType].Reset();
}

void C_MultiplayerLoadouts::ClearAbilityInCurrent(const UI::E_MULTIPLAYER_LOADOUT_SLOT eSlotType)
{
	MultiplayerLoadOut &loadOut = mLoadOuts[meCurrentLoadoutType];

	loadOut.SetAbilityInSlot(eSlotType, UI::E_MultiplayerVehicleAbilities_None);
}

void C_MultiplayerLoadouts::ClearAbilityInLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const UI::E_MULTIPLAYER_LOADOUT_SLOT eSlotType)
{
	BIZ_ASSERT_MSG(eLoadOutType < UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX, "C_MultiplayerProfile::ClearAbilityInLoadOut() > Invalid load out");
	MultiplayerLoadOut &loadOut = mLoadOuts[eLoadOutType];

	loadOut.SetAbilityInSlot(eSlotType, UI::E_MultiplayerVehicleAbilities_None);
}

void C_MultiplayerLoadouts::PopulateAllFromSorcery()
{
	//Go through the load outs in sorcery
	UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType = UI::E_MULTIPLAYER_LOADOUT_TYPE_NONE;
	for(int i = 0; i < C_MultiplayerData::Instance().m_pConfig->m_LoadOuts.count; i++ )
	{
		eLoadOutType = C_MultiplayerData::Instance().m_pConfig->m_LoadOuts[i].mType;

		PopulateLoadOutFromSorcery(eLoadOutType, i);
	}
}

void C_MultiplayerLoadouts::PostRaceUpdate(const C_MultiplayerProfile& profile)
{
	bool loadOutEmpty=true;

	const MultiplayerLoadOut& loadOut=mLoadOuts[UI::E_MULTIPLAYER_LOADOUT_TYPE_DEFAULT1];

	for(int i=0; i<loadOut.slots.Size(); i++)
	{
		if (loadOut.slots[i]!=UI::E_MultiplayerVehicleAbilities_None)
		{
			loadOutEmpty=false;
			break;
		}
	}

	if (profile.GetFeatureUnlocked(UI::E_MultiplayerFeature_ModShop)&&loadOutEmpty)
	{
		PopulateAllFromSorcery();
	}
}


}
