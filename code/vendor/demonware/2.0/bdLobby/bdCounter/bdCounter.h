// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_COUNTER_H
#define BD_COUNTER_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyErrorCode.h>
#include <bdLobby/bdCounter/bdCounterValue.h>

BD_REFERENCE(bdRemoteTask);
class bdRemoteTaskManager;

#define BD_COUNTER_INCREMENT_COUNTER		(1u)
#define BD_COUNTER_GET_COUNTER_TOTAL		(2u)


/// Counters
class bdCounter
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor
		bdCounter(bdRemoteTaskManager *const remoteTaskManager);

		/// Destructor
		~bdCounter();
	
		/// Increments counters by the deltas specified.
		/// \param counterIncrements[in]:	A pointer to an array of (counterID:bdUInt32, delta:bdInt64); for
		///									each pair the counter referred to by counterID will be incremented by delta on the server.
		/// \param numEntries[in]		:	The number of counter / delta pairs in the array \a counterIncrements.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success. 
		bdRemoteTaskRef incrementCounters(bdCounterValue *const counterIncrements, const bdUInt numEntries);


		/// Gets the current total for each of the specified counters.
		/// \param results[in|out]		:	A pointer to an array of bdCounterValue objects,
		///								to specify which counters to retrieve totals for AND 
		///								to store the results for the query when the task has
		///								completed. The size of the array has to match the size
		///								as specified by numCounterIDs.
		/// \param numCounterIDs[in]:	The number of counterIDs in the array \a counterIDs.
		/// \return	A reference to the task handle that should be pumped to completion,
		///			or BD_NULL if the operation could not be performed (message 
		///			could not be sent, or maximum number of tasks was exceeded).
		/// \note	When this task completes, the results are contained in the
		///			supplied bdCounterValue array.
		/// \note	When complete, the returned bdRemoteTask will specify:
		///			BD_NO_ERROR - on success.  
		bdRemoteTaskRef getCounterTotals(bdCounterValue results[], const bdUInt numCounterIDs);

	protected:

		/// remote task manager
		bdRemoteTaskManager *m_remoteTaskManager;



};

#endif //BD_COUNTER_H

