// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE:

#ifndef BD_LOBBY_ERROR_CODE_H
#define BD_LOBBY_ERROR_CODE_H

// set up the macros to create an enum
#define BD_LOBBY_ERROR_CODES_BEGIN enum bdLobbyErrorCode {
#define BD_LOBBY_ERROR_CODE(c, v) c = v,
#define BD_LOBBY_ERROR_CODES_END(c) c };

// include the enum values
#include <bdLobby/bdLobbyErrorCodes.h>

#undef BD_LOBBY_ERROR_CODES_BEGIN
#undef BD_LOBBY_ERROR_CODE
#undef BD_LOBBY_ERROR_CODES_END

#endif // BD_LOBBY_ERROR_CODE_H
