// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdUtilities/bdStringEscape.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

bdUInt bdStringEscape::escape(const bdNChar8* src,
								bdNChar8* dest,
								bdUInt srcLen)
{
	// Record the initial value of dest so that we can test how long the 
	// final length of the string we put there is.
	bdNChar8* destStart;
	destStart = dest;

	// Loop through each character in 'src'
	bdUInt i;
	for(i=0; i<srcLen; i++)
	{
		bdNChar8 c = *(src+i);
		// If the character is outside the range of alphanumerics,
		// replace it with its hex code, prefixed with '%'.
		if(		(c < 0x2f)
			||	(c > 0x39 && c < 0x41) 
			||	(c > 0x5a && c < 0x61) 
			||	(c > 0x7a)	)
		{
			bdSnprintf(dest, 1 + BD_STR_ESC_CHAR_LENGTH + 1, "%%%02x", static_cast<bdUByte8>(c));
			// Increment 'dest' by the length of the encoded char.
			dest += BD_STR_ESC_CHAR_LENGTH;
		}
		else
		{
			// It's alphanumeric, so just copy it.
			*dest = c;
			dest += 1;
		}
	}
	// Terminate 'dest' with a null
	*dest = BD_NULL;
	// Return the length of the destination string.
	return static_cast<bdUInt>(dest - destStart);
}

bdUInt bdStringEscape::unEscape(const bdNChar8* src, 
								bdNChar8* dest,
								bdUInt srcLen)
{
	//Record the initial value of dest so that we can test how long the 
	// final length of the string we put there is.
	bdNChar8* destStart;
	destStart = dest;

	// Loop through each character in the escaped 'src' string.
	bdUInt i;
	for(i=0; i<srcLen;)
	{
		bdNChar8 c = *(src + i);

		// If the character is a '%' then the subsequent characters are
		// the hex value of the unescaped character.
		if(c == '%')
		{
			// Copy out the subsequent characters into a string.
			bdNChar8 escChar[BD_STR_ESC_CHAR_LENGTH];
			escChar[0] = *(src+i+1);
			escChar[1] = *(src+i+2);
			escChar[2] = BD_NULL;
			// Convert that string into a number
			bdUInt32 val;
			val = bdStrtoui32(escChar, BD_NULL, 16);
			// Assign the number to the current character in the 'dest' string.
			*dest = static_cast<bdNChar8>(val);
			i += BD_STR_ESC_CHAR_LENGTH;
		}
		else
		{
			// The character is not escaped, so just copy it.
			*dest = c;
			i += 1;
		}
		// Increment the current position in the 'dest' string
		dest += 1;
	}
	// Terminate 'dest' with null
	*dest = BD_NULL;
	// Return the length of the destination string.
	return static_cast<bdUInt>(dest - destStart);
}

