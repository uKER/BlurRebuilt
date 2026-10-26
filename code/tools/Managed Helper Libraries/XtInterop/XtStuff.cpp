#include "stdafx.h"
#include "XtInterop.h"

XT_APPLICATION
XT_END_APPLICATION

void DebugLn(const char* text, ...)
{
	char buffer[512];
	va_list vl;

	va_start(vl, text);
	_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, text, vl);
	va_end(vl);

	OutputDebugStringA(buffer);
	OutputDebugStringA("\r\n");
}
