#include "GameLogic/MultiplayerCarChallenges.h"

#include <system/serialise/BinarySerialiseAutoEndian.h>

namespace Game
{
	void C_MultiplayerCarChallenges::BuildChallengesFromSorcery(XtArray<XtHandle<MultiplayerChallenge> > challenges, S32 level, UI::E_ChallengePack pack )
	{
		BIZ_ASSERT_MSG_SPEED( challenges[0].Object(), "Invalid car challenge(Completed Races)");
		mCompletedRaces = C_MultiplayerChallenge( challenges[0], level, pack);

		BIZ_ASSERT_MSG_SPEED( challenges[1].Object(), "Invalid car challenge(Winner)");
		mWinner = C_MultiplayerChallenge( challenges[1], level, pack );
	}

	void C_MultiplayerCarChallenges::ReloadSorceryData( XtArray<XtHandle<MultiplayerChallenge> > challenges, S32 level )
	{
		BIZ_ASSERT_MSG_SPEED( challenges[0].Object(), "Invalid car challenge(Completed Races)");
		mCompletedRaces.ReloadSorceryData( challenges[0], level );

		BIZ_ASSERT_MSG_SPEED( challenges[1].Object(), "Invalid car challenge(Winner)");
		mWinner.ReloadSorceryData( challenges[1], level );
	}

	C_MultiplayerChallenge& C_MultiplayerCarChallenges::GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE eType)
	{
		switch(eType)
		{
			case UI::E_MULTIPLAYER_CHALLENGE_TYPE_COMPLETED_RACES:
			{
				return mCompletedRaces;
			}
			case UI::E_MULTIPLAYER_CHALLENGE_TYPE_WINNER:
			{
				return mWinner;
			}
			default:
			{
				BIZ_ASSERT_FAIL( "Car challenge doesnt exist" );
				return mCompletedRaces; //to shut the compiler up
			}
		}
	}

	const Btl::BitArray16& C_MultiplayerCarChallenges::GetUpgradesEnabled() const 								
	{
		return mUpgrades;
	}
		
	void C_MultiplayerCarChallenges::SetUpgradesEnabled( Btl::BitArray16 upgrades )
	{
		mUpgrades |= upgrades;
	}

	void C_MultiplayerCarChallenges::Reset()
	{
		mCompletedRaces.Reset();
		mWinner.Reset();
		mUpgrades.ClearBits();
	}

	void C_MultiplayerCarChallenges::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		U16 carUpgrades = *mUpgrades.Data();
		stream << mCompletedRaces << mWinner << carUpgrades;
	}
	void C_MultiplayerCarChallenges::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		U16 carUpgrades;
		stream >> mCompletedRaces >> mWinner >> carUpgrades;
		*(mUpgrades.Data()) = carUpgrades;
	}

	const bool C_MultiplayerCarChallenges::GetValid() const
	{
		return mValid;
	}

	void C_MultiplayerCarChallenges::SetValid( const bool val )
	{
		mValid = val;
	}

	C_MultiplayerCarChallenges::C_MultiplayerCarChallenges():
		mValid(true)
	{

	}
}; // namespace Game
