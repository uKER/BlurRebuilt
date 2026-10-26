// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdContainers/bdByteBuffer.h>
#include <bdLobby/bdTags/bdTagQueryParams.h>

bdTagQueryParams::bdTagQueryParams()
{
}

bdTagQueryParams::~bdTagQueryParams()
{
}

void bdTagQueryParams::serialize(bdByteBuffer & /*buffer*/) const
{
}

// Override this function in any subclass
bdUInt bdTagQueryParams::sizeOf() const
{
	return sizeof(bdTagQueryParams);
}
