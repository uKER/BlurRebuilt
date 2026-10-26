#include <stdafx.h>
#include <Core/core.h>
#include <Network/Core.h>
#include <SystemProfile/SystemProfile.h>
#include <ui/Game/Profiles/ProfileInterface.h>
#include <GameLogic/GameData/GameData.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic/Profile/MultiplayerProfile.h>
#include "NetCreationHelper.h"
#include "NetLogger.h"
#include "../Misc/NetDebug.h"
#include "../Player/NetPlayerManager.h"
#include "../Player/NetPlayer.h"
#include "NetParty.h"


#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

#ifndef CFG_GOLD
Debug::Int g_NetNatTypeOverride("Network.Core.OverrideNatType", 0);
#endif

namespace RacerNet
{
	C_NetCreationHelper::C_NetCreationHelper() :	m_LocalSessionManagerIndex(-1), m_ConsoleId(0), m_DlcMask(0), m_UserCreatedContentAvailable(true)
	{
	}

	C_NetCreationHelper::~C_NetCreationHelper()	
	{
	}

	bool C_NetCreationHelper::BuildLocalPlayersForLan(const U64 consoleId, const U16 bandwidthUpKbps, const U16 bandwidthDownKbps)
	{
		m_ConsoleId = consoleId;
		m_UpStreamBandwidthKbps = bandwidthUpKbps;
		m_DownStreamBandwidthKbps = bandwidthDownKbps;
		m_UserCreatedContentAvailable = false;
		
		m_LocalPlayerData.Clear();
		m_LocalSessionManagerIndex = -1; // not required in lan games
		const U32 numProfiles = UI::C_ProfileInterface::Instance().NumActive();
		if ( 0 == numProfiles )
		{
			LOGWARN(g_Log, "C_NetCreationHelper::BuildLocalPlayers() - not enough system profiles active for network play.");
			return false; // too many profiles active.
		}

		U8 numFullVoiceBanned = 0;

		const U8 padIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(padIndex);
		if (fp.Active())
		{
			LocalPlayer lp;
			lp.m_SystemProfile = fp;
			
			if (fp.m_CommunicationsPrivilege)
				lp.m_VoiceBanned = false;
			else
			{
				lp.m_VoiceBanned = true;
				if (!fp.m_IsGuest)
					numFullVoiceBanned += 1;
			}

			m_LocalPlayerData.PushBack(lp);
		}

		// on xbox 360 we must take the voice privilege from the most restricted profile, guests dont count towards this as they
		// are voice banned anyway.
		if (numFullVoiceBanned)
		{
			for (S32 i=0; i<m_LocalPlayerData.Size(); i++)
				m_LocalPlayerData[i].m_VoiceBanned = true;
		}

		// content pack 0 is always available since that defines content that was shipped with the game
		m_DlcMask =  C_GameData::Instance().GenerateAvailableDlcBits();

		SetupGameProfileDataForPlayers(true, eNatOpen);

		return m_LocalPlayerData.Size() != 0 ? true : false;
	}

