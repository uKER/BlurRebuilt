#include "stdafx.h"

#include "RichPresenceBuilder.h"
#include <Ui\UiHelpers.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\GameModes\GameModeBase.h>
#include <GameLogic\GameModes\GameModeDefines.h>
#include <GameLogic\RaceModes\RaceModeDefines.h>

#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic\SinglePlayer\SinglePlayer.h>
#include <GameLogic\FriendsChallenges\FriendChallenges.h>

#include <Network\Core\NetServices.h>
#include <Network\Core\NetRace.h>
#include <Network\Core\NetParty.h>

#include "Xlast/Amax.spa.h"

namespace Game
{
	//	static instance
	C_RichPresenceBuilder C_RichPresenceBuilder::s_Instance;

//	Returns true if we need to update the presence based on the enum we've passed in
//	On PS3/PC this enum may be changed here to reference a non-Xenon rich presence index
	bool C_RichPresenceBuilder::BuildRichPresence( const U32 userIndex, RacerNet::eRichPresence& richPresence )
	{
	switch(richPresence)
	{
		case RacerNet::eRichPresence_InRace:
			{
				E_RaceMode currentRaceMode = UI::GetRaceBookLayer()->GetRaceMode();
				//	We're only interested in setting the presence here if we're either playing single player or online.
				if(!UI::GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer())
				{
					//	Single player OR Friend demands
					switch(currentRaceMode)
					{
					case E_RaceMode_StreetRace:	// Fall-through
					case E_RaceMode_Destruction:// Fall-through
					case E_RaceMode_Checkpoint:	// Fall-through
					case E_RaceMode_BossBattle:	// Fall-through
						{
							if(currentRaceMode == E_RaceMode_StreetRace)
								richPresence = RacerNet::eRichPresencePS3_SpRace;
							else if(currentRaceMode == E_RaceMode_Destruction)
								richPresence = RacerNet::eRichPresencePS3_SpDestruction;
							else if(currentRaceMode == E_RaceMode_Checkpoint)
								richPresence = RacerNet::eRichPresencePS3_SpCheckpoint;
							else if(currentRaceMode == E_RaceMode_BossBattle)
								richPresence = RacerNet::eRichPresencePS3_SpBoss;
							break;
						}
					case E_RaceMode_FriendDemand_StreetRace:	// Fall-through
					case E_RaceMode_FriendDemand_Destruction:	// Fall-through
					case E_RaceMode_FriendDemand_Checkpoint:	// Fall-through
					case E_RaceMode_FriendDemand_FanDemand:	// Fall-through
					case E_RaceMode_FriendDemand_BossBattle:	// Fall-through
						{
							//	Find out if it's a new challenge or re-challenge
							bool reChallenge = C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mReChallenge;
							richPresence = (reChallenge) ? RacerNet::eRichPresencePS3_RC_ReChallenge : RacerNet::eRichPresencePS3_FC_Challenge;
							break;
						}
					}
				}
				else if(UI::GetRaceBookLayer()->GetGameModePtr()->IsOnline())
				{
					if(UI::GetRaceBookLayer()->GetGameModePtr()->IsRanked())
					{
						//	Ranked Online game
						if(currentRaceMode == E_RaceMode_Multiplayer_Racing)
							richPresence = RacerNet::eRichPresencePS3_OnlineRacing;
						else if(currentRaceMode == E_RaceMode_Multiplayer_Destruction)
							richPresence = RacerNet::eRichPresencePS3_OnlineMM;
					}
					else
					{
						return false;	//	Don't need to set the Private Game presence here, we'll have set that in the lobby
					}
				}
				else
					return false;	//	We'll set Split Screen and LAN presence when entering those modes, not on the HUD
				break;
			}
		case RacerNet::eRichPresence_MpLobby:
			{
				if(UI::GetRaceBookLayer()->GetGameModePtr()->IsOnline())
				{
					bool ranked = UI::GetRaceBookLayer()->GetGameModePtr()->IsRanked();
					//	[TODO_RP] based on the ranked flag, we know if this is a public or private game
					if(!ranked)
						richPresence = RacerNet::eRichPresencePS3_OnlinePrivate;
				}
				else
				{
					//	System Link/LAN - which we don't need to do anything with as we set this presence when you
					//	choose it from the MP carousel. Bail out if it's not XENON so we don't keep our existing LAN presence.
					return false;
				}
				break;
			}
	}

	return true;
}

void C_RichPresenceBuilder::GetRankAndLegend( S32& rank, S32& legendLevel )
{
	C_GameProfile* pProf = C_GameProfileManager::Instance().GetPrimaryProfile();
	if(pProf)
	{
		rank = pProf->mMultiplayerProfile.GetCurrentLevel() + 1;	//	Gives us 1-50
		legendLevel = pProf->mMultiplayerProfile.GetLegendLevel() - 1; // Gives us 0-9 (and -1 which means non-Legend)
	}
}


//	PS3 + PC join/invite data needs to be built into the presence structure here dependant on the presence type
void C_RichPresenceBuilder::SetJoinSessionInfo( NetPlay::RichPresence& out, const RacerNet::eRichPresence state, bool allowJoinInProgress )
{
	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
	NetPlay::RichPresence pres;
	switch(state)
	{
	case RacerNet::eRichPresence_SignedIn:
	case RacerNet::eRichPresence_Idle:
	case RacerNet::eRichPresence_Stickerbook:		
	case RacerNet::eRichPresence_Leaderboards:		
	case RacerNet::eRichPresence_Help:				
	case RacerNet::eRichPresence_Showroom:		
	case RacerNet::eRichPresence_SpMain:			
	case RacerNet::eRichPresence_Photos:			
	case RacerNet::eRichPresence_FriendChallenges:
	case RacerNet::eRichPresence_SpCareer:			
	case RacerNet::eRichPresence_Sp_Choosing:
	case RacerNet::eRichPresencePS3_SpBoss:
	case RacerNet::eRichPresencePS3_SpRace:
	case RacerNet::eRichPresencePS3_SpDestruction:
	case RacerNet::eRichPresencePS3_SpCheckpoint:
	case RacerNet::eRichPresencePS3_FC_Challenge:
	case RacerNet::eRichPresencePS3_RC_ReChallenge:
	case RacerNet::eRichPresence_SplitScreen:
	case RacerNet::eRichPresence_LAN:
		{
			pres.m_Index = state;
			out = pres;
			break;
		}

	case RacerNet::eRichPresence_MpMain:					
	case RacerNet::eRichPresence_RaceHistory:		
	case RacerNet::eRichPresence_OnlineMain:		
	case RacerNet::eRichPresence_ModShop:				
	case RacerNet::eRichPresence_Challenges:		
	case RacerNet::eRichPresence_MpLobby:
	case RacerNet::eRichPresencePS3_OnlineRacing:
	case RacerNet::eRichPresencePS3_OnlineMM:
	case RacerNet::eRichPresencePS3_OnlinePrivate:
		{
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
}

} // namespace Game