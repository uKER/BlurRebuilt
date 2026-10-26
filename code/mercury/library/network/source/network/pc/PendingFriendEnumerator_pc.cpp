#include <StdAfx.h>
#include <Network/Core.h>
#include <Network/Network.h>
#include <Network/PendingFriendEnumerator.h>
#include <Network/DemonwareTask.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdFriends/bdFriends.h>

namespace NetPlay
{
	C_PendingFriendsEnumerator::C_PendingFriendsEnumerator() :	m_Initialised(false), m_Mode(eModeNoneSet), m_UserIndex(0), 
																m_NumPendingFriends(0), m_Status(eStatusIdle)
	{
	}

	C_PendingFriendsEnumerator::~C_PendingFriendsEnumerator()
	{
		Cleanup();
	}

	void C_PendingFriendsEnumerator::Cleanup(void)
	{
		Stop();
		m_Initialised = false;
	}

	bool C_PendingFriendsEnumerator::Initialise(void)
	{
		if (m_Initialised)
			return true;
		m_Initialised = true;
		return m_Initialised;
	}

	bool C_PendingFriendsEnumerator::StartEnumeration(const SysProfile::FullProfile& inUser, ePendingEnumerateMode mode)
	{
		if (!m_Initialised)
			return false;

		if (eModeNoneSet == mode)
		{
			LOGFATAL(g_Log, "C_PendingFriendsEnumerator::StartEnumeration() - invalid mode passed.");
			return false;
		}
		
		// if we are replying to an invite request then we should not stop it. game should really pump these tasks until completion.
		if (eStatusIdle != m_Status || NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask) == NetPlay::DemonwareRemoteTask::ePending)
		{
			LOGWARN(g_Log, "C_PendingFriendsEnumerator::StartEnumeration() - can't start enumeration until previous asynchronous operation completed.");
			return false;
		}
		else
			Stop();

		if (!inUser.Active() || SysProfile::eProfileStateOnline != inUser.m_State || inUser.m_IsGuest)
			return false; // can't get friends for inactive / offline players / guests

		// store user index.
		m_UserIndex = inUser.m_UserIndex;

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL != lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
		{
			bdFriends* friendsService =	lobbyService->getFriends();
			if (friendsService != NULL)
			{
				m_Mode = mode;
				
				if (eModeInviteReceived == m_Mode)
				{	
					// get a list of friends invites sent to me.
					m_AsyncTask = friendsService->getIncomingProposals(m_PendingFriends, MAX_FRIENDS);
					if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
						m_Status = eStatusEnumerating;
					else
						Stop();
				}
				else if (eModeInviteSent == m_Mode)
				{
					// request friends invites that i have sent out.
					m_AsyncTask = friendsService->getOutgoingProposals(m_PendingFriends, MAX_FRIENDS);
					if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
						m_Status = eStatusEnumerating;
					else
						Stop();
				}
				else
					Stop();
			}
		}

