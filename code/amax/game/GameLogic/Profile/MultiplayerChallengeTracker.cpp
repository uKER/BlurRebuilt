#include "stdafx.h"
#include "GameLogic/Profile/MultiplayerChallengeTracker.h"
#include "GameLogic/Structures/MultiplayerChallenge.h"
#include "GameLogic/StickerManager.h"

#include <Ui/UiInterface.h>
#include <Ui/UiMessages.h>
#include "Ui/SoundEffects.h"

namespace Game
{	

void C_MultiplayerChallengeTracker::Clear()
{
	m_ChallengeList.Clear();
	m_FansEarned = 0;
}

void C_MultiplayerChallengeTracker::HandleCompletedChallenge( const C_MultiplayerChallenge& challenge, const S8 tierCompleted )
{

	// [JBunner]BUG: 37523: fill Challenge complete data from passed in params, so we can handle 
	// multiple completed  tiers of the same challenge
	ChallengeComplete data;
	data.type =	challenge.GetType();
	data.tier = tierCompleted; 
	data.pack = challenge.GetPack();
	data.fansEarned = challenge.GetFansReward( data.tier );

	// cache the challenge that was completed
	m_ChallengeList.PushBack( data );
	m_FansEarned += data.fansEarned;

	// PR - I'll have this please
	_UI_MESSAGE2( UI::E_UiGameMsg_MpChallengeComplete, data.type, data.tier );
	
	// As will I (Says the Sticker Book man)
	Game::C_StickerManager::Instance().CheckMpChallenges(data.type, data.tier);

}

}; // namespace Game
