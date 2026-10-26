// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Clients side interface to the DemonWare friends service. 

#ifndef BD_FRIENDS_H
#define BD_FRIENDS_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdFriends/bdRichPresence.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdFriendInfo;
class bdFriendProposal;
class bdGroupInfo;

// Friends operation types.
#define BD_FRIEND_PROPOSE_FRIENDSHIP		(1u)
#define BD_FRIEND_ACCEPT_PROPOSAL			(2u)
#define BD_FRIEND_REJECT_PROPOSAL			(3u)
#define BD_FRIEND_REVOKE_FRIENDSHIP			(4u)
#define BD_FRIEND_GET_FRIENDS				(5u)
#define BD_FRIEND_WITHDRAW_PROPOSAL			(6u)
#define BD_FRIEND_SET_GROUP					(7u)
#define BD_FRIEND_SET_GROUP_NAME			(8u)
#define BD_FRIEND_GET_GROUPS				(9u)
#define BD_FRIEND_GET_GROUPED_FRIENDS		(10u)
#define BD_FRIEND_DELETE_GROUP_NAME			(11u) 
#define BD_FRIEND_GET_OUTGOING_PROPOSALS	(12u) 
#define BD_FRIEND_REMOVE_ALL_FRIENDS		(13u)
#define BD_FRIEND_GET_INCOMING_PROPOSALS	(14u) 
#define BD_FRIEND_SEND_INSTANT_MESSAGE		(15u)
#define BD_FRIEND_SET_RICH_PRESENCE			(16u)
#define BD_FRIEND_GET_RICH_PRESENCE			(17u)
#define BD_FRIEND_PROPOSE_FRIENDSHIPS		(18u)
#define BD_FRIEND_ACCEPT_PROPOSALS			(19u)
#define BD_FRIEND_REJECT_PROPOSALS			(20u)
#define BD_FRIEND_REVOKE_FRIENDSHIPS		(21u)
#define BD_FRIEND_WITHDRAW_PROPOSALS		(22u)


