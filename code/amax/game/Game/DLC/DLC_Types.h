#ifndef DLC_TYPES_H
#define DLC_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/xtgame.h>
#include <GameLogic/GameData/GameDataDefs.h>
#include <Game\GameControls\GameControls.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>

enum E_CHEAT_TYPE
{
	E_CHEAT_TYPE_NONE,
	E_CHEAT_TYPE_FANS,
	E_CHEAT_TYPE_INCOME,
	E_CHEAT_TYPE_MULTIPLAYER_EVENT,
	E_CHEAT_TYPE_CHROME,
	E_CHEAT_TYPE_VEHICLE,
	E_CHEAT_TYPE_UNLOCK_ID,
};

struct RaceBookVehicle;
struct AmaxVehicle;
struct MultiplayerEvent;

class AmaxCheatBase : public AmaxGameDataObj
{
public:
	virtual E_CHEAT_TYPE GetType() { return E_CHEAT_TYPE_NONE; };
};

class AmaxCheatIncome : public AmaxCheatBase
{
public:
	virtual E_CHEAT_TYPE GetType() { return E_CHEAT_TYPE_INCOME; };

	float	mMultiplier;
	int		mNumRaces;
	bool	mSingleplayer;
	bool	mMultiplayer;
};

class AmaxCheatFans : public AmaxCheatBase
{
public:
	virtual E_CHEAT_TYPE GetType() { return E_CHEAT_TYPE_FANS; };

	float						mMultiplier;
	int							mNumRaces;
	XtArray<UI::E_RaceType>		mRaceTypes;
};

class AmaxCheatMultiplayerEvent : public AmaxCheatBase
{
public:
	virtual E_CHEAT_TYPE GetType() { return E_CHEAT_TYPE_MULTIPLAYER_EVENT; };

	XtHandle<MultiplayerEvent>	mEvent;
};

class AmaxCheatUnlockId : public AmaxCheatBase
{
public:
	virtual E_CHEAT_TYPE GetType() { return E_CHEAT_TYPE_UNLOCK_ID; };

	int mId;
};

class AmaxCheatVehicle : public AmaxCheatBase
{
public:
	virtual E_CHEAT_TYPE GetType() { return E_CHEAT_TYPE_VEHICLE; };

	XtHandle<RaceBookVehicle>	mRaceBookVehicle;
	XtHandle<AmaxVehicle>		mAmaxVehicle;
};

class AmaxCheatChromeVehicle : public AmaxCheatBase
{
public:
	AmaxCheatChromeVehicle() : mCurrentSequenceIdx(0) 
	{
		mButtonSequence[0] = mButtonSequence[2] = Game::E_UiButton_LeftTrigger;
		mButtonSequence[1] = mButtonSequence[3] = Game::E_UiButton_RightTrigger;
	};

	virtual E_CHEAT_TYPE GetType() { return E_CHEAT_TYPE_CHROME; };

	XtHandle<AmaxVehicle>											mAmaxVehicle;
	XtHandle<AmaxVehicle>											mAmaxVehicleUnlock;

	Btl::Vector<Game::E_UiButton, Btl::VectorParameters_Array<4> >	mButtonSequence;
	int																mCurrentSequenceIdx;
};

class AmaxDLCPack : public AmaxGameDataObj
{
public:
	U32				mPackID;
	XtArray<XtSz8>	mLoadableItems;
};

#endif //DLC_TYPES_H