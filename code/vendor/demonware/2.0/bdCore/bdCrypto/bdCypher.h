// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Interface for Cypher implementations.

#ifndef BD_CYPHER_H
#define BD_CYPHER_H

#include <bdCore/bdMemory/bdMemory.h>

class bdCypher
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor
		virtual ~bdCypher();

		virtual bdBool encrypt(const bdUByte8 *iv,
							   const bdUByte8 *pt,
							   bdUByte8 *ct,
							   const bdUInt size) = 0;

		virtual bdBool decrypt(const bdUByte8 *iv,
							   const bdUByte8 *ct,
							   bdUByte8 *pt,
							   const bdUInt size) = 0 ;
		
};

#endif // BD_CYPHER_H

