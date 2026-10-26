// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_MESSAGE_FLAGS_H
#define BD_MESSAGE_FLAGS_H

enum bdMessageFlag
{
	BD_RELIABLE_0			= (1 << 0),
	BD_RELIABLE_1			= (1 << 1),
	BD_IS_ACKNOWLEDGEMENT	= (1 << 2)
};

typedef bdUByte8 bdMessageFlags;

#endif // BD_MESSAGE_FLAGS_H
