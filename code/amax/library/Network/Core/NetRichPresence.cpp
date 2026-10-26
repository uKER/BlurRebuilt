#include <stdafx.h>
#include <Core/core.h>
#include <SystemProfile/SystemProfileManager.h>
#include <SystemProfile/SystemProfile.h>
#include <Network/Presence.h>
#include "../Misc/NetDebug.h"
#include "NetRichPresence.h"
#include "NetServices.h"
#include "NetParty.h"
#include "NetRace.h"


#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
#if defined ( PLATFORM_PS3 ) || defined ( PLATFORM_PC )

	static void BuildPresence(const U8 userIndex, const eRichPresence state, NetPlay::RichPresence& out, const bool allowJoinInProgress)
	{
		C_NetServices& ns = C_NetServices::Instance();
		NetPlay::RichPresence pres;
		switch (state)
		{
		case eRichPresenceDefault:	
		case eRichPresenceLan:			
		case eRichPresenceSplitScreen:
		case eRichPresenceTimeAttack:		
		case eRichPresenceCareer:			
			pres.m_Index = state;
			out = pres;
			break;

		case eRichPresenceOnline: 
		case eRichPresenceOnlineWorldTour: 
			pres.m_Index = state;

			// check for join in progress on any of our sessions.
			if (allowJoinInProgress)
			{
				if (ns.IsInitialised())
				{
					bdSecurityID sessionId;
					if (ns.Race().JoinInProgressAvailable(sessionId))
					{
						pres.m_JoinInProgressPossible = true;
						pres.m_OnlineSessionId = sessionId;
					}
					else if (ns.Party().JoinInProgressAvailable(sessionId))
					{
						pres.m_JoinInProgressPossible = true;
						pres.m_OnlineSessionId = sessionId;
					}
				}
			}
			out = pres;
			break;
		}
	}

#else 
	static void BuildPresence(const U8 userIndex, const eRichPresence state, NetPlay::RichPresence& out)
	{
		NetPlay::RichPresence pres;
		out = pres;
	}

#endif

	void SetRichPresence(const U8 userIndex, const eRichPresence state, const bool allowJoinInProgress)
	{
		//NetPlay::RichPresence presenceData;
		/*BuildPresence(userIndex, state, presenceData, allowJoinInProgress);

		switch (state)
		{
		case eRichPresenceDefault:
		case eRichPresenceOnline:	
		case eRichPresenceOnlineWorldTour:	
		case eRichPresenceLan:			
		case eRichPresenceSplitScreen:
		case eRichPresenceTimeAttack:		
		case eRichPresenceCareer:			*/
			SysProfile::C_Manager::Instance().SetPresence(userIndex, state, allowJoinInProgress);
			break;
		/*}*/
	}
} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
