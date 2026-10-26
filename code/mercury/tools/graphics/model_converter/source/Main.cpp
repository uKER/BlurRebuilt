#include "stdafx.h"
#include "Application.h"

int main(int argc, char* argv[])
{
#ifdef BUILD_CONFIG_DEBUG
//	MessageBox(NULL, "Example A3D Converter: Attach now", "Status", MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif

	C_Application app(
		"Example A3D Converter",
		"Example A3D Converter",
		"tools.examplea3dconverter",
		"",
		true);
		
	return app.Run(argc, argv);
}
