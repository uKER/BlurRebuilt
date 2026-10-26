#ifndef __VIS_CMD_H__
#define __VIS_CMD_H__

class VisApp : public Tools::C_ToolsApplication
{
public:
	VisApp() :
		Tools::C_ToolsApplication(
			"Vis",									// application name
			"Bizarre Creations PVS Rendering Tool",	// application description
			"tools.vis",							// application logger
			"")			// .dmp file destination path
	{
	}

	/// main function of the application
	int Main(int argc, char **argv, Progress::I_ProgressBar *pProgress);
};

#endif
