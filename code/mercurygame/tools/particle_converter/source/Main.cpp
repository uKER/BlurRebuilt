#include "stdafx.h"
#include "Application.h"

int main(int argc, char* argv[])
{
#ifdef BUILD_CONFIG_DEBUG
	MessageBox(NULL, "Particle Converter: Attach now", "Status", MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif

	C_Application app(
		"Particle Converter",
		"Particle Converter",
		"tools.particleconverter",
		"",
		true);
		
	return app.Run(argc, argv);
}

