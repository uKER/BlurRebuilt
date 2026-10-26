// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PSP-ADHOC PS3 WII XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_HASH_TIGER_192_H
#define BD_HASH_TIGER_192_H

#include <bdCore/bdCrypto/bdHash.h>

class bdHashTiger192 : public bdHash
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		///Default constructor
		bdHashTiger192();

		/// Virtual destructor
		virtual ~bdHashTiger192();

		/// Calculate a tiger hash of 'data'.
		/// This will create a 24 byte hash data and place it in 'result'.
		/// \param data[in] The data you wish to hash
		/// \param dataSize[in] The length in bytes of the data to hash
		/// \param result[out]	Where to store the digest.
		///						The size of the array must be 24 bytes !!
		/// \todo consider removing this resultSize param as it must always 
		///       be 24 !!
		virtual bdBool hash(const bdUByte8 *data,
							const bdUInt dataSize,
							bdUByte8 *result,
							bdUInt &resultSize);

	protected:


};

#endif // BD_HASH_TIGER_192_H