/// Clients side interface to the DemonWare friends service. 
class bdFriends
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS		
		
		/// Destructor.
		~bdFriends();

		/// Invite a user to become a friend.
		/// \param friendID[in]			The friend's user ID.
		/// \param attachment[in]		An optional attachment to send with the
		///								proposal message.
		/// \param attachmentSize[in]	Size of the attachment in bytes.
		/// \return		A reference to the task that handles the request. BD_NULL 
		///				is returned if an error occurred. 
		/// \note		The invited user must call bdFriends::getIncomingProposals() to retrieve
		///				the friendship proposal. If the invited user is online at the time when the 
		///				proposal is sent, he/she will receive an onFriendshipProposal event.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success. 
		///				BD_PENDING_FRIENDSHIP_EXISTS - if an existing friendship 
		///				proposal has yet to be dealt with.
		///				BD_FRIENDSHIP_EXISTS - if the specified friendID is already
		///				present in this user's friend list.
		bdRemoteTaskRef proposeFriendship(	const bdOnlineUserID friendID,
											const void *const attachment = BD_NULL,
											const bdUInt attachmentSize = 0);
		/// Invite multiple users to become a friend.
		/// \param proposals[] [in]		The proposals which contiain the friend's userID.
		///								and attachment to send with the proposal message.
		/// \param numProposals[in]		The number of friend proposals.
		/// \return		A reference to the task that handles the request. BD_NULL 
		///				is returned if an error occurred. 
		/// \note		The invited user must call bdFriends::getIncomingProposals() to retrieve
		///				the friendship proposals. If the invited user is online at the time when the 
		///				proposal is sent, he/she will receive an onFriendshipProposal event.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - all proposals succeeded. 
		///				BD_PENDING_FRIENDSHIP_EXISTS - if one or more existing friendship 
		///				proposals has yet to be dealt with.
		///				BD_FRIENDSHIP_EXISTS - if one or more friendIDs is already
		///				present in this user's friend list.
		///				BD_SELF_FRIENDSHIP_NOT_ALLOWED - if the requester has included 
		///				themselves in the list
		///				BD_INVALID_USER_ID - if one or more friendIDs is invalid
		///				BD_USERID_BANNED - if one or more friends has banned us
		/// \note		If any one of the proposals fails the remaining will not be
		///				processed
		bdRemoteTaskRef proposeFriendships(	bdFriendProposal proposals[],
											const bdUInt numProposals );

		/// Revoke a proposal for friendship that has not yet been accepted or
		/// rejected.
		/// \param friendID[in]		The user to which the original proposal was 
		///							addressed.
		/// \return		A reference to the task that handles the request. BD_NULL 
		///				is returned if an error occurred.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		bdRemoteTaskRef withdrawProposal(const bdOnlineUserID friendID);

		/// Revoke multiple proposals for friendship that have not yet been accepted or
		/// rejected.
		/// \param friendIDs[in]	The users to which the original proposals were 
		///							addressed.
		/// \param numFriends[in]	The number of proposals to withdraw.
		/// \return		A reference to the task that handles the request. BD_NULL 
		///				is returned if an error occurred.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		bdRemoteTaskRef withdrawProposals(bdOnlineUserID friendIDs[],
											const bdUInt numFriends );

		/// Get a list of all outgoing friendship proposals sent by this user that have not
		/// yet been accepted or rejected.
		/// \param proposedFriends[in]	An array of bdFriendProposal to store the friendship
		///								proposals.
		/// \param maxNumProposals[in] The maximum number of proposals to be retrieved
		/// \return		A reference to the task that handles the request. BD_NULL is 
		///				returned if an error occurred. 
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success. 
		bdRemoteTaskRef getOutgoingProposals(	bdFriendProposal proposedFriends[],
												const bdUInt maxNumProposals);

		/// Get a list of all incoming friendship proposals that this user has not
		/// yet accepted or rejected.
		/// \param incomingProposals[in] An array of bdFriendProposal to store the friendship
		///								 proposals.
		/// \param maxNumProposals[in] The maximum number of proposals to be retrieved
		/// \return		A reference to the task that handles the request. BD_NULL is 
		///				returned if an error occurred. 
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success. 
		bdRemoteTaskRef getIncomingProposals(	bdFriendProposal incomingProposals[],
												const bdUInt maxNumProposals);

		/// Accept a friendship proposal.
		/// This function should be invoked when the local users wishes to accept
		/// a friendship proposal that has been received. This completes
		/// the friendship proposal process.
		/// \param proposer[in]			The user ID of the user that made the proposal.
		/// \return		A reference to the task handle that should be pumped to completion,
		///				or BD_NULL if the operation could not be performed (message 
		///				could not be sent, or maximum number of tasks was exceeded).
		/// \note		A BD_FRIENDSHIP_ACCEPTED notification will be delivered to the
		///				original proposer, together with an onNewNotification event. 
		///				An onPlayerStatusUpdate event will be delivered to both users 
		///				to inform each party of the friend's online status.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		///				BD_FRIENDSHIP_NOT_REQUSTED - if the potential friend never 
		///				proposed friendship.
		///				BD_FRIENDSHIP_EXISTS - if the potential friend is already
		///				in this user's friends list.
		bdRemoteTaskRef acceptProposal(const bdOnlineUserID proposer);

		/// Accepts multiple friendship proposals.
		/// This function should be invoked when the local users wishes to accept
		/// friendship proposals that have been received. This completes
		/// the friendship proposal process.
		/// \param proposers[in]		The user IDs of the users that have made proposals.
		/// \param numProposers			The number of proposers
		/// \return		A reference to the task handle that should be pumped to completion,
		///				or BD_NULL if the operation could not be performed (message 
		///				could not be sent, or maximum number of tasks was exceeded).
		/// \note		A BD_FRIENDSHIP_ACCEPTED notification will be delivered to the
		///				original proposers, together with an onNewNotification event. 
		///				An onPlayerStatusUpdate event will be delivered to both users 
		///				to inform each party of the friend's online status.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		///				BD_FRIENDSHIP_NOT_REQUSTED - if one or more of the potential friends
		///				never proposed friendship.
		///				BD_FRIENDSHIP_EXISTS - if one or more of the potential friends 
		///				is already in this user's friends list.
		///				BD_FRIENDS_FULL - if we cannot have any more friends
		///				BD_INVALID_USER_ID - if one or more of the userIDs is invalid
		/// \note		If any one of the accepts fails the remaining will not be
		///				processed
		bdRemoteTaskRef acceptProposals(bdOnlineUserID proposers[],
										const bdUInt numProposers );

		/// Reject a friendship proposal.
		/// This function should be invoked when the local users wishes to reject
		/// a friendship proposal that has been received. This completes
		/// the friendship proposal process.
		/// \param proposer[in]			The user ID of the user that made the proposal.
		/// \return		A reference to the task handle that should be pumped to completion,
		///				or BD_NULL if the operation could not be performed (message 
		///				could not be sent, or maximum number of tasks was exceeded).
		/// \note		A BD_FRIENDSHIP_REJECTED notification will be delivered to the
		///				original proposer, together with an onNewNotification event.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		///				BD_FRIENDSHIP_NOT_REQUSTED - if the potential friend never 
		///				proposed friendship.
		bdRemoteTaskRef rejectProposal(const bdOnlineUserID proposer);

		/// Reject multiple friendship proposals.
		/// This function should be invoked when the local users wishes to reject
		/// friendship proposals that have been received. This completes
		/// the friendship proposal process.
		/// \param proposers[in]			The user IDs of the users that made the proposal.
		/// \param numProposers				The number of proposers
		/// \return		A reference to the task handle that should be pumped to completion,
		///				or BD_NULL if the operation could not be performed (message 
		///				could not be sent, or maximum number of tasks was exceeded).
		/// \note		A BD_FRIENDSHIP_REJECTED notification will be delivered to the
		///				original proposer, together with an onNewNotification event.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		/// \note		No notification will be given if we attempt to reject a proposal
		///				that does not exist
		bdRemoteTaskRef rejectProposals(bdOnlineUserID proposers[],
										const bdUInt numProposers );

		/// Remove a user from the friends list. 
		/// \param friendID[in]	The friend's user ID.
		/// \return		A reference to the task that handles the request. BD_NULL 
		///				is returned in case an error occurred.
		/// \note		A BD_FRIENDSHIP_REVOKED will be delivered to the
		///				user specified by friendID, together with an onNewNotification event.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		///				BD_NOT_A_FRIEND - if the user specified by friendID is
		///				not in this user's friend list.
		bdRemoteTaskRef removeFriend(const bdOnlineUserID friendID);

		/// Remove a user from the friends list. 
		/// \param friendIDs[in]	The friend user IDs we wish to remove.
		/// \param numFriends[in]	The number of friends in friendIDs
		/// \return		A reference to the task that handles the request. BD_NULL 
		///				is returned in case an error occurred.
		/// \note		A BD_FRIENDSHIP_REVOKED will be delivered to the
		///				user specified by friendID, together with an onNewNotification event.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		/// \note		No notification will be given if we attempt to remove a friendID
		///				who is not our friend
		bdRemoteTaskRef removeFriends(	bdOnlineUserID friendIDs[],
										const bdUInt numFriends );

		/// Remove all friends from the users friends list. 
		/// Intended for use when removing a user and completely deleting all server
		/// data of that user.
		/// \return		A reference to the task that handles the request. BD_NULL 
		///				is returned in case an error occurred.
		/// \note		A BD_FRIENDSHIP_REVOKED will be delivered to all existing 
		///				friends, together with an onNewNotification event.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		bdRemoteTaskRef removeAllFriends();

		/// Retrieve the friend list.
		/// \param friendInfos[in]	 An array of bdFriendInfo to store the friend list							
		/// \param maxNumFriends[in] The maximum number of friends to be retrieved
		/// \return		A reference to the task that handles the request. BD_NULL 
		///				is returned in case an error occurred.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		bdRemoteTaskRef getFriends(	bdFriendInfo friendInfos[],
									const bdUInt maxNumFriends);	
		
		/// Associate a groupID with a particular friend.
		/// The groupIDs are created and associated with strings using 
		/// bdFriends::setGroupName, so that the friend list can be classified 
		/// into sections with readable names.
		/// \param friendID[in]			The user ID of the friend to be classified.
		/// \param groupID[in]			The ID of the group, which was previously
		///								created with bdFriends::setGroupName.
		/// \return		A reference to the task that handles the request. BD_NULL is returned
		///				in case an error occurred.
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		///				BD_NOT_A_FRIEND - if the user specified by friendID
		///				is not in the caller's friends list.
		bdRemoteTaskRef setFriendGroup(	const bdOnlineUserID friendID,
										const bdUByte8 groupID);

		/// Creates or updates a group's name.
		/// \param groupInfo[in]		An object of type bdGroupInfo to set specify the
		///								group ID and name for a group.
		/// \return		A reference to the task handle that should be pumped to completion,
		///				or BD_NULL if the operation could not be performed (message 
		///				could not be sent, or maximum number of tasks was exceeded).
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		bdRemoteTaskRef setGroupName(bdGroupInfo *groupInfo);

		/// Get a list of group names and IDs that have been created by this user.
		/// \param groupInfos[in]	 An array of bdGroupInfo to store the group information.
		/// \param maxNumGroups[in] The maximum number of groups to be retrieved
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success.
		bdRemoteTaskRef getGroupNames(	bdGroupInfo groupInfos[],
										const bdUInt maxNumGroups);

		/// Remove a group name.
		/// \param groupID[in]			The ID of the group name to be removed.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		bdRemoteTaskRef deleteGroupName(const bdUByte8 groupID);

		/// Send an instant message to a friend.
		/// \param friendUID[in]	The user ID of the recipient
		/// \param message[in]		A blob containing the message
		/// \param msgSize[in]		Size of the message in bytes. A maximum of BD_MAX_CHAT_MESSAGE_SIZE
		///							bytes can be sent.
		///	\note	The recipient will receive an onInstantMessage() event.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		bdRemoteTaskRef sendInstantMessage(	const bdOnlineUserID friendUID, 
											const void *const message, 
											const bdUInt msgSize);

		/// Set a user's rich presence data. 
		/// \param data[in]		Pointer to a buffer which contains the rich presence data
		/// \param dataSize[in]	Size of the rich presence data in bytes. A maximum of BD_MAX_RICH_PRESENCE_UPDATE
		///						can be specified.
		/// \note	All friends who are currently online will receive an onRichPresenceUpdate() event. Friends who
		///			go online at a later stage will also receive an onRichPresenceUpdate() event upon signing in.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		bdRemoteTaskRef setRichPresence(const void *const data, 
										const bdUInt dataSize);

		/// Gets the rich presence data for a user
		/// \param friendUID[in] User ID of the friend to retrieve presence data for
		/// \param richPresence[in] Pointer to the rich presence data
		/// \note Responsibility is on the user to create and delete the bdRichPresence structure and it's buffer
		bdRemoteTaskRef getRichPresence(const bdOnlineUserID friendUID, bdRichPresence *richPresence);

	protected:

		friend class bdLobbyService;

		/// Constructs a friend manager with the given task handler.
		/// Note: never call this constructor directly, use bdLobbyService::getFriends() instead.
		/// \param remoteTaskManager[in] A pointer to the remote task manager that handles
		/// requests and replies
		bdFriends(bdRemoteTaskManager *const remoteTaskManager);

		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif // BD_FRIENDS_H
