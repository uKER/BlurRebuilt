// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class that associates a bdCommonAddrRef and bdSecurityID and provides
// a hashing class.

#ifndef BD_ENDPOINT
#define BD_ENDPOINT

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdSocket/bdSecurityID.h>

BD_REFERENCE(bdCommonAddr);

/// A class that associates a bdCommonAddrRef and a bdSecurityID
class bdEndpoint 
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		/// Default constructor for deserialization
		bdEndpoint();

		/// Constructor which creates a bdEndpoint from a bdCommonAddrRef and a bdSecurityID
		/// \param addr[in] Our endpoint's address
		/// \param secID[in] Our endpoint's security id
		bdEndpoint(const bdCommonAddrRef addr, 
			const bdSecurityID& secID);

		/// Equals operator to allow comparison of bdEndpoints
		/// \param other[in] The bdEndpoint to compare with this one.
		/// \return
		///  - True if the 2 bdEndpoints match
		///  - False otherwise.
		bdBool operator == (const bdEndpoint &other) const;		

		/// Returns the member variable m_ca of bdEndpoint object
		/// \return member variable m_ca
		bdCommonAddrRef getCommonAddr() const;

		/// Returns the member variable m_secID of bdEndpoint object
		/// \return member variable m_secID
		const bdSecurityID& getSecID() const;

		/// Returns the number of bytes this endpoint uses when serialized 
		/// \return number of bytes in serialized representation
		bdUInt getSerializedLength() const;

		/// Serialize this object into a byte buffer.
		/// Safely write out this object into a byte buffer.
		/// \param data[in] Pointer to the start of the byte buffer.
		/// \param size[in] Size of the buffer, in bytes.
		/// \param offset[in] Position within the byte buffer to write into. Equals to the
		/// number of bytes already stored in the buffer.
		/// \param newOffset[out] New adjusted position within the byte buffer where subsequent
		/// writes should be made to.
		/// \return True, if successful, false otherwise.
		bdBool serialize(void *data, const bdUInt size, const bdUInt offset, bdUInt &newOffset) const;

		/// Deserialize this object from a byte buffer.
		/// Safely read in this object from a byte buffer.
		/// \param bdCommonAddrRef[in] My common addr, for loopback check
		/// \param data[in] Pointer to the start of the byte buffer.
		/// \param size[in] Size of the buffer, in bytes.
		/// \param offset[in] Position within the byte buffer to read from. Equals to the
		/// number of bytes already read from the buffer.
		/// \param newOffset[out] New adjusted position within the byte buffer where subsequent
		/// read should be made from.
		/// \return True, if successful, false otherwise.
		bdBool deserialize(bdCommonAddrRef me, const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset);

	protected:

		bdCommonAddrRef m_ca;

		bdSecurityID m_secID;

};

/// bdEndpoints hashing class
class bdEndpointHashingClass
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		/// Forms a hash from the endpoints bdCommonAddrRef and bdSecurityID
		/// \param other[in] the bdEndpoint to form the hash from
		/// \return the hash for this bdEndpoint
		bdUInt getHash(const bdEndpoint &other) const;
};

#endif // BD_ENDPOINT

