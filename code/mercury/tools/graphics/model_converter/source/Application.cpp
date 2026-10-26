#include "stdafx.h"
#include <system/xml/XmlDom.h>

#include <system/debug/log/debuglog.h>
#include <system/debug/log/subscribers/logfilesubscriber.h>
#include <system/debug/log/ExecuterFeedback.h>
#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>

#include "game_shared/ExampleExportData.h"

#include "Options.h"
#include "Application.h"
#include "ExampleConverter.h"

#ifndef _DEBUG
#define MEMORY_TRACKER_CALLSTACK 0
#endif

#ifdef PLATFORM_PC
#include "system\memory\MemoryTrackerDefines.h"
#include "system\memory\MemoryTracker.h"
#include "system\memory\AllocatorFunctions.h"

#if 0	/// ENABLE THIS FOR MEMORY TRACKING
void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }
void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }
void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }
void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }
#endif
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
#ifdef CFG_DEBUG
	Memory::Tracker::Instance().DumpStatsOnExit(false);
	Memory::Tracker::Instance().DumpBlocksOnExit(true);
#endif
#endif
}

C_Application::~C_Application()
{
}

S32 C_Application::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	Btl::AutoPointer<Progress::UpdateSimple> progress = new Progress::UpdateSimple(pProgress, "Main");
	m_pProgress = &*progress;

	C_Options conversionOptions;
	bool success = true;

	//Build options struct
	{
		Util::C_String xmlFilePath;
		//Parse our command-line options.
		success = conversionOptions.ParseCommandLine(xmlFilePath, argc, argv);

		if (!success) goto exit;	
/* // JAN - took this out for now so that we can debug without passing in executer .xml files
		LOGINFO(Tools::g_pMyApplication->GetLogger(), "Parsing: %s\n", xmlFilePath.Chars());
		
		Sys::C_XmlDom dom;
		success = dom.Read(xmlFilePath.Chars());
		if (!success) goto exit;

		success = conversionOptions.ParseXml(xmlFilePath, dom);
		if (!success) goto exit;
*/
	}

	//Perform expot.
	{
		Tools::Shim::C_FileCopyXmlExport::ClearXmlFileList(
			conversionOptions.GetInputTempFilePath(), "");

		Tools::FileCopy::C_FileCopyList uberCopyList;
		success = ExportData(uberCopyList, conversionOptions);
		if (!success) goto exit;

#if 0
		success = Tools::Shim::C_FileCopyXmlExport::ExportXmlFileList(
			uberCopyList, 
			conversionOptions.GetInputTempFilePath(),
			conversionOptions);
#endif
		if (!success) goto exit;
	}

exit:
	if (success)
	{
		EXECUTER_INFO("Processing completed succesfully!\n");
	}
	else
	{
		EXECUTER_INFO("Processing failed!\n");
	}

	if (success)
		return 0;
	else
		return 1;
}

bool C_Application::ExportData(Tools::FileCopy::C_FileCopyList& uberCopyList, const C_Options& options)
{
	bool success;
	//Our exported data container.
	C_ExampleExportData exampleExportData;

	m_pProgress->SetProgress(0.f);

	success = Aftk::C_Global::Create();
	if (!success) goto exit;
	{
		Aftk::C_Scene aftkScene;

		//Read in input data.
		success = aftkScene.Read(options.GetInputTempFilePath().Chars());
		if (!success) goto exit;

		//Convert our particle data into our in-memory format.
		success = C_ExampleConverter::Convert(
			exampleExportData, 
			aftkScene,
			options);
		if (!success) goto exit;
	}

	m_pProgress->SetProgress(25.f);

	//Write out data for our different skus.
	for(S32 platformNo=0; platformNo < Tools::E_Platform_Count; platformNo++)
    {
		m_pProgress->SetProgress(Maths::Lerp(25.f, 100.f, float(platformNo) / float(Tools::E_Platform_Count)));

        Util::C_String fileExt;
        
		bool endianSwap;

		//Define additional platforms here if required.
        switch(platformNo)
        {
            case Tools::E_Platform_PC:
                fileExt = "examplepc";
				endianSwap = false;
            break;
            
            case Tools::E_Platform_360:
				fileExt = "example360";
				endianSwap = true;
            break;
            
            case Tools::E_Platform_PS3:
				fileExt = "exampleps3";
				endianSwap = true;
            break;
		}

		Util::C_String relativeOutputFile = options.GetInputTempFilePath().IReplace(options.GetRootTempPath(), "");
		relativeOutputFile = Util::RemoveFileExtension(relativeOutputFile);
		Util::C_String outputFilePath = 
			options.GetCopyLocations()[platformNo].workingPath + relativeOutputFile + "." + fileExt;

		//Create output folder.
		success = Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(outputFilePath));
		if (!success) goto exit;
		
		uberCopyList.AddFileCopyItem(
			(Tools::E_Platform)platformNo,
			Tools::FileCopy::C_FileCopyItem((const char*)outputFilePath.Chars(), ""));
		Sys::I_File* m_pFile = Sys::C_FileSystem::Instance().Open(
			outputFilePath.Chars(), 
			Sys::E_FileAccess_Write);
		if (!m_pFile)
		{
			success = false;
			goto exit;
		}
		m_pFile->SetEndianSwapFlag(endianSwap);

		success = exampleExportData.Serialise(*m_pFile);
		if (!success) goto exit;

		delete m_pFile;
		m_pFile = 0;

		//Verify we can read the file back in as a sanity check if we're debugging.
#ifdef BUILD_CONFIG_DEBUG
		if (platformNo == Tools::E_Platform_PC)
		{
			m_pFile = Sys::C_FileSystem::Instance().Open(
				outputFilePath.Chars(), 
				Sys::E_FileAccess_Read);
			if (!m_pFile)
			{
				success = false;
				goto exit;
			}

			C_ExampleExportData data;
			success = data.Deserialise(*m_pFile);
			if (!success) goto exit;

			delete m_pFile;
			m_pFile = 0;
		}
#endif
	}

exit:
	Aftk::C_Global::Destroy();

	m_pProgress->SetProgress(100.f);

	return success;
}