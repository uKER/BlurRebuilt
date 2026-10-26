// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Used for accessing default string handling operations for a platform.

#ifndef BD_PLATFORM_STRING_H
#define BD_PLATFORM_STRING_H

#include <stdarg.h>
#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

typedef va_list	bdVaList;
#define bdVaStart(argPtr, format) va_start(argPtr, format)
#define bdVaEnd(argPtr) va_end(argPtr)

/// Converts a character to lower case.
/// \param c[in] The character to convert.
/// \return A lower case version of \a c.
inline bdInt BD_CALL bdToLower(const bdInt c);

/// Converts a character to upper case.
/// \param c[in] The character to convert.
/// \return A upper case version of \a c.
inline bdInt BD_CALL bdToUpper(const bdInt c);

/// Converts a string to lower case.
/// \param s[in] A pointer to the string to be converted.
/// \return A pointer to the converted string.
inline bdNChar8* BD_CALL bdStrlwr(bdNChar8 *const s);

/// Converts a string to upper case.
/// \param s[in] A pointer to the string to be converted.
/// \return A pointer to the converted string.
inline bdNChar8* BD_CALL bdStrupr(bdNChar8 *const s);

/// Converts a string representation of an integer to a signed integer.
/// \param s[in] The string to convert. 
/// \param endptr[out] A pointer to the first unrecognized character 
///  in the string if one was detected, otherwise this will be null or
///  point to \a s where no integer was found.
/// \param radix[in] The base to use when interpreting the integer.
/// \return The integer equivalent of the passed in string or zero if
///  no valid integer can be found.
inline bdInt BD_CALL bdStrtoi32(const bdNChar8 *const s,
								bdNChar8** endptr, 
								const bdUInt radix);		

/// Converts a string representation of an integer to a unsigned
/// integer.
/// \param s[in] The string to convert. 
/// \param endptr[out] A pointer to the first unrecognized character
///  in the string if one was detected, otherwise this will be null or
///  point to \a s where no integer was found.
/// \param radix[in] The base to use when interpreting the integer.		
/// \return The unsigned integer equivalent of the passed in string or
///  zero if no valid integer can be found.
inline bdUInt BD_CALL bdStrtoui32(const bdNChar8 *const s,
								  bdNChar8** endptr, 
								  const bdUInt radix);		

/// Converts a string representation of an integer to a 64 bit integer.
/// \param s[in] The string to convert. 
/// \param endptr[out] A pointer to the first unrecognized character 
///  in the string if one was detected, otherwise this will be null or
///  point to \a s where no integer was found.
/// \param radix[in] The base to use when interpreting the integer.
/// \return The 64 bit integer equivalent of the passed in string or
///  zero if no valid integer can be found.
/// \note Due to implementation differences the results of this 
/// function may differ between platforms.
inline bdInt64 BD_CALL bdStrtoi64(const bdNChar8 *const s,
								  bdNChar8** endptr, 
								  const bdUInt radix);		

/// Converts a string representation of an integer to a unsigned 64 bit
/// integer.
/// \param s[in] The string to convert. 
/// \param endptr[out] A pointer to the first unrecognized character 
///  in the string if one was detected, otherwise this will be null or
///  point to \a s where no integer was found.
/// \param radix[in] The base to use when interpreting the integer.
/// \return The unsigned 64 bit integer equivalent of the passed in 
///  string or zero if no valid integer can be found.
/// \note Due to implementation differences the results of this 
/// function may differ between platforms.
inline bdUInt64 BD_CALL bdStrtoui64(const bdNChar8 *const s,
									bdNChar8** endptr, 
									const bdUInt radix);

/// Converts a string representation of an float to a 32 bit float.
/// \param s[in] The string to convert. 
/// \param endptr[out] A pointer to the first unrecognized character 
///  in the string if one was detected, otherwise this will be null or
///  point to \a s where no float was found.
/// \return The 32 bit float equivalent of the passed in string or
///  zero if no valid float can be found.
inline bdFloat32 BD_CALL bdStrtof32(const bdNChar8 *const s,
									bdNChar8** endptr);

