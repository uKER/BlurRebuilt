// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_GROUP_H
#define BD_GROUP_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdGroup/bdGroupCount.h>
#include <bdLobby/bdGroup/bdGroupID.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;

#define BD_SET_GROUPS				(1u)
#define BD_SET_ENTITY_GROUPS		(2u)
#define BD_GET_ENTITY_GROUPS		(3u)
#define BD_GET_GROUP_COUNTS			(4u)

/// Groups
class bdGroup
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdGroup(bdRemoteTaskManager *const remoteTaskManager);

		/// Destructor
		~bdGroup();
	
		/// Set the group memberships for the logged in user.
		/// \param groupIDs[in]:	A pointer to an array of (groupIDs:bdUInt32)
		/// \param numEntries[in]		:	The number of groupIDs in the array.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef setGroups(	const bdUInt32 *const groupIDs, 
									const bdUInt numEntries);

		/// Set the group memberships for a user.
		/// \param entityID[in]:	The entity to set the group info for.
		/// \param groupIDs[in]:	A pointer to an array of (groupIDs:bdUInt32)
		/// \param numEntries[in]		:	The number of groupIDs in the array.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		bdRemoteTaskRef setGroupsForEntity(	const bdEntityID entityID,
											const bdUInt32 *const groupIDs,
											const bdUInt numEntries);


		/// Get the current group memberships for a user.
		/// \param entityID[in]:	The entity to get the group info for.
		/// \param results[out]:	A pointer to the results array 
		/// \param numEntries[in]:	The size of the results array.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		bdRemoteTaskRef getEntityGroups(const bdEntityID entityID,
										bdGroupID result[],
										const bdUInt numEntries);

		/// Get the current group counts.
		/// \param groupIDs[in]:	A pointer to the array of groupIDs to get counts for.
		/// \param numGroupIDs[in]:	The number of groupIDs in the array.
		/// \param results[out]:	A pointer to the results array 
		/// \param numEntries[in]:	The size of the results array.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.
		bdRemoteTaskRef getGroupCounts(	const bdUInt32 *const groupIDs,
										const bdUInt numGroupIDs,
										bdGroupCount results[], 
										const bdUInt numEntries);


	protected:

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;



};

#endif //BD_GROUP_H

