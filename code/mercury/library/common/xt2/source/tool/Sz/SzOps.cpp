#include "Common/XtPch.h"
#include <Xt/XtTool.h>

template<typename Type> inline Type Max(const Type& lhs, const Type& rhs)
{
	return (rhs > lhs) ? rhs : lhs;
}

template<typename Type> inline Type Min(const Type& lhs, const Type& rhs)
{
	return (lhs > rhs) ? rhs : lhs;
}

//
// SplitPath
//

template<typename Char>
bool SplitPath(const Char* fullPath, Sz<Char>& path, Sz<Char>& file, Sz<Char>& ext)
{
	const Char* ptr		= fullPath;
	const Char* ptrPath	= ptr;
	const Char* ptrFile	= ptr;
	const Char* ptrExt	= ptr;

	for(; *ptr != '\0'; ptr++)
	{
		if ((*ptr == '\\') || (*ptr == '/'))
			ptrFile	= ptr + 1;
		else if (*ptr == '.')
			ptrExt = ptr;
	}

	if (ptrExt <= ptrFile)
		ptrExt = ptr;

	return path.Set(ptrPath, ptrFile - ptrPath) && file.Set(ptrFile, ptrExt - ptrFile) && ext.Set(ptrExt, ptr - ptrExt);
}

template bool SplitPath(const char* fullPath, Sz<char>& path, Sz<char>& file, Sz<char>& ext);
template bool SplitPath(const wchar_t* fullPath, Sz<wchar_t>& path, Sz<wchar_t>& file, Sz<wchar_t>& ext);

//
// SplitFileExt
//

template<typename Char>
Sz<Char> SplitFileExt(const Char* fullPath)
{
	const Char* ptr		= fullPath;
	const Char* ptrFile	= ptr;

	for(; *ptr != '\0'; ptr++)
	{
		if ((*ptr == '\\') || (*ptr == '/'))
			ptrFile	= ptr + 1;
	}

	return Sz<Char>(ptrFile, ptr - ptrFile);
}

template Sz<char> SplitFileExt(const char* fullPath);
template Sz<wchar_t> SplitFileExt(const wchar_t* fullPath);

//
// CleanUpFilename
//

template<typename Char>
bool CleanUpFilename(Sz<Char>& filename, bool useForwardSlashes)
{
	size_t length = filename.Length();

	Char* base;

	if ((base = filename.Capture(length, true)) == 0)
		return false;

	const Char* src = base;
	const Char* end = base + length;

	while((src < end) && ((*src == ' ') || (*src == '\t')))
		src++;

	Char* dst = base;

	if (useForwardSlashes)
	{
		for(; src < end; src++)
			*dst++ = (*src == '\\') ? '/' : *src;
	}
	else
	{
		for(; src < end; src++)
			*dst++ = (*src == '/') ? '\\' : *src;
	}

	while((dst > base) && ((dst[-1] == ' ') || (dst[-1] == '\t')))
		dst--;

	filename.Truncate(dst - base);

	return true;
}

template bool CleanUpFilename(Sz<char>& filename, bool useForwardSlashes);
template bool CleanUpFilename(Sz<wchar_t>& filename, bool useForwardSlashes);

//
// CleanUpPath
//

template<typename Char>
bool CleanUpPath(Sz<Char>& path, bool useForwardSlashes, bool addTrailingSlash)
{
	if (!*path)
		return true;

	const Char* ptr			= path;
	size_t		newLength	= 0;

	for(; (*ptr == '\\') || (*ptr == '/'); ptr++)
		newLength++;

	bool hasNetPath = false;

	if (newLength >= 2)
	{
		newLength	= 2;
		hasNetPath	= true;
	}
	else
		newLength = 0;

	while(*ptr != '\0')
	{
		for(; (*ptr != '\\') && (*ptr != '/') && (*ptr != '\0'); ptr++)
			newLength++;

		while((*ptr == '\\') || (*ptr == '/'))
			ptr++;

		if (*ptr == '\0')
		{
			if (addTrailingSlash)
				newLength++;

			break;
		}

		newLength++;
	}

	Char*		dst = path.Capture(Max(path.Length(), newLength), true);
	const Char*	src = dst;

	if (!dst)
		return false;

	Char slash = useForwardSlashes ? '/' : '\\';

	while((*src == '\\') || (*src == '/'))
		src++;

	if (hasNetPath)
	{
		*dst++ = slash;
		*dst++ = slash;
	}

	while(*src != '\0')
	{
		while((*src != '\\') && (*src != '/') && (*src != '\0'))
			*dst++ = *src++;

		while((*src == '\\') || (*src == '/'))
			src++;

		if (*src == '\0')
			break;

		*dst++ = slash;
	}

	if (addTrailingSlash)
		*dst++ = slash;

	path.Truncate(newLength);

	return true;
}

