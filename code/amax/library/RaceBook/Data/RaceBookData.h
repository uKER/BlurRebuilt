#ifndef RACEBOOK_DATA_H
#define RACEBOOK_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameData/GameVehicleData.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <GameLogic/GameData/GameData.h>

enum RaceBookQuantifier
{
	E_QUANTIFIER_NULL,
	E_QUANTIFIER_LESSTHAN,
	E_QUANTIFIER_LESSTHANOREQUAL,
	E_QUANTIFIER_EQUAL,
	E_QUANTIFIER_GREATERTHANOREQUAL,
	E_QUANTIFIER_GREATERTHAN
};

enum RaceBookUiIconOveride
{
	E_RACEBOOK_ICONOVERIDE_NONE,
	E_RACEBOOK_ICONOVERIDE_MESSAGE,
	E_RACEBOOK_ICONOVERIDE_AUDIO,
	E_RACEBOOK_ICONOVERIDE_PHOTO,
	E_RACEBOOK_ICONOVERIDE_VIDEO,
	E_RACEBOOK_ICONOVERIDE_PRIORITYPHOTO,
	E_RACEBOOK_ICONOVERIDE_PRIORITYVIDEO,
	E_RACEBOOK_ICONOVERIDE_LINKTOGARAGE,
	E_RACEBOOK_ICONOVERIDE_LINKTOOPTIONS,
	E_RACEBOOK_ICONOVERIDE_LINKTOSHOP,
	E_RACEBOOK_ICONOVERIDE_GIFTCAR,
	E_RACEBOOK_ICONOVERIDE_GIFTCASH,
	E_RACEBOOK_ICONOVERIDE_GIFTFANS,
	E_RACEBOOK_ICONOVERIDE_EVENT,
	E_RACEBOOK_ICONOVERIDE_CARCHALLENGE,
};

enum RaceBookCharacterType
{
	E_CHARACTER_TYPE_PLAYER,
	E_CHARACTER_TYPE_BUDDY,
	E_CHARACTER_TYPE_LEAD_GIRL,
	E_CHARACTER_TYPE_ARCH_RIVAL,
	E_CHARACTER_TYPE_MENTOR,
	E_CHARACTER_TYPE_NORMAL,
	E_CHARACTER_TYPE_LEAD,
	E_CHARACTER_TYPE_BOSS,
	E_CHARACTER_TYPE_ANY,
};

struct RaceBookBase : AmaxGameDataObj
{

};

struct RaceBookVehicle : RaceBookBase
{
	XtHandle<AmaxVehicle> vehicle;
	bool isForSale;
	int  cost;
	bool disallowedInSplitscreen; 


	XtLocId sell_text;
	XtHandle<RaceBookCharacter> seller;
	XtHandle<AmaxEvent> put_up_for_sale_after;

	int mNumberOfBaseSlots;
	XtArray<int> mAdditionalSlotsCost;
};

struct RaceBookCharacter : RaceBookBase
{
	XtHandle<AmaxBaseCharacter> base_character;

	XtHandle<AIXt> ai_template;

	XtArray< XtHandle<RaceBookVehicle> > garage;
	RaceBookCharacterType type;

	XtLocId status;
	int cash, level, perkLevel, xpPoints, racesWon, racesLost;
	XtU16 avatarID;

	XtSz8 dialogueGroup;
	float dialogueProbability;

	// -----------
	// Personality
	// -----------
	XtArray< XtHandle< RaceBookCharacter> > charactersNeverTargetted;
	XtArray< E_PICKUP_TYPE > preferredPickups;
};

struct RaceBookDefaults : RaceBookBase
{
	XtArray<int> experience_per_level;
	XtArray<int> perk_level_points_gained_per_level;
};

class C_RaceBookData
{
public:
	void Init();

	static C_RaceBookData& Instance() {return s_instance;}

	AmaxGameData<RaceBookCharacter*> mCharacters;
	AmaxGameData<RaceBookVehicle*> mVehicles;
	AmaxGameData<RaceBookDefaults*> mDefaults;

	XtPack*	mXtAmaxPack;

private:
	static C_RaceBookData s_instance;

};

#endif // RACEBOOK_DATA_H
