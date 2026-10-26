#ifndef GAMEMODES_H
#define GAMEMODES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameModes/GameModeDefines.h>
#include <GameLogic/GameModes/GameModeBase.h>

#include <GameLogic/GameModes/GameModeDebug.h>
#include <GameLogic/GameModes/GameModeSinglePlayer.h>
#include <GameLogic/GameModes/GameModeMultiPlayer.h>
#include <GameLogic/GameModes/GameModeNetwork.h>
#include <GameLogic/GameModes/GameModeOnline.h>
#include <GameLogic/GameModes/GameModeSplitScreen.h>
#include <GameLogic/GameModes/GameModeSystemLink.h>


namespace Game
{

class C_GameModeStore
{

public:
	C_GameModeStore()
	{
		//Make an array of them (in E_GameMode order)
		mGameModes.PushBack(&mGameModeNothing);
		mGameModes.PushBack(&mGameModeDebug);
		mGameModes.PushBack(&mGameModeSinglePlayer);
		mGameModes.PushBack(&mGameModeMultiPlayer);
		mGameModes.PushBack(&mGameModeNetwork);
		mGameModes.PushBack(&mGameModeOnline);
		mGameModes.PushBack(&mGameModeSplitScreen);
		mGameModes.PushBack(&mGameModeSystemLink);
	};

	inline C_GameModeBase* operator() (E_GameMode gm)
	{
		return mGameModes[gm];
	}

private:
	Ut::Vector<C_GameModeBase*> mGameModes;

	//Create all the game modes
	C_GameModeBase			mGameModeNothing;
	C_GameModeDebug			mGameModeDebug;
	C_GameModeSinglePlayer	mGameModeSinglePlayer;
	C_GameModeMultiPlayer	mGameModeMultiPlayer;
	C_GameModeNetwork		mGameModeNetwork;
	C_GameModeOnline		mGameModeOnline;
	C_GameModeSplitScreen	mGameModeSplitScreen;
	C_GameModeSystemLink	mGameModeSystemLink;
};

extern C_GameModeStore gGameModeStore;

} // namespace Game

#endif // GAMEMODES_H
