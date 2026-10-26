#include "stdafx.h"
#include "Application.h"

int main(int argc, char* argv[])
{
#ifdef BUILD_CONFIG_DEBUG
	MessageBox(NULL, "Camera Converter: Attach now", "Status", MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif

	C_Application app(
		"Camera Converter",
		"Convert cameras to a CSE file",
		"mercury.camera2",
		"",
		true);
		
	return app.Run(argc, argv);
}
