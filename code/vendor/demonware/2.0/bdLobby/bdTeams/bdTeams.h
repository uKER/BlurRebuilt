// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The client-side interface to the teams service.

#ifndef BD_TEAM_MANAGEMENT_H
#define BD_TEAM_MANAGEMENT_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyErrorCode.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdTeamInfo;
class bdTeamProposal;
class bdTeamMember;
class bdTeamProfile;
class bdTeamMemberProfile;

// Teams operations
#define	BD_TEAM_CREATE								(1u)
#define BD_TEAM_UPDATE_NAME							(2u)
#define BD_TEAM_PROMOTE_MEMBER_TO_ADMIN				(3u)
#define BD_TEAM_KICK_MEMBER							(4u)
#define BD_TEAM_LEAVE								(5u)
#define BD_TEAM_PROPOSE_MEMBERSHIP					(6u)
#define BD_TEAM_REJECT_MEMBERSHIP					(7u)
#define BD_TEAM_ACCEPT_MEMBERSHIP					(8u)
#define BD_TEAM_GET_PUBLIC_PROFILES					(9u)
#define BD_TEAM_GET_PRIVATE_PROFILE					(10u)
#define BD_TEAM_GET_PUBLIC_MEMBER_PROFILES			(11u)
#define BD_TEAM_GET_PRIVATE_MEMBER_PROFILES			(12u)
#define BD_TEAM_SET_PUBLIC_PROFILE					(13u)
#define BD_TEAM_SET_PRIVATE_PROFILE					(14u)
#define BD_TEAM_SET_PUBLIC_MEMBER_PROFILE			(15u)
#define BD_TEAM_SET_PRIVATE_MEMBER_PROFILE			(16u)
#define BD_TEAM_GET_MEMBERSHIPS						(17u)
#define BD_TEAM_GET_MEMBERS							(18u)
#define BD_TEAM_GET_OUTGOING_PROPOSALS				(19u)
#define BD_TEAM_WITHDRAW_PROPOSAL					(20u)
#define BD_TEAM_DEMOTE_MEMBER						(21u)
#define BD_TEAM_PROMOTE_MEMBER_TO_OWNER				(22u)
#define BD_TEAM_GET_TEAMID							(23u)
#define BD_TEAM_GET_INCOMING_PROPOSALS				(24u)
#define BD_TEAM_SEND_INSTANT_MESSAGE				(25u)
#define BD_TEAM_SET_RICH_PRESENCE					(26u)
#define BD_TEAM_GET_MEMBERSHIPS_USER				(27u)
#define BD_TEAM_SEND_INSTANT_MESSAGE_TO_TEAM		(28u)



/// The client-side interface to the teams service.
/// This service enables the creation and management of teams of players, 
/// sometimes referred to as 'clans'. It also provides functionality for team-based
/// presence awareness and intra-team messaging, so team-centric community 
/// features can be implemented in-game.
/// Any user is able to create a new team, and can proceed to invite other user
/// to join it. As well has defining owner and ordinary-member roles, an 
/// administrator role is optionally provided. This allows the team owner to 
/// appoint people to help out with the management of the team, most specifically,
/// recruitment of new members.
/// The use of customized team profiles is also supported, as is customized profiles
/// for individual team members, and the tracking of team stats, using the 
/// leaderboards available via the bdStats interface.
/// Each of the methods provided return a task handle, which should be pumped to 
/// completion by calling its getStatus() function. 
class bdTeams
{
	public:
	
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		~bdTeams();

