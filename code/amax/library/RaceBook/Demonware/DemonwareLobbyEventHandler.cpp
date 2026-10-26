#include <stdafx.h>
#include <Core/Core.h>
#include <Network/Core.h>
#include <SystemProfile/SystemProfilemanager.h>
#include <bdCore/bdCore.h>
#include <bdNet/bdNet.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdMessaging/bdMessaging.h>
#include "DemonwareLobbyEventHandler.h"
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>
#include "Ui/UiMessages.h"
#include <GameLogic\FriendsChallenges/MessagingManager.h>

namespace Demonware
{
	C_LobbyEventHandler::C_LobbyEventHandler() : m_Registered(false), m_NewNotifications(false), m_RetrievingNotifications(false)
	{
	}

	C_LobbyEventHandler::~C_LobbyEventHandler()
	{
		UnRegister();
	}

	bool C_LobbyEventHandler::Register(void)
	{
		bdLobbyService* pLobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();

		if (pLobbyService)
		{
			pLobbyService->registerEventHandler(this);
			m_Registered = true;
			return true;
		}

		return false;
	}

	bool C_LobbyEventHandler::UnRegister(void)
	{
		KillNotifications();
	
		bdLobbyService* pLobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
		if (pLobbyService)
			pLobbyService->registerEventHandler(NULL);
			
		m_Registered = false;
		return true;
	}

	void C_LobbyEventHandler::Update(void)
	{
		if (!m_Registered)
			return;

		UpdateNotifications();
	}

	void C_LobbyEventHandler::onPlayerStatusUpdate(bdOnlineUserID userID, bdNChar8* userName, bdUByte8 status)
	{
		SysProfile::C_Manager::Instance().DemonwarePlayerStatusUpdate(userID, userName, status);
	}

	void C_LobbyEventHandler::onTeamMemberStatusUpdate(bdOnlineUserID userID, bdNChar8* userName, bdTeamID teamID, bdNChar8* teamName, bdUByte8 status)
	{
	}

	void C_LobbyEventHandler::onRichPresenceUpdate(bdOnlineUserID userID, bdNChar8* userName, void* data, bdUInt dataSize, bdTeamID teamID)
	{
		if (0 == teamID)
			SysProfile::C_Manager::Instance().DemonwareRichPresenceUpdate(userID, userName, data, dataSize);
		else
		{
			// Team status..
		}
	}

	void C_LobbyEventHandler::onChatChannelUpdate(bdUInt64 channelID, bdOnlineUserID userID, bdNChar8* userName, bdUByte8 status)
	{
	}

	void C_LobbyEventHandler::onChatChannelMessage(bdUInt64 channelID, bdOnlineUserID senderID, bdNChar8* senderName, void* message, bdUInt messageSize)
	{
	}

	void C_LobbyEventHandler::onInstantMessage(bdOnlineUserID senderID, bdNChar8* senderName, void* message, bdUInt messageSize, bdTeamID teamID)
	{
	}

	void C_LobbyEventHandler::onSessionInvite(bdSessionInvite &invite)
	{
		SysProfile::C_Manager::Instance().DemonwareSessionInvite(invite.m_senderID, invite.m_senderName, invite.m_sessionID.m_sessionID, invite.m_attachment, invite.m_attachmentSize);
		_UI_MESSAGE( UI::E_UiGameMsg_InviteReceived );
	}

	void C_LobbyEventHandler::onFriendshipProposal()
	{
		// TODO -> ui can show a noticication upon receiving this.
	}

	void C_LobbyEventHandler::onTeamProposal()
	{
	}

	void C_LobbyEventHandler::onNewNotification()
	{
		// will get new notifications asap.
		m_NewNotifications = true;
	}

	void C_LobbyEventHandler::onNewMail()
	{
		// TODO -> ui can show a noticication upon receiving this.
		Game::C_MessagingManager::Instance().NewMail();
	}

	void C_LobbyEventHandler::onNotifyLeave(bdOnlineUserID userID, bdSecurityID secID)
	{
	}

	void C_LobbyEventHandler::onMultipleLogon(bdOnlineUserID userID)
	{
#if defined ( PLATFORM_PC )
		SysProfile::C_Manager::Instance().DemonwareDuplicateLogon();
#endif
	}

	void C_LobbyEventHandler::KillNotifications(void)
	{
		m_NewNotifications = false;
		m_RetrievingNotifications = false;
#if defined ( PLATFORM_PC )
		NetPlay::DemonwareRemoteTask::Close(m_NotificationsTask);
#endif
	}

	void C_LobbyEventHandler::UpdateNotifications(void)
	{
#if defined ( PLATFORM_PC )

		if (m_RetrievingNotifications)
		{
			if (NetPlay::DemonwareRemoteTask::IsBusy(m_NotificationsTask))
				return; // async request still running.

			if (NetPlay::DemonwareRemoteTask::CheckState(m_NotificationsTask) == NetPlay::DemonwareRemoteTask::eDone)
			{
				for (U32 i=0; i<m_NotificationsTask->getNumResults(); i++)
				{
					switch (m_Notifications[i].m_type)
					{
					case BD_FRIENDSHIP_REJECTED:
						break;
					case BD_FRIENDSHIP_ACCEPTED:
						break;
					case BD_FRIENDSHIP_REVOKED:
						break;

					default:
						// not using team service yet so should not get any notifications related to it.
						assert(false);
						break;
					}
				}

				const S32 remaining = m_NotificationsTask->getTotalNumResults() - m_NotificationsTask->getNumResults();
				
				m_RetrievingNotifications = false;
				NetPlay::DemonwareRemoteTask::Close(m_NotificationsTask);

				// if their is still un read notifications on the server get those to.
				if (remaining > 0)
					m_NewNotifications = true; 
			}
			else
				KillNotifications();
		}
		else
		{
			if (m_NewNotifications)
			{
				assert(m_RetrievingNotifications == false);

				// going to hold off requesting the notifications until bandwidth scan has finiahed after initial connection.
				RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
				if (services.IsInitialised() && !services.Router().BandwidthEvaluationInProgress())
				{
					bdLobbyService* pLobbyService = NetPlay::C_Core::Instance().DemonwareLobbyService();
					
					if (pLobbyService && pLobbyService->getStatus() == bdLobbyConnection::BD_CONNECTED)
					{
						bdMessaging* pMessaging = pLobbyService->getMessaging();
						if (pMessaging)
						{
							// request a number of notifications, delete frokm server during read so we don' t require a seperate 
							// server call to acknowledge the notification.
							m_NotificationsTask = pMessaging->getNotifications(&m_Notifications[0], 0, m_Notifications.Size(), true);

							if (NetPlay::DemonwareRemoteTask::IsBusy(m_NotificationsTask))
							{
								m_NewNotifications = false;
								m_RetrievingNotifications = true;
								return;
							}
						}
					}

					// failed to request notifications if we get here
					KillNotifications();
				}
			}
		}
#endif
	}
}

