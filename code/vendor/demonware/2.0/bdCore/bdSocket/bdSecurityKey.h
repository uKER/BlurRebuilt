// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Security key, used to identify trusted remote entities.

#ifndef BD_SECURITY_KEY_H
#define BD_SECURITY_KEY_H

#include <bdCore/bdMemory/bdMemory.h>

#define BD_SECURITY_KEY_LENGTH		(16)

#if (defined BD_PLATFORM_XBOX) || (defined BD_PLATFORM_XENON)

typedef XNKEY bdSecurityKey;

BD_COMPILE_ASSERT(sizeof (bdSecurityKey) == BD_SECURITY_KEY_LENGTH, BD_SECURITY_KEY_LENGTH_difffers_from_XNKEY);

#else
/// Represents a security key.
class bdSecurityKey
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Basic constructor.
		bdSecurityKey();

		// Copy constructor.
		bdSecurityKey(const bdSecurityKey &other);

		/// Equals operator to allow easy comparison of bdSecurityKeys.
		/// \param other[in] The sec key to compare with this one.
		/// \return
		///  - True if the 2 sec keys match.
		///  - False otherwise.
		bdBool operator == (const bdSecurityKey &other) const;

	public:
	
		bdUByte8 ab[BD_SECURITY_KEY_LENGTH];

};

#endif // BD_PLATFORM_XBOX || BD_PLATFORM_XENON

#endif // BD_SECURITY_KEY_H