		/// Create a new team.
		/// Team profile information can be set later by calling bdTeams::setPublicInfo
		/// or bdTeams::setPrivateInfo.
		/// \param teamName[in]		A string containing the name of the team.
		/// \param teamInfo[out]	A pointer to a team info object which will contain
		///							the team ID of the newly created team once the task is completed.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success. 
		///				BD_TEAMNAME_ALREADY_EXISTS - if an existing team exists  
		///				with this teamname.
		///				BD_MAX_TEAM_MEMBERSHIPS_LIMITED - if the caller is already
		///				a member of the number of teams this title is configured to
		///				allow.
		///				BD_MAX_TEAM_OWNERSHIPS_LIMITED - if the caller is already
		///				an owner of the number of teams this title is configured to
		///				allow.
		///				BD_VULGAR_TEAM_NAME - if the team name was deemed vulgar,
		///				and vulgarity checking is enabled.
		bdRemoteTaskRef createTeam(	const bdNChar8 *const teamName,
									bdTeamInfo *const teamInfo);

		/// Retrieve the team ID of a team given the team's name
		/// \param teamName[in]		The teams name.
		/// \param teamInfo[out]	A pointer to a team info object which will contain
		///							the team ID of once the task is completed.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		bdRemoteTaskRef getTeamInfo(const bdNChar8 *const teamName,
									bdTeamInfo *const teamInfo);


		/// Change the team name.
		/// This changes the team name. If you need to change team profile information,
		/// use the bdTeams::setPublicInfo or bdTeams::setPrivateInfo functions.
		/// \param teamID[in]			The team ID.
		/// \param teamName[in]			The new team name.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	A BD_TEAMNAME_UPDATED notification will be delivered to all 
		///			team members.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if the teamID supplied does not exist
		///			BD_NOT_AN_ADMIN_OR_OWNER - if the caller does not have the right
		///			to update the team name.
		///			BD_TEAMNAME_ALREADY_EXISTS - if the new team name is already taken. 
		///         BD_VULGAR_TEAM_NAME - if the team name was deemed vulgar,
		///         and vulgarity checking is enabled.
		bdRemoteTaskRef updateTeamName(	const bdTeamID teamID, 
										const bdNChar8 *const teamName);

		/// Promote an ordinary team member to become an administrator of the team.
		/// This can only be performed by the team owner.
		/// \param teamID[in]			The team ID.
		/// \param userID[in]			The user ID of the member to be promoted.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	A BD_TEAMMEMBER_PROMOTED_TO_ADMIN notification will be delivered to all 
		///			team members.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID does not represent a team
		///			BD_NOT_A_TEAM_OWNER - if the caller is not the owner of the
		///			team represented by teamID.
		///			BD_NOT_A_TEAM_MEMBER - if the user specified by userID is not
		///			a member of the team represented by teamID.
		bdRemoteTaskRef promoteMember(	const bdTeamID teamID, 
										const bdOnlineUserID userID);

		/// Promote an ordinary team member to become the owner of the team.
		/// This can only be performed by the team owner, who is demoted to an
		/// administrator of the team, if the promotion is successful. 
		/// \param teamID[in]			The team ID.
		/// \param userID[in]			The user ID of the member to be promoted.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	A BD_TEAMMEMBER_PROMOTED_TO_OWNER notification will be delivered to all 
		///			team members.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID does not represent a team
		///			BD_NOT_A_TEAM_OWNER - if the caller is not the owner of the
		///			team represented by teamID.
		///			BD_NOT_A_TEAM_MEMBER - if the user specified by userID is not
		///			a member of the team represented by teamID.
		///			BD_MAX_TEAM_OWNERSHIPS_LIMITED - if the user specified by userID is already
		///			an owner of the number of teams this title is configured to allow.
		bdRemoteTaskRef promoteMemberToOwner(	const bdTeamID teamID, 
												const bdOnlineUserID userID);

		/// Demote a team administrator to an ordinary member of the team.
		/// This can only be performed by the team owner.
		/// \param teamID[in]			The team ID.
		/// \param userID[in]			The user ID of the administrator to be demoted.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	A BD_TEAMMEMBER_DEMOTED_TO_ORDINARY notification will be delivered to 
		///			all team members.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID does not represent a team
		///			BD_NOT_A_TEAM_OWNER - if the caller is not the owner of the
		///			team represented by teamID.
		///			BD_NOT_A_TEAM_MEMBER - if the user specified by userID is not
		///			a member of the team represented by teamID.
		bdRemoteTaskRef demoteMember(	const bdTeamID teamID, 
										const bdOnlineUserID userID);

		/// Removes a member from the team. 
		/// Only the owner and team administrators can kick a team member.
		/// \param teamID[in]			The team ID.
		/// \param userID[in]			The user ID of the member to be kicked.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	A BD_TEAMMEMBER_KICKED will be delivered to all members of the team.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID does not represent a team
		///			BD_NOT_A_TEAM_OWNER - if the caller is not the owner of the
		///			team represented by teamID and is attempting to kick an administrator.
		///			BD_NOT_AN_ADMIN_OR_OWNER - if the caller is not an administrator or owner.
		bdRemoteTaskRef kickMember(	const bdTeamID teamID, 
									const bdOnlineUserID userID);

		/// Cancel the local user's membership of a team.
		/// \param teamID[in]			The team ID
		/// \param newOwnerID[in]			(Optional, specify only if the user leaving is the owner of the team). 
		///								If specified by an owner user leaving the team, 
		///								and newOwnerID is a member of the team, newOwnerID becomes the new owner.
		/// 							If no newOwnerID is specified and the owner is leaving the team, 
		///								the team will be disbanded & deleted - regardless of 
		///								how many members are left on the team.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	A BD_TEAMMEMBER_LEFT notification will be delivered to all team members.
		/// \note	A BD_TEAMMEMBER_PROMOTED_TO_OWNER notification will be delivered to the 
		///			user specified by newOwnerID, if any.
		/// \note	A BD_TEAM_DISSOLVED notification will be delivered to all 
		///			teamMembers if the local user was the owner and no newOwnerID was specified.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID does not represent a team
		///			BD_NOT_A_TEAM_MEMBER - if the user specified by newOwnerID
		///			is not a member of the team specified by teamID.
		///			BD_MAX_TEAM_OWNERSHIPS_LIMITED - if the user specified by newOwnerID
		///			is already owner of the max number of teams and therefore can't be promoted
		///			ownner of this team.
		bdRemoteTaskRef leaveTeam(	const bdTeamID teamID, 
									const bdOnlineUserID newOwnerID = BD_NULL);

		/// Propose team membership to a user. 
		/// This can only be performed by the owner or an administrator of the team.
		/// The user that receives the proposal must accept or reject it for the
		/// process to complete, using the bdTeams::acceptMembership or 
		/// bdTeams::rejectMembership functions.
		/// \param teamID[in]			The team ID.
		/// \param newUserID[in]		The user ID to send the proposal to.
		/// \param attachment[in]		Pointer to an optional attachment which will be sent with
		///								the proposal.
		/// \param attachmentSize[in]   Size of the attachment in bytes. The maximum size that can be 
		///								specified is BD_MAX_INVITE_ATTACHMENT_SIZE
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	The invited user must retrieve the team membership proposal by calling
		///			bdTeams::getIncomingProposals(). If the invited user is online at the time when
		///			the proposal is sent, he/she will receive an onTeamProposal() event.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_NOT_AN_ADMIN_OR_OWNER - if the caller is not an administrator
		///			or owner of this team.
		///			BD_MEMBER_EXISTS - if newUserID is already a member of the team.
		///			BD_USERID_BANNED - if newUserID has a fully blocked the caller
		///			by calling bdFriends::setBlockedStatus
		///			BD_MEMBER_PROPOSAL_EXISTS - if newUserID has already received an invitation to the
		///			team from the local user.
		bdRemoteTaskRef proposeMembership(	const bdTeamID teamID, 
											const bdOnlineUserID newUserID,
											const void *const attachment = BD_NULL,
											const bdUInt attachmentSize = 0);
		
		/// Revoke a membership proposal that has not yet been accepted or rejected.
		/// This allows an owner or administrator to cancel outstanding membership
		/// proposals that he/she has sent. This list can be obtained using
		/// bdTeams::getProposalList.
		/// \param userID[in]			The user to which the original proposal 
		///								was addressed.
		/// \param teamID[in]			The team ID.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_MEMBER_NO_PROPOSAL - if no membership proposal exists.
		bdRemoteTaskRef withdrawProposal(	const bdOnlineUserID userID,
											const bdTeamID teamID);

		/// Reject a team membership proposal.
		/// This function can be called by a user who has received a team membership
		/// proposal, but does not wish to accept it.
		/// \param teamID[in]			The team ID.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	A BD_TEAMMEMBER_INVITE_REJECTED notification will be delivered to the original
		///			inviter.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_MEMBER_NO_PROPOSAL - if no membership proposal exists.
		bdRemoteTaskRef rejectMembership(const bdTeamID teamID);

		/// Accept a team membership proposal.
		/// This function can be called by a user who has received a team membership
		/// proposal, and wishes to accept it. After calling this function, the
		/// user will be a member of the team.
		/// \param teamID[in]			The team ID.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	A BD_TEAMMEMBER_INVITE_ACCEPTED will be delivered to the original
		///			inviter, and an onTeamMemberStatusUpdate() event will be delivered
		///			to all team members.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_MEMBER_NO_PROPOSAL - if no membership proposal exists.
		///			BD_MAX_TEAM_MEMBERSHIPS_LIMITED - if the caller is already
		///			a member of the number of teams this title is configured to
		///			allow.
		///			BD_TEAM_FULL - The team is full and no additional members can join.
		bdRemoteTaskRef acceptMembership(const bdTeamID teamID);
        
		/// Get a list of outstanding team membership proposals that have been 
		/// issued by the local user, and which have not yet been accepted or 
		/// rejected.
		/// bdTeams::withdrawProposal can be called on any of the proposals obtained
		/// via this function.
		/// \param teamProposals[out]	List of outstanding team proposals.
		/// \param maxNumProposals[in]	Maximum number of proposals to be retrieved.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef getOutgoingProposals(	bdTeamProposal teamProposals[],
												const bdUInt maxNumProposals);

		/// Get a list of all incoming team proposals that this user has not
		/// yet accepted or rejected.
		/// \param incomingProposals[in] An array of bdTeamProposal to store the team
		///								 proposals.
		/// \param maxNumProposals[in] The maximum number of proposals to be retrieved
		/// \return		A pointer to the task that handles the request. BD_NULL is 
		///				returned if an error occurred. 
		/// \note		When complete, the returned bdRemoteTask will specify:
		///				BD_NO_ERROR - on success. 
		bdRemoteTaskRef getIncomingProposals(	bdTeamProposal incomingProposals[],
												const bdUInt maxNumProposals);

		/// Retrieve the public team profile.
		/// This retrieves publicly accessible profile information about a team
		/// that has been created by the team owner using bdTeams::setPublicInfo.
		/// \param teamIDs[in]			List of team IDs for which the profile is to be retrieved.
		/// \param publicProfiles[out]	List of public team profiles.
		/// \param numTeams[in]			Number of teams for which the profile info is requested.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		bdRemoteTaskRef getPublicProfiles(	const bdTeamID teamIDs[],
											bdTeamProfile publicProfiles[],
											const bdUInt numTeams);


		/// Retrieve the private team profile.
		/// This retrieves private profile information about a team
		/// that has been created by the team owner using bdTeams::setPrivateInfo.
		/// This can only be performed by a team member.
		/// \param teamID[in]			The team ID.
		/// \param privateProfile[out]	Pointer to the private team profile. This should be a
		///								subclass of bdTeamProfile to match the custom attributes
		///								specified for the private profile.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		bdRemoteTaskRef getPrivateProfile(	const bdTeamID teamID,
											bdTeamProfile *const privateProfile);

		/// Set all fields of the public team profile.  
		/// This may only be performed by the team owner.
		/// \param teamID[in]			The team ID.
		/// \param teamInfo[in]			An instance of a class derived from 
		///								bdTeamProfile that represents the public 
		///								profile info.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_PARAM_MISMATCHED_TYPE - if the format of teamInfo did not
		///			match the format of the database.
		///			BD_NOT_A_TEAM_MEMBER - if the caller is not a member of the
		///			team represented by teamID.
		bdRemoteTaskRef setPublicProfile(const bdTeamID teamID,
									     bdTeamProfile *publicProfile);
		
		/// Set all fields of the private team profile.  
		/// This may only be performed by the team owner.
		/// \param teamID[in]			The team ID.
		/// \param teamInfo[in]			An instance of a class derived from 
		///								bdTeamProfile that represents the private 
		///								profile info.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_PARAM_MISMATCHED_TYPE - if the format of teamInfo did not
		///			match the format of the database.
		///			BD_NOT_A_TEAM_MEMBER - if the caller is not a member of the
		///			team represented by teamID.
		bdRemoteTaskRef setPrivateProfile(	const bdTeamID teamID,
											bdTeamProfile *privateProfile);
		
		/// Retrieve the public member profiles of a team.
		/// \param teamID[in]		 The team ID.
		/// \param publicMemberInfos[out] Pointer to an array of bdTeamMember objects which will
		///							contain the returned member profiles.
		/// \param numTeamMembers[in]	 Size of the publicMemberInfos array.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_NOT_A_TEAM_MEMBER - if the caller is not a member of the 
		///			team specified by teamID.
		bdRemoteTaskRef getPublicMemberProfiles(const bdTeamID teamID,
												bdTeamMemberProfile publicMemberProfiles[],
												const bdUInt numTeamMembers);

		/// Retrieve the private profile of a team member.
		/// This can only be performed by a team member.
		/// \param teamID[in]		 The team ID.
		/// \param privateMemberProfiles[out] Pointer to an array of bdTeamMember objects which will
		///							contain the returned member profiles.
		/// \param numTeamMembers[in]	 Size of the publicMemberInfos array.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When this task completes, the results are contained in the 
		///			privateMemberProfiles array.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_NOT_A_TEAM_MEMBER - if the caller is not a member of the 
		///			team specified by teamID.
		bdRemoteTaskRef getPrivateMemberProfiles(	const bdTeamID teamID, 
													bdTeamMemberProfile privateMemberProfiles[],
													const bdUInt numTeamMembers);

		/// Set all fields of the public profile of a team member.  
		/// To avoid overwriting previous changes, first call bdTeams::getPublicMemberInfo() 
		/// to retrieve the current public team member data.
		/// \param teamID[in]			The team ID.
		/// \param teamInfo[in]			An instance of a class derived from 
		///								bdTeamMember that represents the public 
		///								profile info.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_NOT_A_TEAM_MEMBER - if the caller is not a member of the 
		///			team specified by teamID.
		///			BD_PARAM_MISMATCHED_TYPE - if the format of teamInfo did not
		///			match the format of the database.
		bdRemoteTaskRef setPublicMemberProfile(	const bdTeamID teamID, 
												bdTeamMemberProfile *teamMemberProfile);
		
		/// Set all fields of the private profile of a team member.  
		/// To avoid overwriting previous changes, first call bdTeams::getPrivateMemberInfo() 
		/// to retrieve the current private team member data.
		/// \param teamID[in]			The team ID.
		/// \param teamInfo[in]			An instance of a class derived from 
		///								bdTeamMember that represents the private 
		///								profile info.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		///			BD_NOT_A_TEAM_MEMBER - if the caller is not a member of the 
		///			team specified by teamID.
		///			BD_PARAM_MISMATCHED_TYPE - if the format of teamInfo did not
		///			match the format of the database.
		bdRemoteTaskRef setPrivateMemberProfile(const bdTeamID teamID, 
												bdTeamMemberProfile *teamMemberProfile);

		/// Get the list of teams that a user is a member of.
		/// \param teamInfos[out]	A pointer to an array of bdTeamInfo objects to store
		///							the team information.
		///	\param maxNumTeams[in]	Maximum number of team memberships to be returned.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef getMemberships(	bdTeamInfo teamInfos[],
										const bdUInt maxNumTeams);

		/// Get the list of teams that a user is a member of by user id.
		/// param userID[in]		The user to retrieve memberships for.
		/// \param teamInfos[out]	A pointer to an array of bdTeamInfo objects to store
		///							the team information.
		///	\param maxNumTeams[in]	Maximum number of team memberships to be returned.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef getMembershipsUser(	const bdOnlineUserID userID,
											bdTeamInfo teamInfos[],
										    const bdUInt maxNumTeams);

		/// Get the list of members of a specified team.
		/// \param teamID[in]			 The team ID to retrieve the members of.
		/// \param teamMembers[out]		 A pointer to an array of bdTeamMember objects to store
		///								 the team member information.
		///	\param maxNumTeamMembers[in] Maximum number of team members to be returned.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if teamID is invalid.
		bdRemoteTaskRef getMembers(	const bdTeamID teamID,
									bdTeamMember teamMembers[],
									const bdUInt maxNumTeamMembers);

		/// Send an instant message to a team member.
		/// \param teamMemberUID[in]	The user ID of the recipient
		/// \param message[in]			A blob containing the message
		/// \param msgSize[in]			Size of the message in bytes. A maximum of BD_MAX_CHAT_MESSAGE_SIZE
		///								bytes can be sent.
		///	\note	The recipient will receive an onInstantMessage() event.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_USERID_BANNED - if teamMemberUID has a fully blocked the caller
		///			by calling bdFriends::setBlockedStatus
		///			BD_NOT_A_TEAM_MEMBER - if the caller and teamMemberUID
		///			are not on the same team.
		bdRemoteTaskRef sendInstantMessage(	const bdOnlineUserID teamMemberUID, 
											const void *const message, 
											const bdUInt msgSize);
		
		/// Set a team's rich presence data. 
		/// \param teamID[in]	The teamID for which the rich presence data is set
		/// \param data[in]		Pointer to a buffer which contains the rich presence data
		/// \param dataSize[in]	Size of the rich presence data in bytes. A maximum of BD_MAX_RICH_PRESENCE_UPDATE
		///						can be specified.
		/// \note	All team members who are currently online will receive an onRichPresenceUpdate() event. Team
		///			members who go online at a later stage will also receive an onRichPresenceUpdate() event upon signing in.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		bdRemoteTaskRef setRichPresence(const bdTeamID teamID, 
										const void *const data, 
										const bdUInt dataSize);

		/// Send an instant message to all members of a team.
		/// \param teamID[in]	The user ID of the recipient
		/// \param message[in]			A blob containing the message
		/// \param msgSize[in]			Size of the message in bytes. A maximum of BD_MAX_CHAT_MESSAGE_SIZE
		///								bytes can be sent.
		///	\note	The recipients will receive an onInstantMessage() event.
		/// \return	A pointer to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		///			BD_INVALID_TEAM_ID - if the team does not exist.
		///			BD_NOT_A_TEAM_MEMBER - if the caller is not a member of the given team and
		///			sending messages to teams that users are not a part of is disabled.
		///			BD_TEAM_EMPTY - if the team has no members online.
		bdRemoteTaskRef sendInstantMessageToTeam(	const bdOnlineUserID teamID, 
													const void *const message, 
													const bdUInt msgSize);


	protected:

		friend class bdLobbyService;

		/// Constructs a team manager with the given task handler.
		/// Note: never call this constructor directly, use bdLobbyService::getTeams() instead.
		/// \param remoteTaskManager[in] A pointer to the remote task manager that handles
		/// requests and replies
		bdTeams(bdRemoteTaskManager *const remoteTaskManager);

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif // BD_TEAM_MANAGEMENT_H
