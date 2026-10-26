// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit.h>

#include <xonline.h>

#define BD_LOG_LEVEL "bdExampleCommon/init"

bdPlatformParams::bdPlatformParams()
{
}

bdInt BD_CALL bdExamplePlatformInit::init(bdNetStartParams &/*params*/,
										  bdPlatformParams *)
{
	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);

	// uncomment to disable security
	//xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;

	bdInt err = XNetStartup(&xnsp);

	BD_ASSERT(err == 0, "Error starting XBox network");

	// Init winsock 2.2
	WSADATA wsaData;
	err = WSAStartup( MAKEWORD(2,2), &wsaData );

	BD_ASSERT(err == 0, "Error starting XBox (winsock) network");

#if !defined BD_PLATFORM_XENON
	err = XOnlineStartup(BD_NULL);
	BD_ASSERT(err == S_OK, "Error starting Live libraries.");
#else
	err = XOnlineStartup();
	BD_ASSERT(err == ERROR_SUCCESS, "Error starting Live libraries.");
#endif

	return err;
}


void BD_CALL bdExamplePlatformInit::quit()
{
	XOnlineCleanup();
	WSACleanup();
	XNetCleanup();
}

