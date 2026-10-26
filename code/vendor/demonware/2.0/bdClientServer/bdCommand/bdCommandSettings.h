// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A place to store/record all the command settings for bitdemon.

#ifndef BD_COMMAND_SETTINGS_H
#define BD_COMMAND_SETTINGS_H

#include <bdCore/bdCore.h>

/// The number of commands to be buffered in the bdCommandWindow.
/// Must be a power of 2.
#define BD_COMMAND_HISTORY_SIZE (64u) 

/// The number of old commands to send with each new command.
/// The last BD_COMMAND_REDUNDANCY+1 commands will be sent.
#define BD_COMMAND_REDUNDANCY (2u) 

/// The data type to use for command sequence numbering.
/// This should be an unsigned type.
typedef bdUInt16 bdCommandSeqNum;
#define BD_COMMAND_SEQ_NUM_READ readUInt16
#define BD_COMMAND_SEQ_NUM_WRITE writeUInt16

/// Used to verify the command settings are sensible in debug builds.
/// Incorrect settings are indicated by throwing BD_ASSERT, BD_ERR or
/// BD_WARN.
/// This function is empty unless BD_DEBUG is defined.
void BD_CALL bdCheckCommandSettings();

#endif // BD_COMMAND_SETTINGS_H