template bool CleanUpPath(Sz<char>& path, bool useForwardSlashes, bool addTrailingSlash);
template bool CleanUpPath(Sz<wchar_t>& path, bool useForwardSlashes, bool addTrailingSlash);

//
// SplitIntercaps
//

template<typename Char>
Sz<Char> SplitIntercaps(const Sz<Char>& text)
{
	size_t length = 0;

	if (*text)
	{
		length++;

		for(const Char* s = (const Char*)text + 1; *s != '\0'; s++)
		{
			if ((s[-1] != ' ') && (s[0] != ' ') && (s[-1] != '_') && (s[0] != '_'))
			{
				if (iswlower(s[-1]) && iswupper(s[0]))
					length++;
				else if ((iswalpha(s[-1]) != 0) != (iswalpha(s[0]) != 0))
					length++;
				else if (iswupper(s[-1]) && iswupper(s[0]) && iswlower(s[1]))
					length++;
			}

			length++;
		}
	}

	Sz<Char> label;

	if (length > 0)
	{
		const Char*	s = text;

		if (Char* d = label.Capture(length, false))
		{
			*d++ = toupper(*s++);

			for(bool upper = false; *s != '\0'; s++)
			{
				if ((s[-1] != ' ') && (s[0] != ' ') && (s[-1] != '_') && (s[0] != '_'))
				{
					if (iswlower(s[-1]) && iswupper(s[0]))
						*d++ = ' ';
					else if ((iswalpha(s[-1]) != 0) != (iswalpha(s[0]) != 0))
						*d++ = ' ';
					else if (iswupper(s[-1]) && iswupper(s[0]) && iswlower(s[1]))
						*d++ = ' ';
				}

				if ((*s == ' ') || (*s == '_'))
				{
					*d++ = ' ';
					upper = true;
				}
				else if (upper)
				{
					*d++ = toupper(*s);
					upper = false;
				}
				else
					*d++ = *s;
			}
		}
	}

	return label;
}

template Sz<char> SplitIntercaps(const Sz<char>& text);
template Sz<wchar_t> SplitIntercaps(const Sz<wchar_t>& text);

//
// CleanUpLabel
//

template<typename Char>
Sz<Char> CleanUpLabel(const Sz<Char>& text)
{
	Sz<Char>	label	= SplitIntercaps(text);
	size_t		length	= label.Length();

	if (length > 0)
	{
		bool isAllCaps = true;

		for(const Char* p = label; *p != '\0'; p++)
		{
			if (iswalpha(*p) && iswlower(*p))
				isAllCaps = false;
		}

		if (isAllCaps)
		{
			if (Char* p = label.Capture(length, true))
			{
				for(p++; *p != '\0'; p++)
				{
					if (iswalnum(p[-1]) && iswalpha(*p))
						p[0] = tolower(p[0]);
				}
			}
		}
	}

	return label;
}

template Sz<char> CleanUpLabel(const Sz<char>& text);
template Sz<wchar_t> CleanUpLabel(const Sz<wchar_t>& text);

//
// Format
//

namespace
{
	int DoFormatCount(const char* format, va_list vl)
		{ return _vscprintf(format, vl); }

	int DoFormatCount(const wchar_t* format, va_list vl)
		{ return _vscwprintf(format, vl); }

