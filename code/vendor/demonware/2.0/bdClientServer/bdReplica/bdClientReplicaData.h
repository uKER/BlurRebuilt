// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Data required for replicas on a client.


#ifndef BD_CLIENT_REPLICA_DATA_H
#define BD_CLIENT_REPLICA_DATA_H

#include <bdCore/bdCore.h>

class bdRollbackWindow;
class bdInterpolationData;

/// Data required for replicas on a client.
class bdClientReplicaData
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdClientReplicaData();

		~bdClientReplicaData();

		/// Create the data required for performing interpolation.
		/// \return A pointer to the newly created interpolation data.
		bdInterpolationData* createInterpolationData();

		/// Get  a pointer to the interpolation data, if any.
		/// \return
		/// - A pointer to the current interpolation data.
		/// - BD_NULL if createInterpolationData has not been called.
		bdInterpolationData* getInterpolationData();

		bdRollbackWindow* createRollbackWindow();

		bdRollbackWindow* getRollbackWindow();

	protected:

		bdRollbackWindow *m_rollbackWindow;

		bdInterpolationData *m_interpolationData;

};

#endif // BD_CLIENT_REPLICA_DATA_H
