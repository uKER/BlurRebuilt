#include "stdafx.h"
#include <system/xml/XmlDom.h>

#include <system/debug/log/DebugLog.h>
#include <system/debug/log/subscribers/logfilesubscriber.h>
#include <system/debug/log/ExecuterFeedback.h>
#include <tools_shared/executer/ExecuterHelpers.h>
#include <tools_shared/shim/Shim.h>
#include <tools_shared/shim/ShimErrorHelpers.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>
#include <animation/tools/CycleBuilder.h>
#include <animation/tools/Scene.h>
#include <animation/skeleton/Skeleton.h>
#include <converter_shared/helpers/ExportHelpers.h>

#include "Application.h"
#include "Options.h"

#ifndef WIN64
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='X86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
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
}

C_Application::~C_Application()
{
}

S32 C_Application::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	C_Options conversionOptions;
	bool success = true;

	Sys::C_XmlDom dom;
	
	// Build options struct
	{
		//Parse our command-line options.
		Util::C_String xmlFilePath;
		success = conversionOptions.ParseCommandLine(xmlFilePath, argc, argv);
		if (!success)
		{
			goto exit;
		}

		success = dom.Read(xmlFilePath.Chars());
		if (!success)
		{
			goto exit;
		}

		success = conversionOptions.ParseXml(xmlFilePath, dom);
		if (!success)
		{
			goto exit;
		}
	}

	// Perform export.
	{
		Tools::Shim::C_FileCopyXmlExport::ClearXmlFileList(
			conversionOptions.GetInputTempFilePath(), 
			conversionOptions.GetExportName());

		Tools::FileCopy::C_FileCopyList uberCopyList;

		// Execute the conversion.
		success = ExportData( uberCopyList, conversionOptions ); 
		if (!success) goto exit;

		success = Tools::Shim::C_FileCopyXmlExport::ExportXmlFileList(
			conversionOptions.GetRootTempPath(),
			uberCopyList, 
			conversionOptions.GetInputTempFilePath(),
			conversionOptions.GetCopyLocations(),
			conversionOptions.GetLaunchComputerName(),
			conversionOptions.GetExportName());
		if (!success) goto exit;	
	}

exit:
	if (success)
	{
		EXECUTER_INFO("Animation converter processing completed succesfully!\n");
	}
	else
	{
		EXECUTER_INFO("Animation converter processing failed!\n");
	}

	if (success)
		return 0;
	else
		return 1;
}

void C_Application::RemoveSceneZFlip( Aftk::C_Scene& scene )
{
	// Remove Maya Z-flip.
	Aftk::Matrix44 identityWorldMatrix;
	UtMaths::SetIdentity( &identityWorldMatrix );
	scene.GetRootSceneNodePtr()->SetWorldMatrix( identityWorldMatrix );
}


bool C_Application::ExportData( Tools::FileCopy::C_FileCopyList& uberCopyList, const C_Options& options )
{
	bool success = Aftk::C_Global::Create();
	
	if ( !success )
	{
		return false;
	}
	
	switch ( options.GetMode() )
	{
		case C_Options::E_Mode_ExportSkeleton:
			success = ExportSkeleton( uberCopyList, options );
			break;
		
		case C_Options::E_Mode_ExportCycle:
			success = ExportCycle( uberCopyList, options );
			break;
	}
	
	Aftk::C_Global::Destroy();

	return success;
}

bool C_Application::ExportSkeleton( Tools::FileCopy::C_FileCopyList& uberCopyList, const C_Options& options )
{
	bool success = false;
	Aftk::C_Scene aftkScene;
	
	// Read in input data.
	Aftk::C_Status aftkStatus = aftkScene.Read( options.GetInputSkeletonFilePath().Chars() );
	
	if ( !aftkStatus )
	{
		ERROR_FATAL( "Error loading scene: %s\n", aftkStatus.GetErrorString().c_str() );
		return false;
	}
	
	AnimationTools::C_Scene skeletonScene;
	AnimationTools::C_Status status = skeletonScene.Import( aftkScene, options.GetSceneConfig() );
	
	if ( !status )
	{
		ERROR_FATAL( "Error importing animated scene: %s\n", status.GetErrorMessage().Chars() );
		return false;
	}	
	
	// Write out data for our different skus.
	for ( S32 platformNo = 0; platformNo < Util::E_Platform_Count; platformNo++ )
    {
        Util::E_Platform platform = static_cast< Util::E_Platform >( platformNo );
		Util::C_String fileExt = GetSkeletonExt( platform );
		bool endianSwap = GetEndianSwap( platform );

		// using output path and asset filename.
		Util::C_String relativeOutputFile = options.GetOutputPath().IReplace( options.GetRootTempPath(), "" ) + options.GetOutputFilename();
		relativeOutputFile = Util::RemoveFileExtension( relativeOutputFile );

		Util::C_String outputFilePath = 
			options.GetCopyLocations()[ platformNo ].workingPath + relativeOutputFile + "." + fileExt;
			
		Sys::I_File* file = ExportHelpers::WriteExportFile( outputFilePath, platform, &uberCopyList );
		
		if ( NULL == file )
		{
			ERROR_FATAL( "Error creating output file '%s'\n", outputFilePath.Chars() );
			success = false;
			return false;
		}

		status = AnimationTools::SerialiseSkeleton( skeletonScene, *file );
				
		delete file;
		file = 0;
		
		if ( !status )
		{
			ERROR_FATAL( "Error serialising skeleton file '%s'\n", outputFilePath.Chars() );
			return false;
		}
	}

	return true;
}