/// Converts a string representation of an float to a 64 bit float.
/// \param s[in] The string to convert. 
/// \param endptr[out] A pointer to the first unrecognized character 
///  in the string if one was detected, otherwise this will be null or
///  point to \a s where no float was found.
/// \return The 64 bit float equivalent of the passed in string or
///  zero if no valid float can be found.
/// \note Due to implementation differences the results of this 
/// function may differ between platforms.
inline bdFloat64 BD_CALL bdStrtof64(const bdNChar8 *const s,
									bdNChar8** endptr);

/// Appends \a src to \a dest.
/// \param dest[in,out] A pointer to the string to append the \a src
///  string to.
/// \param src[in] A pointer to the string to append to the \a dest
///  string.
/// \param size[in] The number of bytes of \a src to add to \a dest 
/// \return The size of the now extended \a dest string.
inline bdUWord BD_CALL bdStrlcat(bdNChar8 * const dest, 
								 const bdNChar8 * const src, 
								 const bdUWord size);

/// Finds the first occurrence of \a c in \a s.
/// \param s[in] A pointer to the string to search.
/// \param c[in] The character to search for.
/// \return A pointer to the place in \a s where \a c was found or 
///  \a BD_NULL if an occurrence was not found.
/// \note Const version of strchr().
inline const bdNChar8* BD_CALL bdStrchr(const bdNChar8 *const s,
									   const bdInt c);

/// Finds the first occurrence of c in s.
/// \param s[in] A pointer to the string to search.
/// \param c[in] The character to search for.
/// \return A pointer to the place in \a s where \a c was found or 
///  \a BD_NULL if an occurrence was not found.
/// \note Non const version of strchr().
inline bdNChar8* BD_CALL bdStrchr(bdNChar8 *const s,
								 const bdInt c);

/// Finds the last occurrence of \a c in \a s.
/// \param s[in] A pointer to the string to search.
/// \param c[in] The character to search for.
/// \return A pointer to the place in \a s where \a c was found or 
///  \a BD_NULL if an occurrence was not found.
/// \note Const version of strrchr().
inline const bdNChar8* BD_CALL bdStrrchr(const bdNChar8 *const s,
										const bdInt c);

/// Finds the last occurrence of \a c in \a s.
/// \param s[in] A pointer to the string to search.
/// \param c[in] The character to search for.
/// \return A pointer to the place in \a s where \a c was found or 
///  \a BD_NULL if an occurrence was not found.
/// \note Non const version of strrchr().
inline bdNChar8* BD_CALL bdStrrchr(bdNChar8 *const s,
								  const bdInt c);

/// Lexicographically compares two strings.
/// \param s1[in] The first string to use in the comparison.
/// \param s2[in] The second string to use in the comparison.
/// \return
///  - < 0, if \a s1 is less than \a s2. 
///  - 0, if \a s1 is identical to \a s2. 
///  - > 0, if \a s1 is greater than \a s2. 
inline bdInt BD_CALL bdStrcmp(const bdNChar8 *const s1,
							  const bdNChar8 *const s2);

/// Lexicographically compares two strings up to \a len bytes.
/// Compares at most the first \a len characters of \a s1 and \a s2.
/// \param s1[in] The first string to use in the comparison.
/// \param s2[in] The second string to use in the comparison.
/// \param len[in] The maximum number of characters to compare.
/// \return
///  - < 0, if \a s1 substring is less than \a s2 substring.
///  - 0, if \a s1 substring is identical to \a s2 substring.
///  - > 0, if \a s1 substring is greater than \a s2 substring.
inline bdInt BD_CALL bdStrncmp(const bdNChar8 *const s1,
							   const bdNChar8 *s2,
							   const bdInt len);

/// Lexicographically compares two strings in a 
/// case insensitive manner.
/// \param s1[in] The first string to use in the comparison.
/// \param s2[in] The second string to use in the comparison.
/// \return
///  - < 0, if \a s1 is less than \a s2.
///  - 0, if \a s1 is identical to \a s2.
///  - > 0, if \a s1 is greater than \a s2.
/// \note This is a case insensitive equivalent of strcmp().
inline bdInt BD_CALL bdStrcasecmp(const bdNChar8 *const s1,
								  const bdNChar8 *const s2);

