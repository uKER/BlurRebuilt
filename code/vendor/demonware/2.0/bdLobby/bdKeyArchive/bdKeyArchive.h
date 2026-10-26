// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:	The client-side API for writing and reading indexed values

#ifndef BD_KEY_ARCHIVE_H
#define BD_KEY_ARCHIVE_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;
class bdKeyValuePair;

// bdKeyArchive operations
#define BD_KEYARCHIVE_WRITE		(1u)
#define BD_KEYARCHIVE_READ		(2u)
#define BD_KEYARCHIVE_READ_ALL	(3u)

// The client-side API for writing and reading indexed values
class bdKeyArchive
{
	public:
		
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		~bdKeyArchive();

		/// Writes an array of Key/Value pairs to the server.
		/// Standard Client(users) can only write their own keyPairs, or of a teams keyPairs if the 
		/// client is a member of the team.
		/// Dedicated servers can write any users keyPairs, and any teams keyPairs
		/// \param entityID[in]	 The user/team id for which the values are written.
		/// \param isTeam[in]	 Flag to indicate whether the entity ID pertains to a team (true) or to a normal user (false)
		/// \param values[in]	 An array of key/value pairs. 
		/// \param numValues[in] The number of values to be written.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		bdRemoteTaskRef write(	const bdEntityID entityID,
								const bdBool isTeam,
								const bdKeyValuePair values[],
								const bdUInt numValues);

		/// Reads a specific set of key/value pairs from the server.
		/// Both clients and dedicated servers can read all users and all teams bdKeyPairs.
		/// \param entityID[in]	 The user/team id of which the values are read.
		/// \param isTeam[in]	 Flag to indicated whether the entity ID pertains to a user or team.
		/// \param readDedicated[in]	Specifies whether to read keypairs which were written by users (false), or 
		///								to read keypairs which were written by dedicated servers (true).
		/// \param keyValues[in|out] A pointer to an array of bdKeyValuePair to specify which values are to be read
		///						  AND to store the result set. 
		/// \param numValues[in] The size of the keyValues array.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		bdRemoteTaskRef read(	const bdEntityID entityID,
								const bdBool isTeam,
								const bdBool readDedicated,
								bdKeyValuePair keyValues[],
								const bdUInt numValues);

		/// Reads all existing key/value pairs from the server.
		/// Both clients and dedicated servers can read all users and all teams bdKeyPairs.
		/// \param entityID[in]	 The user/team id of which the values are read.
		/// \param isTeam[in]	 Flag to indicated whether the entity ID pertains to a user or team.
		/// \param readDedicated[in]	Specifies whether to read keypairs which were written by users (false), or 
		///								to read keypairs which were written by dedicated servers (true).
		/// \param keyValues[out] A pointer to an array of bdKeyValuePair store the result set. 
		/// \param numValues[in] The size of the keyValues array.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		bdRemoteTaskRef readAll(const bdEntityID entityID,
								const bdBool isTeam,
								const bdBool readDedicated,
                                bdKeyValuePair keyValues[],
                                const bdUInt numValues);

	protected:

		friend class bdLobbyService;

		bdKeyArchive(bdRemoteTaskManager *const remoteTaskManager);

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;

};

#endif //BD_KEY_ARCHIVE_H
