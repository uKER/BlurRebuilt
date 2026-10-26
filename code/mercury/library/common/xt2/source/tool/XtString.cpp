#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace
{
	template<typename Type> inline Type Min(const Type& lhs, const Type& rhs)
	{
		return (lhs > rhs) ? rhs : lhs;
	}

	template<typename Type> inline Type Max(const Type& lhs, const Type& rhs)
	{
		return (lhs > rhs) ? lhs : rhs;
	}
}

//
// kXtNullString
//

const Sz8	kXtNullString;
const Sz16	kXtNullStringW;

//
// XtString encoding
//

XtString XtsEncodeString(const char* text)
{
	int length = 2;

	for(const char* p = text; *p != '\0'; p++)
	{
		if ((*p == '\'') || (*p == '\"') || (*p == '\n') || (*p == '\r') || (*p == '\t'))
			length++;

		length++;
	}

	XtString newString;

	if (char* newText = newString.Capture(length, false))
	{
		*newText++ = '\"';

		for(const char* p = text; *p != '\0'; p++)
		{
			switch(*p)
			{
				case '\'': *newText++ = '\''; *newText++ = '\''; break;
				case '\"': *newText++ = '\''; *newText++ = '\"'; break;
				case '\n': *newText++ = '\''; *newText++ = 'n'; break;
				case '\r': *newText++ = '\''; *newText++ = 'r'; break;
				case '\t': *newText++ = '\''; *newText++ = 't'; break;
				default: *newText++ = *p; break;
			}
		}

		*newText++	= '\"';
		*newText	= '\0';

		newString.Truncate(length);
	}

	return newString;
}

XtString XtsDecodeString(const char* text, int length)
{
	int		newLength	= 0;
	bool	inEscape	= false;

	for(int i = 0; i < length; i++)
	{
		char c = text[i];

		if (inEscape)
			inEscape = false;
		else if (c == '\"')
			continue;
		else if (c == '\'')
		{
			inEscape = true;
			continue;
		}

		newLength++;
	}

	XtString newString;

	if (char* newText = newString.Capture(newLength, false))
	{
		inEscape = false;

		for(int i = 0; i < length; i++)
		{
			char c = text[i];

			bool wasInEscape = inEscape;

			if (inEscape)
				inEscape = false;
			else if (c == '\"')
				continue;
			else if (c == '\'')
			{
				inEscape = true;
				continue;
			}

			if (wasInEscape)
			{
				switch(c)
				{
					case 'n':	*newText++ = '\n';	break;
					case 'r':	*newText++ = '\r';	break;
					case 't':	*newText++ = '\t';	break;
					default:	*newText++ = c;		break;
				}
			}
			else
				*newText++ = c;
		}

		*newText = '\0';

		newString.Truncate(newLength);
	}

	return newString;
}

//
// XtString miscellaneous
//

bool XtsIsWhitespace(char c)
{
	return (c == ' ') || (c == '\t');
}

bool XtsIsIdentifierStart(char c)
{
	return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_'));
}

bool XtsIsIdentifierBody(char c)
{
	return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_'));
}

bool XtsIsNameBody(char c)
{
	return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_') || (c == '-'));
}

bool XtsIsIdentifier(const char* text)
{
	if (!text || !*text)
		return false;

	if (!XtsIsIdentifierStart(*text))
		return false;

	for(text++; *text != '\0'; text++)
		if (!XtsIsIdentifierBody(*text))
			return false;

	return true;
}

bool XtsIsName(const char* text)
{
	if (!text || !*text)
		return false;

	if (!XtsIsNameBody(*text))
		return false;

	bool wasSpace = false;

	for(text++; *text != '\0'; text++)
	{
		if (*text == ' ')
		{
			if (wasSpace)
				return false;

			wasSpace = true;
		}
		else
		{
			if (!XtsIsNameBody(*text))
				return false;

			wasSpace = false;
		}
	}

	return !wasSpace;
}

bool XtsSplit(const char* text, char c, XtString& before, XtString& after)
{
	const char* search = "";
	const char* ptr = text;

	for(; *ptr != '\0'; ptr++)
	{
		if (*ptr == c)
		{
			search = ptr + 1;
			break;
		}
	}

	return (before.Set(text, (int)(ptr - text)) && after.Set(search));
}

XtString XtsStripHead(const char* text, char c)
{
	if (c == '\0')
		return kXtNullString;

	while(*text == c)
		text++;

	return XtString(text);
}

XtString XtsStripTail(const char* text, char c)
{
	if (c == '\0')
		return kXtNullString;

	const char* ptr = text;

	while(*ptr != '\0')
		ptr++;

	while((ptr > text) && (*(ptr - 1) == c))
		ptr--;

	return XtString(text, ptr - text);
}

XtString XtsStripBoth(const char* text, char c)
{
	if (c == '\0')
		return kXtNullString;

	while(*text == c)
		text++;

	const char* ptr = text;

	while(*ptr != '\0')
		ptr++;

	while((ptr > text) && (*(ptr - 1) == c))
		ptr--;

	return XtString(text, ptr - text);
}

