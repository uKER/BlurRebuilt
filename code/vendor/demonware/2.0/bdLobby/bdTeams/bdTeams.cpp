// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTeams/bdTeams.h>
#include <bdLobby/bdTeams/bdTeamInfo.h>
#include <bdLobby/bdTeams/bdTeamMember.h>
#include <bdLobby/bdTeams/bdTeamProposal.h>
#include <bdLobby/bdTeams/bdTeamProfile.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdTeams/bdTeamMemberProfile.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

#define BD_LOG_LEVEL "teams"
#define TYPE_CHECKED true

bdTeams::bdTeams(bdRemoteTaskManager *const remoteTaskManager)
: m_remoteTaskManager(remoteTaskManager)
{
}


bdTeams::~bdTeams()
{
}


bdRemoteTaskRef bdTeams::createTeam(const bdNChar8 *const teamName,
									bdTeamInfo *const teamInfo)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	bdUInt taskSize = BD_TASK_OVERHEAD + ((bdUInt)(bdStrnlen(teamName, BD_MAX_TEAMNAME_LENGTH))) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_CREATE);

	const bdBool wroteString = buffer.writeString(teamName, BD_MAX_TEAMNAME_LENGTH);
	if(true == wroteString)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			const bdUInt numResults = 1;
			remoteTask->setTaskResult(teamInfo, numResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write string %s into buffer" , teamName);
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::getTeamInfo(const bdNChar8 *const teamName,
									 bdTeamInfo *const teamInfo)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	bdUInt taskSize = BD_TASK_OVERHEAD + ((bdUInt)(bdStrnlen(teamName, BD_MAX_TEAMNAME_LENGTH))) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_TEAMID);

	const bdBool wroteString = buffer.writeString(teamName, BD_MAX_TEAMNAME_LENGTH);
	if(true == wroteString)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			const bdUInt numResults = 1;
			remoteTask->setTaskResult(teamInfo, numResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write string %s into buffer" , teamName);
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::updateTeamName(bdTeamID teamID,
										const bdNChar8 *const teamName)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdTeamID)) + (bdUInt)(bdStrnlen(teamName, BD_MAX_TEAMNAME_LENGTH)) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_UPDATE_NAME);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeString(teamName, BD_MAX_TEAMNAME_LENGTH);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	return remoteTask;
}

bdRemoteTaskRef bdTeams::promoteMember(	const bdTeamID teamID,
										const bdOnlineUserID userID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdTeamID)) + (sizeof(bdOnlineUserID)) +  numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_PROMOTE_MEMBER_TO_ADMIN);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeUInt64(userID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}


bdRemoteTaskRef bdTeams::promoteMemberToOwner(	const bdTeamID teamID, 
												const bdOnlineUserID userID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdTeamID)) + (sizeof(bdOnlineUserID)) + numParameters ;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_PROMOTE_MEMBER_TO_OWNER);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeUInt64(userID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::demoteMember(	const bdTeamID teamID, 
										const bdOnlineUserID userID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdTeamID)) + (sizeof(bdOnlineUserID)) + numParameters ;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_DEMOTE_MEMBER);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeUInt64(userID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::kickMember(const bdTeamID teamID, 
									const bdOnlineUserID userID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + sizeof(bdOnlineUserID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_KICK_MEMBER);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeUInt64(userID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::leaveTeam(	const bdTeamID teamID,
									const bdOnlineUserID newOwnerID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdTeamID)) + (sizeof(bdOnlineUserID)) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_LEAVE);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeUInt64(newOwnerID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::proposeMembership(	const bdTeamID teamID, 
											const bdOnlineUserID newUserID,
											const void *const attachment,
											const bdUInt attachmentSize)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 3;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdTeamID)) + (sizeof(bdOnlineUserID)) + attachmentSize + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_PROPOSE_MEMBERSHIP);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeUInt64(newUserID);
	if(attachment != BD_NULL)
	{
		wroteParam = wroteParam && buffer.writeBlob(attachment,attachmentSize);
	}
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::withdrawProposal(	const bdOnlineUserID userID,
											const bdTeamID teamID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + sizeof(bdTeamID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_WITHDRAW_PROPOSAL);

	bdBool wroteParam = buffer.writeUInt64(userID);
	wroteParam = wroteParam && buffer.writeUInt64(teamID);

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::rejectMembership(const bdTeamID teamID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdTeamID)) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_REJECT_MEMBERSHIP);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::acceptMembership(const bdTeamID teamID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdTeamID)) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_ACCEPT_MEMBERSHIP);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::getOutgoingProposals(	bdTeamProposal teamProposals[],
												const bdUInt maxNumProposals)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 0;
	const bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_OUTGOING_PROPOSALS);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		remoteTask->setTaskResult(teamProposals, maxNumProposals);
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::getIncomingProposals(	bdTeamProposal teamProposals[],
												const bdUInt maxNumProposals)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 0;
	const bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_INCOMING_PROPOSALS);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		remoteTask->setTaskResult(teamProposals, maxNumProposals);
	}
	
	return remoteTask;
}


bdRemoteTaskRef bdTeams::getPublicProfiles(	const bdTeamID teamIDs[],
											bdTeamProfile publicProfiles[],
											const bdUInt numTeams)
											
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = numTeams ;
	const bdUInt taskSize = BD_TASK_OVERHEAD + (sizeof(bdTeamID)* numTeams) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_PUBLIC_PROFILES);

	bdBool wroteParam = true;
	for(bdUInt i = 0; i < numTeams; i++)
	{
		wroteParam = wroteParam && buffer.writeUInt64(teamIDs[i]);
	}

	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			remoteTask->setTaskResult(publicProfiles, numTeams);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}



