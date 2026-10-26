#include "stdafx.h"
#include "Application.h"
 
int main(int argc, char* argv[])
{
#ifdef BUILD_CONFIG_DEBUG
	//MessageBox(NULL, "Null Converter: Attach now", "Status", MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);
#endif

	C_Application app(
		"Animation Converter",
		"Bizarre Creations Animation Converter",
		"tools.animationconverter",
		"",
		true);
		
	return app.Run(argc, argv);
}

