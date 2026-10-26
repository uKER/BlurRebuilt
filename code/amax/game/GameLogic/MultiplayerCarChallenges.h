#ifndef GAME_MULTIPLAYER_CAR_CHALLENGES_H
#define GAME_MULTIPLAYER_CAR_CHALLENGES_H

#include "UI/Game/Multiplayer/MultiplayerDefines.h"
#include "GameLogic/Structures/MultiplayerChallenge.h"
#include <btl/Containers/bitvector.h>

namespace Sys
{
	class BinarySerialiserAutoEndian;
	class BinaryDeserialiserAutoEndian;
}

namespace Game
{

	/*---------------------------------------------------------------------------------------
		Name : C_MultiplayerCarChallenges

		Desc : Container for all challenges that are completed on a per-vehicle basis. Note
				that new per-vehicle challenges must be added into here by hand in the 
				appropriate places.
	---------------------------------------------------------------------------------------*/
	class C_MultiplayerCarChallenges
	{
	private:
		C_MultiplayerChallenge			mCompletedRaces;
		C_MultiplayerChallenge			mWinner;
		Btl::BitArray16					mUpgrades;
		bool							mValid;
	public:
		C_MultiplayerCarChallenges();

		C_MultiplayerChallenge&			GetCompletedRacesChallenge() { return mCompletedRaces; }
		C_MultiplayerChallenge&			GetWinnerChallenge() { return mWinner; }

		C_MultiplayerChallenge&			GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE eType);

		void							Reset();	
		const Btl::BitArray16&			GetUpgradesEnabled() const; 								
		void							SetUpgradesEnabled( Btl::BitArray16 upgrades );

		void BuildChallengesFromSorcery(XtArray<XtHandle<MultiplayerChallenge> > challenges, S32 level, UI::E_ChallengePack pack );

		// TODO_MP_TS - remove me post beta and fix up the multiplayer packs
		void ReloadSorceryData( XtArray<XtHandle<MultiplayerChallenge> > challenges, S32 level );

		void							Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void							Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		const bool						GetValid() const;
		void							SetValid(const bool val);

	};
}

#endif // GAME_MULTIPLAYER_CAR_CHALLENGES_H
