#include "stdafx.h"
#include <system/xml/XmlDom.h>
#include <system/debug/log/logger.h>
#include <system/debug/log/subscribers/LogFileSubscriber.h>

#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>

#include "Application.h"

C_Application::C_Application(
	const char *szName,
	const char *szSubtitle,
	const char *szLogger,
	const char *szDmpPath,
	bool bExecuter)
	:
	Tools::C_ToolsApplication(
		szName,
		szSubtitle,
		szLogger,
		szDmpPath,
		bExecuter)
{
}

C_Application::~C_Application()
{
}

S32 C_Application::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	C_Options options;
	bool success = true;

	Debug::LogFileSubscriber* pLogFile = new Debug::LogFileSubscriber("cmd_line_tool.log");
	GetLogger().AddSubscriber( pLogFile );

	//Build options struct
	{
		//Parse our command-line options.
		success = ParseCommandLineOptions(argc, argv, options);
		if (!success) goto exit;
	}

	//This is where you would do some stuff.
	{
	}

exit:
	if (success)
	{
		LOGINFO(GetLogger(), "Processing completed succesfully!\n");
	}
	else
	{
		LOGINFO(GetLogger(),"Processing failed!\n");
	}

	GetLogger().RemoveSubscriber(pLogFile);
	delete pLogFile;

	return success ? 0 : 1;
}

bool C_Application::ParseCommandLineOptions(
	int argc, 
	char* argv[], 
	C_Options& options)
{
	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems. 
	try
	{  
		//Define the command line object.
		TCLAP::CmdLine cmd("Example Command-line Tool.", ' ', "1.00");

		//Define a value argument and add it to the command line.
		TCLAP::ValueArg<S32> exampleArg1(
			"a","alpha","Argument 1",true,0,"This is argument 1");
		cmd.add(exampleArg1);
		TCLAP::ValueArg<std::string> exampleArg2(
			"b","beta","Argument2",true,"","This is argument 2");
		cmd.add(exampleArg2);

		//Parse the args.
		cmd.parse(argc, argv);

		//Get the values
		options.exampleArg1 = exampleArg1.getValue();
		options.exampleArg2 = exampleArg2.getValue().c_str();
	} 
	catch (TCLAP::CmdLineParseException &e)
	{
		std::cerr << "Failed to parse arguments: " << e.argId() << std::endl;
		return false;
	}
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		return false;
	}

	return true;
}
