#ifndef _MULTIPLAYER_CHALLENGE_TRACKER_H_
#define _MULTIPLAYER_CHALLENGE_TRACKER_H_
#include <Ui/Game/Multiplayer/MultiplayerDefines.h>	// [Blur2PC]

namespace Game
{

class C_MultiplayerChallenge;

class C_MultiplayerChallengeTracker
{
public:
	struct ChallengeComplete
	{
		UI::E_MULTIPLAYER_CHALLENGE_TYPE	type;
		S8									tier;
		UI::E_ChallengePack					pack;
		S32									fansEarned;
	};

	typedef Btl::Vector< ChallengeComplete > TChallengeList;

public:
	C_MultiplayerChallengeTracker()
		: m_FansEarned( 0 )
	{
	}

	void						Clear(); // call this before the race starts

	void						HandleCompletedChallenge( const C_MultiplayerChallenge& challenge, const S8 tierCompleted );
	
	const TChallengeList&		GetChallengesComplete() const 
								{
									return m_ChallengeList;
								}

	S32							GetFansEarned() const
								{
									return m_FansEarned;
								}

	S32							GetNumChallengesComplete() const
								{
									return m_ChallengeList.Size();
								}

private:
	TChallengeList				m_ChallengeList;
	S32							m_FansEarned;
};

}

#endif // _MULTIPLAYER_CHALLENGE_TRACKER_H_
