#ifndef DEMONWARE_LOBBYEVENTHANDLER_H
#define DEMONWARE_LOBBYEVENTHANDLER_H

#ifdef _MSC_VER
#pragma once
#endif


#include <Network/DemonwareTask.h>
#include <bdLobby/bdLobbyEventHandler.h>
#include <bdLobby/bdMessaging/bdNotification.h>

// forward declerations.
class bdLobbyService;

namespace Demonware
{
	/// Used to receive incoming events / messages from the bdLobbyService.
	/** look at bdLobbyEventHandler to get of full list of what events / messages we can receive.
	 */
	class C_LobbyEventHandler : public bdLobbyEventHandler
	{
	public:
		C_LobbyEventHandler();
		virtual ~C_LobbyEventHandler();

		// Register this object to start receiving data from demonware.
		bool Register(void);

		// UnRegister this object to stop receiving data from demonware.
		bool UnRegister(void);

		// Call once per frame to poll any downloads.
		void Update(void);

		/// Called in the event a player's friend status changes.
		/// \param userID	The user ID of the player whose status changed.
		/// \param userName Null terminated string of the player's user name
		/// \param status	The new status (BD_STATUS_ONLINE/BD_STATUS_OFFLINE)
		virtual void onPlayerStatusUpdate(bdOnlineUserID userID, bdNChar8 *userName, bdUByte8 status);

		/// Called in the event a player's team member status changes.
		/// \param userID	The user ID of the player whose status changed.
		/// \param userName Null terminated string of the player's user name
		/// \param teamID	The relevant team ID
		/// \param teamName The relevant team name
		/// \param status	The new status (BD_STATUS_ONLINE/BD_STATUS_OFFLINE)
		virtual void onTeamMemberStatusUpdate(bdOnlineUserID userID, bdNChar8 *userName, bdTeamID teamID, bdNChar8 *teamName, bdUByte8 status);

		/// Called in the event a friend or team member has updated their rich presence information.
		/// \param userID	The user ID of the user who updated their rich presence information.
		/// \param userName Null terminated string of the sender's user name
		/// \param data		Pointer to a blob containing the updated presence information.
		/// \param dataSice Actual size of the presence information.
		/// \param teamID	Team ID in case a team member's rich presence information was updated.
		virtual void onRichPresenceUpdate(bdOnlineUserID userID, bdNChar8 *userName, void *data, bdUInt dataSize, bdTeamID teamID = 0);

		/// Called in the event a player joins or leaves a chat channel.
		/// \param channelID	The relevant channel ID
		/// \param userID		The user ID of the player who joined/left
		/// \param userName		The relevant user name
		/// \param status		The new status (BD_USER_SUBSCRIBED/BD_USER_UNSUBSCRIBED)
		virtual void onChatChannelUpdate(bdUInt64 channelID, bdOnlineUserID userID, bdNChar8 *userName, bdUByte8 status);

		/// Called in the event a chat channel message is received.
		/// \param channelID	The relevant channel ID
		/// \param senderID		The user ID of the sender
		/// \param senderName	The user name of the sender
		/// \param message		Pointer to a blob containing the message
		/// \param messageSize	The size of the message in bytes
		virtual void onChatChannelMessage(bdUInt64 channelID, bdOnlineUserID senderID, bdNChar8 *senderName, void *message, bdUInt messageSize);

		/// Called in the event an instant message from a friend or team member is received.
		/// \param senderID		The user ID of the sender
		/// \param senderName	The user name of the sender
		/// \param message		Pointer to a blob containing the message
		/// \param messageSize  Size of the message in bytes
		/// \param teamID		The team ID in case the message was sent from a team member.
		///						Set to 0 if the message was sent from a friend.
		virtual void onInstantMessage(bdOnlineUserID senderID, bdNChar8 *senderName, void *message, bdUInt messageSize, bdTeamID teamID = 0);

		/// Called in the event a session invite is received.
		/// \param invite		The received invite.
		/// \note	The game code is responsible for taking a copy of the information
		///			in the invite as it is deleted once the handler returns.
		virtual void onSessionInvite(bdSessionInvite &invite);

		/// Called in the event a player disconnects from the MM+ lobby. The player has to be previously
		/// registered with bdMatchMaking::notifyJoin(). Only the game host who advertises the game session
		/// will receive this event.
		///	\param userID	The user ID of the player who disconnected
		/// \param secID	The security ID of the advertised matchmaking session
		virtual void onNotifyLeave(bdOnlineUserID userID, bdSecurityID secID);

		/// Called in the event a new friendship proposal is received
		virtual void onFriendshipProposal();

		/// Called in the event a new team membership proposal is received
		virtual void onTeamProposal();

		/// Called in the event a new notification is received
		virtual void onNewNotification();

		/// Called in the event a new mail is received
		virtual void onNewMail();

		/// Called in the event the same user is logged on elsewhere
		/// This will disconnect the player from the lobby server.
		virtual void onMultipleLogon(bdOnlineUserID userID);

		// retrieve notifications from server. Block this until 
		// the bandwidth evaluation is done.
		virtual void RetrieveNotifications(bool onOff) { m_RetrievingNotificationsEnabled = onOff; }

	private:
		void KillNotifications(void);
		void UpdateNotifications(void);

	private:
		bool m_Registered;
		bool m_NewNotifications;
		bool m_RetrievingNotifications;
		bool m_RetrievingNotificationsEnabled;

#if defined (PLATFORM_PC)
		// we only use notification son PC for friends request replies.
		bdRemoteTaskRef m_NotificationsTask;
		Btl::Vector<bdNotification, Btl::VectorParameters_Array<5> >m_Notifications;
#endif
	};
}

#endif //DEMONWARE_LOBBYEVENTHANDLER_H