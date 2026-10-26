#include "stdafx.h"

#include <system\debug\log\debuglog.h>
#include <CmdLine.h>

#include <nexus_cmd/NexusApp.h>


//-------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------

Nexus::Core* g_pNexusCore = 0;

//Debug::Logger &g_Log = Debug::GetLogger("genesis.tools.sceneconverter");
Tu::DebugManager* NexusCmd::g_pDebugManager = 0;
Tu::DebugManager* NexusCmd::g_pOutputManager = 0;

namespace NexusCmd
{

const Util::C_String applicationName = Util::C_String("Nexus V") + NEXUS_CORE_VERSION;


//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------
NexusCmd::ConsoleProgress* g_pProgress = 0;



void DoMessage(const std::string& text)
{
	printf("\n");
	printf("%s", text.c_str());
	*g_pOutputManager << text;
}

void DoWarning(const std::string& text)
{
	printf("\n");
	printf("%s", text.c_str());
	*g_pOutputManager << text;
}

void DoError(const std::string& text)
{
	printf("\n");
	printf("%s", text.c_str());
	*g_pOutputManager << text;
}

MainApp::MainApp() 
: C_ToolsApplication(
	applicationName.Chars(),			
	"Command-line Version",			
	"tools.nexus",			
	"",				
	true)
{
}

MainApp::~MainApp()
{
}

void MainApp::SearchAndReplace
(
	std::string& theString, 
	const std::string& theSearchSubString, 
	const std::string& theNewSubString
)
{
    using std::string;
    string::size_type pos = 0;
    while ( (pos = theString.find(theSearchSubString, pos)) != string::npos )
    {
		theString.replace( pos, 1, theNewSubString);
		pos += theNewSubString.size();
    }
}

void MainApp::DoHeader()
{
	std::stringstream ss;

	ss << "AFTK version [" << Aftk::C_Global::GetVersion() << "]\n";
//	ss << "AFTK build stamp [" << Aftk::C_Global::GetBuildStamp() << "]\n";
//	ss << "AFTK build PC [" << Aftk::C_Global::GetBuildPc() << "]\n";
//	ss << "AFTK SVN revision(s) [" << Aftk::C_Global::GetSvnRevision() << "]\n";
//	ss << "A3D exported file version [" << versionHigh << "]\n";
//	ss << "A3D importable file versions [" << versionLow << "] to [" << versionHigh << "]\n";

	*g_pOutputManager << ss.str();
}

Tu::Status MainApp::ParseCommandLineOptions(
	int argc, 
	char* argv[], 
	CommandLineOptions& commandLineOptions)
{
	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems. 
	try
	{  
		// Define the command line object.
		TCLAP::CmdLine cmd("This program performs scene processing.", ' ', NEXUS_CORE_VERSION);

		// Define a value argument and add it to the command line.
		TCLAP::ValueArg<std::string> logArg("l","log","logfile",false,"","log");
		cmd.add(logArg);
		TCLAP::ValueArg<std::string> appendLogArg("a","appendlog","Log file to append",false,"","append log");
		cmd.add(appendLogArg);
		TCLAP::ValueArg<std::string> commandString("s","commandstring","Lua command string",true,"","Lua command string");
		TCLAP::ValueArg<std::string> commandFile("f","filename","Lua command file",true,"","Lua command file");
		cmd.xorAdd(commandString, commandFile);

		// Parse the args.
		cmd.parse(argc, argv);

		// Get the value parsed by each arg.
		if (commandString.isSet())
			commandLineOptions.commandString = commandString.getValue();
		else if (commandFile.isSet())
			commandLineOptions.commandFile = commandFile.getValue();

		if (logArg.isSet() && appendLogArg.isSet())
		{
			std::cerr << "The -l and -a arguments are mutually exclusive." << std::endl;
		}
	
		if (logArg.isSet())
		{
			commandLineOptions.logFile = logArg.getValue();
			commandLineOptions.overwriteLog = true;
		}
		if (appendLogArg.isSet())
		{
			commandLineOptions.logFile = appendLogArg.getValue();
			commandLineOptions.overwriteLog = false;
		}
		SearchAndReplace(commandLineOptions.commandString, ";", "\n");
		SearchAndReplace(commandLineOptions.commandString, "'", "\"");
	} catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
	}

	return Tu::Status::SUCCESS;	
}

void MainApp::CleanUpMaya()
{
	if (g_pNexusCore)
	{
#ifdef NEXUS_CMD_MAYA
		if (g_pNexusCore->MayaInitialised())
		{
			MLibrary::cleanup(0);
		}
#endif
	}
}

