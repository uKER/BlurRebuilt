// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_DISCOVERY_H
#define BD_DISCOVERY_H

#include <bdCore/bdCore.h>

// The size of the nonce in bytes
#define BD_NONCE_SIZE (8)

#ifndef BD_PLATFORM_XENON

// The port on which discovery messages are sent
#define BD_DEFAULT_DISCOVERY_PORT (50000)

#else

/// The port on which discovery messages are sent
#define BD_DEFAULT_DISCOVERY_PORT (1200)

#endif//#ifndef BD_PLATFORM_XENON


// The minimum size of a discovery message (BD_NONCE_SIZE + Title ID + MessageType)
#define BD_MIN_DISCOVERY_SIZE (11)

#endif // BD_DISCOVERY_H



