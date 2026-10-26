// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class that can represent both private and public IP addresses.
// This class contains enough information to completely and uniquely identify
// an instance of bitdemon running locally or remotely, even through a NAT.

#ifndef BD_COMMON_ADDR_H
#define BD_COMMON_ADDR_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdContainers/bdArray.h>
#include <bdCore/bdSocket/bdInetAddr.h>
#include <bdCore/bdSocket/bdPort.h>
#include <bdCore/bdSocket/bdAddr.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

BD_REFERENCE(bdCommonAddr);
BD_REFERENCE(bdBitBuffer);

enum bdNATType
{
	BD_NAT_OPEN = 1,
	BD_NAT_MODERATE,
	BD_NAT_STRICT
};

/// The size (in bytes) of the common addr once serialized.
#if defined BD_PLATFORM_WIN32 || \
	defined BD_PLATFORM_UNIX || \
	defined BD_PLATFORM_PS2 || \
	(defined BD_PLATFORM_PSP && !defined BD_PLATFORM_PSP_ADHOC) || \
	defined BD_PLATFORM_PS3 || \
	defined BD_PLATFORM_WII
#	define BD_MAX_LOCAL_ADDRS	(3)
#	define BD_COMMON_ADDR_SERIALIZED_SIZE ((BD_MAX_LOCAL_ADDRS + 1) * BD_ADDR_SIZE + 1)
#elif defined BD_PLATFORM_PSP_ADHOC
// Size: addr size + port number size.
#	define BD_COMMON_ADDR_SERIALIZED_SIZE (BD_IN_ADDR_SIZE + 2)
#elif (defined BD_PLATFORM_XBOX) || (defined BD_PLATFORM_XENON)
// serialized size = XNAddr + a Port number + a titleid (which could be 0).
#	define BD_COMMON_ADDR_SERIALIZED_SIZE (sizeof(XNADDR) + sizeof(bdPort) + sizeof(bdUWord))
#endif

/// A class that can represent both private and public IP addresses.
/// This class contains enough information to completely and uniquely identify
/// an instance of bitdemon running locally or remotely, even through a NAT.
class bdCommonAddr : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor. This will create a blank and unusable common addr.
		bdCommonAddr();

		/// Destructor.
		~bdCommonAddr();

#if defined BD_PLATFORM_WIN32 || defined BD_PLATFORM_UNIX \
	|| defined BD_PLATFORM_PS2 \
	||(defined BD_PLATFORM_PSP && !defined BD_PLATFORM_PSP_ADHOC) \
	|| defined BD_PLATFORM_PS3 \
	|| defined BD_PLATFORM_WII


		/// Creates the common addr for this instance of the engine.
		/// \note Use a different constructor to create remote common addrs.
		/// \param localAddrs[in] Our local address.
		/// \param publicAddr[in] Our public address.
		/// \param natType[in] Our NAT type.
		bdCommonAddr(const bdArray<bdAddr> &localAddrs,
					 const bdAddr& publicAddr,
					 const bdNATType natType = BD_NAT_OPEN);

		/// Creates the common addr for a remote host.		
		/// \param me[in] Our local address.
		/// \param localAddrs[in] The local addresses of the remote host.
		/// \param publicAddr[in] The public address of the remote host
		/// \param natType[in] The NAT type of the remote host.
		bdCommonAddr(bdCommonAddrRef me,
					 const bdArray<bdAddr> &localAddrs,
					 const bdAddr& publicAddr,
					 const bdNATType natType);

		/// Creates the common addr
		/// \param publicAddr[in] Our public address.
		bdCommonAddr(const bdAddr& publicAddr);

		bdNATType getNATType() const;

#elif (defined BD_PLATFORM_XBOX) || (defined BD_PLATFORM_XENON)
		/// Creates the common addr for this instance of the engine.
		/// \note Use a different constructor to create remote common addrs.
		/// \param addr[in] The local instances XNADDR.
		/// \param port[in]
		bdCommonAddr(const XNADDR &addr, const bdPort port);

		bdCommonAddr(bdCommonAddrRef me, const XNADDR &addr, const bdPort port);

		bdCommonAddr(bdCommonAddrRef me, const TSADDR &addr, const bdPort port, const bdUWord titleId);

		/// Returns the XNADDR for this common addr.
		/// \param The XNADDR.
		const XNADDR& getXNAddr() const;

