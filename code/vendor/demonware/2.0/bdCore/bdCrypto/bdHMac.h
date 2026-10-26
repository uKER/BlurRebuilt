// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Interface for HMac implementations.

#ifndef BD_HMAC_H
#define BD_HMAC_H

#include <bdCore/bdMemory/bdMemory.h>

class bdHMac
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor
		virtual ~bdHMac();

		virtual bdBool process(const bdUByte8 *const data,
							   const bdUInt length) = 0;

		virtual bdBool getData(bdUByte8 *dst,
							   bdUInt &length) = 0;

};

#endif // BD_HMAC_H

