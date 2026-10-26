// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Maps the relationship between address handles and common addresses.

#ifndef BD_ADDRESS_MAP_H
#define BD_ADDRESS_MAP_H

#include <bdCore/bdCore.h>

#include <bdCore/bdSocket/bdEndpoint.h>
#include <bdSocket/bdAddrHandle.h>

/// The address map is a class that maps address handles to real address 
/// information. The address handles are used to refer to endpoints before the
/// correct address has been resolved by NAT traversal.
class bdAddressMap
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Basic Constructor.
		bdAddressMap();

		/// Converts a common address and security id to an address handle.
		/// The address handle can then be used to send data to the client that
		/// the common address specified and will be returned when data is 
		/// received from that client.
		/// If a handle entry does not exist in the map, one will be created.
		/// \param ca[in] The common address of the remote client.
		/// \param id[in] The security id to be used.
		/// \param addrHandle[out] Address handle ref that receives the created 
		///	                       address handle.
		/// \return:
		///  - True if the addrHandle was successfully found/created.
		///  - False otherwise.
		bdBool getAddrHandle(const bdCommonAddrRef ca,
							 const bdSecurityID &id,
							 bdAddrHandleRef &addrHandle);

		/// Converts a common address and security id to an address handle.
		/// The address handle can then be used to send data to the client that
		/// the common address specified and will be returned when data is 
		/// received from that client.
		/// Unlike getAddrHandle, if a handle entry does not exist, none will be created
		/// \param ca[in] The common address of the remote client.
		/// \param id[in] The security id to be used.
		/// \param addrHandle[out] Address handle ref that receives the created 
		///	                       address handle.
		/// \return:
		///  - True if the addrHandle was successfully found
		///  - False otherwise.
		bdBool findAddrHandle(const bdCommonAddrRef ca,
				  			  const bdSecurityID &id,
							  bdAddrHandleRef &addrHandle) const;

		/// Get the common addr and sec id associated with an address handle.
		/// \param addrHandle[in] A reference to a valid address handle.
		/// \param ca[out] A reference to a common address. This will be 
		///                changed to point to the common address of the 
		///                address handle.
		/// \param id[out] A reference to where the security id for the address 
		///                handle will be copied.
		bdBool addrToCommonAddr(const bdAddrHandleRef &addrHandle,
								bdCommonAddrRef &ca,
								bdSecurityID &id) const;

		/// Gets a string representation of the address handle.
		/// If the address handle has not yet been resolved by the NAT
		/// traversal then it will return "not_resolved" as the string unless
		/// the address is a local loopback address in which case str is set to
		/// "loopback".
		/// \param addrHandle[in] The address handle to get a string 
		///                       representation of.
		/// \param str[out] A pointer to where the string will be written.
		/// \param size[in] The size of the char array pointed to by \c str.
		/// \return The size of the fully expanded string.
		///	        If this value is equal to or greater than 'size' it means 
		///         that the output was truncated.
		static bdUWord BD_CALL addrToString(const bdAddrHandleRef &addrHandle,
											bdNChar8 *const str,
											const bdUWord size);

		/// \deprecated Use getAddrHandle() instead. This will be removed after the 
		///             DemonWare 2.0 release.
		bdBool commonAddrToAddr(const bdCommonAddrRef ca,
								const bdSecurityID &id,
								bdAddrHandleRef &addrHandle);

#if !defined BD_PLATFORM_PSP_ADHOC
		/// Clears the address map completely.
		/// This will result in all entries in the map being
		/// removed. Calling this function when connected to remote
		/// nodes has undefined behaviour.
		void clear();
#endif

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
		/// Setup the local common addr.
		/// \param localCommonAddr[in] The local common addr.
		void setupLocalCommonAddr(bdCommonAddrRef localCommonAddr);

		bdBool findAddrHandle(const bdAddr &ra,  bdAddrHandleRef &addrHandle) const;

		void print();
#endif

	protected:

		friend class bdDTLSAssociation;
		friend class bdSocketRouter;
		friend class bdRelaySocketRouter;
		/// Unregisters any data associated with the address handle. If a secure association
		/// exists it will also be torn down.
		/// \param addrHandle[in] the bdAddrHandleRef to be unregistered
		/// \ return:
		///  - True if the bdAddrHandleRef was unregistered
		///  - False otherwise.
		bdBool unregisterAddr(bdAddrHandleRef &addrHandle);

		/// Sets the bdAddr status to BD_ADDR_UNRESOLVED
		/// \param realAddr[in] the bdAddr to be unregistered
		/// \return:
		///  - True if the bdAddr status was changed
		///  - False otherwise.
		bdBool unregisterRealAddr(const bdAddr realAddr);

	protected:

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
		bdCommonAddrRef m_me;
#endif

#if !defined BD_PLATFORM_PSP_ADHOC

		/// an Array of bdAddrHandleRefs, the address map
		bdArray<bdAddrHandleRef> m_addrHandles;

#endif
};

#endif // BD_ADDRESS_MAP_H
