// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Represents a platform independent relationship between a common 
//          address and a real address used for communication.

#ifndef BD_ADDR_HANDLE_H
#define BD_ADDR_HANDLE_H

#include <bdCore/bdCore.h>
#include <bdCore/bdSocket/bdEndpoint.h>

#if !defined BD_PLATFORM_PSP_ADHOC

BD_REFERENCE(bdAddrHandle);

/// This class represents, in a platform independent manner, the relationship
/// between a bdCommonAddr and the real address used for communication.
class bdAddrHandle : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		/// Status of the handle.
		enum bdAddrHandleStatus
		{
			BD_ADDR_NOT_CONSTRUCTED,
			BD_ADDR_NOT_RESOLVED,
			BD_ADDR_ERROR,
			BD_ADDR_RESOLVED,
			BD_ADDR_UNRESOLVED
		};

		///Get the security address for this handle
		/// \return the security id for this handle
		void getSecID(bdSecurityID& secID) const;

		/// Get the real address for this handle.
		/// \return The real addr for this handle.
		const bdAddr getRealAddr() const;

		/// Equals operator.
		/// \param other[in] The addr handle to compare this one to.
		/// \return:
		///  - True if this addrHandle matches other.
		///  - False otherwise.
		bdBool operator == (const bdAddrHandle &other) const;

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON
		/// Setup the real address for this handle.
		/// \param realAddr[in] The real addr.
		void setRealAddr(const in_addr &realAddr, bdPort port);

		void print();

#else

		/// Setup the real address for this handle.
		/// \param realAddr[in] The real addr.
		void setRealAddr(const bdAddr &realAddr);

		/// Setup the status of this handle.
		/// \param status[in] The new handle status.
		void setStatus(const bdAddrHandleStatus &status);

		/// Get the status of this handle.
		/// \return The status of this handle.
		bdAddrHandleStatus getStatus() const;

#endif

#if defined BD_PLATFORM_XBOX || defined BD_PLATFORM_XENON

	public:
		/// the remote entities endpoint
		bdEndpoint m_endpoint;

		/// The real address used for communication
		in_addr m_addr;

		/// The port used for communication.
		bdPort m_port;

	protected:

		/// Addr Handles can only be created by bdAddressMap so the 
		/// constructors are protected.
		friend class bdAddressMap;

		/// Complex Constructor.
		/// \param ca[in] The common address portion of the handle.
		/// \param secId[in] The security id portion of the handle.
		bdAddrHandle(bdCommonAddrRef ca, const bdSecurityID &secID);

#else // other platforms.

	public:
		/// the remote entities endpoint
		bdEndpoint m_endpoint;

	protected:
		/// The status of the handle.
		bdAddrHandleStatus m_status;

		/// The real address used for communication.
		bdAddr m_realAddr;

	protected:

		/// Addr Handles can only be created by bdAddressMap so the 
		/// constructors are protected.
		friend class bdAddressMap;
		
		/// Complex Constructor.
		/// \param ca[in] The common address portion of the handle.
		/// \param secId[in] The security id portion of the handle.
		bdAddrHandle(bdCommonAddrRef ca, const bdSecurityID &secID);

#endif


};

#else

typedef bdAddr bdAddrHandle;
typedef bdAddrHandle bdAddrHandleRef;

#endif

#endif // BD_ADDR_HANDLE_H

