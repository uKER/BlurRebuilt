
#include "stdafx.h"
#include "Xenon.h"

// Xenon/Xbox 360 function calls to get the address of the Xbox 360

static HMODULE Module=NULL;
static HRESULT (__stdcall *ImGetAltAddress)(LPDWORD)=NULL;

int XenonInit()
{
	char xbdm[260] = {0};

	// Load xbdm.dll dynamically, in case xbdm.dll is not on the machine:
	GetEnvironmentVariable("XEDK", xbdm, 192);
	strcat(xbdm, "\\bin\\win32\\xbdm.dll");

	Module = LoadLibrary(xbdm);
	if (Module == NULL)
		return 1;
	
	// Dynamically find the addresses of functions:
	ImGetAltAddress = (HRESULT (__stdcall *)(LPDWORD))GetProcAddress(Module, "_DmGetAltAddress@4");
	
	return 0;
}

unsigned int XenonAltAddress()
{
	DWORD ip = 0;
	if (ImGetAltAddress)
		ImGetAltAddress(&ip);

	return ip;
}
