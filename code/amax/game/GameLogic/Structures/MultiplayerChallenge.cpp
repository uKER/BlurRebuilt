#include "stdafx.h"

#include <GameLogic/Structures/MultiplayerChallenge.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include "profile/multiplayerchallengetracker.h"

Debug::Bool g_OverrideHiddenProgress("Multiplayer.Challenges.OverrideHiddenProgress", false );

namespace Game
{

void C_MultiplayerChallenge::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	stream << m_Amount << m_Tier;
}

void C_MultiplayerChallenge::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	stream >> m_Amount >> m_Tier;
}

void C_MultiplayerChallenge::ReloadSorceryData( XtHandle<MultiplayerChallenge>& data, S32 unlockLevel )
{
	m_Data = data;
	m_UnlockLevel = unlockLevel;
}

void C_MultiplayerChallenge::SendToLua( lua_State* pState ) const
{
	Lua::AddDataMember( pState, "type",		GetType() );
	Lua::AddDataMember( pState, "tier",		GetTier() + 1 );
	Lua::AddDataMember( pState, "tiers",	GetMaxTiers() );
	Lua::AddDataMember( pState, "progress",	GetProgress() );
	Lua::AddDataMember( pState, "complete",	IsCompleted() );
}

S32 C_MultiplayerChallenge::IncreaseAmount( const S32 sAmount )
{
	const S32 maxTier = m_Data->mTiers.count-1;
	const S32 maxAmount = (S32)m_Data->mTiers[maxTier].mTargetAmount;

	// This challenge has been completed
	if ( m_Amount == maxAmount )
		return 0;

	// Add on the amount and check to see if we've completed the current challenge part.
	m_Amount = Maths::Clamp( m_Amount + sAmount, 0, maxAmount );

	S32 target = (S32)m_Data->mTiers[m_Tier].mTargetAmount;
	S32 tiersComplete = 0;

	// current target
	while ( m_Amount >= target )
	{
		tiersComplete++;

		// Increase the tier if there are outstanding parts to the challenge
		if ( m_Tier < m_Data->mTiers.count-1 )
		{
			m_Tier++;
			target = (S32)m_Data->mTiers[m_Tier].mTargetAmount;
		}
		else
		{
			break;
		}
	}
	return tiersComplete;
}

F32	C_MultiplayerChallenge::GetProgress(const bool overall) const
{
	F32 target = 0.f;
	if(overall)
	{
		int noTiers = m_Data->mTiers.count;

		//Work out the total target
		target = (F32)( m_Data->mTiers[noTiers-1].mTargetAmount );
	}
	else
	{
		target = (F32)( m_Data->mTiers[m_Tier].mTargetAmount );
	}

	if(HideProgress())
	{
		//If we're hiding the progress of the challenge its all or nothing
		if( (m_Amount / target) >= 1.f )
		{
			return 1.f;
		}
		else 
		{
			return 0.f;
		}

	}
	else
	{
		return Maths::Clamp( m_Amount / target, 0.0f, 1.0f );
	}
}

bool C_MultiplayerChallenge::IsCompleted() const
{
	const S32 maxTier = m_Data->mTiers.count-1;
	const S32 maxAmount = (S32)m_Data->mTiers[maxTier].mTargetAmount;

	return ( m_Amount == maxAmount );
}


void C_MultiplayerChallenge::ForceComplete()
{
	const S32 maxTier = m_Data->mTiers.count-1;
	const S32 maxAmount = (S32)m_Data->mTiers[maxTier].mTargetAmount;

	m_Amount = maxAmount;
	m_Tier = maxTier;
}
bool C_MultiplayerChallenge::IsUnlocked(const S32 level) const
{
	return level>=m_UnlockLevel;
}

}; // namespace Game
