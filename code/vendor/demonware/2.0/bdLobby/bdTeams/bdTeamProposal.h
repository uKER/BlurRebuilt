 // DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Result set container for bdTeams::getProposalsList tasks.

#ifndef BD_TEAM_PROPOSAL_H
#define BD_TEAM_PROPOSAL_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>


class bdTeamProposal : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		bdTeamProposal();

		~bdTeamProposal();

		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		/// Size of instances of this class. 
		virtual bdUInt sizeOf();
		
		/// The user ID of the player who sent the team proposal
		bdOnlineUserID m_userID;

		/// The team ID of the membership proposal
		bdTeamID m_teamID;

		/// The name of the proposing player
		bdNChar8 m_userName[BD_MAX_USERNAME_LENGTH];

		/// The name of the proposing team
		bdNChar8 m_teamName[BD_MAX_TEAMNAME_LENGTH];

		/// Optional attachment that was sent with the membership proposal
		bdUByte8 m_attachment[BD_MAX_INVITE_ATTACHMENT_SIZE];

		/// Size of the attachment in bytes
		bdUInt	m_attachmentSize;
};

#endif // BD_TEAM_PROPOSAL_H
