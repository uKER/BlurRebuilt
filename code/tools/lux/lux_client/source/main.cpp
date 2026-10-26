#include "stdafx.h"
#include <fcntl.h>
#ifdef __CYGWIN__
#include <io.h>
#endif
#include <system\debuglog.h>
#include <system/debug/log/debuglog.h>
#include <system\memory.h>
#include <system\debug\log\subscribers\logfilesubscriber.h>
#include <system\debug\log\subscribers\consolesubscriber.h>
#include <system\debug\log\ExecuterFeedback.h>
#include <system\misc\progress.h>
#include <core\utility\scopedpointer.h>
#include <tools_shared\ToolsShared.h>
#include <tools_shared\VersionStrings.inl>

#include "lux_client\Main.h"
#include "Lux/Core.h"
#include "Lux/LightingModelProperties.h"
#include "CServ/ClusterServHeaders.h"
#include "cc++/Thread.h"
#include "time.h"

#include "CmdLine.h"

#ifdef WIN32
#include "lux_common/mdump.h"
#endif

void *tracked_alloc(size_t);
void *tracked_aligned_alloc(size_t, size_t);
void tracked_free(void*);
void tracked_aligned_free(void*);

void* operator new[]( size_t size )
{
	void *p = tracked_alloc(size);
	if (!p) throw std::bad_alloc();
	return p;
}
void operator delete[]( void* ptr ) { tracked_free(ptr); }
void * operator new (size_t size)
{
	void *p = tracked_alloc(size);
	if (!p) throw std::bad_alloc();
	return p;
}
void operator delete(void *mem) { tracked_free(mem); }
void *BizAlignedMalloc(size_t size, size_t align)
{
	void *p = tracked_aligned_alloc(size,align);
	if (!p) throw std::bad_alloc();
	return p;
}
void BizAlignedFree(void *p) { tracked_aligned_free(p); }

using namespace TCLAP;
using namespace std;
using namespace LPVS;
using namespace LUX;

std::string g_inputSceneFile;
std::string g_outputSceneFile;
std::string g_lightingModelFile;
std::string g_lightMapOutputFolder;
extern std::string g_clientExePath;
extern std::string g_clientElfPath;
extern char gpszGothamPathName[512];
char gpszLogFile[512]="";

std::string g_lightMapFilePrefix = "lightmap";
LUX::LuxLightMapFileTypeEnum g_lightMapFileType = LUX::LUX_LIGHT_MAP_FILE_TYPE_BMP;

std::string g_projectFile;
extern bool g_useNetworkClients;
extern bool g_silentExit;
extern bool gbDebugMode;

LUX::Core *g_pLuxCore;

void MemMirrorOn();
void InitializeMemoryTracker();

extern int GetNumProcessors();

void CheckForWorkerMode(int argc, char* argv[])
{
	if (argc < 2)
		return;

	// check for 'worker' mode which connects to a given server IP to help them lightmap a scene
	if (!strcmp(argv[1],"/worker"))
	{
		if (!argv[2])
		{
			printf("No IP address to connect to specified.\n");
			exit(-1);
		}

		ClientThread *pClient = StartClient(argv[2]);

		ost::Thread::sleep(1000); // give the thread a chance to start

		pClient->join();

		int iExitCode = pClient->m_iExitCode;

		delete pClient; // wait for thread to close and delete it

		exit(iExitCode);
	}
}

void Shutdown()
{
	g_pLuxCore->Destroy();
	delete g_pLuxCore;

	Aftk::C_Global::Destroy();
}

bool LightScene(Progress::I_ProgressBar *pProgress)
{
	time_t ltime;

	Progress::UpdateMulti multi(pProgress, "Lighting");
	Ut::ScopedPointer<Progress::I_ProgressBar> pLoading( multi.AddChild(426.f) );	// loading
	Ut::ScopedPointer<Progress::I_ProgressBar> pLighting(multi.AddChild(780.f + 3000.f + 180.f + 240.f + 120.f)); // unwrapping, rendering, packing, validate, welding
	Ut::ScopedPointer<Progress::I_ProgressBar> pSaving(multi.AddChild(95.f));	// saving

	MEMORY_TRACKER_PUSH("tools.lux.LightScene");

	LUX::Lux* pLux = g_pLuxCore->GetLightingSystemPtr();

	pLux->SetOutputSceneFileName(g_outputSceneFile);
	pLux->SetLightMapFolder(g_lightMapOutputFolder);
	pLux->SetLightMapFilePrefix(g_lightMapFilePrefix);
	pLux->SetLightMapFileType(g_lightMapFileType);
	pLux->SetInputSceneFileName(g_inputSceneFile);
	pLux->SetLightingModelFileName(g_lightingModelFile);

	// write out a log file in the same place as the .a3d output file
	strcpy(gpszLogFile,g_outputSceneFile.c_str());
	char *psz = strrchr(gpszLogFile,'.');
	if (psz)
	{
		strcpy(psz,".log");
		Debug::LogFileSubscriber *pLogFile = new Debug::LogFileSubscriber;

		pLogFile->SetXMLParameter("file", gpszLogFile);
		pLogFile->Start();
		Debug::GetRootLogger().AddSubscriber( pLogFile );
	}

	// print time and date
	ltime = time( NULL ); LOGINFO( LUX::g_Log, ctime( &ltime ) );

	if (!pLux->ReadLightingModelProperties())
	{
		return false;
	}

	if (!g_pLuxCore->ReadScene(0, Pointer(pLoading)))
	{
		return false;
	}

	if (!pLux->GetLightingModelPropertiesPtr()->AddLightsFromScene(pLux->GetAftkScene()))
		LOGWARN( LUX::g_Log, "Multiple directional lights exist. Check your input scene and your lighting.xml file.\n");

	if (!g_pLuxCore->LightScene(Pointer(pLighting)))
	{
		return false;
	}

	if (!g_pLuxCore->WriteScene(Pointer(pSaving)))
	{
		return false;
	}

	// print time and date
	ltime = time( NULL ); LOGINFO( LUX::g_Log, ctime( &ltime ) );

	MEMORY_TRACKER_POP("tools.lux.LightScene");

	return true;
}

