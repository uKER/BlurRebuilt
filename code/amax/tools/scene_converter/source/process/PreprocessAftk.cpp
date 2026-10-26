#include "stdafx.h"

#include <btl/Utility/scopedpointer.h>

#include <AftkUt/AftkUt.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <converter_shared/helpers/AftkProgress.h>
#include <converter_shared/config/PolyflagConfig.h>
#include <Polyflags.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>

#include <process/PreprocessAftk.h>

bool C_ProcessPreprocessAftk::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateMulti progressMulti(pProgress, GetName());
	Btl::ScopedPointer<Progress::I_ProgressBar> pProgressReadAftkScene( progressMulti.AddChild(1.0f) );
	Btl::ScopedPointer<Progress::I_ProgressBar> pProgressProcessAftk( progressMulti.AddChild(1.0f) );

	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	C_DataAftkScene* pOutputPreprocessedAftk = (C_DataAftkScene*)GetOutput( E_GraphData_PreprocessedAftk );
	assert(pOutputPreprocessedAftk);

	if (PreprocesssedFileUpToDate(pInputConversionOptions) && !pInputConversionOptions->ForceFullExport())
	{
		LOGINFO( Tools::g_pMyApplication->GetLogger(), "PreProcessAftk, file %s not preprocessed, no change detected\n", pInputConversionOptions->GetSourceAftkFilename().Chars());
		pOutputPreprocessedAftk->SetValid(false);
		return true;
	}

	Aftk::C_Scene& aftkScene = pOutputPreprocessedAftk->GetAftkScene();
	Util::C_String aftkFilename = pInputConversionOptions->GetSourceAftkFilename();    

	// Read Aftk Scene
	bool success = AftkHelpers::ReadAftkScene( aftkScene, aftkFilename, &*pProgressReadAftkScene );
	if(!success)
		return false;

	success = ProcessAftk( aftkScene, &*pProgressProcessAftk );
	if(!success)
		return false;

	return true;
}

const Util::C_String C_ProcessPreprocessAftk::GetName() const 
{
	return Util::C_String("C_ProcessPreprocessAftk");
}

bool C_ProcessPreprocessAftk::ProcessAftk( Aftk::C_Scene& aftkScene, Progress::I_ProgressBar* pProgress )
{
	bool success = true;

	Progress::UpdateSimple progressSimple(pProgress, GetName() + "ProcessAftk" );

	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	// apply global scene transform
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Apply Global Scene Transform.\n");

		const F32 globalScale = 1.0f; // TODO: [PaulM] read this from executer settings
		const bool rotateY180 = false;

		success = AftkHelpers::ApplyGlobalSceneTransform( aftkScene, globalScale, rotateY180 );
	}
	if (!success) return false;

	// triangulate
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Triangulate.\n");
		success = Aftk::C_SceneNodeVisit::Traverse( &aftkScene, Aftk::C_MeshTriangulate::C_TriangulateVisitor());
	}
	if (!success) return false;

	// calculate tangents and binormals
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Calculate Tangents And Binormals.\n");
		Aftk::C_MeshCalculateTangents::C_CalculateTangentsVisitor visitor;
		visitor.normalise = false;
		visitor.useVertexNormal = false;
		success = Aftk::C_SceneNodeVisit::Traverse(
			&aftkScene, 
			visitor,		
			0);
	}
	if (!success) return false;

	// make double sided
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Make Double Sided.\n");
		Util::C_String doubleSidedFlagName = pInputConversionOptions->GetPolyflagConfig().GetItem( E_Polyflag_DoubleSided ).GetFirstFlagId();
		success = Aftk::C_SceneNodeVisit::Traverse( &aftkScene, AftkUt::C_MakeDoubleSidedPolygons::C_MakeDoubleSidedPolygonsVisitor( doubleSidedFlagName ), 0);
	}
	if (!success) return false;

	// Iterate scene and generate speedtree meshes
	//{
	//	AftkSpeedTree::SpeedTree speedTree;

	//	// setup base path and lighting
	//	speedTree.m_sBasePath = "";
	//	speedTree.m_nLightmapResolution = 1;
	//	speedTree.m_bGenerateLeaves = false;
	//	speedTree.m_bDoubleSidedFronds = true;
	//	//speedTree.m_bGenerateFronds = false;
	//	speedTree.m_bGenerateBillboards = false;

	//	// look for trees in scene and generate them
	//	speedTree.ProcessAftk(aftkScene);
	//}

	// TODO: Flatten Small Instances

	// TODO: Weld Vertices
	// TODO: Remove Degenerates

	// TODO: Expand Unique Remapped Instances

	{
		// Remove base instances.
		AftkUt::C_RemoveBaseInstances::RemoveBaseInstances(&aftkScene);
		AftkUt::C_ReplaceWithActorNodes::ReplaceWithActorNodes(&aftkScene);
	}

	{
		// Warn about ambiguous node names
		AftkUt::C_FindAmbiguousActorNames::C_FindClashingActorNames(&aftkScene);
	}

	// if using basis lighting, remove vertex lighting
	//{
	//	RemoveVertexLighting(aftkScene);
	//}

	if (aftkScene.Validate() != 0)
		LOGWARN( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Scene Invalid.\n");

	return success;
}