bool C_Application::ExportCycle( Tools::FileCopy::C_FileCopyList& uberCopyList, const C_Options& options )
{
	bool success = false;
	
	//
	// Read in cycle scene and build cycle.
	//
	Aftk::C_Scene aftkCycleScene;
	Aftk::C_Status aftkStatus = aftkCycleScene.Read( options.GetInputCycleFilePath().Chars() );
	if ( !aftkStatus )
	{
		ERROR_FATAL( "Error loading cycle scene '%s': %s\n", options.GetInputCycleFilePath().Chars(), aftkStatus.GetErrorString().c_str() );
		return false;
	}
	
	Aftk::C_Scene aftkSubtractedCycleScene;
	
	if ( options.IsAdditive() )
	{
		aftkStatus = aftkSubtractedCycleScene.Read( options.GetInputSubtractedCycleFilePath().Chars() );
		
		if ( !aftkStatus )
		{
			ERROR_FATAL( "Error loading cycle scene '%s': %s\n", options.GetInputSubtractedCycleFilePath().Chars(), aftkStatus.GetErrorString().c_str() );
			return false;
		}
	}
	
	AnimationTools::C_CycleBuilder cycleBuilder;
	AnimationTools::C_SceneConfig sceneConfig( options.GetSceneConfig() );
	AnimationTools::C_Scene cycleScene;
	AnimationTools::C_Scene subtractedCycleScene;
	
	AnimationTools::C_Status status;
	status = cycleScene.Import( aftkCycleScene, sceneConfig );
	if ( !status )
	{
		ERROR_FATAL( "Error importing scene: %s\n", status.GetErrorMessage().Chars() );
		return false;
	}

	if ( options.IsAdditive() )
	{
		status = subtractedCycleScene.Import( aftkSubtractedCycleScene, sceneConfig );
		if ( !status )
		{
			ERROR_FATAL( "Error importing scene: %s\n", status.GetErrorMessage().Chars() );
			return false;
		}
	}
	
	if ( options.IsAdditive() )
	{
		status = cycleBuilder.BuildAdditiveCycle(
			cycleScene,
			subtractedCycleScene,
			options.GetCycleParameters()
		);
	}
	else
	{
		status = cycleBuilder.BuildCycle(
			cycleScene,
			options.GetCycleParameters()
		);
	}
	
	if ( !status )
	{
		ERROR_FATAL( "Error building cycle: %s\n", status.GetErrorMessage().Chars() );
		return false;
	}

	// Write out data for our different skus.
	for ( S32 platformNo = 0; platformNo < Util::E_Platform_Count; platformNo++ )
    {
        Util::E_Platform platform = static_cast< Util::E_Platform >( platformNo );
		Util::C_String fileExt = GetCycleExt( platform );
		bool endianSwap = GetEndianSwap( platform );
		
		// using output path and asset filename.
		Util::C_String relativeOutputFile = options.GetOutputPath().IReplace( options.GetRootTempPath(), "" ) + options.GetOutputFilename();
		relativeOutputFile = Util::RemoveFileExtension( relativeOutputFile );

		Util::C_String outputFilePath = 
			options.GetCopyLocations()[ platformNo ].workingPath + relativeOutputFile + "." + fileExt;
			
			
		Sys::I_File* file = ExportHelpers::WriteExportFile( outputFilePath, platform, &uberCopyList );
		if (!file)
		{
			ERROR_FATAL( "Error creating output file '%s'\n", outputFilePath.Chars() );
			success = false;
			return false;
		}
				
		cycleBuilder.Serialise( *file );
		
		delete file;
		file = 0;
	}

	return true;
}

Util::C_String C_Application::GetCycleExt( Util::E_Platform platform ) const
{
	return "cycle";
}

Util::C_String C_Application::GetSkeletonExt( Util::E_Platform platform ) const
{
	return "skeleton";
}

bool C_Application::GetEndianSwap( Util::E_Platform platform ) const
{
#if 1
	return true;
#else
	switch ( platform )
    {
        case Tools::E_Platform_PC:	return false; break;
        case Tools::E_Platform_360:	return true; break;
	    case Tools::E_Platform_PS3: return true; break;
	    default: break;
	}
	return false;
#endif
}