#ifndef SPEEDTREE_BILLBOARD_APPLICATION_INCLUDE
#define SPEEDTREE_BILLBOARD_APPLICATION_INCLUDE

class SPTBillboardApp : public Tools::C_ToolsApplication
{
public:
	SPTBillboardApp()
		: Tools::C_ToolsApplication(
			"spt_billboard",
			"Bizarre Creations Speedtree Billboard Generator",
			"tools.speedtreebillboard",
			"")
	{
	}

	int Main(int argc, char **argv, Progress::I_ProgressBar *pProgress);
};


#endif