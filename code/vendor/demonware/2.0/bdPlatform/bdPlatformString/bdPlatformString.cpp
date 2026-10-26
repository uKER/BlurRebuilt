// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <string.h>

bdUWord BD_CALL bdStrGetToken(const bdNChar8 *const str,
							  const bdNChar8 *const delimeters,
							  bdNChar8 *const tokenBuffer,
							  const bdUInt tokenBufferSize,
							  const bdNChar8** end)
{
	bdUWord tokenSize = 0;

	if(str)
	{
		// skip any leading demiliters
		const bdUWord offset = ::strspn(str, delimeters);

		// find the number of characters before the next delimiter
		tokenSize = ::strcspn(str+offset, delimeters);

		// copy the token
		bdStrlcpy(tokenBuffer, str+offset, BD_MIN(tokenSize+1, tokenBufferSize));
		
		if(end)
		{
			//bdNChar8 *string = str;
			*end = str + offset + tokenSize;
		}
	}

	return tokenSize;
}

bdBool BD_CALL bdDelimSubstr(const bdNChar8 *str,
							 const bdNChar8 *substr,
							 const bdNChar8 *const delimiters)
{
	const bdUInt tokenLength = 64;
	bdNChar8 strToken[tokenLength];
	bdNChar8 subStrToken[tokenLength];		

	bdBool isSubStr = true;
	bdBool haveSubStrToken = false;

	do
	{
		
		// read the next token from the potential substring
		bdMemset(subStrToken, BD_NULL, tokenLength);
		const bdUWord subStrTokenLength = bdStrGetToken(substr, delimiters, subStrToken, tokenLength, &substr);
		//BD_ASSERT(subStrTokenLength < tokenLength);

		haveSubStrToken = (subStrTokenLength > 0);

		if((subStrTokenLength < tokenLength) && haveSubStrToken)
		{
			// read the next token from the channel name
			bdMemset(strToken, BD_NULL, tokenLength);
			const bdUWord strTokenLength = bdStrGetToken(str, delimiters, strToken, tokenLength, &str);
			//BD_ASSERT(strTokenLength < tokenLength);

			if(strTokenLength < tokenLength)
			{
				isSubStr = (bdStrcasecmp(subStrToken, strToken) == 0);
			}
		}

	}
	while(isSubStr && haveSubStrToken);

	return isSubStr;
}
