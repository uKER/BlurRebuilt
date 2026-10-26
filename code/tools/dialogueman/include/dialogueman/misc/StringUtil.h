#ifndef DMAN_STRING_UTIL_H
#define DMAN_STRING_UTIL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

static const wxCharBuffer ToAscii(const wxString& src)
{
	return src.mb_str(*wxConvCurrent);
}  

static wxString ToUnicode(const char* src)
{
	return wxString(src, *wxConvCurrent);
}

static wxString ToString(U32 src)
{
#ifdef UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	ss << src;
	return ss.str();
}

static wxString ToString(U64 src)
{
#ifdef UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	ss << src;
	return ss.str();
}

static wxString ToString(S32 src)
{
#ifdef UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	ss << src;
	return ss.str();
}

static wxString ToString(S64 src)
{
#ifdef UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	ss << src;
	return ss.str();
}

static wxString ToString(F32 src)
{
#ifdef UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	ss << src;
	return ss.str();
}

static wxString ToString(F64 src)
{
#ifdef UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	ss << src;
	return ss.str();
}

static U32 ToU32(const wxString& src)
{
#ifdef UNICODE
	std::wistringstream from(src.wc_str(*wxConvCurrent));
#else
	std::istringstream from(src.c_str());
#endif
	U32 to;
	from >> to;
	return to;
}

static U64 ToU64(const wxString& src)
{
#ifdef UNICODE
	std::wistringstream from(src.wc_str(*wxConvCurrent));
#else
	std::istringstream from(src.c_str());
#endif
	U64 to;
	from >> to;
	return to;
}

static S32 ToS32(const wxString& src)
{
#ifdef UNICODE
	std::wistringstream from(src.wc_str(*wxConvCurrent));
#else
	std::istringstream from(src.c_str());
#endif
	S32 to;
	from >> to;
	return to;
}

static S64 ToS64(const wxString& src)
{
#ifdef UNICODE
	std::wistringstream from(src.wc_str(*wxConvCurrent));
#else
	std::istringstream from(src.c_str());
#endif
	S64 to;
	from >> to;
	return to;
}

wxString BuildColumnKey(const wxString& typeName, const wxString& xtFieldName);

wxString XtFieldNameFromColumnKey(const wxString& columnKey);

wxString XtTypeNameFromColumnKey(const wxString& columnKey);

wxString	ColNumToStr(U32 colNum);
U32			ColStrToNum(const wxString& colStr);

static bool PathHasForbiddenChars(const wxString& path)
{
	const static wxString forbiddenChars = wxT("*?:\"<>|");

	for(size_t i = 0; i < forbiddenChars.Len(); i++)
	{
		wxChar fc = forbiddenChars.GetChar(i);
		if(path.Find(fc) != wxNOT_FOUND)
		{
			return true;
		}
	}
	return false;
}

Util::C_String RemoveUnexpandedMacros(const Util::C_String& str);

Util::C_String RemoveFoldersFromRecordName(const Util::C_String& str);

Util::C_String CurrentDialogueRecordName();

} //namespace DMan

#endif //DMAN_STRING_UTIL_H
