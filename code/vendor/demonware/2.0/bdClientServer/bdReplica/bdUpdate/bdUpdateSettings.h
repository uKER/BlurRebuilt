// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The update related settings for bitdemon.

#ifndef BD_UPDATE_SETTINGS_H
#define BD_UPDATE_SETTINGS_H

#include <bdCore/bdCore.h>

/// The size of bdUpdateWindow on the server and the size of bdRollbackWindow
/// on the client.
/// This must be a power of 2 and less than max possible value of 
/// bdUpdateSeqNum.
#define BD_UPDATE_HISTORY_SIZE (16u)

/// The data type to use for update sequence numbering.
/// This should be an unsigned type.
typedef bdUByte8 bdUpdateSeqNum;
#define BD_UPDATE_SEQ_NUM_READ readUByte8
#define BD_UPDATE_SEQ_NUM_WRITE writeUByte8

/// Used to verify the update settings are sensible in debug builds.
/// Incorrect settings are indicated by throwing BD_ASSERT, BD_ERR or
/// BD_WARN.
/// This function is empty unless BD_DEBUG is defined.
void BD_CALL bdCheckUpdateSettings();

#endif // BD_UPDATE_SETTINGS_H
