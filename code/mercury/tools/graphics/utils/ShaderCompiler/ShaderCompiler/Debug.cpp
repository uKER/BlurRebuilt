#include "StdAfx.h"
#include "Debug.h"

void DBPrint(const char* text, ...)
{
	static char temp1[512];
	static char temp2[512];

	va_list arglist;
	va_start(arglist, text);
	vsprintf_s(temp1, 512, text, arglist);
	va_end(arglist);

	sprintf_s(temp2, 512, "BSC: %s", temp1);
	OutputDebugString(temp2);
	
	std::cout << temp2;
}

void DBError(const char* text, ...)
{
	static char temp1[512];
	static char temp2[512];

	va_list arglist;
	va_start(arglist, text);
	vsprintf_s(temp1, 512, text, arglist);
	va_end(arglist);

	sprintf_s(temp2, 512, "BSC: Error - %s", temp1);
	OutputDebugString(temp2);
	
	std::cerr << temp2;
}
