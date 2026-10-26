// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: The result set obtained following a call to bdTitleUtilities::verifyString.

#ifndef BD_VERIFY_STRING_H
#define BD_VERIFY_STRING_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

/// Container for the result of a call to verifyString.
/// \todo Decide whether this code should differentiate partial matches 
///       and full matches? 
/// \note Currently partial matches will be considered profane!
class bdVerifyString : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		// Constructor.
		bdVerifyString();

		// See documentation in base class bdTaskResult.
		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		// See documentation in base class bdTaskResult.
		virtual bdUInt sizeOf();

		/// - True: the string has been verified
		/// - False: string did not pass the vulgarity filter
		bdBool m_verified;

};

#endif // BD_VERIFY_STRING_H
