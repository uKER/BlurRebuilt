#include "stdafx.h"

#include <Game\dlc\DLC_Controller.h>
#include <GameLogic\GameData\GameData.h>
#include <Ui\game\Multiplayer\MultiplayerHelpers.h>
#include <GameLogic\Profile\MultiplayerStatistics.h>

namespace UI
{
	bool DLCController::IsGameDataObject(XtBase& base)
	{
		// Hardcoded the type, because there's no other way of getting it. Nice. 
		if(!strcmp(base.type->name, "DLCBase"))
			return true;

		for( U32 j = 0; j < base.type->baseCount; j++ )
			return IsGameDataObject(base.type->bases[j]);

		return false;
	}

	bool DLCController::IdIsUnlocked(int id)
	{
		for( int i = 0; i < C_GameData::Instance().mCheatPacks.Number(); i++ )
		{
			AmaxCheatBase* pCheatBase = C_GameData::Instance().mCheatPacks[i];
			if( pCheatBase && pCheatBase->GetType() == E_CHEAT_TYPE_UNLOCK_ID && pCheatBase->dlcLicensed )
			{
				AmaxCheatUnlockId* pUnlockIdCheat = (AmaxCheatUnlockId*)pCheatBase;

				if(pUnlockIdCheat && pUnlockIdCheat->mId > 0 && pUnlockIdCheat->mId == id)
					return true;
			}
		}

		return false;
	}

	float DLCController::GetFansMultiplier(Game::MultiplayerStatistics* pStatistics, Game::E_RaceMode raceMode)
	{
		float multiplier = 1.f;

		if(pStatistics)
		{
			for( int i = 0; i < C_GameData::Instance().mCheatPacks.Number(); i++ )
			{
				AmaxCheatBase* pCheatBase = C_GameData::Instance().mCheatPacks[i];
				if( pCheatBase && pCheatBase->GetType() == E_CHEAT_TYPE_FANS && pCheatBase->dlcLicensed )
				{
					AmaxCheatFans* pFansCheat = (AmaxCheatFans*)pCheatBase;

					int numRaces = 0;
					for( int k = 0; k < pFansCheat->mRaceTypes.count; k++ )
					{
						Game::E_RaceMode cheatRaceMode = ConvertUiToGame_RaceMode(pFansCheat->mRaceTypes[k]);
						numRaces += pStatistics->mGameModesPlayed[cheatRaceMode];
					}

					if(pFansCheat && pFansCheat->mMultiplier > 0 && numRaces < pFansCheat->mNumRaces)
					{
						for( int j = 0; j < pFansCheat->mRaceTypes.count; j++ )
						{
							if(ConvertUiToGame_RaceMode(pFansCheat->mRaceTypes[j]) == raceMode)
							{
								multiplier *= pFansCheat->mMultiplier;
								break;
							}
						}
					}
				}
			}	
		}

		return multiplier;
	}
}

