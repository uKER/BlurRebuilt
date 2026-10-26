#ifndef LEVEL_STATE_MULTIPLAYER_SETTINGS_H
#define LEVEL_STATE_MULTIPLAYER_SETTINGS_H

#include <Ui\game\Multiplayer\MultiplayerDefines.h>
#include <GameLogic\Structures\MultiplayerAIDifficulty.h>

namespace Game
{
	struct LevelStateMultiplayerSettings
	{
		void Clear()
		{
			mAIMode			= UI::E_AiOff;
			mHandicap		= UI::E_HandicapOff;
			mUpgrades		= UI::E_CarUpgrades_On;
			mModSettings	= UI::E_PowerupMods_On;
			mAlowTimeout	= true;
			mLockToHostCar	= false;
		}

		// Multiplayer specific settings
		UI::E_AiMode						mAIMode;
		UI::E_Handicap						mHandicap;
		UI::E_CarUpgrades					mUpgrades;
		UI::E_PowerupMods					mModSettings;
		bool								mAlowTimeout;
		bool								mLockToHostCar;

		/// Old withered variables that i want to bury some day
		U32									mScoringRules;
		S32									mScoreLimit;
		UI::E_EliminationEvent				mEliminationEvent;					//	How often (if at all) to eliminate a vehicle.
		UI::E_EliminationType				mEliminationType;					//	What to eliminate on during a MP Elimination race.
	};
};



#endif //LEVEL_STATE_MULTIPLAYER_SETTINGS_H	