	int DoFormatPrint(char* dst, size_t capacity, const char* format, va_list vl)
		{ return _vsnprintf_s(dst, capacity + 1, _TRUNCATE, format, vl); }

	int DoFormatPrint(wchar_t* dst, size_t capacity, const wchar_t* format, va_list vl)
		{ return _vsnwprintf_s(dst, capacity + 1, _TRUNCATE, format, vl); }
}

template<typename Char>
Sz<Char> FormatV(const Char* format, va_list vl)
{
	int count = DoFormatCount(format, vl);

	if (count < 0)
		return SzError<Char>(kSzInvalidParameter);

	Sz<Char> out;

	Char* d;

	if ((d = out.Capture(count, false)) == 0)
		return SzError<Char>(kSzOutOfMemory);

	int result = DoFormatPrint(d, count, format, vl);

	if (result < 0)
		return SzError<Char>(kSzInvalidParameter);

	out.Truncate(result);

	return out;
}

template<typename Char>
Sz<Char> Format(const Char* format, ...)
{
	va_list vl;

	va_start(vl, format);

	const Sz<Char>& out = FormatV(format, vl);

	va_end(vl);

	return out;
}

template Sz<char> FormatV(const char* format, va_list vl);
template Sz<wchar_t> FormatV(const wchar_t* format, va_list vl);

template Sz<char> Format(const char* format, ...);
template Sz<wchar_t> Format(const wchar_t* format, ...);

//
// Fill
//

template<typename Char>
Sz<Char> Fill(Char character, size_t count)
{
	Sz<Char> out;

	if (count > 0)
	{
		if (Char* d = out.Capture(count, false))
		{
			for(size_t i = 0; i < count; i++)
				d[i] = character;
		}
	}

	return out;
}

template<typename Char>
void Fill(Char* d, const Char* pattern, size_t count)
{
	const Char* s = pattern;

	for(size_t i = 0; i < count; i++)
	{
		d[i] = *s++;

		if (*s == '\0')
			s = pattern;
	}
}

template<typename Char>
Sz<Char> Fill(const Char* pattern, size_t count)
{
	Sz<Char> out;

	if ((count > 0) && (*pattern != '\0'))
	{
		if (Char* d = out.Capture(count, false))
			Fill(d, pattern, count);
	}

	return out;
}

template Sz<char> Fill(char character, size_t count);
template Sz<wchar_t> Fill(wchar_t character, size_t count);

template Sz<char> Fill(const char* pattern, size_t count);
template Sz<wchar_t> Fill(const wchar_t* pattern, size_t count);

//
// PadStart
//

template<typename Char>
Sz<Char> PadStart(const Sz<Char>& sz, Char character, size_t count)
{
	if (count <= 0)
		return sz;

	Sz<Char> out;

	size_t szLength		= sz.Length();
	size_t totalLength	= szLength + count;

	if (totalLength > 0)
	{
		if (Char* d = out.Capture(totalLength, false))
		{
			for(size_t i = 0; i < count; i++)
				d[i] = character;

			SzMemCpy<Char>(d + count, sz, szLength);
		}
	}

	return out;
}

template<typename Char>
Sz<Char> PadStart(const Sz<Char>& sz, const Char* pattern, size_t count)
{
	if (count <= 0)
		return sz;

	Sz<Char> out;

	size_t szLength		= sz.Length();
	size_t totalLength	= szLength + count;

	if (totalLength > 0)
	{
		if (Char* d = out.Capture(totalLength, false))
		{
			Fill(d, pattern, count);

			SzMemCpy<Char>(d + count, sz, szLength);
		}
	}

	return out;
}

template Sz<char> PadStart(const Sz<char>& sz, char character, size_t count);
template Sz<wchar_t> PadStart(const Sz<wchar_t>& sz, wchar_t character, size_t count);

template Sz<char> PadStart(const Sz<char>& sz, const char* pattern, size_t count);
template Sz<wchar_t> PadStart(const Sz<wchar_t>& sz, const wchar_t* pattern, size_t count);

//
// PadEnd
//

