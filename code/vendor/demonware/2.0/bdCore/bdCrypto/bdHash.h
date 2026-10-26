// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PSP-ADHOC PS3 WII XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_HASH_H
#define BD_HASH_H

#include <bdCore/bdMemory/bdMemory.h>

class bdHash
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor
		virtual ~bdHash();

		virtual bdBool hash(const bdUByte8 *data,
							const bdUInt dataSize,
							bdUByte8 *result,
							bdUInt &resultSize) = 0;

};

#endif // BD_HASH_H

