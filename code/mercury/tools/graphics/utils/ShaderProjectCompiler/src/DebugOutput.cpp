#include "StdAfx.h"
#include "DebugOutput.h"

void DBPrint(const char* pString, ...)
{
    static char buffer[2048];
   
	va_list arglist;
	va_start(arglist, pString);
	vsprintf_s(buffer, sizeof(buffer), pString, arglist);
	va_end(arglist);

    std::cout << buffer;
	OutputDebugString(buffer);
}

void DBError(const char* pString, ...)
{
    static char buffer[2048];
   
	va_list arglist;
	va_start(arglist, pString);
	vsprintf_s(buffer, sizeof(buffer), pString, arglist);
	va_end(arglist);

	static char buffer1[512];

	sprintf_s(buffer1, sizeof(buffer1), "ERROR: %s", buffer);
    std::cerr << buffer1;
}

Debug::Logger& g_Log = Debug::GetLogger("shaderprojectcompiler");
