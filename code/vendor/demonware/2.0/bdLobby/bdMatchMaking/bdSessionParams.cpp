// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdMatchMaking/bdSessionParams.h>

bdSessionParams::bdSessionParams()
{
}

bdSessionParams::~bdSessionParams()
{
}

void bdSessionParams::serialize(bdByteBuffer & /*buffer*/)const
{
}

bdUInt bdSessionParams::sizeOf()
{
	return sizeof(bdSessionParams);
}
