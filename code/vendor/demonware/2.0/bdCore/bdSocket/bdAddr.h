// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class that wraps an IPv4 address and Port number.

#ifndef BD_ADDR_H
#define BD_ADDR_H

#include <bdCore/bdMemory/bdMemory.h>

#include <bdCore/bdSocket/bdInetAddr.h>
#include <bdCore/bdSocket/bdPort.h>

#include <bdCore/bdSocket/bdSocketConfig.h>

#define BD_ADDR_STRING_SIZE (22u)
#define BD_ADDR_SIZE		(BD_INET_ADDR_SIZE + 2)
#define BD_INVALID_PORT (0u)

/// A class that wraps an IPv4 address and Port number. 
class bdAddr
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		/// Constructs an host with an invalid IP and
		/// port 0.
		bdAddr();

		/// Copy constructor.
		/// \param other[in]	The other host.
		bdAddr(const bdAddr &other);

		/// Construct using IP address and port.
		/// \param address[in]	The address.
		/// \param port[in]		The virtual port.
		bdAddr(const bdInetAddr &address,
			   const bdPort port);

		/// Construct using IP address and port.
		/// \param socketAddress[in] The address in the form "ip:port" with:
		///   - ip in the form "a.b.c.d".
		///   - port a 16-bit unsigned integer value.
		bdAddr(const bdNChar8 *socketAddress);

		/// Construct using IP address and port.
		/// \param socketAddress[in] The address in the form "ip:port" with:
		///   - ip in the form "a.b.c.d".
		///   - port a 16-bit unsigned integer value.
		void set(const bdNChar8 *socketAddress);

		/// Set the internal state of this object.
		/// \param address[in]	The address.
		/// \param port[in]		The virtual port.
		void set(const bdInetAddr &address,
				 const bdPort port);

		/// Equality operator.
		/// Compare this to other.
		/// \param other[in]	The address to compare to.
		/// \return 
		///  - True if equal.
		///  - False otherwise.
		bdBool operator == (const bdAddr &other) const;

		/// Inequality operator.
		/// Compare this to other.
		/// \param other[in]	The address to compare to.
		/// \return 
		///  - True if \b NOT equal.
		///  - False if equal.
		bdBool operator != (const bdAddr &other) const;

		/// Less than operator.
		/// Compare this to other. 
		/// \param other[in] The address to compare to.
		/// \return 
		///  - True if \a this is less than other.
		///  - False if \a this is equal to or greater than other.
		/// \note This operator is used in bdSet.
		bdBool operator < (const bdAddr &other) const;

		/// Accessor function for the IP address section of this socket address.
		/// \return The address.
		const bdInetAddr& getAddress() const;

		/// Accessor function for the IP address section of this socket address.
		/// \return The address (const).
		/// \note A non const version of the previous function.
		bdInetAddr& getAddress();

		void setPort(const bdPort port);

		/// Accessor function for the port section of this socket address.
		/// \return The port.
		bdPort getPort() const;

		/// Create a string representation of the IP address and Port in the 
		/// form of "ip:port".		
		bdUWord toString(bdNChar8 *const str, 
						 const bdUWord size) const;

		bdUInt getHash() const;

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
		/// \param data[in] Pointer to the start of the byte buffer.
		/// \param size[in] Size of the buffer, in bytes.
		/// \param offset[in] Position within the byte buffer to read from. Equals to the
		/// number of bytes already read from the buffer.
		/// \param newOffset[out] New adjusted position within the byte buffer where subsequent
		/// read should be made from.
		/// \return True, if successful, false otherwise.
		bdBool deserialize(const void* data, const bdUInt size, const bdUInt offset, bdUInt &newOffset);

		/// Returns number of bytes needed to serialize this object.
		/// \return Maximum number of bytes needed to serialize this object instance.
		bdUInt getSerializedSize() const;

#ifdef BD_PLATFORM_PSP_ADHOC

		/// Convenience functions to make bdAddr on the PSP similar to bdReference.
		bdBool isNull();
		
		bdBool notNull();
		
#endif

	protected:

		/// The IP address section of this socket address.
		bdInetAddr	m_address;

		/// The port section of this socket address.
		bdPort	m_port;

		static bdUInt serializedSize;
};

class bdAddrHash
{
	public:

		inline bdUInt getHash(const bdAddr &addr) const 
		{
			return addr.getHash();
		}
};

#endif // BD_ADDR_H
