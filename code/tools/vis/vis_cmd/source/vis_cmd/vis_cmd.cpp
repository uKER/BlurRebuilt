// nexus_cmd.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <system/debug/log/debuglog.h>
#include "vis_core/vis.h"
#include "CServ/ClusterServHeaders.h"
#include "vis_core/mdump.h"
#include "tools_debug/DebugManager.h"
#include <crtdbg.h>
#include <system\debuglog.h>
#include <system\debug\log\subscribers\logfilesubscriber.h>
#include <tools_shared\ToolsShared.h>
#include <tools_shared\VersionStrings.inl>
#include "vis_cmd.h"

#define ENABLE_MEMORY_TRACKER

#include "system\memory\MemoryTrackerDefines.h"
#include "system\memory\MemoryTracker.h"
//#include "system\memory\AllocatorFunctions.inl"	// comment this in for memory tracking "goodness"

#ifndef WIN64
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='X86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
#endif

using namespace TCLAP;
using namespace std;
extern bool gbExitIfNoD3D, gbCrowdPvsLayerOnly, gbSilentMode;

static Debug::Logger &g_Log = Debug::GetLogger("tools.vis");

void CreateDummyD3DDevice();

// check for 'worker' mode which connects to a given server IP to help them PVS a scene
void CheckForWorkerMode(int argc, char** argv)
{
	if (argc != 3)
		return;

	if (strcmp(argv[1],"/worker"))
		return;

	if (!argv[2])
	{
		printf("No IP address to connect to specified.\n");
		exit(-1);
	}

	gbExitIfNoD3D = true;
	ClientThread *pClient = StartClient(argv[2]);

	ost::Thread::sleep(1000); // give the thread a chance to start

	pClient->join();

	int iExitCode = pClient->m_iExitCode;

	delete pClient; // wait for thread to close and delete it

	exit(iExitCode);
}

void MemMirrorOn();

int main(int argc, char** argv)
{
	VisApp myApp;

	return myApp.Run(argc,argv);
}

