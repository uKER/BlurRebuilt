// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class that wraps a MAC address.

#ifndef BD_MAC_ADDR_H
#define BD_MAC_ADDR_H

#include <bdCore/bdMemory/bdMemory.h>

#define BD_MAC_ADDR_SIZE (6u)

/// A class to represent a MAC[Media Access Control] address.
class bdMACAddr
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdMACAddr();

		bdUByte8 m_data[6];

};


#endif // BD_MAC_ADDR_H
