#include "stdafx.h"
#include <system/xml/XmlDom.h>
#include <system/debug/log/logger.h>

#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>
#include <tools_shared/Argument/Argument.h>
#include <tools_shared/Argument/ArgumentParser.h>
#include <tools_shared/Argument/Syntax.h>
#include <tools_shared/Argument/ArgumentDataBase.h>

#include "Commands/PakAddUpdate.h"
#include "Commands/PakCreate.h"
#include "Commands/PakExtract.h"
#include "Commands/PakList.h"
#include "Commands/PakTest.h"
#include "Application.h"
#include <system/scheduler.h>
#include "tools_shared/SchedulerConfig.h"
#include <PakFs/PakFs.h>

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

	LOGINFO(g_log, "Processing %s...\n", options.pakFilePath.Chars());
	Pak::_PakPacketZLibPolicy = C_SchedulerConfig::Instance().GetPakPolicy();
	return Process(options);
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
      c  create new pak\n\
      i  create pak via incremental update\n\
      a  add/update files in existing pak\n\
      d  delete files from existing pak\n\
      l  list pak contents\n\
      t  verify pack integrety\n\
      x  extract pak contents",
		Tools::C_Argument::E_Required_True);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'f', 
			"file",
			"pak file",
			Tools::C_Argument::E_Required_True);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'i', 
			"input",
			"files/dirs to pak",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Tools::C_ArgType(Util::E_VarType_String, true));
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'*', 
			"xd",
			"dirs to exclude",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Tools::C_ArgType(Util::E_VarType_String, true));
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'm', 
			"method",
			"compression method",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Util::E_VarType_String);
		arg.GetDefaults().PushBack(Btl::String("ZLib"));
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'o', 
			"output",
			"output directory",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Tools::C_ArgType(Util::E_VarType_String));
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'p', 
			"pathseparator",
			"path separator",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Tools::C_ArgType(Util::E_VarType_String));
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'r', 
			"recurse",
			"recurse subdirectories",
			Tools::C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			's', 
			"secure",
			"enable security",
			Tools::C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'b', 
			"bigendian",
			"big endian",
			Tools::C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'w', 
			"sector",
			"sector size",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Tools::C_ArgType(Util::E_VarType_U32));
		arg.GetDefaults().PushBack(Btl::String("2048"));
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'd', 
			"metadata",
			"metadata",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Tools::C_ArgType(Util::E_VarType_String));
		pSyntax->AddArg(arg);
	}
	{
		Tools::C_Argument arg(
			'z', 
			"root",
			"root path",
			Tools::C_Argument::E_Required_False);
		arg.GetDataTypes().PushBack(Tools::C_ArgType(Util::E_VarType_String));
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
			't', 
			"time",
			"time operation",
			Tools::C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}

	Tools::C_ArgumentDatabase argDb;
	Tools::C_ArgumentParser parser(pSyntax);
	if (!parser.Parse(argDb, args))
	{
		return false;
	}


	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('c');
		BIZ_ASSERT(pArgData && pArgData->Size() == 1);
		Btl::String str;
		if (pArgData->Size())
			pArgData->Back().GetValue(str);
		options.command = GetCommand(str.Chars());
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('f');
		BIZ_ASSERT(pArgData && pArgData->Size() == 1);
		Btl::String str;
		if (pArgData->Size())
			pArgData->Back().GetValue(str);
		options.pakFilePath = str;
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('i');
		if (pArgData)
		{
			for (S32 i = 0; i < pArgData->Size(); ++i)
			{
				Btl::String str;
				(*pArgData)[i].GetValue(str);
				options.toProcess.filePaths.PushBack(str);
			}
		}	
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData("xd");
		if (pArgData)
		{
			for (S32 i = 0; i < pArgData->Size(); ++i)
			{
				Btl::String str;
				(*pArgData)[i].GetValue(str);
				options.toProcess.excludedPaths.PushBack(str);
			}
		}	
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('p');
		if (pArgData)
		{
			Btl::String str;
			if (pArgData->Size())
				pArgData->Back().GetValue(str);
			options.pathSeparator = str.Length() > 0 ? str.Chars()[0] : '\0';
		}
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData("root");
		if (pArgData)
		{
			Btl::String str;
			if (pArgData->Size())
				pArgData->Back().GetValue(str);
			options.pakRootPath = str;
		}
		if (options.pakRootPath == "")
			options.pakRootPath = Sys::ExtractDirectory(options.pakFilePath);
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('w');
		if (pArgData)
			pArgData->Back().GetValue(options.switches.sectorSize);
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('d');
		if (pArgData)
			pArgData->Back().GetValue(options.switches.metadata);
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('o');
		if (pArgData)
		{
			Btl::String str;
			if (pArgData->Size())
				pArgData->Back().GetValue(str);
			options.switches.outputDirectory = str;
		}
		options.switches.outputDirectory = Sys::ConvertToCurrentDirectory(
			Sys::C_FileSystem::Instance().GetWorkingDirectory(), options.switches.outputDirectory);
	}
	{
		const Tools::ArgDataArray* pArgData = argDb.GetArgData('m');
		if (pArgData)
		{
			Btl::String str;
			if (pArgData->Size())
				pArgData->Back().GetValue(str);
			options.switches.compressType = GetCompressType(str.Chars());
		}
	}

	options.switches.recurseSubdirectories = argDb.IsSet('r');
	options.switches.secure = argDb.IsSet('s');
	options.switches.bigEndian = argDb.IsSet('b');
	options.switches.quiet = argDb.IsSet('q');
	options.switches.timeOperation = argDb.IsSet('t');

	if (options.switches.quiet)
		Debug::GetRootLogger().RemoveSubscriber(g_pConsoleSubscriber);

	LOGINFO(g_log, "BizPak - Bizarre Creations command line pak file utility.");
	return status;
}

E_ReturnCode C_Application::Process(const C_Options& options)
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	LARGE_INTEGER freq;
	E_ReturnCode retval = E_ReturnCode_Fatal;

	if (options.switches.timeOperation)
	{
		QueryPerformanceCounter(&start);
	}
	switch(options.command)
	{
	case E_Command_Create:
		retval = Commands::PakCreate(options);
		break;
	case E_Command_CreateIncremental:
		retval = Commands::PakUpdate(options);
		break;
	case E_Command_AddUpdate:
		retval = Commands::PakUpdate(options);
		break;
	case E_Command_Delete:
		retval = Commands::PakUpdate(options);
		break;
	case E_Command_List:
		retval = Commands::PakList(options);
		break;
	case E_Command_Test:
		retval = Commands::PakTest(options);
		break;
	case E_Command_Extract:
		retval = Commands::PakExtract(options);
		break;
	default:
		LOGERROR(g_log, "Unknown command!\n");
		break;
	};

	if (options.switches.timeOperation)
	{
		QueryPerformanceCounter(&end);
		QueryPerformanceFrequency(&freq);
		float time = ((float)(end.QuadPart-start.QuadPart)) / (float)freq.QuadPart;
		LOGINFO(g_log, "Time Taken (C_Application::Process) %4.4f(s)\n",time);
	}

	return retval;
}
