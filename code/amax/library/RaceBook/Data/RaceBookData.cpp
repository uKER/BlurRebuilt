#include "stdafx.h"

#include <RaceBook/Data/RaceBookData.h>

C_RaceBookData C_RaceBookData::s_instance;

#define RACEBOOK_DATA_CALLBACK(Type, Member) \
	class Callback##Type { public: AmaxGameData<Type*>* operator()() { return &C_RaceBookData::Instance().Member; } };

XT_TYPE(RaceBookBase,"RaceBookBase")
	XT_BASE(AmaxGameDataObj)
XT_END_TYPE

RACEBOOK_DATA_CALLBACK(RaceBookCharacter, mCharacters)
XT_TYPE(RaceBookCharacter,"RaceBookCharacter")
	XT_CALLBACK((GameDataHandler<RaceBookCharacter,CallbackRaceBookCharacter>))
	XT_FIELD(base_character,"BaseCharacter")
	XT_FIELD(ai_template,"AITemplate")
	XT_FIELD(status, "Status")
	XT_FIELD(cash, "Cash")
	XT_FIELD(level, "Level")
	XT_FIELD(perkLevel, "PerkLevel")
	XT_FIELD(xpPoints, "XpPoints")
	XT_FIELD(racesWon, "RacesWon")
	XT_FIELD(racesLost, "RacesLost")
	XT_FIELD(avatarID, "AvatarID")
	XT_BASE(RaceBookBase)
	XT_FIELD(type,"Type")
	XT_FIELD(garage,"Garage")
	XT_FIELD(dialogueGroup, "DialogueGroup")
	XT_FIELD(dialogueProbability, "DialogueProbability")

	XT_FIELD(charactersNeverTargetted, "CharactersNeverTargetted")
	XT_FIELD(preferredPickups, "PreferredPickups")
XT_END_TYPE

XT_TYPE( RaceBookCharacterType, "CharacterType" )
	XT_OPTION( E_CHARACTER_TYPE_PLAYER, "Player" )
	XT_OPTION( E_CHARACTER_TYPE_BUDDY, "Buddy" )
	XT_OPTION( E_CHARACTER_TYPE_LEAD_GIRL, "LeadGirl" )
	XT_OPTION( E_CHARACTER_TYPE_ARCH_RIVAL, "ArchRival" )
	XT_OPTION( E_CHARACTER_TYPE_MENTOR, "Mentor" )
	XT_OPTION( E_CHARACTER_TYPE_NORMAL, "NormalCharacter" )
	XT_OPTION( E_CHARACTER_TYPE_LEAD, "Lead" )
	XT_OPTION( E_CHARACTER_TYPE_BOSS, "Boss" )
	XT_OPTION( E_CHARACTER_TYPE_ANY, "Anybody")
XT_END_TYPE

XT_TYPE( RaceBookUiIconOveride, "IconOverideType" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_NONE,				"None" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_MESSAGE,			"Message" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_AUDIO,			"Audio" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_PHOTO,			"Photo" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_VIDEO,			"Video" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_PRIORITYPHOTO,	"PriorityPhoto" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_PRIORITYVIDEO,	"PriorityVideo" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_LINKTOGARAGE,		"LinkToGarage" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_LINKTOOPTIONS,	"LinkToOptions" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_LINKTOSHOP,		"LinkToShop" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_GIFTCAR,			"GiftCar" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_GIFTCASH,			"GiftCash" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_GIFTFANS,			"GiftFans" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_EVENT,			"Event" )
	XT_OPTION( E_RACEBOOK_ICONOVERIDE_CARCHALLENGE,		"CarChallenge" )
XT_END_TYPE

RACEBOOK_DATA_CALLBACK(RaceBookVehicle, mVehicles)
	XT_TYPE(RaceBookVehicle	,"RaceBookVehicle")
	XT_CALLBACK((GameDataHandler<RaceBookVehicle,CallbackRaceBookVehicle>))
	XT_BASE(RaceBookBase)
	XT_FIELD(vehicle,"Vehicle")
	XT_FIELD(isForSale, "ForSale")
	XT_FIELD(cost,"Cost")

	XT_FIELD(sell_text,"SellText")
	XT_FIELD(seller,"Seller")
	XT_FIELD(put_up_for_sale_after,"PutUpForSaleAfter")
	XT_FIELD(disallowedInSplitscreen,"DisallowedInSplitscreen")

	XT_FIELD(mNumberOfBaseSlots,"NumberOfBaseSlots")
	XT_FIELD(mAdditionalSlotsCost,"AdditionalSlotsCost")
XT_END_TYPE

XT_TYPE( RaceBookQuantifier, "Quantifier" )
	XT_OPTION( E_QUANTIFIER_NULL, "Null" )
	XT_OPTION( E_QUANTIFIER_LESSTHAN, "LessThan" )
	XT_OPTION( E_QUANTIFIER_LESSTHANOREQUAL, "LessThanOrEqual" )
	XT_OPTION( E_QUANTIFIER_EQUAL, "Equal" )
	XT_OPTION( E_QUANTIFIER_GREATERTHANOREQUAL, "GreaterThanOrEqual" )
	XT_OPTION( E_QUANTIFIER_GREATERTHAN, "GreaterThan" )
XT_END_TYPE

RACEBOOK_DATA_CALLBACK(RaceBookDefaults, mDefaults)
XT_TYPE( RaceBookDefaults, "RaceBookDefaults" )
	XT_CALLBACK((GameDataHandler<RaceBookDefaults,CallbackRaceBookDefaults>))
	XT_BASE(RaceBookBase)
	XT_FIELD(experience_per_level, "ExperiencePerLevel")
	XT_FIELD(perk_level_points_gained_per_level, "PerkLevelPointsGainedPerLevel")
XT_END_TYPE

XT_LIBRARY(RACEBOOKDATA)
	XT_REGISTER_TYPE(RaceBookBase)
	XT_REGISTER_TYPE(RaceBookQuantifier)
	XT_REGISTER_TYPE(RaceBookCharacter)
	XT_REGISTER_TYPE(RaceBookVehicle)
	XT_REGISTER_TYPE(RaceBookCharacterType)
	XT_REGISTER_TYPE(RaceBookDefaults)

	XT_REGISTER_TYPE(RaceBookUiIconOveride)
XT_END_LIBRARY

void C_RaceBookData::Init()
{
	//Debug Code to print out the Sorcery IDs for Agora
	//printf("Characters\n");
	//for(int a=0;a<mCharacters.Number();a++)
	//{
	//	printf("ID %x = %S\n",mCharacters[a]->ID,UI::C_TextStringID(mCharacters[a]->loc_string.id).String());
	//}

	mCharacters.AlphaSort();
	mVehicles.TagSort();
}