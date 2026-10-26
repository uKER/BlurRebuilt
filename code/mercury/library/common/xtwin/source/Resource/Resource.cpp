#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0410
#define _WIN32_IE 0x0600

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, void*)
{
	return TRUE;
}