bool C_ProcessPreprocessAftk::PreprocesssedFileUpToDate( const C_DataConversionOptions* pConversionOptions ) const
{
	Sys::C_Fileinfo sourceAftkFileDate;
	Sys::C_Fileinfo preprocessedAftkFileData;
	Sys::C_Fileinfo sceneConverterFileDate;
	

	if (!Sys::C_FileSystem::Instance().FileInfo(pConversionOptions->GetSourceAftkFilename(), &sourceAftkFileDate))
		return false;

	if (!Sys::C_FileSystem::Instance().FileInfo(pConversionOptions->GetPreprocessedAftkFilename(), &preprocessedAftkFileData))
		return false;

	if (!Sys::C_FileSystem::Instance().FileInfo(pConversionOptions->GetExecutableFileNameWin32(), &sceneConverterFileDate))
		return false;

	//Check if preprocessed is newer
	if (sourceAftkFileDate.m_FileTime > preprocessedAftkFileData.m_FileTime)
		return false;

#ifndef NOCHECK_SCENECONVERTER_FILE_TIME
	//Check if scene_converter.exe is newer
	if (sceneConverterFileDate.m_FileTime > preprocessedAftkFileData.m_FileTime)
		return false;
#endif

	//It is up to date
	return true;
}

/////////////////////////////////////////////////
//    GET ME IN GAME VERSION
/////////////////////////////////////////////////

	/*
	aftkStatus = aftk.Read(filename);

	aftk.Repair();

	if (aftk.Validate() != 0)
		LOGDEBUG( g_Log, "ProcessAftk: Scene Invalid After Reading.\n");

	if (aftkStatus.GetErrorCode() == Aftk::C_Status::E_Status_Success)
	{
		std::string cameraDir;
		
		LOGDEBUG( g_Log, "Searching for cameras...\n");
		// Make sure camera directory exists:
		cameraDir=g_SetupGMIG.LocalPCFolder()+"\\GameImage";
		CreateDirectory(cameraDir.c_str(),NULL);

		cameraDir+="\\Cameras";
		CreateDirectory(cameraDir.c_str(),NULL);
		CameraExportSceneToFiles(aftk,cameraDir,m_exportSettings.m_cityName);

		LOGDEBUG( g_Log, "End of searching for cameras.\n");

		LOGDEBUG( g_Log, "ProcessAftk: Triangulating.\n");
		Aftk::C_SceneNodeVisit::Traverse(
			&aftk, 
			Aftk::C_MeshTriangulate::C_TriangulateVisitor());
		{
			int nPolygons = aftk.GetNumMeshPolygons();
			int nPolygonVertices = aftk.GetNumMeshPolygonVertices();
			int nVertexPositions = aftk.GetNumMeshVertexPositions();

			LOGDEBUG( g_Log, "Before processing aftk scene has %i polygons, %i polygon vertices and %i vertex positions.\n", nPolygons, nPolygonVertices, nVertexPositions);
		}

		if (!m_gameLogic)
		{
			LOGDEBUG( g_Log, "ProcessAftk: Calculating tangents and Binormals.\n");
			Aftk::C_MeshCalculateTangents::C_CalculateTangentsVisitor visitor;
			visitor.normalise = false;
			visitor.useVertexNormal = false;
			Aftk::C_SceneNodeVisit::Traverse(
				&aftk, 
				visitor,		
				0);
			LOGDEBUG( g_Log, "ProcessAftk: CalculateTangentsAndBinormals - Done.\n");
		}

		if (aftk.Validate() != 0)
			LOGWARN( g_Log, "ProcessAftk: Scene Invalid.\n");

		if (m_exportSettings.m_polyReduceCoplanar)
			LOGDEBUG(g_Log, "m_exportSettings.m_polyReduceCoplanar = true\n");

		LOGDEBUG(g_Log, "ProcessAftk: Completing Mesh Processing...\n");
		{
			Aftk::C_SceneNodeIterator it(&aftk);
			for (; it; it++)
			{
				Aftk::C_SceneNode* pAftkSceneNode = it;
				if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
				{
					Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*) pAftkSceneNode;
					if (pAftkSceneMeshNode->IsSkinned())
						continue;

					if (m_gameLogic)
					{
//						LOGDEBUG(g_Log, "ProcessAftk: Deleting uvs, colours and materials.\n");
						// remove UVs and vertex colours
						pAftkSceneMeshNode->DeleteVertexDataChannels();
						pAftkSceneMeshNode->DeleteTextureCoordinateSets();

						// and materials
						pAftkSceneMeshNode->DeleteMaterials();

						// to do: remove building mesh
//						LOGDEBUG(g_Log, "ProcessAftk: Deleting uvs, colours and materials - Done.\n");
					}
					
					if (m_exportSettings.m_polyReduceCoplanar)
					{
//						LOGDEBUG(g_Log, "ProcessAftk: Mesh reducing.\n");
						// Ian's code for tracksurface polygon reduction
						Aftk::C_Status status;
						AftkUt::RadialEdgeMesh::C_Mesh* pRadialMesh = new AftkUt::RadialEdgeMesh::C_Mesh(pAftkSceneMeshNode);
						pRadialMesh->Build(status);
						pRadialMesh->CreateUVIntegerOffsets(status);
						pRadialMesh->RemoveCoPlanarPolys(status);
						pRadialMesh->BuildAftk(status);
						delete pRadialMesh;
//						LOGDEBUG(g_Log, "ProcessAftk: Mesh reducing - Done.\n");
					}
					else
					{
						//LOGDEBUG(g_Log, "m_exportSettings.m_polyReduceCoplanar = false\n");
					}

					if (!m_gameLogic)
					{
//						LOGDEBUG(g_Log, "ProcessAftk: Smoothing tangent space.\n");
						AftkUt::C_SmoothTangents::Smooth(
							pAftkSceneMeshNode, 
							1.0f, 
							30.f,
							true);

						AftkUt::C_SmoothNormals::Smooth(
							pAftkSceneMeshNode, 
							30.f);
//						LOGDEBUG(g_Log, "ProcessAftk: Smoothing tangent space - Done.\n");
					}
				}
			}
		}
		LOGDEBUG(g_Log, "ProcessAftk: Mesh Processing Complete.\n");

		if (aftk.Validate() != 0)
			LOGWARN( g_Log, "ProcessAftk: Scene Invalid.\n");

		// apply scaling straight away
		ApplyScalingFactor(aftk);

		if (!m_gameLogic)
		{
			progress_SetWorkingOn("Generating barrier damage materials...");
			//Adds barrier damage textures to barrier materials, duh :).
			AftkBarrierDamage::AddBarrierDamageTexturesToBarrierMaterials(aftk);
			
			progress_SetWorkingOn("Expanding remapped instances...");
			AftkUt::C_ExpandUniqueRemappedInstances::Expand(&aftk);
		}

		if (!m_gameLogic)
		{
			progress_SetWorkingOn("Generating SpeedTrees...");
			ExecuteSpeedTree(aftk);
		}

		if (aftk.Validate() != 0)
			LOGWARN( g_Log, "ProcessAftk: Scene Invalid.\n");

		if (!m_gameLogic)
		{
			progress_SetWorkingOn("Generating Flares.");

			LOGDEBUG(g_Log, "ExecuteLightFlares about to run.\n");
			ExecuteLightFlares(aftk);
			LOGDEBUG(g_Log, "ExecuteLightFlares done.\n");

			progress_SetWorkingOn("Duplicating Track Surface.");

			// copy stuff flagged as ground mesh into it's own layer so the lighting tool can recognize it easily
			LOGDEBUG(g_Log, "DuplicateTrackSurfaceIntoRoutes about to run.\n");
			DuplicateTrackSurfaceIntoRoutes(aftk);
			LOGDEBUG(g_Log, "DuplicateTrackSurfaceIntoRoutes done.\n");
		}

		if (aftk.Validate() != 0)
			LOGWARN( g_Log, "ProcessAftk: Scene Invalid After Generating Flares.\n");

		progress_SetWorkingOn("SmallMeshToInstance + MakeDblSided.");
		{
			LOGDEBUG(g_Log, "ProcessAftk: ConvertSmallInstancesToMesh.\n");
			AFTKTraverseFullHierachy(aftk.GetRootSceneNodePtr(), ConvertSmallInstancesToMesh());
			LOGDEBUG(g_Log, "ProcessAftk: Done.\n");

			LOGDEBUG(g_Log, "ProcessAftk: MakeDoubleSidedPolygons.\n");
			AFTKTraverseFullHierachy(aftk.GetRootSceneNodePtr(), MakeDoubleSidedPolygons());
			LOGDEBUG(g_Log, "ProcessAftk: Done.\n");
		}

	progress_SetWorkingOn("Welding mesh.");
	LOGDEBUG(g_Log, "ProcessAftk: Welding all mesh nodes.\n");
	{
		//The class below contains memory pools we'll need.
		Aftk::C_MeshWeldMemoryManager weldMemoryManager(
			AFTK_MESH_WELD_MEMORY_MANAGER_LARGE_CAPACITY);
		//This will be visiting our nodes.
		Aftk::C_MeshWeld::C_WeldVisitor visitor;
		//Set tolerances.
		visitor.positionThreshold	= AFTK_EPSILON;
		visitor.normalThreshold		= AFTK_EPSILON;
		visitor.uvThreshold			= AFTK_EPSILON;
		visitor.tangentThreshold	= 0.01f;

		//Perform work.
		aftkStatus = Aftk::C_SceneNodeVisit::Traverse(
			&aftk, 
			Aftk::C_MeshWeld::C_WeldVisitor(),
			weldMemoryManager, 
			0);
	}
	LOGDEBUG(g_Log, "ProcessAftk: Done.\n");

		LOGDEBUG(g_Log, "ProcessAftk: Removing degenerates.\n");
		{
			aftkStatus = Aftk::C_SceneNodeVisit::Traverse(
				&aftk, 
				Aftk::C_MeshRemoveDegenerates::C_RemoveDegeneratesVisitor(),
				0);
		}
		LOGDEBUG(g_Log, "ProcessAftk: Done.\n");

		{
			int nPolygons = aftk.GetNumMeshPolygons();
			int nPolygonVertices = aftk.GetNumMeshPolygonVertices();
			int nVertexPositions = aftk.GetNumMeshVertexPositions();

			LOGDEBUG( g_Log, "After processing aftk scene has %i polygons, %i polygon vertices and %i vertex positions.\n", nPolygons, nPolygonVertices, nVertexPositions);
		}

		progress_SetWorkingOn("Validating.");
		LOGDEBUG(g_Log, "ProcessAftk: Validating post RemoveDegeneratePolygons.\n");

		if (aftk.Validate() != 0)
		{
			LOGDEBUG( g_Log, "ProcessAftk: Scene Invalid After Repair Mesh Nodes.\n");
			//return false;
		}
		LOGDEBUG(g_Log, "ProcessAftk: Done.\n");

		progress_SetWorkingOn("Crowd preprocessing.");
		LOGDEBUG(g_Log, "CrowdPvsBuilder: Pre-process scene.\n");
		// Crowds: drop zones onto collision mesh, insert crowd PVS blocks into scene.
		CrowdPvsBuilder crowdPvsBuilder;
		crowdPvsBuilder.PreProcessScene(aftk, !m_gameLogic);
		LOGDEBUG(g_Log, "CrowdPvsBuilder: Done.\n");

		LOGDEBUG(g_Log, "ProcessAftk: Writing pre-processed A3D file [%s].\n", GetPreprocessedFilename().c_str());
		if (!aftk.Write(GetPreprocessedFilename()))
			return false;
	}
	else
	{
		//if (!doingAutoRefresh)
		{
			const std::string& errorString = aftkStatus.GetErrorString();

			if (errorString.length() > 0)
			{
				MessageBox(NULL, aftkStatus.GetErrorString().c_str(), "ProcessAftk: Aftk error", MB_OK | MB_ICONERROR);
			}

			LOGERROR( g_Log, "ProcessAftk: Conversion failed.\n");
		}
		return false;
	}
	*/

