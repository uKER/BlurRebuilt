#ifndef __WIDESTRINGREFORMAT_H__
#define __WIDESTRINGREFORMAT_H__

namespace UI
{
#ifdef PLATFORM_PS3
const WCHAR* Ps3_Reformat( WCHAR* formatted_string, int formatted_string_length, const WCHAR* str );
#endif

}

#endif
