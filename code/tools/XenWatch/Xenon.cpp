
#include "stdafx.h"
#include "app.h"

static HMODULE Module=NULL;
static HRESULT (__stdcall *ImGetAltAddress)(LPDWORD)=NULL;
static HRESULT (__stdcall *ImGetNameOfXbox)(LPSTR,LPDWORD,BOOL)=NULL;
static unsigned int XenonIp=0;
static char XenonName[64]={0};

static int GetIp()
{
	DWORD size=sizeof(XenonName);
	DWORD ip=0;

	ImGetNameOfXbox(XenonName,&size,false);
	ImGetAltAddress(&ip);
	XenonIp=ip;
	
	return 0;
}

int XenonInit()
{
	char xbdm[260]="";

	// Load xbdm.dll dynamically, in case xbdm.dll is not on the machine:
	GetEnvironmentVariable("XEDK",xbdm,192);
	strcat(xbdm,"\\bin\\win32\\xbdm.dll");

	Module=LoadLibrary(xbdm); if (Module==NULL) return 1;
	
	ImGetAltAddress=(HRESULT (__stdcall *)(LPDWORD))GetProcAddress(Module,"_DmGetAltAddress@4");
	ImGetNameOfXbox=(HRESULT (__stdcall *)(LPSTR,LPDWORD,BOOL))GetProcAddress(Module,"_DmGetNameOfXbox@12");

	GetIp();

	return 0;
}

unsigned int XenonAltAddress()
{
	DWORD ip=0;
	if (ImGetAltAddress) ImGetAltAddress(&ip);
	return ip;
}
