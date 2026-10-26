#ifndef RICH_PRESENCE_BUILDER_H
#define RICH_PRESENCE_BUILDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <network\Presence.h>

namespace Game // seems like the best place for it, as it needs to read all the game-side stuff most of the time
{

class C_RichPresenceBuilder
{
public:
	static C_RichPresenceBuilder&				Instance() { return s_Instance; }

	bool										BuildRichPresence(const U32 userIndex, RacerNet::eRichPresence& richPresence);

	void										SetJoinSessionInfo(NetPlay::RichPresence& out, const RacerNet::eRichPresence state, bool allowJoinInProgress);

private:
	C_RichPresenceBuilder() {} ;
	~C_RichPresenceBuilder() {};

	static C_RichPresenceBuilder				s_Instance;

	void										GetRankAndLegend(S32& rank, S32& legendLevel);

};

}	// namespace Game


#endif // RICH_PRESENCE_BUILDER_H