/// Lexicographically compares two strings in a 
/// case insensitive manner.
/// Compares at most the first \a len characters of \a s1 and \a s2.
/// \param s1[in] The first string to use in the comparison.
/// \param s2[in] The second string to use in the comparison.
/// \param len[in] The maximum number of characters to compare.
/// \return
///  - < 0, if \a s1 substring is less than \a s2 substring.
///  - 0, if \a s1 substring is identical to \a s2 substring.
///  - > 0, if \a s1 substring is greater than \a s2 substring.
/// \note This is a case insensitive equivalent of strncmp().
inline bdInt BD_CALL bdStrncasecmp(const bdNChar8 *const s1, 
								   const bdNChar8 *s2,
								   const bdInt len);

/// Copies \a src to \a dest.
/// Up to a maximum of \a size bytes or the length of \a src whichever
/// is smaller.
/// \param dst[in,out] The section of memory to copy the data to.
/// \param src[in] The section of memory to copy the data from.
/// \param size[in] The size of the \a dst buffer.
/// \return The length of the string pointed to by \a src, so
///			you can check for potential overflow.
inline bdUWord BD_CALL bdStrlcpy(bdNChar8 *const dst,
								 const bdNChar8 *const src,
								 const bdUWord size);

/// Calculates the length of the string \a s.
/// \param s[in] A pointer to the string whose length is to be 
/// calculated.
/// \return The length of \a s.
/// \note The returned value does not include the null terminator.
inline bdUWord BD_CALL bdStrlen(const bdNChar8 *const s);

/// Calculates the length of the string \a s.
/// \param s[in] A pointer to the string whose length is to be 
/// calculated.
/// \param maxLen[in] The maximum length of the string. This function
///					  will return when either a null terminator has been found or
///                   maxLen bytes is reached.
/// \return The length of \a s.
/// \note The returned value does not include the null terminator.
inline bdUWord BD_CALL bdStrnlen(const bdNChar8 *const s,
								 const bdUWord maxLen);


/// Finds the first occurrence of \a searchStr in \a str.
/// \param str[in] The string to search.
/// \param searchStr[in] The substring to search for.
/// \return 
///  - A pointer to the substring if it is found.
///  - \a BD_NULL if the string is not found. 
///  - \a str if \a searchStr is of length zero.
/// \note Const version.
inline const bdNChar8* BD_CALL bdStrstr(const bdNChar8 *const str, 
									   const bdNChar8 *const searchStr);

/// Finds the first occurrence of \a searchStr in \a str.
/// \param str[in] The string to search.
/// \param searchStr[in] The substring to search for.
/// \return 
///  - A pointer to the substring if it is found.
///  - \a BD_NULL if the string is not found. 
///  - A pointer to \a str if \a searchStr is of length zero.
/// \note Non const version.
inline bdNChar8* BD_CALL bdStrstr(bdNChar8 *const str,
								 const bdNChar8 *const searchStr);

/// Writes a formatted string to a buffer.
/// The output is limited to \a maxlen characters.
/// \param buf[in,out] The destination buffer for the string.
/// \param maxlen[in] The maximum number of characters to write to \a buf.
///	\param format[in] The formatting parameters to use.
/// \param ...[in] Optional parameter(s) that contain the data to be 
///  inserted instead of % tags specified in the \a format parameter.
/// \return The number of characters in the formatted string. 
///			Excluding the null terminator.
/// \note There must be the same number of these optional parameters
///  as the number of format tags in \a format.
inline bdInt BD_CALL bdSnprintf(bdNChar8 *buf, 
								const bdUWord maxlen, 
								const bdNChar8 *format,
								...);

