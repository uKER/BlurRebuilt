// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// Purpose: Provides static functions to safely escape and unescape 
// non-alphanumeric in strings.

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

/// The number of characters an escaped character takes up.
#define BD_STR_ESC_CHAR_LENGTH	3u

/// Macro to determine the number of bytes that may be required to contain
/// the escaped version of a string.
#define BD_STR_TO_ESCAPED_SIZE(n) ((n)*3u)+1

/// Macro to determine the number of bytes required to contain an un-escaped
/// version of an escaped string.
#define BD_ESCAPED_TO_STR_SIZE(n) ((n))

/// This class provides a facility for escaping and un-escaping non-alphanumeric
/// characters in strings. This is useful for strings that may be used in statements
/// that will be evaluated in someway, to prevent injection attacks.
/// The escaping technique is to replace all characters that are not alphanumeric
/// with "%xx" where "xx" is the hexadecimal ASCII code of the character.
class bdStringEscape
{
	public:

		/// Escape all non-alphanumeric characters in a string.
		/// \param	src[in]		The string to escape.
		/// \param	dest[out]	A buffer to contain the escaped version of the 
		///						string.
		/// \param	srcLen[in]	The number of characters in the source string.
		/// The size of the destination buffer should be sufficient to contain
		/// the largest possible escaped version of the source string. Use the
		/// BD_STR_TO_ESCAPED_SIZE macro to determine a safe size.
		static bdUInt BD_CALL escape(const bdNChar8* src,
						   bdNChar8* dest,
						   bdUInt srcLen);

		/// Un-escape a string that was originally escaped with escape().
		/// \param	src[in]		The escaped string.
		/// \param	dest[out]	A buffer to contain the un-escaped version of the 
		///						string.
		///	\param	srcLen[in]	The length of the source string (the escaped string).
		/// The length of the unescaped string will be less than or equal to srcLen.
		/// The BD_ESCAPED_TO_STR_SIZE macro should be used to determine a safe size
		/// for the destination buffer, in case the escaping technique changes in
		/// the future.
		static bdUInt BD_CALL unEscape(const bdNChar8* src, 
						       bdNChar8* dest,
						       bdUInt srcLen);

};
