#include "stdafx.h"
#include "Application.h"

int main(int argc, char* argv[])
{
#ifdef BUILD_CONFIG_DEBUG
	MessageBox(NULL, "Texture Converter: Attach now", "Status", MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif

	C_Application app(
		"Texture Converter",
		"Texture Converter",
		"tools.textureconverter",
		"",
		true);
		
	return app.Run(argc, argv);
}

