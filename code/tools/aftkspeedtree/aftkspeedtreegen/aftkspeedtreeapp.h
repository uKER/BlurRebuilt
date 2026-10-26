#ifndef	AFTK_SPEEDTREE_APP
#define AFTK_SPEEDTREE_APP

#ifdef _MSC_VER
#pragma once
#endif

class AftkSpeedTreeApp : public Tools::C_ToolsApplication
{
public:
	AftkSpeedTreeApp()
		: Tools::C_ToolsApplication(
			"aftkspeedtreegen",
			"Bizarre Creations Speedtree Mesh Generator",
			"tools.aftkspeedtreegen",
			"")
	{
	}

	int Main(int argc, char **argv, Progress::I_ProgressBar *pProgress);
};


#endif

