#include "stdafx.h"
#include <system/xml/XmlDom.h>

#include <system/debug/log/debuglog.h>
#include <system/debug/log/subscribers/logfilesubscriber.h>
#include <system/debug/log/ExecuterFeedback.h>
#include <system/time/time.h>
#include <system/memory/MemoryTracker.h>

#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>

#include <converter_shared/ProcessGraph.h>

#include <Application.h>

#include <tex_atlas/TexAtlas.h>

#include <data/AftkSceneData.h>
#include <data/ConversionOptions.h>
#include <data/DataTypes.h>
#include <data/NullData.h>
#include <data/PhysicsData.h>
#include <data/RenderData.h>
#include <data/SceneDescription.h>
#include <data/LightListData.h>
#include <data/LightSamplePointsData.h>
#include <data/PvsData.h>
#include <data/NavMeshData.h>
#include <data/CubemapPointsData.h>
#include <data/PortalData.h>
#include <data/CrowdData.h>
#include <data/CropSheetData.h>
#include <data/AdvertisingData.h>

#include <process/BuildPhysicsData.h>
#include <process/BuildRenderData.h>
#include <process/BuildSceneDescription.h>
#include <process/CreateLightSamplePointsFile.h>
#include <process/ExportGameScene.h>
#include <process/LightAftk.h>
#include <process/NullProcess.h>
#include <process/PreprocessAftk.h>
#include <process/ReadLitAftk.h>
#include <process/WritePreprocessedAftk.h>
#include <process/LightList.h>
#include <process/ReadLightSamplePointsFile.h>
#include <process/BuildNavMeshData.h>
#include <process/CreatePvsSamplePointsFile.h>
#include <process/GeneratePvsData.h>
#include <process/ReadPvsData.h>
#include <process/BuildCubemapPointsData.h>
#include <process/BuildPortalData.h>
#include <process/BuildCrowdData.h>
#include <process/GenerateCropSheets.h>
#include <process/BuildAdvertisingData.h>

#include <resource.h>
#include <Debug.h>

void ChangeIcon( const HICON hNewIcon )
{
   // Load kernel 32 library
   HMODULE hMod = LoadLibrary( _T( "Kernel32.dll" ));
   assert( hMod ); 

   // Load console icon changing procedure
   typedef DWORD ( __stdcall *SCI )( HICON );
   SCI pfnSetConsoleIcon = reinterpret_cast<SCI>( GetProcAddress( hMod, "SetConsoleIcon" ));
   if( pfnSetConsoleIcon )
   {	  
	   // Call function to change icon
	   pfnSetConsoleIcon( hNewIcon ); 
   }
   FreeLibrary( hMod );
}

void SetConsoleIcon( S32 icon )
{
	HMODULE hMainMod = GetModuleHandle( 0 );

	HICON hMainIcon = ::LoadIcon( hMainMod, MAKEINTRESOURCE( icon ));

	// Change main window icon
	ChangeIcon( hMainIcon );

}

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
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute( consoleHandle, FOREGROUND_GREEN | FOREGROUND_INTENSITY );

	SetConsoleIcon( IDI_ICON1 );

	m_pAppMetaDebug = new C_AppMetaDebug;
}

C_Application::~C_Application()
{
	SAFE_DELETE_PTR(m_pAppMetaDebug);
	ChangeIcon(0);
}

void AddInput( Tools::ProcessGraph::C_Process& process, Tools::ProcessGraph::C_Data* pData, E_GraphData dataType )
{
	Util::C_String input = "Input:";
	process.AddInput( pData, dataType, input + GraphDataName( dataType ) );
}

void AddOutput( Tools::ProcessGraph::C_Process& process, Tools::ProcessGraph::C_Data* pData, E_GraphData dataType )
{
	Util::C_String output = "Output:";
	process.AddOutput( pData, dataType, output + GraphDataName( dataType ) );
}