		return m_Status == eStatusEnumerating;
	}

	bool C_PendingFriendsEnumerator::StartRevokeFriendRequest(const PendingFriendProfile& toRevoke)
	{
		if (!m_Initialised)
			return false;

		if (eStatusIdle != m_Status)
			return false; // can't revoke while some task is still active..

		if (eModeInviteSent != m_Mode)
		{
			LOGWARN(g_Log, "C_PendingFriendsEnumerator::StartRevokeFriendRequest() - can't rekove without first enumerating pending friends.");
			return false;
		}

		// only revoke friends who we have sent invites to.. 
		if (eFriendRequestSent != toRevoke.m_FriendState)
		{
			LOGWARN(g_Log, "C_PendingFriendsEnumerator::StartRevokeFriendRequest() - requested friend to revoke is in the wrong state.");
			return false;
		}

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL != lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
		{
			bdFriends* friendsService =	lobbyService->getFriends();
			if (friendsService != NULL)
			{
				m_AsyncTask =  friendsService->withdrawProposal(toRevoke.m_Profile.m_Uid);
				if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
					m_Status = eStatusRevoking;
				else
					Stop();
			}
			else
				Stop();
		}
		else
			Stop();

		return m_Status == eStatusRevoking;
	}

	bool C_PendingFriendsEnumerator::StartAnswerFriendRequest(const PendingFriendProfile& toReply, const bool accept)
	{
		if (!m_Initialised)
			return false;

		if (eStatusIdle != m_Status)
			return false; // can't revoke while some task is still active..

		if (eModeInviteReceived != m_Mode)
		{
			LOGWARN(g_Log, "C_PendingFriendsEnumerator::StartAnswerFriendRequest() - can't answer friend invite without first enumerating requests.");
			return false;
		}

		// only answer friends who have sent us friend messages.
		if (eFriendRequestReceived != toReply.m_FriendState)
		{
			LOGWARN(g_Log, "C_PendingFriendsEnumerator::StartAnswerFriendRequest() - requested friend to revoke is in the wrong state.");
			return false;
		}

		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (NULL != lobbyService && lobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
		{
			bdFriends* friendsService =	lobbyService->getFriends();
			if (friendsService)
			{
				if (accept)
					m_AsyncTask = friendsService->acceptProposal(toReply.m_Profile.m_Uid);
				else
					m_AsyncTask = friendsService->rejectProposal(toReply.m_Profile.m_Uid);

				if (NetPlay::DemonwareRemoteTask::IsBusy(m_AsyncTask))
					m_Status = eStatusReplying;
				else
					Stop();
			}
			else
				Stop();

			return m_Status == eStatusReplying;
		}

		Stop();
		return false;
	}

	bool C_PendingFriendsEnumerator::ContinueTask(bool& errorOccurred)
	{
		errorOccurred = true;
		if (!m_Initialised)
			return true;

		if (eStatusIdle == m_Status)
		{
			// no task currently active.
			errorOccurred = false;
			return true;
		}

		bdNetImpl *const net = bdNet::getInstance();
		bdLobbyService *lobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();	
		if (net->getStatus() != bdNetImpl::BD_NET_DONE || lobbyService->getStatus() != bdLobbyConnection::BD_CONNECTED)
		{
			Stop();
			return true;
		}

		if (eStatusEnumerating == m_Status)
		{	
			switch (NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask))
			{
			case NetPlay::DemonwareRemoteTask::ePending:
				return false;
				break;

			case NetPlay::DemonwareRemoteTask::eFailed:
				Stop();
				return true;
				break;

			default:
				errorOccurred = false;
				break;
			}

			m_NumPendingFriends = m_AsyncTask->getNumResults();
			NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);
			m_Status = eStatusIdle;
		}
		else if (eStatusRevoking == m_Status)
		{
			switch (NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask))
			{
			case NetPlay::DemonwareRemoteTask::ePending:
				return false;
				break;

			case NetPlay::DemonwareRemoteTask::eFailed:
				Stop();
				return true;
				break;

			default:
				break;
			}

			NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

			// rescan proposal list now..
			SysProfile::FullProfile tmp;
			tmp.m_UserIndex = m_UserIndex;

			if(StartEnumeration(tmp, eModeInviteSent))
				return false; 

			Stop();
			return true;
		}
		else if (eStatusReplying == m_Status)
		{
			switch (NetPlay::DemonwareRemoteTask::CheckState(m_AsyncTask))
			{
			case NetPlay::DemonwareRemoteTask::ePending:
				return false;
				break;
			}

			if (BD_NO_ERROR == m_AsyncTask->getErrorCode() || BD_FRIENDSHIP_EXISTS == m_AsyncTask->getErrorCode() || BD_FRIENDSHIP_NOT_REQUSTED == m_AsyncTask->getErrorCode())
			{
				NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);

				// accept / decline succeeded so re enumerate again..

				SysProfile::FullProfile tmp;
				tmp.m_UserIndex = m_UserIndex;

				if(StartEnumeration(tmp, eModeInviteReceived))
					return false; 

				Stop();
				return true;
			}
			else
				Stop();
		}
		else
		{
			LOGERROR(g_Log, "C_PendingFriendsEnumerator::Continue() - Unknown status !!.");
		}

		return true;
	}

	void C_PendingFriendsEnumerator::Stop(void)
	{
		if (!m_Initialised)
			return;

		NetPlay::DemonwareRemoteTask::Close(m_AsyncTask);
	
		m_Status = eStatusIdle;
		m_NumPendingFriends = 0;
		m_Mode = eModeNoneSet;
		m_UserIndex = 0;
	}

	U32 C_PendingFriendsEnumerator::ExtractPendingFriends(PendingFriendProfile* pOutput, const U32 maxOutput)
	{
		if (!m_Initialised || NULL == pOutput || 0 == maxOutput || eModeNoneSet == m_Mode || eStatusIdle != m_Status)
			return 0;

		U32 numPendingFriendsReturned = 0;
		for (U32 i=0; i<m_NumPendingFriends; i++)
		{
			if (numPendingFriendsReturned < maxOutput)
			{
				PendingFriendProfile pf;
				pf.m_FriendState	= (m_Mode == eModeInviteReceived) ? eFriendRequestReceived : eFriendRequestSent;
				pf.m_Profile.m_Uid	= m_PendingFriends[i].m_userID;	
				_snwprintf_s(pf.m_Profile.m_Name, lengthof(pf.m_Profile.m_Name), _TRUNCATE, L"%S", m_PendingFriends[i].m_userName);
				pf.m_Profile.m_Name[SYSTEM_PROFILE_NAME_LENGTH-1] = 0;

				pOutput[numPendingFriendsReturned++] = pf;
			}
		}

		return numPendingFriendsReturned;
	}

	bool C_PendingFriendsEnumerator::IsBusy(void) const
	{
		if (!m_Initialised)
			return false;

		if (eStatusIdle == m_Status)
			return false;
		return true;
	}

}// end namespace

