// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Clients side interface to the DemonWare messaging service.

#ifndef BD_MESSAGING_H
#define BD_MESSAGING_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

#define BD_STATUS_OFFLINE		(0u)
#define BD_STATUS_ONLINE		(1u)

#define BD_USER_UNSUBSCRIBED	(0u)
#define BD_USER_SUBSCRIBED		(1u)

// Messaging operation types
#define BD_MESSAGING_GET_MAIL_HEADERS		(1u)
#define BD_MESSAGING_GET_MAIL_BODY			(2u)
#define BD_MESSAGING_DELETE_MAIL			(3u)
#define BD_MESSAGING_SEND_MAIL				(4u)
#define BD_MESSAGING_GET_NOTIFICATIONS		(5u)
#define BD_MESSAGING_DELETE_NOTIFICATIONS	(6u)
#define BD_MESSAGING_SET_BLOCKED_STATUS		(7u)
#define BD_MESSAGING_GET_BLOCKED_LIST		(8u)
#define BD_MESSAGING_GET_MAILS				(9u)
#define BD_MESSAGING_SEND_TEAM_MAIL			(10u)
#define BD_MESSAGING_GET_TEAM_MAIL_HEADERS	(11u)
#define BD_MESSAGING_GET_TEAM_MAILS			(12u)
#define BD_MESSAGING_DELETE_TEAM_MAIL		(13u)


// Channel Chat operation types.
#define BD_CHANNEL_CHAT_SUBSCRIBE				(1u)
#define BD_CHANNEL_CHAT_UNSUBSCRIBE				(2u)
#define BD_CHANNEL_CHAT_DISCOVER				(3u)
#define BD_CHANNEL_CHAT_SEND					(4u)
#define BD_CHANNEL_CHAT_WHISPER					(5u)
#define BD_CHANNEL_CHAT_GET_MEMBERS				(6u)
#define BD_CHANNEL_CHAT_CREATE					(7u)

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdMailHeader;
class bdMailBody;
class bdChannelInfo;
class bdNotification;
class bdChannelMember;
class bdBlockedUser;