extern Tu::DebugManager g_netdebugManager;

int main(int argc, char**argv)
{
	InitializeMemoryTracker();

	MEMORY_TRACKER_PUSH("tools.lux");

	C_LpvsClientApp myApp;

	MEMORY_TRACKER_POP("tools.lux");

	return myApp.Run(argc, argv);
}


// the main function
int C_LpvsClientApp::Main(int argc, char **argv, Progress::I_ProgressBar *pNetProgress)
{
	CheckForWorkerMode(argc, argv);

	MEMORY_TRACKER_PUSH("tools.aftk");

	Aftk::C_Global::Create();

	MEMORY_TRACKER_POP("tools.aftk");

	// connect to progress server
	CmdLine cmd("LPVS - commandline version of the Bizarre Creations Lighting Tool.", ' ', "1.0");

	// specify possible commandline arguments
	UnlabeledValueArg<string> insceneArg("inname",   "Input scene filename (.a3d, .xsi)", "", "input file"  );		cmd.add( insceneArg );
	UnlabeledValueArg<string> outsceneArg("outname", "Lit output scene filename", "", "output file"  );				cmd.add( outsceneArg );
	UnlabeledValueArg<string> xmlArg("xml","XML file containing the lighting model settings","","xml file");		cmd.add( xmlArg );
	UnlabeledValueArg<string> lmappathArg("lmap","Path that the lightmaps will be saved to","","lightmap path");	cmd.add( lmappathArg );

	// server mode path
	ValueArg<string> clientPathWindows("w","clientexe","Override lpvs .exe path that Windows screensaver clients will use",false,"","Win exe path");
	cmd.add( clientPathWindows );

	ValueArg<string> clientPathG5("g","clientelf","Override lpvs .elf path that G5 farm clients will use",false,"","G5 elf path");
	cmd.add( clientPathG5 );

	// texture path
	ValueArg<string> texPath("t","texpath","Provide alternative alienbrain path.",false,"","texpath");
	cmd.add( texPath );

//#ifdef _DEBUG
	// debug switch
	SwitchArg debugSwitch("d","debug","Enable debugging code", false);
	cmd.add( debugSwitch );
//#endif

	// debug switch
	SwitchArg silentSwitch("s","silent","Exit silently when an unhandled exception occurs.", false);
	cmd.add( silentSwitch );

	// parse commandline
	cmd.parse( argc, argv );

	// get results
	g_inputSceneFile = insceneArg.getValue();
	g_outputSceneFile = outsceneArg.getValue();
	g_lightingModelFile = xmlArg.getValue();
	g_lightMapOutputFolder = lmappathArg.getValue();
	g_clientExePath = clientPathWindows.getValue();
	g_clientElfPath = clientPathG5.getValue();
	g_silentExit = silentSwitch.getValue();

	m_bRunLocally = !g_silentExit;

	g_useNetworkClients = (g_clientExePath != "") || (g_clientElfPath != "");

	// get texture path
	if (texPath.isSet())
		strcpy(gpszGothamPathName, texPath.getValue().c_str());

	if ((g_lightMapOutputFolder.c_str()[g_lightMapOutputFolder.size()-1] != '/') &&
		(g_lightMapOutputFolder.c_str()[g_lightMapOutputFolder.size()-1] != '\\'))
	{
		g_lightMapOutputFolder += "/";
	}

//#ifdef _DEBUG
	gbDebugMode = debugSwitch.getValue();
//#endif

	// TODO: put screensaver client exe path into network code
	g_pLuxCore = new LUX::Core();

	if (!LightScene(pNetProgress))
	{
		LOGFATAL( LUX::g_Log, "Failed to light scene.");
		Shutdown();
		return -2;
	}
	else
	{
		Shutdown();
		return 0;
	}
}