	bool C_NetCreationHelper::BuildLocalPlayersForOnline(const eNatType natType, const U64 consoleId, const U16 bandwidthUpKbps, const U16 bandwidthDownKbps)
	{
		m_ConsoleId = consoleId;
		m_UpStreamBandwidthKbps = bandwidthUpKbps;
		m_DownStreamBandwidthKbps = bandwidthDownKbps;
		m_UserCreatedContentAvailable = true;

		m_LocalPlayerData.Clear();
		m_LocalSessionManagerIndex = -1; // will be set below
		const U32 numProfiles = UI::C_ProfileInterface::Instance().NumActive();
		if (0 == numProfiles )
		{
			LOGWARN(g_Log, "C_NetCreationHelper::BuildLocalPlayers() - not enough profiles active for network play.");
			return false; // too many profiles active.
		}

		U8 numFullVoiceBanned = 0;
			
		const U8 padIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(padIndex);
		if (fp.Active())
		{
			// all active full profiles must have online multiplayer privileges
			if (!fp.m_OnlineMultiplayerPrivilege && !fp.m_IsGuest)
			{
				m_LocalPlayerData.Clear();
				return false;
			}

			// set which local player creates the matching sessions.
			// this must be a full account, not a guest.
			if (-1 == m_LocalSessionManagerIndex && !fp.m_IsGuest)
				m_LocalSessionManagerIndex = fp.m_UserIndex;

			LocalPlayer lp;
			lp.m_SystemProfile = fp;

			if (fp.m_CommunicationsPrivilege)
				lp.m_VoiceBanned = false;
			else
			{
				lp.m_VoiceBanned = true;
				if (!fp.m_IsGuest)
					numFullVoiceBanned += 1;
			}

			// can we use user generated content.
			if (!fp.m_IsGuest && !fp.m_ViewUserContentPrivilege)
				m_UserCreatedContentAvailable = false;

			m_LocalPlayerData.PushBack(lp);
		}

		// on xbox 360 we must take the voice privilege from the most restricted profile, guests dont count towards this as they
		// are voice banned anyway.
		if (numFullVoiceBanned)
		{
			for (S32 i=0; i<m_LocalPlayerData.Size(); i++)
				m_LocalPlayerData[i].m_VoiceBanned = true;
		}

		if (-1 == m_LocalSessionManagerIndex)
			m_LocalPlayerData.Clear();

		m_DlcMask = C_GameData::Instance().GenerateAvailableDlcBits();

#ifndef CFG_GOLD
		if (g_NetNatTypeOverride >= RacerNet::eNatOpen && g_NetNatTypeOverride <= RacerNet::eNatStrict) 
		{
			RacerNet::eNatType eTempNat = eNatOpen;
			switch (g_NetNatTypeOverride)
			{
			case 1:
				eTempNat = eNatOpen;
				break;
			case 2:
				eTempNat = eNatModerate;
				break;
			case 3:
				eTempNat = eNatStrict;
				break;
			}
			SetupGameProfileDataForPlayers(false, eTempNat);
		}
		else
			SetupGameProfileDataForPlayers(false, natType);
#else
		SetupGameProfileDataForPlayers(false, natType);
#endif

		return m_LocalPlayerData.Size() != 0 ? true : false;
	}

	bool C_NetCreationHelper::BuildLocalPlayersForOnlineFromParty(const eNatType natType, C_NetParty& party, C_NetPlayerManager& mgr)
	{
		if (party.IsRunning())
		{
			if (BuildLocalPlayersForOnline(natType, 0, 0, 0))
			{
				// same number of local players in the party session as profiles we have just enumerated.
				if (mgr.LocalSize() == m_LocalPlayerData.Size()) 
				{
					// copy data out of primary local player as it must be the same as what we create the race player with.
					C_NetPlayer* p = mgr.Index(0);

					BIZ_ASSERT(p != NULL);
					BIZ_ASSERT(p->m_PartyConnection.IsActive());

					m_ConsoleId					= p->m_ConsoleId;
					m_UpStreamBandwidthKbps		= p->m_UpStreamKbps;
					m_DownStreamBandwidthKbps	= p->m_DownStreamKbps;

					return true;
				}
				else
				{
					BIZ_ASSERT_FAIL("C_NetCreationHelper::BuildLocalPlayersForOnlineFromParty() : local players in party differ from enumerated.");
				}
			}
		}
		return false;
	}

	void C_NetCreationHelper::SetupGameProfileDataForPlayers(const bool lanMode, const eNatType natType)
	{
		for (S32 i=0; i<m_LocalPlayerData.Size(); i++)
		{
			LocalPlayer& lp = m_LocalPlayerData[i];

			if (lp.m_SystemProfile.m_State == SysProfile::eProfileStateBlagged)
			{
				lp.m_ProfileData.Flush();
			}
			else
			{
				// setup game profile data.
				Game::C_GameProfileManager::Instance().BuildNetworkSharedProfileData(lp.m_SystemProfile.m_UserIndex, lp.m_ProfileData.m_Game, true);

				// user create content only on online mode.. if possible
				lp.m_ProfileData.m_UserCreatedContentAvailable = (lanMode == false) ? m_UserCreatedContentAvailable : false;
			}

			lp.m_ProfileData.m_NatType = natType;
		}
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
