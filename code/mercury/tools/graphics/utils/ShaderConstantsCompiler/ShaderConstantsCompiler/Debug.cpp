#include "StdAfx.h"
#include "Debug.h"

void DBError(const char* text, ...)
{
	static char buffer[512];
	static char buffer1[512];

	va_list arglist;
	va_start(arglist, text);
	vsprintf_s(buffer, 512, text, arglist);
	va_end(arglist);

	sprintf_s(buffer1, 512, "ERROR: %s", buffer);
	
	std::cerr << buffer1;
	OutputDebugString(buffer1);

	exit(1);
}
