#include "stdafx.h"
#include "NetworkShim.h"

int main(int argc, char* argv[])
{
#ifdef BUILD_CONFIG_DEBUG
	MessageBox(NULL, "NetworkShim2: Attach now", "Status", MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif

	C_NetworkShim app(
		"NetworkShim2",
		"NetworkShim2",
		"tools.networkshim2",
		"",
		true);
		
	return app.Run(argc, argv);
}
