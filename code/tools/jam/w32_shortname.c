#ifdef OPT_BUILTIN_W32_SHORTNAME_EXT

#include "jam.h"
#include "w32_shortname.h"
#include "newstr.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

const char*
w32_shortname(char* path)
{
    const char *retval = path;
    if (path)
    {
	DWORD retlen;
	DWORD length;
	char *temp;

	temp = strdup(path);
	length = strlen(temp);
	retlen = GetShortPathName(temp, temp, length);
	if (retlen != 0 && retlen != length)
	    retval = newstr(temp);
	free(temp);
    }
    return retval;
}

#endif