/////////////////////////////////////////////////
//    GENESIS SCENE CONVERTER VERSION
/////////////////////////////////////////////////

	/*
	// Iterate scene and generate speedtree meshes
	{
		AftkSpeedTree::SpeedTree speedTree;

		// setup base path and lighting
		speedTree.m_sBasePath = "";
		speedTree.m_nLightmapResolution = 1;
		speedTree.m_bGenerateLeaves = false;
		speedTree.m_bDoubleSidedFronds = true;
		//speedTree.m_bGenerateFronds = false;
		speedTree.m_bGenerateBillboards = false;

		// look for trees in scene and generate them
		speedTree.ProcessAftk(aftkScene);
	}

	{
		// Remove base instances.
		AftkUt::C_RemoveBaseInstances::RemoveBaseInstances(&aftkScene);
		AftkUt::C_ReplaceWithActorNodes::ReplaceWithActorNodes(&aftkScene);
	}

	{
		// Warn about ambiguous node names
		AftkUt::C_FindAmbiguousActorNames::C_FindClashingActorNames(&aftkScene);
	}

	// remove vertex lighting
	{
		RemoveVertexLighting(aftkScene);
	}

	// flatten small instances
	//const bool flattenInstancesStatus = C_MeshFlattenSmallInstances::FlattenSmallInstances(&aftkScene, 50);

	// make double sided
	Aftk::C_Status doubleSidedStatus = Aftk::C_SceneNodeVisit::Traverse(
		&aftkScene, AftkUt::C_MakeDoubleSidedPolygons::C_MakeDoubleSidedPolygonsVisitor( pConvertParam->m_PolyFlagConfig.m_DoubleSidedFlagName ), 0);

	bool overrideNormalsStatus = C_MeshProcessOverrideNormals::OverrideNormals( &aftkScene, pConvertParam->m_PolyFlagConfig.m_OverrideNormalsOutFlagName, pConvertParam->m_PolyFlagConfig.m_OverrideNormalsUpFlagName );

	SetCollisionPrimitivesToBeUnlit( aftkScene );
	*/