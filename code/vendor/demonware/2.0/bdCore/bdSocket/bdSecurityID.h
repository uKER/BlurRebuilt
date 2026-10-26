// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Identifier for security key data.

#ifndef BD_SECURITY_ID_H
#define BD_SECURITY_ID_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdPlatform/bdPlatformError/bdPlatformError.h>

#define BD_SECURITY_ID_LENGTH		(8)

#if (defined BD_PLATFORM_XBOX) || (defined BD_PLATFORM_XENON)

typedef XNKID bdSecurityID;

BD_COMPILE_ASSERT(sizeof (bdSecurityID) == BD_SECURITY_ID_LENGTH, BD_SECURITY_ID_LENGTH_difffers_from_XNKID);

#else

/// Represents a security key data identifier.
class bdSecurityID
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Basic constructor.
		bdSecurityID();

		// Copy constructor.
		bdSecurityID(const bdSecurityID &other);

		/// Equals operator to allow easy comparison of bdSecurityIDs.
		/// \param other[in] The sec id to compare with this one.
		/// \return
		///  - True if the 2 sec ids match.
		///  - False otherwise.
		bdBool operator == (const bdSecurityID &other) const;

		/// Not equals operator to allow easy comparison of bdSecurityIDs.
		/// \param other[in] The sec id to compare with this one.
		/// \return
		///  - True if the 2 sec ids don't match.
		///  - False otherwise.
		bdBool operator != (const bdSecurityID &other) const;

	public:
	
		bdUByte8 ab[BD_SECURITY_ID_LENGTH];

};

#endif // BD_PLATFORM_XBOX || BD_PLATFORM_XENON

#endif // BD_SECURITY_ID_H

