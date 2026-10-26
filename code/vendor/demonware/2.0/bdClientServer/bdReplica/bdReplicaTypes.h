// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Define the various types of bdReplica.

#ifndef BD_REPLICA_TYPES_H
#define BD_REPLICA_TYPES_H

#include <bdCore/bdCore.h>

#define BD_REPLICA		(1u)
#define BD_USER_TYPE	(2u)

/// The data type to use for indentifying different bdReplica derived classes.
/// This should be an unsigned type.
typedef bdUInt32 bdReplicaType;

/// The data type to use for indentifying instances of bdReplica.
/// This should be an unsigned type.
typedef bdUInt16 bdReplicaId;
#define BD_REPLICA_ID_READ readUInt16
#define BD_REPLICA_ID_WRITE writeUInt16

#endif // BD_REPLICA_TYPES_H
