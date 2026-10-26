// crt_compat.cpp — symbols that the VS2005-era vendor libraries (Demonware's
// OpenSSL/curl builds) import from the old MSVCRT but the UCRT no longer exports.
// legacy_stdio_definitions.lib covers printf/scanf; __iob_func needs a body.
#include <stdio.h>

extern "C" FILE* __cdecl __iob_func(void)
{
	// The old CRT exposed stdin/stdout/stderr as an array of FILE objects; the
	// UCRT FILE is a one-pointer placeholder, so copies of the three stay valid.
	static FILE iob[3];
	static bool init = false;
	if (!init)
	{
		iob[0] = *stdin;
		iob[1] = *stdout;
		iob[2] = *stderr;
		init = true;
	}
	return iob;
}
