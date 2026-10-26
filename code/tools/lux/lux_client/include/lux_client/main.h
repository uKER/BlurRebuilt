#ifndef __LPVSCLIENT_MAIN_H__
#define __LPVSCLIENT_MAIN_H__

class C_LpvsClientApp : public Tools::C_ToolsApplication
{
public:
	C_LpvsClientApp() :
		Tools::C_ToolsApplication(
			"Lux",									// application name
			"Bizarre Creations Lighting Tool",		// application description
			"tools.lux",							// application logger
			"")			// .dmp file destination path
	{
	}

	/// main function of the application
	int Main(int argc, char **argv, Progress::I_ProgressBar *pProgress);
};

#endif
