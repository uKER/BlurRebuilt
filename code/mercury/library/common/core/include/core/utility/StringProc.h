#ifndef MERCURY_STRING_PROC_H
#define MERCURY_STRING_PROC_H

#ifdef _MSC_VER
#pragma once
#endif

#include <string>
#include <sstream>
#include <algorithm>

namespace Tu
{
	inline void SearchAndReplace(
		std::string& theString, 
		const std::string& theSearchSubString,
		const std::string& theNewSubString)
	{
		std::string::size_type pos = 0;
		while ( (pos = theString.find(theSearchSubString, pos)) != std::string::npos )
		{
			theString.replace( pos, theSearchSubString.length(), theNewSubString);
			pos += theNewSubString.size();
		}
	}

	inline std::string ReplaceSection(
		const std::string& str,
		std::string::size_type start,
		std::string::size_type end, 
		const std::string& substring)
	{
		std::string newStr = str.substr(0, start);
		newStr += substring;
		newStr += str.substr(end, str.length()-end);
		return newStr;
	}

	inline void ToLower(
		std::string &string)
	{
//		std::transform(string.begin(), string.end(), string.begin(), std::tolower);
		size_t c = string.size();
		for (unsigned int i=0; i<c; i++)
			string.at(i) = tolower(string.at(i));
	}

	inline std::string DirFromString(
		const std::string& inString)
	{
		//Copy incoming string.
		std::string newInString(inString);
		//Change to unix style directory separators.
		SearchAndReplace(newInString, "\\", "/");
		std::string::size_type pos = 0;
		//Find the last directory separator.
		pos = newInString.rfind('/');
		if (pos == std::string::npos)
		{
			//No directory separator, just return the string.
			return inString;
		}
        
		//else we return the path substring.
		return newInString.substr(0, pos+1);
	}

	inline std::string FileNameFromString(
		const std::string& inString)
	{
		//Copy incoming string.
		std::string newInString(inString);
		//Change to unix style directory separators.
		SearchAndReplace(newInString, "\\", "/");
		std::string::size_type pos = 0;
		//Find the last directory separator.
		pos = newInString.rfind('/');
		if (pos == std::string::npos)
		{
			//No directory separator, just return the string.
			return inString;
		}
        
		//else we return the file name substring.
		return newInString.substr(pos+1, newInString.size() - (pos+1));
	}

	inline std::string FileExtension(const std::string& inString)
	{
		std::string::size_type pos = inString.rfind('.');
		if (pos == std::string::npos)
		{
			//No extension separator, just return the string.
			return inString;
		}
		return inString.substr(pos+1, inString.size());
	}

	inline std::string RemoveFileExtension(const std::string& inString)
	{
		std::string::size_type pos = inString.rfind('.');
		if (pos == std::string::npos)
		{
			//No extension separator, just return the string.
			return inString;
		}
		return inString.substr(0, pos);
	}

	inline std::string ReplaceFileExtension(const std::string& inString, const std::string& extension)
	{
		std::string::size_type pos = inString.rfind('.');
		if (pos == std::string::npos)
		{
			//No extension separator, just return the string.
			return inString;
		}
		std::string newString = inString.substr(0, pos+1);
		newString += extension;
		return newString;
	}

	inline std::string CreateNumberedFileName(const std::string inString, unsigned int number)
	{
		std::stringstream ss;
		ss << Tu::RemoveFileExtension(inString);
		ss << number;
		ss << ".";
		ss << Tu::FileExtension(inString);
		return ss.str();
	}

	// returns 0 if two strings are the same
	inline static int CompareNoCase(const std::string& s1, const std::string &s2, unsigned int maxlength=(unsigned int)-1)
	{
		std::string::const_iterator p1 = s1.begin();
		std::string::const_iterator p2 = s2.begin();
		unsigned int i=0;
		while (p1 != s1.end() && p2 != s2.end()) {
			if (tolower(*p1) != tolower(*p2)) {
			return (tolower(*p1) < tolower(*p2)) ? -1 : 1;
			}
			++p1;
		++p2;
			++i;
			if (i >= maxlength) {
			return 0;
			}
		}
		return (s2.size() == s1.size()) ? 0 : (s1.size() < s2.size()) ? -1 : 1; // size is unsigned
	}

 inline bool IsAlphabetic(const char c)
 {
   if (( c > 64 && c < 91) ||
       ( c > 96 && c < 123)) {
     return true;
   }
   return false;
 }


 inline bool IsNumeric(const char c)
 {
   if (( c > 47 && c < 58)) {
     return true;
   }
   return false;
 }
 
 
 inline bool IsWhitespace(const char c)
 {
   if ((( c >= 9 ) && ( c <= 13 )) ||
       (c == 32)) {
     return true;
   }
   return false;
 }
 
 
 inline bool IsPunctuation(const char c)
 {
   if (( c > 32 && c < 48) ||
       ( c > 57 && c < 65) ||
       ( c > 90 && c < 97) ||
       ( c > 122 && c < 127)) {
     return true;
   }
   return false;
 }
 
 
 inline bool IsAlphanumeric(const char c)
 {
  if ( IsAlphabetic(c) || IsNumeric(c) ) {
     return true;
   }
   return false;
 }
 
 
 inline bool IsVisible(const char c)
 {
   if ( IsAlphanumeric(c) || IsPunctuation(c) ) {
     return true;
   }
   return false;
 }
 
 
 inline bool IsPrintable(const char c)
 {
   if ( IsVisible(c) || IsWhitespace(c) ) {
     return false;
   }
   return true;
}

 inline void PtrToString(std::string& str, void* ptr)
 {
	std::stringstream ss;
	#pragma warning(push)
	#pragma warning(disable: 4311)
		#if WORD_SIZE == 64
			U64 integer = reinterpret_cast<U64>(ptr);
		#else
			U32 integer = reinterpret_cast<U32>(ptr);
		#endif
	#pragma warning(pop)
	ss << integer;
	ss >> str;
 }

inline void* StringToPtr(const std::string& str)
{
	std::stringstream ss;
	#if WORD_SIZE == 64
		U64 integer;
	#else
		U32 integer;
	#endif
	ss << str;
	ss >> integer;

	unsigned char* ptr = 0;
	ptr+=integer;
	return ptr;
}

}

#endif