/// Writes a formatted string to a stream.
/// \param stream[in] A handle to the output stream.
///	\param format[in] The formatting parameters to use.
/// \param ...[in] Optional parameter(s) that contain the data to be 
///  inserted instead of % tags specified in the \a format parameter.
/// \return The number of bytes written to \a stream.
/// \note There must be the same number of these optional parameters
///  as the number of format tags in \a format.
inline bdInt BD_CALL bdFprintf(bdFileHandle stream, 
							   const bdNChar8 *format,
							   ...);

/// Writes a formatted string to the standard output stream.
///	\param format[in] The formatting parameters to use.
/// \param ...[in] Optional parameter(s) that contain the data to be 
///  inserted instead of % tags specified in the \a format parameter.
/// \return The number of bytes written to \a stream.
/// \note There must be the same number of these optional parameters
///  as the number of format tags in \a format.
inline bdInt BD_CALL bdPrintf(const bdNChar8 *format,
							  ...);

/// Writes a formatted string to a buffer using a pointer to a list of 
/// arguments. The buffer will be null terminated the number of characters
/// written is limited.
/// \param buf[in,out] The destination buffer for the string.
/// \param maxlen[in] The maximum number of characters to write to \a buf.
///	\param format[in] The formatting parameters to use.
/// \param argptr[in] A pointer to a list of arguments.
/// \return The number of characters in the formatted string. 
///			Excluding the null terminator.
inline bdInt BD_CALL bdVsnprintf(bdNChar8 *buf, 
								 bdUWord maxlen, 
								 const bdNChar8 *format, 
								 bdVaList argptr);

bdUWord BD_CALL bdStrGetToken(const bdNChar8 *const str,
							  const bdNChar8 *const delimeters,
							  bdNChar8 *const tokenBuffer,
							  const bdUInt tokenBufferSize,
							  const bdNChar8** end);

bdBool BD_CALL bdDelimSubstr(const bdNChar8 *str,
							 const bdNChar8 *substr,
							 const bdNChar8 *const delimiters);

/// Get a handle to the standard output stream.
/// \return A handle to stdout.
inline bdFileHandle BD_CALL bdGetStdout();

/// Get a handle to the standard error stream.
/// \return A handle to stderr.
inline bdFileHandle BD_CALL bdGetStderr();

/// Get a handle to the standard input stream.
/// \return A handle to stdin.
inline bdFileHandle BD_CALL bdGetStdin();		

#include <bdPlatform/bdPlatformString/bdPlatformString.inl>

// This is messy but it's the only way to reap the rewards of inlining.
#if defined (BD_PLATFORM_WIN32)
#	include <bdPlatform/bdPlatformString/bdPlatformString-win32.inl>
#elif defined (BD_PLATFORM_PS2) || defined(BD_PLATFORM_PSP) || defined (BD_PLATFORM_PS3)
#	include <bdPlatform/bdPlatformString/bdPlatformString-ps2.inl>
#elif defined (BD_PLATFORM_UNIX)
#	include <bdPlatform/bdPlatformString/bdPlatformString-unix.inl>
#elif defined (BD_PLATFORM_WII)
#	include <bdPlatform/bdPlatformString/bdPlatformString-wii.inl>
#elif defined (BD_PLATFORM_XBOX)
#	include <bdPlatform/bdPlatformString/bdPlatformString-win32.inl>
#elif defined (BD_PLATFORM_XENON)
#	include <bdPlatform/bdPlatformString/bdPlatformString-win32.inl>
#else
#	error Unknown platform, not supported.
#endif

/// Macro to get a bdFileHandle to stdout.
/// Enables code such as:
/// \code 
/// bdFprintf(BD_STDOUT, format, ...);
/// \endcode
#define BD_STDOUT bdGetStdout()

/// Macro to get a bdFileHandle to stderr.
/// Enables code such as:
/// \code 
/// bdFprintf(BD_STDERR, format, ...);
/// \endcode
#define BD_STDERR bdGetStderr()

/// Macro to get a bdFileHandle to stdin.
/// Enables code such as:
/// \code 
/// bdIStream is(BD_STDIN);
/// \endcode
#define BD_STDIN bdGetStdin()


#endif // BD_PLATFORM_STRING_H
