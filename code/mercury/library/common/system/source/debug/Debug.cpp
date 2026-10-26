#include "SystemPch.h"
#include <Core/Core.h>
#include <System/Debug/Debug.h>

#ifdef COMPILER_VC

void DebugLn(const char* text, ...)
{
#ifndef CFG_GOLD
	const int kMaxChars = 512;

	char buffer[kMaxChars + 3];
	va_list vl;

	va_start(vl, text);
	int written = _vsnprintf(buffer, kMaxChars, text, vl);
	va_end(vl);

	if (written < 0)
		written = 512;

	buffer[written + 0] = '\r';
	buffer[written + 1] = '\n';
	buffer[written + 2] = '\0';

	OutputDebugString(buffer);

	buffer[written] = '\0';

	Debug::DebugLogLn("", Debug::kSeverityNormal, buffer);
#endif
}

#else

void DebugLn(const char* text, ...)
{
#ifndef CFG_GOLD
	va_list vl;

	va_start(vl, text);
	vprintf(text, vl);
	va_end(vl);
	
	printf("\r\n");
#endif
}

#endif