S32 C_Application::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{	
	MEMORY_TRACKER_PUSH("ApplicationMain");

	bool success = Aftk::C_Global::Create();
	if(!success) goto exit;
	
	// see /docs/sceneprocessingflow.jpg
	{
		C_DataConversionOptions conversionOptions;
		success = conversionOptions.SetupFromCommandLine( argc, argv );
		if(!success) goto exit;

		C_DataAftkScene preprocessedAftk;
		C_DataNull preprocessedAftkFile("PreprocessedAftkFile");
		C_DataNull lightSamplePointsFile("LightSamplePointsFile");
		C_DataNull litAftkFile("LitAftkFile");
		C_DataNull lightSamplesFile("LightSamplesFile");
		C_DataAftkScene litAftk;
		C_DataSceneDescription sceneDescription;
		C_DataLightSamplePoints lightSamples;
		C_DataNull pvsSamplePointsFile("PvsSamplePointsFile");
		C_DataNull pvsDataFile("PvsDataFile");
		C_DataPvs pvsData;
		C_DataRenderData renderData( conversionOptions.GetToolConfig().GetTextureCachePath(), conversionOptions.GetToolConfig().GetFxbPath() );
		C_DataPortalData portalRooms;
		C_DataPhysicsData physicsData;
		C_DataLightList lightList;
		C_DataNavMesh navMesh;
		C_DataCrowd crowdData;
		C_DataNull gameSceneFile("GameSceneFile");
		C_DataCubemapPoints cubemapPoints;
		C_DataCropSheet cropSheets;
		C_DataAdvertising advertData;

		//preventing litaftk from being destroyed too early. 
		litAftk.IncRefCount();

		bool reuseLastPreprocessedLevel = conversionOptions.ReuseLastPreprocessedLevel();
		bool lightScene = conversionOptions.GetLightingOptions().GetLightScene();
		bool visibilityScene = conversionOptions.GetVisibilityOptions().GetVisibilityScene();
		bool reuseLastLitLevel = conversionOptions.ReuseLastLitLevel();
		bool reuseLastVisibilityLevel = conversionOptions.ReuseLastVisibilityLevel();
		bool exportRenderData = conversionOptions.ExportRenderData();
		bool exportPhysicsData = conversionOptions.ExportPhysicsData();
		bool exportCrowdData = conversionOptions.ExportCrowdData();

		// Set node initial states
		conversionOptions.SetProcessedState( E_GraphData_ConversionOptions );

		if( reuseLastPreprocessedLevel )
		{
			preprocessedAftkFile.SetProcessedState( E_GraphData_PreprocessedAftkFile );
		}
		
		if( reuseLastLitLevel )
		{
			litAftkFile.SetProcessedState( E_GraphData_LitAftkFile );
			lightSamplesFile.SetProcessedState( E_GraphData_LightSamplesFile );
		}

		if (reuseLastVisibilityLevel)
		{
			pvsDataFile.SetProcessedState( E_GraphData_PvsDataFile);
		}



		// Build process graph

		C_ProcessPreprocessAftk processPreprocessAftk;
		AddInput( processPreprocessAftk, &conversionOptions, E_GraphData_ConversionOptions );
		AddOutput( processPreprocessAftk, &preprocessedAftk, E_GraphData_PreprocessedAftk );

		C_ProcessCreateLightSamplePointsFile processCreateLightSamplePointsFile;
		AddInput( processCreateLightSamplePointsFile, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processCreateLightSamplePointsFile, &preprocessedAftk, E_GraphData_PreprocessedAftk );
		AddOutput( processCreateLightSamplePointsFile, &lightSamplePointsFile, E_GraphData_LightSamplePointsFile );

		C_ProcessWritePreprocessedAftk processWritePreprocessedAftk;
		AddInput( processWritePreprocessedAftk, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processWritePreprocessedAftk, &preprocessedAftk, E_GraphData_PreprocessedAftk );
		AddOutput( processWritePreprocessedAftk,  &preprocessedAftkFile, E_GraphData_PreprocessedAftkFile );

		C_ProcessLightAftk processLightAftk;
		AddInput( processLightAftk, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processLightAftk, &lightSamplePointsFile, E_GraphData_LightSamplePointsFile );
		AddInput( processLightAftk, &preprocessedAftkFile, E_GraphData_PreprocessedAftkFile );
		AddInput( processLightAftk, &preprocessedAftk, E_GraphData_PreprocessedAftk );
		AddOutput( processLightAftk, &litAftkFile, E_GraphData_LitAftkFile );
		AddOutput( processLightAftk, &lightSamplesFile, E_GraphData_LightSamplesFile );

		C_ProcessReadLitAftk processReadLitAftk;
		AddInput( processReadLitAftk, &conversionOptions, E_GraphData_ConversionOptions );
		if(lightScene)
		{
			AddInput( processReadLitAftk, &litAftkFile, E_GraphData_LitAftkFile );
		}
		else
		{
			AddInput( processReadLitAftk, &preprocessedAftkFile, E_GraphData_PreprocessedAftkFile );
		}
		AddOutput( processReadLitAftk, &litAftk, E_GraphData_LitAftk );

		C_ProcessBuildAdvertisingData processBuildAdvertData;
		AddInput( processBuildAdvertData, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processBuildAdvertData, &litAftk, E_GraphData_LitAftk );
		AddOutput( processBuildAdvertData, &advertData, E_GraphData_AdvertisingData );

		C_ProcessBuildSceneDescription processBuildSceneDescription;
		AddInput( processBuildSceneDescription, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processBuildSceneDescription, &litAftk, E_GraphData_LitAftk );
		AddOutput( processBuildSceneDescription, &sceneDescription, E_GraphData_SceneDescription );

		C_ProcessGenerateCropSheets processGenerateCropSheet;
		AddInput( processGenerateCropSheet, &conversionOptions, E_GraphData_ConversionOptions);
		AddInput( processGenerateCropSheet, &sceneDescription, E_GraphData_SceneDescription );
		AddInput( processGenerateCropSheet, &litAftk, E_GraphData_LitAftk );
		AddOutput( processGenerateCropSheet,  &cropSheets, E_GraphData_CropSheet );

		C_ProcessReadLightSamplePointsFile processReadLightSamplesFile;
		AddInput( processReadLightSamplesFile, &conversionOptions, E_GraphData_ConversionOptions );
		if(lightScene)
		{
			AddInput( processReadLightSamplesFile, &lightSamplesFile, E_GraphData_LightSamplesFile );
		}
		AddOutput( processReadLightSamplesFile, &lightSamples, E_GraphData_LightSamples );

		C_ProcessCreatePvsSamplePointsFile processCreatePvsSamplePointsFile;
		AddInput( processCreatePvsSamplePointsFile, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processCreatePvsSamplePointsFile, &sceneDescription, E_GraphData_SceneDescription );
		if(lightScene)
		{
			AddInput( processCreatePvsSamplePointsFile, &litAftkFile, E_GraphData_LitAftkFile );
		}
		else
		{
			AddInput( processCreatePvsSamplePointsFile, &preprocessedAftkFile, E_GraphData_PreprocessedAftkFile );
		}
		AddOutput( processCreatePvsSamplePointsFile, &pvsSamplePointsFile, E_GraphData_PvsSamplePointsFile );

		C_ProcessGeneratePvsData processGeneratePvsData;
		AddInput( processGeneratePvsData, &conversionOptions, E_GraphData_ConversionOptions );
		if(lightScene)
		{
			AddInput( processGeneratePvsData, &litAftkFile, E_GraphData_LitAftkFile );
		}
		else
		{
			AddInput( processGeneratePvsData, &preprocessedAftkFile, E_GraphData_PreprocessedAftkFile );
		}
		AddInput( processGeneratePvsData, &pvsSamplePointsFile, E_GraphData_PvsSamplePointsFile );
		AddOutput( processGeneratePvsData, &pvsDataFile, E_GraphData_PvsDataFile );
	
		C_ProcessReadPvsData processReadPvsData;
		AddInput( processReadPvsData, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processReadPvsData, &pvsDataFile, E_GraphData_PvsDataFile );
		AddOutput( processReadPvsData, &pvsData, E_GraphData_PvsData );

		C_ProcessBuildPortalData processBuildPortalRooms;
		AddInput( processBuildPortalRooms, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processBuildPortalRooms, &sceneDescription, E_GraphData_SceneDescription );
		AddOutput( processBuildPortalRooms, &portalRooms, E_GraphData_PortalRooms );

		C_ProcessBuildCubemapPoints processBuildCubemapPoints;
		AddInput( processBuildCubemapPoints, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processBuildCubemapPoints, &litAftk, E_GraphData_LitAftk );
		AddInput( processBuildCubemapPoints, &portalRooms, E_GraphData_PortalRooms );
		AddOutput( processBuildCubemapPoints, &cubemapPoints, E_GraphData_CubemapPoints );

		C_ProcessBuildRenderData processBuildRenderData;
		AddInput( processBuildRenderData, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processBuildRenderData, &sceneDescription, E_GraphData_SceneDescription ); 
		if (visibilityScene)
			AddInput( processBuildRenderData, &pvsData, E_GraphData_PvsData );
		AddInput( processBuildRenderData, &portalRooms, E_GraphData_PortalRooms );
		AddInput( processBuildRenderData, &cubemapPoints, E_GraphData_CubemapPoints );
		AddInput( processBuildRenderData, &cropSheets, E_GraphData_CropSheet );
		AddOutput( processBuildRenderData, &renderData, E_GraphData_RenderData );

		C_ProcessBuildPhysicsData processBuildPhysicsData;
		AddInput( processBuildPhysicsData, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processBuildPhysicsData, &sceneDescription, E_GraphData_SceneDescription );
		AddOutput( processBuildPhysicsData, &physicsData, E_GraphData_PhysicsData );

		C_ProcessLightList processBuildLightList;
		AddInput( processBuildLightList, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processBuildLightList, &sceneDescription, E_GraphData_SceneDescription );
		AddOutput( processBuildLightList, &lightList, E_GraphData_LightList );

		C_ProcessBuildNavMeshData processBuildNavMesh;
		AddInput( processBuildNavMesh, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processBuildNavMesh, &sceneDescription, E_GraphData_SceneDescription );
		AddOutput( processBuildNavMesh, &navMesh, E_GraphData_NavMesh );

		C_ProcessBuildCrowdData processBuildCrowdData;
		AddInput( processBuildCrowdData, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processBuildCrowdData, &sceneDescription, E_GraphData_SceneDescription );
		if (visibilityScene)
			AddInput( processBuildCrowdData, &pvsData, E_GraphData_PvsData );
		AddOutput( processBuildCrowdData, &crowdData, E_GraphData_CrowdData );

		C_ProcessExportGameScene processExportGameScene;
		AddInput( processExportGameScene, &conversionOptions, E_GraphData_ConversionOptions );
		AddInput( processExportGameScene, &sceneDescription, E_GraphData_SceneDescription );
		AddInput( processExportGameScene, &navMesh, E_GraphData_NavMesh );
		AddInput( processExportGameScene, &advertData, E_GraphData_AdvertisingData );
		if(exportRenderData)
		{
			AddInput( processExportGameScene, &lightSamples, E_GraphData_LightSamples );
			AddInput( processExportGameScene, &portalRooms, E_GraphData_PortalRooms );
			AddInput( processExportGameScene, &lightList, E_GraphData_LightList );
			AddInput( processExportGameScene, &renderData, E_GraphData_RenderData );
			AddInput( processExportGameScene, &cubemapPoints, E_GraphData_CubemapPoints );
			if (visibilityScene)
				AddInput( processExportGameScene, &pvsData, E_GraphData_PvsData );
		}
		if(exportPhysicsData)
		{
			AddInput( processExportGameScene, &physicsData, E_GraphData_PhysicsData );
		}
		if(exportCrowdData)
		{
			AddInput( processExportGameScene, &crowdData, E_GraphData_CrowdData );
		}
		AddOutput( processExportGameScene, &gameSceneFile, E_GraphData_GameSceneFile );

		//gameSceneFile.DumpGraph();

		success = gameSceneFile.Build( E_GraphData_GameSceneFile, "Convert Scene", pProgress );
	}
	

exit:
	Aftk::C_Global::Destroy();

	m_pAppMetaDebug->GetRootNode()->Dump();

	if (success)
	{
		EXECUTER_INFO("Processing completed succesfully!\n");
	}
	else
	{
		EXECUTER_INFO("Processing failed!\n");
	}

	MEMORY_TRACKER_POP("ApplicationMain");

	if (success)
		return 0;
	else
		return 1;


}

