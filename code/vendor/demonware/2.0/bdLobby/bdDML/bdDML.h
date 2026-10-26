// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The client-side interface to the DML service.

#ifndef BD_DML_H
#define BD_DML_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;

#define BD_RECORD_IP			(1u)

/// Collection of various utility functions 
class bdDML
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdDML(bdRemoteTaskManager *const remoteTaskManager);

		/// Destructor
		~bdDML();

		/// Stores the client's IP address on the Matchmaking+ database. This is primarily
		/// intended to be used by XBoxes that connect through LSP.
		/// \param ipAddr[in]: The client's public IP address
		/// \return	A reference to the task handle that should be pumped to completion
		bdRemoteTaskRef recordIP(const bdUInt32 ipAddr);

	protected:

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;
};


#endif		//BD_DML_H