#elif defined BD_PLATFORM_PSP_ADHOC

		/// Creates a loopback common addr
		bdCommonAddr(const bdAddr &addr);

		/// Creates a common addr
		bdCommonAddr(bdCommonAddrRef me, const bdAddr &addr);

		void setAddr(const bdAddr &addr);

#endif

		/// The hash is calculated using the public address, if valid, otherwise the local address.
		/// \return The hash for this common addr.
		bdUInt getHash() const;

		/// Loopback means that this common address is the common address for the local
		/// engine instance.
		/// \return
		///  - True if this instance is loopback..
		///  - False otherwise.
		bdBool isLoopback() const;

		/// Serialize the common address into the buffer.
		/// \param buffer[out] The buffer to serialize the address into.
		void serialize(bdUByte8 buffer[BD_COMMON_ADDR_SERIALIZED_SIZE]) const;

		/// Serialize the common address into the buffer.
		/// \param buffer[out] The buffer to serialize the address into.
		void serialize(bdBitBufferRef buffer) const;

		/// Construct a common address from the data in the buffer.
		/// \param me[in] The common addr of the local instance of the engine.
		/// \param buffer[in] The buffer with the serialized common addr.
		bdBool deserialize(bdCommonAddrRef me,
						   const bdUByte8 buffer[BD_COMMON_ADDR_SERIALIZED_SIZE]);

		/// Construct a common address from the data in the buffer.
		/// \param me[in] The common addr of the local instance of the engine.
		/// \param buffer[in] The buffer with the serialized common addr.
		bdBool deserialize(bdCommonAddrRef me,
						   bdBitBufferRef buffer);

		/// Equality operator.
		/// Compare this to other.
		/// \param other[in]	The address to compare to.
		/// \return
		///  - True if equal.
		///  - False otherwise.
		bdBool operator == (const bdCommonAddr &other) const;

		/// Inequality operator.
		/// Compare this to other.
		/// \param other[in]	The address to compare to.
		/// \return
		///  - True if \b NOT equal.
		///  - False if equal.
		bdBool operator != (const bdCommonAddr &other) const;

		/// Less than operator.
		/// Compare this to other.
		/// \param other[in] The address to compare to.
		/// \return
		///  - True if \a this is less than other.
		///  - False if \a this is equal to or greater than other.
		/// \note This operator is used in bdSet.
		bdBool operator < (const bdCommonAddr &other) const;

#if defined BD_PLATFORM_WIN32 || defined BD_PLATFORM_UNIX \
	|| defined BD_PLATFORM_PS2 \
	|| (defined BD_PLATFORM_PSP && !defined BD_PLATFORM_PSP_ADHOC) \
	|| defined BD_PLATFORM_PS3 \
	|| defined BD_PLATFORM_WII

		const bdAddr &getPublicAddr() const;

		const bdArray<bdAddr> &getLocalAddrs() const;

		const bdAddr &getLocalAddrByIndex(const bdUInt index) const;

#elif (defined BD_PLATFORM_XBOX) || (defined BD_PLATFORM_XENON)

		/// \todo This is really a site id rather than a title id -- rename me appropriately
		bdUWord getTitleId() const;

		/// \todo This is really a site id rather than a title id -- rename me appropriately
		void setTitleId(bdUWord titleid);

		bdPort getPort() const;

#elif defined BD_PLATFORM_PSP_ADHOC

		const bdAddr& getAddr() const;

#endif

		protected:

#if defined BD_PLATFORM_WIN32 || defined BD_PLATFORM_UNIX \
	|| defined BD_PLATFORM_PS2 \
	||(defined BD_PLATFORM_PSP && !defined BD_PLATFORM_PSP_ADHOC) \
	|| defined BD_PLATFORM_PS3 \
	|| defined BD_PLATFORM_WII

		bdArray<bdAddr> m_localAddrs;

		bdAddr m_publicAddr;

		bdNATType m_natType;

#elif (defined BD_PLATFORM_XBOX) || (defined BD_PLATFORM_XENON)

		XNADDR m_addr;

		bdPort m_port;

		bdUWord m_titleId;

#elif defined BD_PLATFORM_PSP_ADHOC

		bdAddr m_addr;
#endif

		bdUInt m_hash;

		bdBool m_isLoopback;


	protected:

		void calculateHash();

};

//TODO can get rid of this class.
class bdCommonAddrHash
{
	public:

		inline bdUInt getHash(const bdCommonAddrRef &addr) const
		{
			return addr->getHash();
		}
};

#endif //BD_COMMON_ADDR_H