template<typename Char>
Sz<Char> PadEnd(const Sz<Char>& sz, Char character, size_t count)
{
	if (count <= 0)
		return sz;

	Sz<Char> out;

	size_t szLength		= sz.Length();
	size_t totalLength	= szLength + count;

	if (totalLength > 0)
	{
		if (Char* d = out.Capture(totalLength, false))
		{
			SzMemCpy<Char>(d, sz, szLength);

			for(size_t i = szLength; i < totalLength; i++)
				d[i] = character;
		}
	}

	return out;
}

template<typename Char>
Sz<Char> PadEnd(const Sz<Char>& sz, const Char* pattern, size_t count)
{
	if (count <= 0)
		return sz;

	Sz<Char> out;

	size_t szLength		= sz.Length();
	size_t totalLength	= szLength + count;

	if (totalLength > 0)
	{
		if (Char* d = out.Capture(totalLength, false))
		{
			SzMemCpy<Char>(d, sz, szLength);

			Fill(d + szLength, pattern, count);
		}
	}

	return out;
}

template Sz<char> PadEnd(const Sz<char>& sz, char character, size_t count);
template Sz<wchar_t> PadEnd(const Sz<wchar_t>& sz, wchar_t character, size_t count);

template Sz<char> PadEnd(const Sz<char>& sz, const char* pattern, size_t count);
template Sz<wchar_t> PadEnd(const Sz<wchar_t>& sz, const wchar_t* pattern, size_t count);


//
// Substring
//

template<typename Char>
Sz<Char> Substring(const Sz<Char>& sz, size_t i, size_t j)
{
	size_t szLength = sz.Length();

	if (i > j || (i > szLength) || (j > szLength))
		return SzError<Char>(kSzInvalidParameter);

	Sz<Char> out;

	size_t subLength = j - i;

	if (subLength > 0)
	{
		Char* d;

		if ((d = out.Capture(subLength, false)) == 0)
			return SzError<Char>(kSzOutOfMemory);

		SzMemCpy<Char>(d, sz + i, subLength);
	}

	return out;
}

template<typename Char>
Sz<Char> Substring(const Sz<Char>& sz, size_t i)
{
	return Substring(sz, i, sz.Length());
}

template Sz<char> Substring(const Sz<char>& sz, size_t i, size_t j);
template Sz<wchar_t> Substring(const Sz<wchar_t>& sz, size_t i, size_t j);

template Sz<char> Substring(const Sz<char>& sz, size_t i);
template Sz<wchar_t> Substring(const Sz<wchar_t>& sz, size_t i);

//
// Replace
//

template<typename Char>
Sz<Char> Replace(const Sz<Char>&sz, Char from, Char to)
{
	if (from == to)
		return sz;

	Sz<Char> result;

	size_t length = sz.Length();

	Char* d;

	if ((d = result.Capture(length, false)) == 0)
		return SzError<Char>(kSzOutOfMemory);

	const Char* s = sz;

	for(size_t i = 0; i < length; i++, s++)
	{
		if (*s == from)
			*d++ = to;
		else
			*d++ = *s;
	}

	return result;
}

template Sz<char> Replace(const Sz<char>&sz, char from, char to);
template Sz<wchar_t> Replace(const Sz<wchar_t>&sz, wchar_t from, wchar_t to);

//
// IndexOf
//

template<typename Char>
int IndexOf(const Sz<Char>& sz, const Char* pattern)
{
	const Char*	text	= sz;
	size_t		length	= sz.Length();

	for(size_t i = 0; i < length; i++)
	{
		Char c = text[i];

		for(const Char* p = pattern; *p != '\0'; p++)
		{
			if (c == *p)
				return i;
		}
	}

	return -1;
}

template int IndexOf(const Sz<char>& sz, const char* pattern);
template int IndexOf(const Sz<wchar_t>& sz, const wchar_t* pattern);

//
// LastIndexOf
//