/// Client side interface to the DemonWare message management service.
/// Manages the retrieval and deletion of messages.
class bdMessaging
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdBlockLevel
		{
			BD_NOT_BLOCKED,
			BD_BLOCKED,
		};

		~bdMessaging();

		/// Retrieve a list of mail header information for the user's inbox.
		/// \param mailHeaders[out]	Array of bdMailHeader to store the mail header information. The size of the array has to
		///							match the size specified in numMails
		///	\param startIndex[in]	Offset into the list of mail headers
		/// \param numMails[in]		Maximum number of mail headers to be retrieved.
		bdRemoteTaskRef getMailHeaders(	bdMailHeader mailHeaders[], 
										const bdUInt startIndex, 
										const bdUInt numMails);

		/// Retrieve a mail body.
		/// \param maildIDs[in]		List of mailIDs to download. The mail IDs must have been previously obtained 
		///							by calling bdMessaging::getMailHeaders()
		/// \param numMailIDs[in]	The number of mail IDs in the list.
		///	\param mails[out]	Pointer to an array of bdMailBodys where the downloaded mails will be stored. This instance should
		///							be initialized to allocate a memory block large enough to contain the mail's . The maximum size
		///							of the mail is contained in the mail header information (see bdMessaging::getMailHeaders())
		/// \param deleteOnRead[in]	Flag to indicate whether the mail should be deleted.
		bdRemoteTaskRef getMails(	const bdUInt64 mailIDs[], 
									const bdUInt numMailIDs,
									bdMailBody mails[],
									const bdBool deleteOnRead = true);

		/// Delete specific mails.
		///	\param mailIDs[in]		List of mail IDs to be deleted.
		/// \param numMailIDs[in]	The number of mail IDs contained in mailIDs.
		bdRemoteTaskRef deleteMail(	const bdUInt64 mailIDs[], 
									const bdUInt numMailIDs);

		/// Send a mail to a user.
		/// \param message[in]	Pointer to a blob containing the message.
		/// \param messageSize[in]	Size of the message in bytes. A maximum of BD_MAXIMUM_CUSTOM_MSG_SIZE can be specified.
		/// \param userIDs[in]	List of recipient's user IDs.
		/// \param numUserIDs[in] The number of recipients
		bdRemoteTaskRef sendMail(	const void *const message, 
									const bdUInt messageSize, 
									const bdOnlineUserID userIDs[], 
									const bdUInt numUserIDs);

		/// Retrieve a list of mail header information for a team's inbox.
		/// \param teamID[in]	Team ID to retrieve mails from.
		/// \param mailHeaders[out]	Array of bdMailHeader to store the mail header information. The size of the array has to
		///							match the size specified in numMails
		///	\param startIndex[in]	Offset into the list of mail headers
		/// \param numMails[in]		Maximum number of mail headers to be retrieved.
		bdRemoteTaskRef getTeamMailHeaders(	const bdTeamID teamID,
											bdMailHeader mailHeaders[],
											const bdUInt startIndex,
											const bdUInt numMails);

		/// Retrieve a team mail body.
		/// \param teamID[in]	Team ID to retrieve mails from.
		/// \param maildIDs[in]		List of mailIDs to download. The mail IDs must have been previously obtained 
		///							by calling bdMessaging::getMailHeaders()
		/// \param numMailIDs[in]	The number of mail IDs in the list.
		///	\param mails[out]	Pointer to an array of bdMailBodys where the downloaded mails will be stored. This instance should
		///							be initialized to allocate a memory block large enough to contain the mails . The maximum size
		///							of the mail is contained in the mail header information (see bdMessaging::getMailHeaders())
		/// \param deleteOnRead[in]	Flag to indicate whether the mail should be deleted.
		bdRemoteTaskRef getTeamMails(	const bdTeamID teamID,
										const bdUInt64 mailIDs[],
										const bdUInt numMailIDs,
										bdMailBody mails[],
										const bdBool deleteOnRead = true);

		/// Delete specific team mails.
		/// \param teamID[in]	Team ID that the mail was sent to.
		///	\param mailIDs[in]		List of mail IDs to be deleted.
		/// \param numMailIDs[in]	The number of mail IDs contained in mailIDs.
		bdRemoteTaskRef deleteTeamMail(	const bdTeamID teamID,
										const bdUInt64 mailIDs[], 
										const bdUInt numMailIDs);

		/// Send a mail to a team.
		/// \param message[in]	Pointer to a blob containing the message.
		/// \param messageSize[in]	Size of the message in bytes. A maximum of BD_MAXIMUM_CUSTOM_MSG_SIZE can be specified.
		/// \param teamID[in]	Team ID to send the mail to.
		bdRemoteTaskRef sendTeamMail(	const void *const message, 
										const bdUInt messageSize, 
										const bdTeamID teamID);

		/// Retrieve a list of notifications.
		/// \param notifications[out] Array in which the notifications will be stored.
		/// \param startIndex[in]		Offset into the list of notifications.
		/// \param numNotifications[in] Maximum number of notifications to be retrieved.
		/// \param deleteOnRead[in]	Flag to indicate whether the notifications should be deleted.
		/// \note  When this task is completed, bdRemoteTask::getNumResults() will contain the actual number of
		///			notifications retrieved, bdRemoteTask::getTotalNumResults() will contain the number of notifications
		///			that are available.
		bdRemoteTaskRef getNotifications(	bdNotification notifications[], 
											const bdUInt startIndex, 
											const bdUInt numNotifications, 
											const bdBool deleteOnRead = true);

		/// Delete specific notifications.
		///	\param notificationIDs[in]	List of notification IDs to be deleted.
		/// \param numNotifications[in]	number of notifications to be deleted.
		/// \note Notifications expire automatically after 1 week.
		bdRemoteTaskRef deleteNotifications(const bdUInt64 notificationIDs[], 
											const bdUInt numNotifications);

		/// Create a private chat channel. 
		/// The creator of the chat channel is automatically subscribed and can invite other players by
		/// passing around the channel ID of the newly created channel. Other players can subscribe as usual
		/// using bdMessaging::subscribeToChannel(). When the last player leaves the channel it is
		/// automatically deleted.
		/// \param channelInfo[out] Contains channel information including the channel ID of the newly created channel
		///							when the task is completed.
		bdRemoteTaskRef createChatChannel(bdChannelInfo *const channelInfo);

		/// Subscribes a user to the specified channel. 
		/// \param channelID[in]		The ID of the chat channel.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note		An onChatChannelUpdate() event will be delivered to all
		///				channel members.
		bdRemoteTaskRef subscribeToChannel(const bdUInt64 channelID);

		/// Unsubscribes a user to the specified channel. 
		/// \param channelID[in]		The ID of the chat channel.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note		An onChatChannelUpdate() event will be delivered to all
		///				channel members.
		bdRemoteTaskRef unsubscribeFromChannel(const bdUInt64 channelID);

		/// Retrieve a list of available chat channels.
		/// \param channelInfos[in]		Array in which to store the available chat channels.
		/// \param maxNumChannels[in]	Maximum number of channel infos to be retrieved.
		bdRemoteTaskRef getChannelList(	bdChannelInfo channelInfos[],
										const bdUInt maxNumChannels);

		/// Send an instant message to a chat channel.
		/// \param channelID[in]   ID of the chat channel.
		/// \param message[in]	   Blob containing the message.
		/// \param messageSize[in] Size of the message to be sent. A maximum of BD_MAX_CHAT_MESSAGE_SIZE
		///						   can be specified.
		/// \note An onChatChannelMessage() event will be delivered to all channel members
		bdRemoteTaskRef sendToChannel(	const bdUInt64 channelID,
										const void *const message,
										const bdUInt messageSize);

		/// Whisper to a channel member.
		/// \param channelID[in]   ID of the chat channel.
		/// \param userID[in]	   The recipient user ID.
		/// \param message[in]	   Blob containing the message.
		/// \param messageSize[in] Size of the message to be sent. A maximum of BD_MAX_CHAT_MESSAGE_SIZE
		///						   can be specified.
		/// \note An onChatChannelMessage() event will be delivered to the recipient.
		bdRemoteTaskRef whisperToChannelMember(	const bdUInt64 channelID,
												const bdOnlineUserID userID,
												const void *const message,
												const bdUInt messageSize);

		/// Retrieve a list of all current channel members
		/// \param channelID[in]   ID of the chat channel.
		/// \param channelMembers[in] Array in which the channel member info is stored.
		/// \param maxNumMembers[in] Maximum number of member infos to retrieve.
		bdRemoteTaskRef getChannelMembers(	const bdUInt64 channelID,
											bdChannelMember channelMembers[],
											const bdUInt maxNumMembers);

		/// Adds/remove a user to/from the block list. 
		/// Blocking a user means that no further messages will be received from
		/// that user. 
		/// \param userID[in] The user ID to be added to/removed from 
		///								the blocked list.
		/// \param blocked[in] True: block this user, False: unblock this user
		/// \return		A pointer to the task that handles the request. BD_NULL 
		///				is returned if an error occurred.
		bdRemoteTaskRef setBlockStatus(const bdOnlineUserID userID,
									   const bdBool blocked);

		/// Get the list of blocked users and the type of block in each case.
		/// \param blockedUsers[in]		Array to store the blocked players info.
		/// \param startIndex[in]		Offset into the blocked user list.
		/// \param maxNumUsers[in]      The maximum number of blocked players to be retrieved
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		bdRemoteTaskRef getBlockedList(	bdBlockedUser blockedUsers[],
										const bdUInt startIndex,
										const bdUInt maxNumUsers);

	

	protected:

		friend class bdLobbyService;

		/// Note: never call this constructor directly, use bdLobbyService::getMessageManager() instead.
		bdMessaging(bdRemoteTaskManager *const remoteTaskManager);

		/// Sets the message manager's task manager.
		void setRemoteTaskManager(bdRemoteTaskManager *const remoteTaskManager);

		bdRemoteTaskManager *m_remoteTaskManager;

};


#endif // BD_MESSAGING_H
