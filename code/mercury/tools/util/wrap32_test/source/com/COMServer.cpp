// COMServer.cpp : Implementierung von WinMain

#include "wrap32/com/stdafx.h"
#include "resource.h"
#include "wrap32/com/wrap32_com.h"


class CCOMServerModule : public CAtlExeModuleT< CCOMServerModule >
{
public :
	DECLARE_LIBID(LIBID_COMServerLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_COMSERVER, "{BAC1A1AB-847B-4220-B4A6-E377D465FF24}")
};

CCOMServerModule _AtlModule;



//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR /*lpCmdLine*/, int nShowCmd)
{
    return _AtlModule.WinMain(nShowCmd);
}