template<typename Char>
int LastIndexOf(const Sz<Char>& sz, const Char* pattern)
{
	const Char*	text	= sz;
	size_t		length	= sz.Length();

	for(size_t i = length; i > 0; )
	{
		Char c = text[--i];

		for(const Char* p = pattern; *p != '\0'; p++)
		{
			if (c == *p)
				return i;
		}
	}

	return -1;
}

template int LastIndexOf(const Sz<char>& sz, const char* pattern);
template int LastIndexOf(const Sz<wchar_t>& sz, const wchar_t* pattern);

//
// Trim
//

/*template<typename Char>
Sz<Char> TrimStart(const Sz<Char>& sz, Char character)
{
	Sz<Char> out;


	
}*/

//
// WcToMb
//

Sz8 WcToMb(const Sz16& sz)
{
	size_t length = sz.Length();

	int newLength = WideCharToMultiByte(CP_UTF8, 0, sz, length, 0, 0, 0, 0);

	if (newLength <= 0)
		return SzError<char>(kSzInvalidParameter);

	Sz8 out;
	char* d;

	if ((d = out.Capture(newLength, false)) == 0)
		return SzError<char>(kSzOutOfMemory);

	if (WideCharToMultiByte(CP_UTF8, 0, sz, sz.Length(), d, newLength, 0, 0) <= 0)
		return SzError<char>(kSzInvalidParameter);

	return out;
}

//
// MbToWc
//

Sz16 MbToWc(const Sz8& sz)
{
	size_t length = sz.Length();

	int newLength = MultiByteToWideChar(CP_UTF8, 0, sz, length, 0, 0);

	if (newLength <= 0)
		return SzError<wchar_t>(kSzInvalidParameter);

	Sz16 out;
	wchar_t* d;

	if ((d = out.Capture(newLength, false)) == 0)
		return SzError<wchar_t>(kSzOutOfMemory);

	if (MultiByteToWideChar(CP_UTF8, 0, sz, sz.Length(), d, newLength) <= 0)
		return SzError<wchar_t>(kSzInvalidParameter);

	return out;
}

//
// IndexOfStringI
//

template<typename Char>
int IndexOfStringI(const Sz<Char>& sz, const Char* substring)
{
	const Char*	text			= sz;
	int			subLength		= SzLength(substring);
	int			lengthToCheck	= sz.Length() - subLength;

	if (subLength <= 0)
		return -1;

	Char first = tolower(*substring);

	for(int i = 0; i <= lengthToCheck; i++, text++)
	{
		if (tolower(*text) != first)
			continue;

		const Char* s = text + 1;
		const Char* d = substring + 1;

		for(int j = subLength - 1; j > 0; j--, s++, d++)
		{
			if (tolower(*s) != tolower(*d))
				goto skipToNextChar;
		}

		return i;

skipToNextChar:;
	}

	return -1;
}

template int IndexOfStringI(const Sz<char>& sz, const char* substring);
template int IndexOfStringI(const Sz<wchar_t>& sz, const wchar_t* substring);

//
// StartsWithI
//

template<typename Char>
bool StartsWithI(const Sz<Char>& sz, const Char* substring)
{
	const Char*	text	= sz;
	size_t		length	= sz.Length();

	for(size_t i = 0; (i < length) && (*substring != '\0'); i++, text++, substring++)
	{
		if (tolower(*text) != tolower(*substring))
			return false;
	}

	return true;
}

template bool StartsWithI(const Sz<char>& sz, const char* substring);
template bool StartsWithI(const Sz<wchar_t>& sz, const wchar_t* substring);

//
// EndsWithI
//

template<typename Char>
bool EndsWithI(const Sz<Char>& sz, const Char* substring)
{
	const Char*	text		= sz;
	size_t		length		= sz.Length();
	size_t		subLength	= SzLength(substring);

	if (subLength > length)
		return false;

	text += length - subLength;

	for(size_t i = 0; i < subLength; i++, text++, substring++)
	{
		if (tolower(*text) != tolower(*substring))
			return false;
	}

	return true;
}

template bool EndsWithI(const Sz<char>& sz, const char* substring);
template bool EndsWithI(const Sz<wchar_t>& sz, const wchar_t* substring);