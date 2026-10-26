// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class that represents an IPv4 address

#ifndef BD_INET_ADDR_H
#define BD_INET_ADDR_H

#include <bdPlatform/bdPlatformSocket/bdInAddr.h>
#include <bdCore/bdMemory/bdMemory.h>

#define BD_INET_ADDR_STRING_SIZE (16u)

#define BD_INET_ADDR_SIZE		 (BD_IN_ADDR_SIZE)

/// A class that represents an IPv4 address
class bdInetAddr
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		static bdInetAddr BD_CALL Loopback();
		static bdInetAddr BD_CALL Broadcast();
		static bdInetAddr BD_CALL Any();


		/// Default constructor.
		/// Create a bdInetAddr with an invalid address.
		/// This address is invalid [ see isValid() ].
		bdInetAddr();

		/// Default destructor.
		~bdInetAddr();

		/// Copy constructor.
		/// \param other[in]	The other host.
		bdInetAddr(const bdInetAddr &other);
#if !defined BD_PLATFORM_PSP_ADHOC
		/// Constructor taking an integer representation of the address.
		/// \param address[in]	The IP address as an unsigned 32-bit integer.
		bdInetAddr(const bdUInt address);
#else

		/// Constructor taking an integer representation of the address, PSP only.
		/// \param address[in]	The IP address as an unsigned 64-bit integer.
		/// This function uses the lower 6 bytes as the address.
		bdInetAddr(const bdUInt64 address);

		/// Constructor taking a character array representation of the address, PSP only.
		/// \param address[in]	The ethernet address .
		bdInetAddr(const bdUByte8 address[6]);

#endif

		/// Constructor taking a string representation of the address.
		/// Address should be in the format of "a.b.c.d"  ie an IP address,
		/// not a hostname.
		/// \param address[in]	The IP address as a string.
		bdInetAddr(const bdNChar8 *address);

		/// Constructor taking a pointer to an bdInAddr.
		/// \param inaddr[in] The address in bdInAddr (in_addr) format.
		bdInetAddr(const bdInAddr inaddr);
	
#if !defined BD_PLATFORM_PSP_ADHOC

		/// Set the address.
		/// Address should be in the format of "a.b.c.d"  ie an IP address,
		/// not a hostname.
		/// \param address[in]	The IP address as a string.
		void set(const bdNChar8 *address);


		/// Set the address.
		/// \param address[in]	The IP address as an unsigned 32-bit integer.
		void set(const bdUInt address);

#endif

		/// Set the address.
		/// \param other[in]	Other instance of bdInetAddr.
		void set(const bdInetAddr &other);

		/// Creates a bdInetAddr with a pointer to an bdInAddr.
		/// \param inaddr[in] The address in bdInAddr (in_addr) format.
		void set(const bdInAddr inaddr);

		/// Equality operator.
		/// Compares this bdInetAddr with another one.
		/// \param other[in]	Address to compare this to.
		/// \return 
		///  - True if equal.
		///  - False otherwise.
		bdBool operator ==(const bdInetAddr &other) const;

		/// Inequality operator.
		/// Compares this bdInetAddr with another one.
		/// \param other[in]	Address to compare this to.
		/// \return 
		///  - True if \b NOT equal.
		///  - False otherwise.
		bdBool operator !=(const bdInetAddr &other) const;

		/// Less than operator.
		/// Compares this bdInetAddr with another one.
		/// \param other[in] Address to compare this to
		/// \return 
		///  - True if this is less than other.
		///  - False if this is equal to or greater than other.
		/// \note This is used by bdSet. 
		bdBool operator < (const bdInetAddr &other) const;

		/// Check if this address valid.
		/// \return 
		///  - True if the address is valid.
		///  - False if the address is invalid.
		bdBool isValid() const;

		/// Check if this address is an address on the local machine.
		/// \return 
		///  - True if address belongs to local machine.
		///  - False otherwise.
		/// \warning This function is quite narrow in scope. It will return
		///  false for any IP address apart from [127.0.0.1] & [0.0.0.0].
		///  Therefore it should be used with caution.
		bdBool isLoopback() const;

		/// Check if this address is a broadcast address.
		/// \return 
		///  - True if address is broadcast.
		///  - False otherwise.
		bdBool isBroadcast() const;

		/// Get an integer representation of this address.
		/// \return The address as an unsigned 32-bit integer.
		bdUInt toUInt32() const;

		/// Gets a string representation of this address.
		bdUWord toString(bdNChar8 *const str, 
						 const bdUWord size) const;

		bdInAddr getInAddr() const;

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

	protected:

		bdInAddr m_addr;

};


#endif // BD_INET_ADDR_H
