// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit.h>
#include <bdNet/bdNet.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BD_LOG_LEVEL "bdExampleCommon/init"

bdPlatformParams::bdPlatformParams()
{
}

bdInt BD_CALL bdExamplePlatformInit::init(bdNetStartParams &params,
										  bdPlatformParams *)
{
	// get IP address
	bdBool success = false;

	bdInt sfd;
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;

	memset(&ifr, 0, sizeof ifr);

	if (0 > (sfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		return -1;
	}

	// TODO: Figure out how we should really be doing this!  (This is crazy.)
#ifdef BD_PLATFORM_APPLE
	strcpy(ifr.ifr_name, "en0");
#else
	strcpy(ifr.ifr_name, "eth0");
#endif
	sin->sin_family = AF_INET;

	if (0 == ioctl(sfd, SIOCGIFADDR, &ifr))
	{
		params.m_localAddresses.pushBack(bdInetAddr(sin->sin_addr.s_addr));
		success = true;
	}

	// TODO: Figure out how to do this properly on Linux, then add support for Apple.
#ifndef BD_PLATFORM_APPLE
	// Output the default gateway to a temp file.
	if( system( "route | grep default | awk {'printf \"%s\", $2'} > gatewayip" ) == 0 )
	{
		FILE *routeFile = fopen("gatewayip", "r");
		if( routeFile != BD_NULL )
		{
			// Read the data from the file
			bdNChar8 gatewayIp[BD_ADDR_STRING_SIZE];
			bdInt read = fread( gatewayIp, 1, BD_ADDR_STRING_SIZE, routeFile );
			if( read > 0 )
			{
				gatewayIp[read] = '\0';

				// remove the temp file
				system( "rm -f gatewayip");

				// Set the gateway address
				params.m_UPnPConfig.m_gatewayAddr = bdInetAddr( gatewayIp );
			}
		}
	}
#endif
	
	return success ? 0 : -1;
}


void BD_CALL bdExamplePlatformInit::quit()
{
}

