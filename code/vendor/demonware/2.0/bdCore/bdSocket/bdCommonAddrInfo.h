// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Get textual information describing a common addr.

#ifndef BD_COMMON_ADDR_INFO_H
#define BD_COMMON_ADDR_INFO_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdReference/bdReference.h>

BD_REFERENCE(bdCommonAddr);

// The suggested buffer size of a buffer to describe a common addr
# define BD_COMMON_ADDR_INFO_SIZE 1024

/// Get a text string describing a bdCommonAddr.
class bdCommonAddrInfo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Writes textual information about a bdCommonAddr to a buffer.
		/// \param addr[in] A pointer to the bdCommonAddr whose
		///		information is required.
		/// \param buf[in,out] The pointer to a buffer into which the
		///		information should be written.
		/// \param length[in] The length of the data pointer to by \a buf
		///		in bytes.
		static bdUInt BD_CALL getInfo(const bdCommonAddr& addr,
									  bdNChar8 *buf,
									  const bdUInt length);

		static bdUInt BD_CALL getInfo(const bdCommonAddrRef addr,
									  bdNChar8 *buf,
									  const bdUInt length);

		static bdUInt BD_CALL getBriefInfo(const bdCommonAddr& addr,
										   bdNChar8 *buf,
										   const bdUInt length);

		static bdUInt BD_CALL getBriefInfo(const bdCommonAddrRef addr,
										   bdNChar8 *buf,
										   const bdUInt length);
	protected:

		/// Private constructor. There should be no need to
		/// new this class as everything is static.
		bdCommonAddrInfo();
};

#endif // BD_COMMON_ADDR_INFO_H
