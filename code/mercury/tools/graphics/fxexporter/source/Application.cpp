#include "stdafx.h"
#include <system/xml/XmlDom.h>

#include <core/maths.h>
#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>
#include <system/perf/profiler.h>
#include <system/perf/profilerbackend.h>
#include <core/threads.h>
#include <system/memory/memorytracker.h>

#include <system/serialise.h>
#include <system/serialise/binaryserialise.h>
#include <system/serialise/binaryserialiseAutoEndian.h>

#define NGL_GPU_D3D_PC 1

#include "ngl/mercury_to_ngl.h"
#include "ngl/defs.h"
#include "ngl/ngl_meshdefs.h"
#include "fx/targets/pcfx.h"
#include "fx/targets/xefx.h"
#include "fx/targets/ps3fx.h"

#include <fx/fxb.h>

#include "Application.h"

#include <System/Memory/MemoryTracker.h>
#include <System/Memory/MemoryTrackerDefines.h>
//#include "OverloadNew.h"

#ifdef PLATFORM_PC
#include "system\memory\MemoryTrackerDefines.h"
#include "system\memory\MemoryTracker.h"
#include "system\memory\AllocatorFunctions.h"

#pragma warning(disable:4996)

void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }
void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }
void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }
void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }
void* BizAlignedMalloc(size_t size, size_t alignment) { return Memory::TrackedAlignedAlloc(size,alignment); }
void BizAlignedFree(void *p) { Memory::TrackedAlignedFree(p); }
#endif


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
#ifdef PLATFORM_PC
	Memory::Tracker::Instance().DumpStatsOnExit(false);
	Memory::Tracker::Instance().DumpBlocksOnExit(true);
#endif
}

C_Application::~C_Application()
{
}

//#define WRITE_OUT_PS3_FORMAT
#define WRITE_OUT_XENON_FORMAT
//#define WRITE_OUT_PC_FORMAT

const char *gszTargetNames[] = { "pc", "xenon", "ps3", "multi", ""};
const int giTargetFlags[] = { FXB_EXPORT_PC, FXB_EXPORT_XENON, FXB_EXPORT_PS3, FXB_EXPORT_PC|FXB_EXPORT_XENON|FXB_EXPORT_PS3 };

S32 C_Application::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	C_Options options;
	bool success = true;

	// Parse our command-line options.
	success = ParseCommandLineOptions(argc, argv, options);
	if (!success)
		return 0;

	int i = 0;
	int iFlags = 0;
	while (1)
	{
		if (gszTargetNames[i][0] == 0)
		{
			LOGFATAL( m_log, "Target platform '%s' is not recognised.\n", options.target.Chars());
			exit(-1);
		}
		if (!stricmp(gszTargetNames[i], options.target.Chars()))
		{
			iFlags = giTargetFlags[i];
			break;
		}
		i++;
	}

	Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open(options.outFile, Sys::E_FileAccess_Write);
	if (!pFile)
	{
		LOGFATAL( m_log, "Failed to open file '%s' for writing.\n", options.outFile.Chars());
		exit(-1);
	}

	if (iFlags != FXB_EXPORT_PC)
		pFile->SetEndianSwapFlag(true);

	bool bResult = ExportFXB(pFile, options.inFile, iFlags, &options.defines[0], &options.includes[0], options.outFile.Chars());

	delete pFile;

	if (!bResult)
	{
		// delete incomplete file
		Sys::GetDefaultFileSystem()->DeleteFile(options.outFile);
		return -1;
	}

	return 0;
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
		// these variables are static so that the strings it parses stay in memory
		static TCLAP::CmdLine cmd("Example Command-line Tool.", ' ', "1.00");

		//Define a value argument and add it to the command line.
		static TCLAP::ValueArg<std::string> outFileArg("o", "outname", "Output binary image", true, "", "output file"  );						cmd.add( outFileArg );
		static TCLAP::UnlabeledValueArg<std::string> inFileArg("inname", "Input image filename (.bmp, .png, etc.)", "", "input file"  );		cmd.add( inFileArg );
		static TCLAP::ValueArg<std::string> targetArg("t", "target", "Output target platform (pc, xenon, ps3, multi)", false, "multi", "target"); cmd.add( targetArg );
		static TCLAP::MultiArg<std::string> includes("I", "include", "Include path to pass to the .FX compiler", false, "include"); cmd.add(includes);
		static TCLAP::MultiArg<std::string> defines("D", "define", "#define to pass to the .FX compiler", false, "define"); cmd.add(defines);

		//Parse the args.
		cmd.parse(argc, argv);

		static const char *pszEmpty = "";

		//Get the values
		options.inFile = inFileArg.getValue().c_str();
		options.outFile = outFileArg.getValue().c_str();
		options.target = targetArg.getValue().c_str();
		for (U32 i=0; i<includes.getValue().size(); i++)
			options.includes.PushBack(includes.getValue()[i].c_str());
		options.includes.PushBack(pszEmpty);
		for (U32 i=0; i<defines.getValue().size(); i++)
			options.defines.PushBack(defines.getValue()[i].c_str());
		options.defines.PushBack(pszEmpty);
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