bdRemoteTaskRef bdTeams::getPrivateProfile(const bdTeamID teamID,
										   bdTeamProfile *const privateProfile)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_PRIVATE_PROFILE);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			const bdUInt numResults = 1;
			remoteTask->setTaskResult(privateProfile, numResults);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	
	return remoteTask;
}

bdRemoteTaskRef bdTeams::setPublicProfile(const bdTeamID teamID,
										  bdTeamProfile *publicProfile)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + publicProfile->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_SET_PUBLIC_PROFILE);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	publicProfile->serialize(buffer);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::setPrivateProfile(	const bdTeamID teamID,
											bdTeamProfile *privateProfile)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + privateProfile->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_SET_PRIVATE_PROFILE);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	privateProfile->serialize(buffer);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::setPublicMemberProfile(const bdTeamID teamID, 
												bdTeamMemberProfile *teamMemberProfile)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + teamMemberProfile->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_SET_PUBLIC_MEMBER_PROFILE);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	teamMemberProfile->serialize(buffer);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::setPrivateMemberProfile(	const bdTeamID teamID, 
													bdTeamMemberProfile *teamMemberProfile)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + teamMemberProfile->sizeOf() + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_SET_PRIVATE_MEMBER_PROFILE);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	teamMemberProfile->serialize(buffer);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}



bdRemoteTaskRef bdTeams::getPublicMemberProfiles(	const bdTeamID teamID,
													bdTeamMemberProfile publicMemberProfiles[],
													const bdUInt numTeamMembers)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_PUBLIC_MEMBER_PROFILES);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			remoteTask->setTaskResult(publicMemberProfiles, numTeamMembers);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}


bdRemoteTaskRef bdTeams::getPrivateMemberProfiles(	const bdTeamID teamID, 
													bdTeamMemberProfile privateMemberProfiles[],
													const bdUInt numTeamMembers)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1 ;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_PRIVATE_MEMBER_PROFILES);
	bdBool wroteParam = buffer.writeUInt64(teamID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			remoteTask->setTaskResult(privateMemberProfiles, numTeamMembers);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::getMemberships(bdTeamInfo teamInfos[],
									    const bdUInt maxNumTeams)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 0;
	const bdUInt taskSize = BD_TASK_OVERHEAD + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_MEMBERSHIPS);

	const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
	if(startTaskResult != BD_NO_ERROR)
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
	}
	else
	{
		remoteTask->setTaskResult(teamInfos, maxNumTeams);
	}
	
	return remoteTask;
}

bdRemoteTaskRef bdTeams::getMembershipsUser(const bdOnlineUserID userID,
											bdTeamInfo teamInfos[],
											const bdUInt maxNumTeams)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 0;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_MEMBERSHIPS_USER);

	bdBool wroteParam = buffer.writeUInt64(userID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			remoteTask->setTaskResult(teamInfos, maxNumTeams);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}
	
	return remoteTask;
}

bdRemoteTaskRef bdTeams::getMembers(const bdTeamID teamID,
									bdTeamMember teamMembers[],
									const bdUInt maxNumTeamMembers)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 1;
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID) + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_GET_MEMBERS);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
		else
		{
			remoteTask->setTaskResult(teamMembers, maxNumTeamMembers);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::sendInstantMessage(const bdOnlineUserID teamMemberUID, 
											const void *const message, 
											const bdUInt msgSize)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt actualMessageSize = BD_MIN(msgSize , BD_MAX_CHAT_MESSAGE_SIZE);
	if (actualMessageSize < msgSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Exceeded maximum message size, message will be truncated.");
	}
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdOnlineUserID)+ actualMessageSize + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_SEND_INSTANT_MESSAGE);

	bdBool wroteParam = buffer.writeUInt64(teamMemberUID);
	wroteParam = wroteParam && buffer.writeBlob(message, actualMessageSize);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::setRichPresence(	const bdTeamID teamID, 
											const void *const data, 
											const bdUInt dataSize)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 0;
	const bdUInt actualDataSize = BD_MIN(dataSize , BD_MAX_RICH_PRESENCE_UPDATE);
	if (actualDataSize < dataSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Exceeded maximum rich presence size, data will be truncated.");
	}
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID)+ actualDataSize + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_SET_RICH_PRESENCE);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeBlob(data, actualDataSize);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

bdRemoteTaskRef bdTeams::sendInstantMessageToTeam(	const bdTeamID teamID,
													const void *const message,
													const bdUInt msgSize)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	const bdUInt numParameters = 2;
	const bdUInt actualMessageSize = BD_MIN(msgSize , BD_MAX_CHAT_MESSAGE_SIZE);
	if (actualMessageSize < msgSize)
	{
		BD_ERR(BD_LOG_LEVEL, "Exceeded maximum message size, message will be truncated.");
	}
	const bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(bdTeamID)+ actualMessageSize + numParameters;
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_TEAM_MANAGEMENT_SERVICE, BD_TEAM_SEND_INSTANT_MESSAGE_TO_TEAM);

	bdBool wroteParam = buffer.writeUInt64(teamID);
	wroteParam = wroteParam && buffer.writeBlob(message, actualMessageSize);
	if(true == wroteParam)
	{
		const bdLobbyErrorCode startTaskResult = m_remoteTaskManager->startTask(remoteTask, buffer);
		if(startTaskResult != BD_NO_ERROR)
		{
			BD_WARN(BD_LOG_LEVEL , "Failed to start task: Error %i" , startTaskResult);
		}
	}
	else
	{
		BD_WARN(BD_LOG_LEVEL , "Failed to write param into buffer" );
	}

	return remoteTask;
}

