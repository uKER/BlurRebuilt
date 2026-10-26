// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Provide a wrapper around an address handle, so it can be correctly 
//          hashed, and thus stored in a hashmap etc...

#ifndef BD_ADDR_HANDLE_REF_WRAPPER_H
#define BD_ADDR_HANDLE_REF_WRAPPER_H

#include <bdCore/bdCore.h>
#include <bdSocket/bdAddrHandle.h>

/// Wrapper class for a bdAddrHandleRef instance. Intelligent implements some comparison 
/// operators to enable the class to be stored in comparison operator-based containers 
/// such as: bdHashMap , bdSet...
class bdAddrHandleRefWrapper
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Basic Constructor.
		bdAddrHandleRefWrapper();

		/// Complex Constructor.
		/// \param handle[in] The address handle to wrap.
		bdAddrHandleRefWrapper(bdAddrHandleRef handle);

		/// Equals operator.
		/// \param other[in] The addr handle wrapper to compare this one to.
		/// \return:
		///  - True if this addrHandleWrapper matches other.
		///  - False otherwise.
		bdBool operator == (const bdAddrHandleRefWrapper &other) const;

		/// Get a hash of this wrapped addr handle.
		bdUInt getHash(const bdAddrHandleRefWrapper &addr) const;
	
		/// return the bdAddrHandleRefWrapper's bdAddrHandleRef m_handle
		/// \return bdAddrHandleRefWrapper member variable m_handle
		bdAddrHandleRef getHandle() const;

	protected:

		/// the wrapped bdAddrHandleRef
		bdAddrHandleRef m_handle;

};


#endif // BD_ADDR_HANDLE_REF_WRAPPER_H

