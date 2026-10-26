#include "stdafx.h"
#include <system/debug/log/logger.h>

#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>
#include <tools_shared/Argument/Argument.h>
#include <tools_shared/Argument/ArgumentParser.h>
#include <tools_shared/Argument/Syntax.h>
#include <tools_shared/Argument/ArgumentDataBase.h>
#include <Aftk/Aftk.h>

#include "Application.h"
#include "Convert.h"
#include "Merge.h"
#include "Query.h"

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
	Debug::GetRootLogger().SetLevel(DebugLevel::Info);
	bool success(true);

	C_Options options;
	if (!ParseCommandLineOptions(argc, argv, options))
		return E_ReturnCode_CmdParseErr;

	E_ReturnCode result;
	if (!Aftk::C_Global::Create())
		return E_ReturnCode_Fatal;
	//Perform work.
	{
		result = Process(options);
	}
	Aftk::C_Global::Destroy();
	return result;
}

bool C_Application::ParseCommandLineOptions(
	int argc, 
	char* argv[], 
	C_Options& options)
{
	Btl::String args;
	for (S32 i = 1; i < argc; ++i)
	{
		args += "\"";
		args += argv[i];
		args += "\"";
	}

	Tools::C_Syntax* pSyntax = new Tools::C_Syntax;
	bool status = true;
	{
		Tools::C_Argument arg(
			'c', 
			"command",
			"command to perform:\n\
      c  convert scene\n\
      m  merge scenes",
		Tools::C_Argument::E_Required_True);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'o', 
			"output",
			"output file",
			Tools::C_Argument::E_Required_True);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'i', 
			"input",
			"files to convert",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Tools::C_ArgType(Util::E_VarType_String, true));
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'x', 
			"inputArgs",
			"input arguments",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'y', 
			"outputArgs",
			"output arguments",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'f', 
			"workFile",
			"work file",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'q', 
			"quiet",
			"quiet mode",
			Tools::C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'z', 
			"force",
			"force conversion",
			Tools::C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}

	Tools::C_ArgumentDatabase argDb;
	Tools::C_ArgumentParser parser(pSyntax);
	if (!parser.Parse(argDb, args))
	{
		LOGERROR(g_log, "Failed to parse arguments!");
		return false;
	}

	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('c');
		BIZ_ASSERT(pArgData);
		Btl::String str;
		if (pArgData->Size())
			pArgData->Back().GetValue(str);
		options.command = GetCommand(str.Chars());
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('o');
		BIZ_ASSERT(pArgData);
		Btl::String str;
		if (pArgData->Size())
			pArgData->Back().GetValue(str);
		options.output = str;
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('i');
		if (pArgData)
		{
			for (S32 i = 0; i < pArgData->Size(); ++i)
			{
				Btl::String str;
				(*pArgData)[i].GetValue(str);
				options.inputs.PushBack(str);
			}
		}	
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('x');
		if (pArgData)
		{
			Btl::String str;
			if (pArgData->Size())
				pArgData->Back().GetValue(str);
			options.inputArgs = str;
		}
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('y');
		if (pArgData)
		{
			Btl::String str;
			if (pArgData->Size())
				pArgData->Back().GetValue(str);
			options.outputArgs = str;
		}
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('f');
		if (pArgData)
		{
			Btl::String str;
			if (pArgData->Size())
				pArgData->Back().GetValue(str);
			options.mergeWorkFile = str;
		}
	}

	options.quiet = argDb.IsSet('q');
	options.forceConversion = argDb.IsSet('z');

	if (options.quiet)
		Debug::GetRootLogger().RemoveSubscriber(g_pConsoleSubscriber);

	LOGINFO(g_log, "AftkConv - Bizarre Creations command line Aftk converter");
	return status;
}

E_ReturnCode C_Application::Process(C_Options& options)
{
	switch(options.command)
	{
	case E_Command_Convert:
		return Commands::Convert(options);
	case E_Command_Merge:
		return Commands::Merge(options);
	case E_Command_Query:
		return Commands::Query(options);
	default:
		LOGERROR(g_log, "Unknown command!");
		return E_ReturnCode_Fatal;
	};

	return E_ReturnCode_Success;
}
