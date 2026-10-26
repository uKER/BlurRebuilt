#ifndef SAFE_STRING_H
#define SAFE_STRING_H

#ifdef _MSC_VER
#pragma once
#endif

#if PLATFORM_PS3
#include "ui/util/widestringreformat.h"
#endif

namespace Amax
{

class C_SafeString
{
public:

	static inline int WideStringPrintF(WCHAR *str, int length, const WCHAR *format, ...)
	{
#if (defined PLATFORM_PS3) 
// PS3
		WCHAR formatted_string[512];
		va_list args;
		int return_value;
		va_start(args,format);
		return_value=vswprintf(str,length,UI::Ps3_Reformat(formatted_string,lengthof(formatted_string),format),args);
		va_end(args);
		return return_value;
#elif (defined PLATFORM_PC)
// PC
		va_list args;
		int return_value;
		va_start(args,format);
		return_value=vswprintf_s(str,length,format,args);
		va_end(args);
		return return_value;
#else
// 360 and everything else
		va_list args;
		int return_value;
		va_start(args,format);
		return_value=vswprintf_s(str,length,format,args);
		va_end(args);
		return return_value;
#endif
	};
	static inline int StringPrintF(char *str, int length, const char *format, ...)
	{
#if (defined PLATFORM_PS3) 
// PS3
		va_list args;
		int return_value;
		va_start(args,format);
		return_value=vsprintf(str,format,args);
		va_end(args);
		return return_value;
#elif (defined PLATFORM_PC)
// PC
		va_list args;
		int return_value;
		va_start(args,format);
		return_value=vsprintf_s(str,length,format,args);
		va_end(args);
		return return_value;
#else
// 360 and everything else
		va_list args;
		int return_value;
		va_start(args,format);
		return_value=vsprintf_s(str,length,format,args);
		va_end(args);
		return return_value;
#endif
	};
	static inline char *StringCopy(char *dst, int length, const char *src)
	{
#if (defined PLATFORM_PS3) 
// PS3
		strcpy(dst,src);
		return dst;
#elif (defined PLATFORM_PC)
// PC
		strcpy_s(dst,length,src);
		return dst;
#else
// 360 and everything else
		strcpy_s(dst,length,src);
		return dst;
#endif
	};
};

} // namespace Amax

#endif // SAFE_STRING_H