int VisApp::Main(int argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	// check for 'worker' mode which connects to a given server IP to help them PVS a scene
	CheckForWorkerMode(argc,argv);

	Memory::Tracker::Instance().DumpBlocksOnExit(true);

	CmdLine cmd("Calculates visibility of a scene file.", ' ', "1.0");

	// specify possible commandline arguments
	UnlabeledValueArg<string> nameArg("inname", "Input scene filename", "", "input .a3d file"  );				cmd.add( nameArg );
	UnlabeledValueArg<string> OutNameArg("outname", "Output data filename", "", "output file"  );				cmd.add( OutNameArg );
	UnlabeledValueArg<string> xmlArg("points","Input binary file specifying sample points","","sample file");	cmd.add( xmlArg );

//	ValueArg<string>		  vs2Arg("v","vs2","Specify dynamic PVS input data file",false,"vs2 file","vs2 file");			cmd.add( vs2Arg );

//	ValueArg<string> visListWrite("w","write","Specify vislist file to create",false,"vislist file","vislist create");
//	cmd.add( visListWrite );
//	ValueArg<string> visListRead("r","read","Specify vislist file to compare with",false,"vislist file","vislist read");
//	cmd.add( visListRead );

	// debug switch
	SwitchArg debugSwitch("d","debug","Enable debugging code", false);
	cmd.add( debugSwitch );

	// crowd pvs layer switch
	SwitchArg crowdSwitch("","crowd","Ignore all instances, except ones on crowd_pvs_layer",false);
	cmd.add( crowdSwitch );

	// server mode path
	ValueArg<string> clientPath("e","clientexe","Override vis.exe path that screensaver clients will use",false,"","svpath");
	cmd.add( clientPath );

	// fxb path
	ValueArg<string> fxbPath("f","fxbpath","Set the fxb shader path",false,"","fxbpath");
	cmd.add( fxbPath );

	// texture cache path
	ValueArg<string> textureCachePath("t","tcachepath","Set the texture cache path",false,"","tcachepath");
	cmd.add( textureCachePath );

	// car switch
	SwitchArg carSwitch("","car","Automatically generate sample points for a car", false);
	cmd.add( carSwitch );

	// samplepoint debug switch
	ValueArg<string> forceSwitch("","force","Force to only render one samplepoint at the specified position", false, "", "force");
	cmd.add( forceSwitch );

	// server mode switch
	SwitchArg serverSwitch("s","server","Use screensaver clients when calculating", false);
	cmd.add( serverSwitch );

	// silent mode switch
	SwitchArg silentSwitch("","silent","Exit silently in case of an error", false);
	cmd.add( silentSwitch );

	// xml config file
	ValueArg<string> configSwitch("", "config", "Specify XML config file", false, "", "config");
	cmd.add( configSwitch );

	// cluster threshold value
	ValueArg<int> clusterThreshold("", "threshold", "Pixel threshold for cluster visibility", false, 1, "threshold");
	cmd.add( clusterThreshold );

	// poly threshold value
	ValueArg<int> polyThreshold("", "polythreshold", "Pixel threshold for polygon visibility", false, 1, "polythreshold");
	cmd.add( polyThreshold );

	// parse commandline
	cmd.parse( argc, argv );

	Aftk::C_Global::Create();
	Aftk::C_Scene *pAftk = new Aftk::C_Scene;

	// load sample points
	Vis v;

	// create a vislist file
//	if (visListWrite.isSet())
//		v.SetVisListPath(visListWrite.getValue(), VISLIST_CREATE);
//	// read back a vislist file
//	if (visListRead.isSet())
//		v.SetVisListPath(visListRead.getValue(), VISLIST_COMPARE);

	// write out a log file in the same place as the .a3d output file
	char buffer[512];
	strcpy(buffer, OutNameArg.getValue().c_str());
	char *psz = strrchr(buffer,'.');
	if (psz)
	{
		strcpy(psz,".log");
		Debug::GetRootLogger().AddSubscriber( new Debug::LogFileSubscriber( buffer ) );
	}

	// get crowd pvs switch
	gbCrowdPvsLayerOnly = crowdSwitch.getValue();

	gbSilentMode = silentSwitch.getValue();
	if (gbSilentMode)
		m_bRunLocally = false;

	gbDebugMode = debugSwitch.getValue();
	if (serverSwitch.getValue())
	{
		gbExitIfNoD3D = false;
		v.SetRemoteClientPath( clientPath.getValue() );
	}

	if (gbDebugMode)
		CreateDummyD3DDevice();

	v.SetPaths(textureCachePath.getValue().c_str(),fxbPath.getValue().c_str());

	if (!v.SetXMLConfigFile( configSwitch.getValue().c_str() ))
		exit(-1);

	if (forceSwitch.isSet())
	{
		LOGINFO( g_Log, "Overriding sample point with '%s'.\n", forceSwitch.getValue().c_str());
		v.OverrideSamplePoints(forceSwitch.getValue());
	}
	else
	{
		// automatically generate sample points for car
		if (carSwitch.isSet())
		{
			LOGINFO( g_Log, "Generating sample points for car...\n");
			v.AutoGenSamplePointsForCar();
		}
		else
		{
			LOGINFO( g_Log, "Reading sample points...\n");
			v.LoadXML( xmlArg.getValue() );
		}
	}

	// try to load the .a3d file
	LOGINFO( g_Log, "Loading scene file...\n");
	Aftk::C_Status status = pAftk->Read( nameArg.getValue() );
	if (!status)
	{
		S_INFO( g_Log, "Failed to load scene " << nameArg.getValue() << " \n     " << status.GetErrorString().c_str() << "\n");
		exit(-1);
	}

	LOGINFO( g_Log, "Scene loaded.\n");
	v.SetScene( pAftk );
//	if (vs2Arg.isSet())
//		v.SetVS2( vs2Arg.getValue().c_str() );

	v.SetClusterThreshold(clusterThreshold.getValue());
	v.SetPolygonThreshold(polyThreshold.getValue());
	v.Process(pProgress);

	LOGINFO( g_Log, "Saving PVS data...\n");
	v.Save( OutNameArg.getValue().c_str() );
	LOGINFO( g_Log, "Finished.\n");

	// aftk gets deleted by v.Process to save memory
	Aftk::C_Global::Destroy();

	return 0;
}

void AddDummyNode(float x, float y, float z)
{
}
