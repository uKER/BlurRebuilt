#include "stdafx.h"
#include <system/xml/XmlDom.h>

#include <system/debug/log/DebugLog.h>
#include <system/debug/log/subscribers/logfilesubscriber.h>
#include <system/debug/log/ExecuterFeedback.h>
#include <tools_shared/executer/ExecuterHelpers.h>
#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>
#include <animation/tools/CycleBuilder.h>
#include <animation/tools/Scene.h>
#include <animation/skeleton/Skeleton.h>

#include "Application.h"
#include "Options.h"

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
		if (!success) goto exit;

		if (!dom.Read(xmlFilePath.Chars()))
			return false;

		if (!conversionOptions.ParseXml(xmlFilePath, dom))
			return false;
	}

	// Perform export.
	{
		Tools::Shim::C_FileCopyXmlExport::ClearXmlFileList(
			conversionOptions.GetInputTempFilePath(), 
			conversionOptions.GetExecuterNodeName());

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
			conversionOptions.GetExecuterNodeName());
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
		EXECUTER_ERROR( "Error loading scene: %s\n", aftkStatus.GetErrorString().c_str() );
		return false;
	}
	
	AnimationTools::C_Scene skeletonScene;
	AnimationTools::C_Status status = skeletonScene.Import( aftkScene, options.GetSceneConfig() );
	
	if ( !status )
	{
		EXECUTER_ERROR( "Error importing animated scene: %s\n", status.GetErrorMessage().Chars() );
		return false;
	}	
	
	// Write out data for our different skus.
	for ( S32 platformNo = 0; platformNo < Tools::E_Platform_Count; platformNo++ )
    {
        Tools::E_Platform platform = static_cast< Tools::E_Platform >( platformNo );
		Util::C_String fileExt = GetSkeletonExt( platform );
		bool endianSwap = GetEndianSwap( platform );

		// using output path and asset filename.
		Util::C_String relativeOutputFile = options.GetOutputPath().IReplace( options.GetRootTempPath(), "" ) + options.GetOutputFilename();
		relativeOutputFile = Util::RemoveFileExtension( relativeOutputFile );

		Util::C_String outputFilePath = 
			options.GetCopyLocations()[ platformNo ].workingPath + relativeOutputFile + "." + fileExt;

		// Create output folder.
		success = Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(outputFilePath));
		if ( !success )
		{
			return false;
		}
		
		uberCopyList.AddFileCopyItem(
			platform,
			Tools::FileCopy::C_FileCopyItem( (const char*) outputFilePath.Chars(), "" ) );
		
		Sys::I_File* m_pFile = Sys::C_FileSystem::Instance().Open(
			outputFilePath.Chars(), 
			Sys::E_FileAccess_Write);
		
		if (!m_pFile)
		{
			EXECUTER_ERROR( "Error writing skeleton file '%s'\n", outputFilePath.Chars() );
			success = false;
			return false;
		}
		m_pFile->SetEndianSwapFlag( endianSwap );
		
		status = AnimationTools::SerialiseSkeleton( skeletonScene, *m_pFile );
				
		delete m_pFile;
		m_pFile = 0;
		
		if ( !status )
		{
			EXECUTER_ERROR( "Error serialising skeleton file '%s'\n", outputFilePath.Chars() );
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
		EXECUTER_ERROR( "Error loading cycle scene '%s': %s\n", options.GetInputCycleFilePath().Chars(), aftkStatus.GetErrorString().c_str() );
		return false;
	}
	
	Aftk::C_Scene aftkSubtractedCycleScene;
	
	if ( options.IsAdditive() )
	{
		aftkStatus = aftkSubtractedCycleScene.Read( options.GetInputSubtractedCycleFilePath().Chars() );
		
		if ( !aftkStatus )
		{
			EXECUTER_ERROR( "Error loading cycle scene '%s': %s\n", options.GetInputSubtractedCycleFilePath().Chars(), aftkStatus.GetErrorString().c_str() );
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
		EXECUTER_ERROR( "Error importing scene: %s\n", status.GetErrorMessage().Chars() );
		return false;
	}

	if ( options.IsAdditive() )
	{
		status = subtractedCycleScene.Import( aftkCycleScene, sceneConfig );
		if ( !status )
		{
			EXECUTER_ERROR( "Error importing scene: %s\n", status.GetErrorMessage().Chars() );
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
		EXECUTER_ERROR( "Error building cycle: %s\n", status.GetErrorMessage().Chars() );
		return false;
	}

	// Write out data for our different skus.
	for ( S32 platformNo = 0; platformNo < Tools::E_Platform_Count; platformNo++ )
    {
        Tools::E_Platform platform = static_cast< Tools::E_Platform >( platformNo );
		Util::C_String fileExt = GetCycleExt( platform );
		bool endianSwap = GetEndianSwap( platform );
		
		// using output path and asset filename.
		Util::C_String relativeOutputFile = options.GetOutputPath().IReplace( options.GetRootTempPath(), "" ) + options.GetOutputFilename();
		relativeOutputFile = Util::RemoveFileExtension( relativeOutputFile );

		Util::C_String outputFilePath = 
			options.GetCopyLocations()[ platformNo ].workingPath + relativeOutputFile + "." + fileExt;

		// Create output folder.
		Util::C_String outDir = Util::ExtractDirectory(outputFilePath);
		success = Sys::C_FileSystem::Instance().CreateDirectory(outDir);
		if ( !success )
		{
			EXECUTER_ERROR( "Error creating output folder '%s'\n", outDir.Chars() );
			return false;
		}
		
		uberCopyList.AddFileCopyItem(
			platform,
			Tools::FileCopy::C_FileCopyItem( (const char*) outputFilePath.Chars(), "" ) );
		
		Sys::I_File* m_pFile = Sys::C_FileSystem::Instance().Open(
			outputFilePath.Chars(), 
			Sys::E_FileAccess_Write);
		
		if (!m_pFile)
		{
			EXECUTER_ERROR( "Error creating output file '%s'\n", outputFilePath.Chars() );
			success = false;
			return false;
		}
		m_pFile->SetEndianSwapFlag( endianSwap );
		
		cycleBuilder.Serialise( *m_pFile );
		
		delete m_pFile;
		m_pFile = 0;
	}

	return true;
}

Util::C_String C_Application::GetCycleExt( Tools::E_Platform platform ) const
{
	return "cycle";
}

Util::C_String C_Application::GetSkeletonExt( Tools::E_Platform platform ) const
{
	return "skeleton";
}

bool C_Application::GetEndianSwap( Tools::E_Platform platform ) const
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