bool XtsSplitFileName(const char* fullFilename, XtString& path, XtString& file, XtString& extension)
{
	const char* ptr		= fullFilename;
	const char* ptrPath	= ptr;
	const char* ptrFile	= ptr;
	const char* ptrExt	= 0;

	for(; *ptr != '\0'; ptr++)
	{
		if ((*ptr == '\\') || (*ptr == '/'))
		{
			ptrFile	= ptr + 1;
			ptrExt	= 0;
		}
		else if ((ptrExt == 0) && (*ptr == '.'))
			ptrExt = ptr;
	}

	if (!ptrExt)
		ptrExt = ptr;

	return (
			path.Set(ptrPath, (int)(XtIntPtr)(ptrFile - ptrPath)) &&
			file.Set(ptrFile, (int)(XtIntPtr)(ptrExt - ptrFile)) &&
			extension.Set(ptrExt, (int)(XtIntPtr)(ptr - ptrExt))
		);
}

bool XtsSplitObjectFileName(const char* fullFilename, XtString& path, XtString& file, XtString& type)
{
	const char* ptr		= fullFilename;
	const char* ptrPath	= ptr;
	const char* ptrFile	= ptr;
	const char* ptrType	= 0;
	const char* ptrXtt	= 0;

	for(; *ptr != '\0'; ptr++)
	{
		if ((*ptr == '\\') || (*ptr == '/'))
		{
			ptrFile	= ptr + 1;
			ptrType	= 0;
			ptrXtt	= 0;
		}
		else if ((ptrType == 0) && (*ptr == '.'))
			ptrType = ptr;
		else if ((ptrXtt == 0) && (*ptr == '.'))
			ptrXtt = ptr;
	}

	if (!ptrType || !ptrXtt || ((ptrType + 1) == ptrXtt))
		return false;

	if (_stricmp(ptrXtt, ".xtt") != 0)
		return false;

	return (
			path.Set(ptrPath, (int)(XtIntPtr)(ptrFile - ptrPath)) &&
			file.Set(ptrFile, (int)(XtIntPtr)(ptrType - ptrFile)) &&
			type.Set(ptrType + 1, (int)(XtIntPtr)(ptrXtt - ptrType - 1))
		);
}

bool XtsSplitObjectName(const char* fullFilename, XtString& path, XtString& type)
{
	const char* ptr		= fullFilename;
	const char* ptrPath	= ptr;
	const char* ptrType	= 0;

	for(; *ptr != '\0'; ptr++)
	{
		if ((*ptr == '\\') || (*ptr == '/'))
		{
			ptrType	= 0;
		}
		else if ((ptrType == 0) && (*ptr == '.'))
			ptrType = ptr;
	}

	const char* ptrPathEnd;

	if (!ptrType)
	{
		ptrPathEnd	= ptr;
		ptrType		= ptr;
	}
	else
	{
		ptrPathEnd	= ptrType;
		ptrType		= ptrType + 1;
	}

	return (
			path.Set(ptrPath, (int)(XtIntPtr)(ptrPathEnd - ptrPath)) &&
			type.Set(ptrType, (int)(XtIntPtr)(ptrType - ptr))
		);
}

bool XtsAppend(XtString& text, const char* toAppend, int length)
{
	int currentLength = text.Length();

	char* ptr = text.Capture(currentLength + length, true);

	if (!ptr)
		return false;

	memcpy(ptr + currentLength, toAppend, length);

	text.Truncate(currentLength + length);

	return true;
}

bool XtsIsValidFileName(const XtString& fileName)
{
	bool validFile = true;

	if (!*fileName)
		validFile = false;

	if ((*fileName == '\\') || (*fileName == '/')  || (*fileName == ' '))
		validFile = false;

	for(const char* ptr = fileName; *ptr; ptr++)
	{
		if (*ptr == ':')
		{
			validFile = false;
			break;
		}

		if (((*ptr == '\\') || (*ptr == '/')) && ((ptr[1] == '\\') || (ptr[1] == '/') || (ptr[1] == '\0') || (ptr[1] == ' ')))
		{
			validFile = false;
			break;
		}

		if ((*ptr == ' ') && ((ptr[1] == '\\') || (ptr[1] == '/') || (ptr[1] == '\0')))
		{
			validFile = false;
			break;
		}
	}

	return validFile;
}

int XtsDecodeHexDigit(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	else if ((c >= 'A') && (c <= 'F'))
		return 10 + c - 'A';

	return 0;
}

Sz16 XtsDecodeUnicodeText(XtString string)
{
	int length = string.Length() / 4;

	wchar_t* text = new wchar_t[length + 1];

	if (text)
	{
		const char* p = string;

		for(int i = 0; i < length; i++)
		{
			text[i] = (
					(XtsDecodeHexDigit(p[(i * 4) + 0]) << 12) |
					(XtsDecodeHexDigit(p[(i * 4) + 1]) << 8) |
					(XtsDecodeHexDigit(p[(i * 4) + 2]) << 4) |
					(XtsDecodeHexDigit(p[(i * 4) + 3]) << 0)
				);
		}

		text[length] = L'\0';
	}

	Sz16 result = text ? text : L"";

	delete text;

	return result;
}