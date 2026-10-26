#ifndef GAME_MULTIPLAYER_CHALLENGES_H
#define GAME_MULTIPLAYER_CHALLENGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\GameData\MultiplayerData.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <UI\Game\UILua.h>

extern Debug::Bool g_OverrideHiddenProgress;

// Forward declarations
namespace Sys
{
	class BinarySerialiserAutoEndian;
	class BinaryDeserialiserAutoEndian;
}

namespace Game
{
class C_MultiplayerChallengeSystem;

/*---------------------------------------------------------------------------------------
	Name : C_MultiplayerChallenge

	Desc : Challenge object which tracks the progress and rewards for the challenge.
---------------------------------------------------------------------------------------*/
class C_MultiplayerChallenge
{
public:
	friend class C_MultiplayerChallengeSystem;	// so the challenge system can call our private methods.

	C_MultiplayerChallenge()
	:	m_Amount( 0 ),
		m_Tier( 0 ),
		m_Data( 0 ),
		m_Type( UI::E_MULTIPLAYER_CHALLENGE_TYPE_NONE ),
		m_UnlockLevel(0),
		m_Pack(UI::E_ChallengePack_Ace)
	{
	}

	C_MultiplayerChallenge( XtHandle<MultiplayerChallenge>& data, S32 unlockLevel, UI::E_ChallengePack pack )
	:	m_Amount( 0 ),
		m_Tier( 0 ),
		m_Data( data ),
		m_Type( data->mType ),
		m_UnlockLevel(unlockLevel),
		m_Pack(pack)
	{
	}

	void 								Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void 								Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

										// TODO_MP_TS - remove me post beta and fix up the multiplayer packs
	void								ReloadSorceryData( XtHandle<MultiplayerChallenge>& data, S32 unlockLevel );

	void								SendToLua( lua_State* pState ) const;

										// Returns a paramitised value of the progress for the current tier
	F32									GetProgress(const bool overall=false) const;

										// Returns true if all tiers of the challenge have been completed
	bool								IsCompleted() const;

										// Returns true if the given level meets the required level to own this challenge
	bool								IsUnlocked(const S32 level) const;

										// Returns the challenge name
	XtLocId								GetName() const
										{
											return m_Data->mName;
										}

										// Returns the challenge description
	XtLocId								GetDesc() const
										{
											return m_Data->mDescription;
										}

	UI::E_MULTIPLAYER_CHALLENGE_TYPE	GetType() const
										{
											return m_Type;
										}

										// Returns the current amount of this challenge.
	S32									GetAmount() const
										{
											return m_Amount;
										}

										// Returns the target amount for the current challenge tier
	S32									GetTargetAmount(bool getPrevTier = false) const
										{
											if(getPrevTier == true)
											{
												if((m_Tier-1) < 0) //make sure we dont try and go below 0!
												{
													BIZ_ASSERT_MSG(false, "TRYING TO INDEX BELOW 0 - TELL AS");
													return (S32)m_Data->mTiers[0].mTargetAmount;													
												}
												else
												{
													return (S32)m_Data->mTiers[m_Tier-1].mTargetAmount;
												}
											}
											else
											{
												return (S32)m_Data->mTiers[m_Tier].mTargetAmount;
											}
										}

										// Returns the target amount for the given tier
	S32									GetTargetAmount( const S32 sTier ) const
										{
											BIZ_ASSERT( sTier >= 0 && sTier < m_Data->mTiers.count );
											return (S32)m_Data->mTiers[sTier].mTargetAmount;
										}
										// Returns the data element for the current challenge tier
	S32									GetData() const
										{
											return (S32)m_Data->mTiers[m_Tier].mData;
										}

										// Returns the data element for the given tier
	S32									GetData( const S32 sTier ) const
										{
											BIZ_ASSERT( sTier >= 0 && sTier < m_Data->mTiers.count );
											return (S32)m_Data->mTiers[sTier].mData;
										}

										// Resets the challenge progress to 0
	void								Reset()
										{
											m_Amount = 0;
											m_Tier = 0;
										}

										// Returns the current challenge tier. 
	S32									GetTier() const
										{
											return m_Tier;
										}

										// Returns the maximum number of challenge tiers.
	S32									GetMaxTiers() const
										{
											return m_Data->mTiers.count;
										}

										// Returns the fan reward for the given tier.
	S32									GetFansReward( const S32 sTier ) const
										{
											BIZ_ASSERT( sTier >= 0 && sTier < m_Data->mTiers.count );
											return m_Data->mTiers[sTier].mFansReward;
										}

	UI::E_MultiplayerVehicleUpgrades	GetReward( const S32 sTier ) const
										{
											BIZ_ASSERT( sTier >= 0 && sTier < m_Data->mTiers.count );
											return m_Data->mTiers[sTier].mReward;
										}

	S32									GetUnlockLevel() const
										{
											return m_UnlockLevel;
										}

	UI::E_ChallengePack					GetPack() const
										{
											return m_Pack;
										}

										// Flag so the UI knows whether to render the amount and target
	bool								HideProgress() const
										{
											if(g_OverrideHiddenProgress)
											{
												return false;
											}
											else
											{
												return m_Data->mHideProgress;
											}
										}

										// Force this challenge's progress to the max
	void								ForceComplete();

private:
										// Increases the current amount of the challenge. It is capped to the max tiers target.
										// This is private because the challenge system logs the returned value so it can keep
										// track of challenges completed that race. Returns the amount of tiers completed.
	S32									IncreaseAmount( const S32 sAmount = 1 );

private:
	S32									m_Amount;		// Current amount
	S32									m_Tier;			// Current tier of the challenge. Some challenges go up to 3
	S32									m_UnlockLevel;	// Level required to use this challenge
	UI::E_MULTIPLAYER_CHALLENGE_TYPE	m_Type;			// Type identifier
	XtHandle<MultiplayerChallenge>		m_Data;			// Handle to the sorcery record
	UI::E_ChallengePack					m_Pack;			// Pack the challenge belongs to
};

}; // namespace Game

#endif // GAME_MULTIPLAYER_CHALLENGES_H
