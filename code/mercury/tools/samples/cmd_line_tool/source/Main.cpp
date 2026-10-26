#include "stdafx.h"
#include "Application.h"

int main(int argc, char* argv[])
{
#ifdef BUILD_CONFIG_DEBUG
	MessageBox(NULL, "Example Command-line Tool: Attach now", "Status", MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif

	C_Application app(
		"Example Command-line Tool",
		"",
		"tools.examplecommandlinetool",
		"",
		false);
		
	return app.Run(argc, argv);
}

