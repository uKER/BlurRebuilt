
#include "stdafx.h"
#include "../XenWatch/Xenon.h"
#include "../XenWatch/CaptureClient.h"

static unsigned int IpAddress = 0;

static void ParseSwitch(const char *swi, const char *val)
{
	if (_stricmp(swi, "ip") == 0)
	{
		int a = 0, b = 0, c = 0, d = 0;
		sscanf(val, "%d.%d.%d.%d", &a, &b, &c, &d);
		IpAddress = (a << 24) | (b << 16) | (c << 8) | d;
	}
}

static void ParseCommandLine(int argc, char* argv[])
{
	int i = 0;
	for (i = 1; i < argc; i++)
	{
		const char *arg = argv[i];
		const char *next = 0;
		
		if (i + 1 < argc)
			next = argv[i + 1];

		if (next && arg[0] == '-' || arg[0] == '/')
		{
			// Parse switch and the next argument
			ParseSwitch(arg + 1, next);
			i++;
		}
	}
}

int main(int argc, char *argv[])
{
	ParseCommandLine(argc, argv);

	if (IpAddress == 0)
	{
		XenonInit();
		IpAddress = XenonAltAddress();
	}

	C_CaptureClient::NetworkInit();
	C_CaptureClient client;

	client.Connect(IpAddress);
	client.ScreenCapture();
	client.Disconnect();

	C_CaptureClient::NetworkExit();

	return 0;
}
