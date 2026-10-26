// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef BD_CONNECTION_STORE_CONFIG_H
#define BD_CONNECTION_STORE_CONFIG_H

#include <bdCore/bdCore.h>


class bdConnectionStoreConfig
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdConnectionStoreConfig();

		void reset();

		bdUInt m_maxReceiveIterations;

		bdFloat32 m_maxShutdownDuration;

		bdUInt m_maxFlushIterations;
};

#endif // BD_CONNECTION_STORE_CONFIG_H