Tu::Status MainApp::Create(NexusCmd::CommandLineOptions& options)
{
	WSADATA wsaData;
	WSAStartup(WINSOCK_VERSION, &wsaData);

	Tu::Status status;

	g_pDebugManager = new Tu::DebugManager();
	g_pOutputManager = new Tu::DebugManager();

	// Start debug log system
	#ifdef _DEBUG
		g_pDebugManager->SetConsoleEnabledFlag(false);
		g_pDebugManager->SetDebugEnabledFlag(false);
		g_pDebugManager->Create("nexus_export.log");
		g_pDebugManager->Create(options.logFile);
		g_pDebugManager->SetLogFileEnabledFlag(true);
	#else
		g_pDebugManager->SetConsoleEnabledFlag(false);
		g_pDebugManager->SetDebugEnabledFlag(false);
		g_pDebugManager->Create("");
		g_pDebugManager->SetLogFileEnabledFlag(false);
	#endif

	if (options.logFile.length() == 0)
	{
		#ifdef _DEBUG
		g_pOutputManager->Create("nexus_export.log");
		g_pOutputManager->SetLogFileEnabledFlag(true);
		g_pOutputManager->SetDebugEnabledFlag(true);
		g_pDebugManager->SetConsoleEnabledFlag(false);
		#else
		g_pOutputManager->Create("nexus_export.log");
		g_pOutputManager->SetConsoleEnabledFlag(false);
		g_pDebugManager->SetConsoleEnabledFlag(false);
		g_pOutputManager->SetLogFileEnabledFlag(true);
		g_pOutputManager->SetDebugEnabledFlag(false);
		#endif
	}
	else
	{
		g_pOutputManager->Create(options.logFile, options.overwriteLog);
		g_pOutputManager->SetLogFileEnabledFlag(true);
		g_pOutputManager->SetConsoleEnabledFlag(false);
		g_pOutputManager->SetDebugEnabledFlag(false);
	}

	g_pProgress = new NexusCmd::ConsoleProgress();
	g_pNexusCore = new Nexus::Core();
	status = g_pNexusCore->Create();
	g_pNexusCore->SetProgressCallback(g_pProgress);
	g_pNexusCore->SetPrintMessageCallback(&DoMessage);
	g_pNexusCore->SetPrintErrorCallback(&DoError);
	g_pNexusCore->SetPrintWarningCallback(&DoWarning);

	return status;
}

void MainApp::Destroy()
{
	g_pNexusCore->Destroy();
	delete g_pNexusCore;
	delete g_pProgress;
#ifdef NEXUS_CMD_MAYA
	CleanUpMaya();
#endif

	g_pDebugManager->Destroy();
	delete g_pDebugManager;
	g_pOutputManager->Destroy();
	delete g_pOutputManager;

	WSACleanup();
}


#include <direct.h>

int MainApp::Main(int argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	Tu::Status status;

	NexusCmd::CommandLineOptions commandLineOptions;
	ParseCommandLineOptions(argc, argv, commandLineOptions);

	status = Create(commandLineOptions);
	if (!status)
		return -2;

	DoHeader();

	if (commandLineOptions.commandString.length() > 0)
	{
		std::stringstream ss;
		ss << "Processed command string:\n" << commandLineOptions.commandString << "\n";
		DoMessage(ss.str());
	}
	if (commandLineOptions.logFile.length() > 0)
	{
		std::stringstream ss;
		ss << "Log file [" << commandLineOptions.logFile << "]\n";
		DoMessage(ss.str());
	}

	DWORD dwStart = GetTickCount();

	if (commandLineOptions.commandString.size() > 0)
	{
		status = g_pNexusCore->ExecuteString(
			commandLineOptions.commandString.c_str()
		);
	}
	else if (commandLineOptions.commandFile.size() > 0)
	{
		status = g_pNexusCore->ExecuteFile(
			commandLineOptions.commandFile.c_str()
		);
	}

	if (!g_pNexusCore->GetStatus())
	{
		status = Tu::Status(
			Tu::Status::FAILURE,
			std::string("An error occurred when executing the given script: [") +
			g_pNexusCore->GetStatus().ErrorString() + 
			std::string("].") );
	}

	DWORD dwEnd = GetTickCount();
	DWORD elapsedTime = dwEnd - dwStart;
	float time = (float) elapsedTime;
	time *= 0.001f;
	std::string messageString = "\nProcessing completed in ";
	{
		char buffer[50];
		sprintf(buffer, "%3.2f", time);
		messageString += buffer;
	}
	messageString += " seconds.\n";
	S32 exitCode = g_pNexusCore->GetExitCode();
	DoMessage(messageString);
	Destroy();
	return exitCode;
}

}// namespace NexusCmd


int main(int argc, char **argv, LPTSTR lpCmdLine)
{
	NexusCmd::MainApp mainApp;

	S32 result = mainApp.Run(argc, argv);
	//LOGDEBUG(Export::g_Log, "Nexus Main app returning with code %i\n", result);

	return result;
}





namespace Curve
{

const F32 kReparamResLow = 1;
const F32 kReparamResMedium = 4;
const F32 kReparamResHigh = 8;
const F32 kReparamResDefault = kReparamResMedium;

//Note that this matrix is transposed as compared to the matrix given on p460 of
//"Mathematics for 3D Game Programming and Computer Graphics"
//Note that it's been necessary to flip matrix horizontally so that the we traverse
//the curve in the correct direction.
extern const Maths::HMat44 kBezierBasis = Maths::HMat44(
	Maths::HVec4(-1.f,  3.f, -3.f, 1.f),
	Maths::HVec4( 3.f, -6.f,  3.f, 0),
	Maths::HVec4(-3.f,	3.f,  0.f, 0),
	Maths::HVec4( 1.f,	0.f,  0.f, 0));

extern const Maths::HMat44 kHermiteBasis = Maths::HMat44(
	Maths::HVec4( 2.f, -2.f,   1.f,  1.f),
	Maths::HVec4(-3.f,  3.f,  -2.f, -1.f),
	Maths::HVec4( 0.f,	0.f,   1.f,  0),
	Maths::HVec4( 1.f,	0.f,   0.f,  0));

//Taken from Mathematics for Game Programming and Computer Graphics p475.
extern const Maths::HMat44 kBSplineBasis = Maths::HMat44(
	Maths::HVec4(-1.f, 3.f,   -3.f,  1.f)/6.f,
	Maths::HVec4( 3.f,  -6.f,  3.f, 0.f)/6.f,
	Maths::HVec4(-3.f,	0.f,   3.f,  0)/6.f,
	Maths::HVec4(1.f,	4.f,   1.f,  0)/6.f);
 
const char* g_pEReparamStringTable[] =
{
#define DEF(e, str) str,
	#include <Misc_Shared/NewCurve/EReparamRes.h>
#undef DEF
};

const char* GetReparamString(U32 i)
{
	return g_pEReparamStringTable[i];
}


}
