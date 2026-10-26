// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_EXAMPLE_COMMON_CS_H
#define BD_EXAMPLE_COMMON_CS_H

#include <bdExample/bdExampleCommon/bdExampleCommon.h>

#include <bdClientServer/bdServer.h>
#include <bdClientServer/bdClient.h>

// listeners
#include <bdClientServer/bdPlayer/bdServerPlayerListener.h>
#include <bdClientServer/bdPlayer/bdClientPlayerListener.h>
#include <bdClientServer/bdReplica/bdUpdate/bdUpdateListener.h>
#include <bdClientServer/bdReplica/bdClientReplicaListener.h>

// factories
#include <bdClientServer/bdCommand/bdCommandFactory.h>
#include <bdClientServer/bdPlayer/bdPlayerFactory.h>
#include <bdClientServer/bdReplica/bdReplicaFactory.h>

#include <bdClientServer/bdCommand/bdCommandGenerator.h>

#include <bdClientServer/bdPlayer/bdPlayer.h>
#include <bdClientServer/bdReplica/bdInterpolationData.h>

// client/server replica data
#include <bdClientServer/bdReplica/bdClientReplicaData.h>
#include <bdClientServer/bdReplica/bdServerReplicaData.h>

#include <bdClientServer/bdPlayer/bdInterest/bdInterestInterface.h>

#endif // BD_EXAMPLE_COMMON_CS_H

