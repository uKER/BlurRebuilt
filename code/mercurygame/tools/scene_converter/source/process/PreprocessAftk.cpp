#include "stdafx.h"

#include <system/time/time.h>
#include <btl/Utility/scopedpointer.h>
#include <tools_shared/utility/Misc.h>
#include <tools_shared/utility/AssetMeta.h>

#include <AftkUt/AftkUt.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <converter_shared/helpers/AftkProgress.h>
#include <converter_shared/config/PolyflagConfig.h>
#include <converter_shared/helpers/ConvertSmallInstancesToMesh.h>
#include <Polyflags.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>

#include <process/PreprocessAftk.h>
#include <process/SpeedTree.h>

#include <util/MakeInvisiblePolygons.h>
#include <util/MakeNoPVSPolygons.h>

#include "process/AftkReplaceLodActors.h"
#include <process/CrowdReplaceSourceNodes.h>
#include "process/CreateLodImposters.h"

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

bool IsPlatform( const C_DataConversionOptions* pOptions, const Aftk::C_SceneNode& aftkSceneNode)
{
	const Aftk::C_SceneNode*  pCurrAftkSceneNode = &aftkSceneNode;
	while(pCurrAftkSceneNode)
	{
		Objectflags::C_Objectflags objectFlags( pOptions->GetObjectflagConfig(), pCurrAftkSceneNode );
		if( objectFlags.GetValue( E_Objectflag_Model ) || objectFlags.GetValue( E_Objectflag_Element ))
		{
			return true;
		}

		pCurrAftkSceneNode = pCurrAftkSceneNode->GetParentPtr();
	}

	return false;
}

void AddPlatformsToPlatformLayer( Aftk::C_Scene& aftkScene, const C_DataConversionOptions* pInputConversionOptions )
{
	Aftk::C_LayerNode* pPlatformLayer = new Aftk::C_LayerNode(&aftkScene);
	pPlatformLayer->SetName("_PlatformLayer");

	Aftk::C_SceneNodeIterator it(&aftkScene);

	while (it)
	{
		const bool isPlatform = IsPlatform( pInputConversionOptions, *it );

		if ( isPlatform )
		{
			//Aftk::C_SceneMeshNode *pNode = (Aftk::C_SceneMeshNode*)&(*it);
			Aftk::C_SceneNode *pNode = (Aftk::C_SceneNode*)&(*it);
			{
				pPlatformLayer->AddMember( pNode );
			}
		}
		++it;
	}
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

	// remove any objects that we don't want to process
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Remove construction nodes.\n");
		U32 nRemoved = AftkUt::RemoveConstructionNodes::Do(aftkScene.GetRootSceneNodePtr());
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: %i nodes removed.\n", nRemoved);
	}

	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: IRM Optimise.\n");

		if (!AftkUt::C_IrmOptimise::RemoveDuplicates(&aftkScene)) return false;

		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: IRM Optimise.\n");
	}

	// Change the source node of crowd instances
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: ReplaceCrowdNodes.\n");
			
		U32 nReplaced = AftkCrowdReplaceSourceNodes::Do(&aftkScene, aftkScene.GetRootSceneNodePtr(),
			pInputConversionOptions->GetCrowdRandomSeed(), pInputConversionOptions->GetToolConfig().CrowdRotateY180());
			
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: %i crowd nodes replaced.\n", nReplaced);
	}


	// replace lod actors
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: ReplaceLodActors.\n");
		bool success = AftkReplaceLodActors::Process(aftkScene);
		if (!success) return false;
	}

	// triangulate
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Triangulate.\n");
		success = Aftk::C_SceneNodeVisit::Traverse( &aftkScene, Aftk::C_MeshTriangulate::C_TriangulateVisitor());
	}
	if (!success) return false;

	// calculate tangents and binormals
	{
		const bool useVertexNormalForTangents = pInputConversionOptions->GetToolConfig().GetUseVertexNormalForTangents();
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Calculate Tangents And Binormals.\n");
		Aftk::C_MeshCalculateTangents::C_CalculateTangentsVisitor visitor;
		visitor.normalise = false;
		visitor.useVertexNormal = useVertexNormalForTangents;
		success = Aftk::C_SceneNodeVisit::Traverse(
			&aftkScene, 
			visitor,		
			0);
	}
	if (!success) return false;

	// smooth tangents and binormals
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Smooth Tangents and Binormals.\n");

        F32 magnitudeScale = 1.0f;	    	        
        F32 normalAngularTolerance = pInputConversionOptions->GetToolConfig().GetSmoothNormalAngularTolerance();  
        F32 tangentAngularTolerance = pInputConversionOptions->GetToolConfig().GetSmoothTangentAngularTolerance();	    	        
        
        bool normalise = true;
	    	    
        Aftk::C_SceneNodeIterator sceneNodeIterator( &aftkScene );
        while(sceneNodeIterator)
        {
            Aftk::C_SceneNode* pCurrAftkNode = sceneNodeIterator;

            if(pCurrAftkNode->GetType() == Aftk::E_ObjType_SceneMesh)
            {
                Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)pCurrAftkNode;
                
				if (normalAngularTolerance > 0.f || tangentAngularTolerance > 0.f)
				{
					Aftk::C_Status status;
					AftkUt::RadialEdgeMesh::C_Mesh* pRadialMesh = new AftkUt::RadialEdgeMesh::C_Mesh(pAftkSceneMeshNode);					
					pRadialMesh->Build(status, false);
					if (status)
					{
						if(normalAngularTolerance > 0.0f && AftkUt::C_SmoothingHelper::ShouldSmooth(pAftkSceneMeshNode))
						{
							AftkUt::C_SmoothNormals().Smooth(
								pRadialMesh,
								pAftkSceneMeshNode,
								normalAngularTolerance);
						}

						if (tangentAngularTolerance > 0.f)
						{
							AftkUt::C_SmoothTangents().Smooth(
								pRadialMesh,
								pAftkSceneMeshNode, 
								magnitudeScale, 
								tangentAngularTolerance, 
								normalise);
						}									
					}

					delete pRadialMesh;
				}
            }
            sceneNodeIterator++;
        }	    	
	}

	// make double sided
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Make Double Sided.\n");
		Polyflags::C_PolyflagNames doubleSidedFlagNames;
		pInputConversionOptions->GetPolyflagConfig().GetItem( E_Polyflag_DoubleSided ).GetFlagNames( doubleSidedFlagNames );
		if( doubleSidedFlagNames.Size() > 0 )
		{
			success = Aftk::C_SceneNodeVisit::Traverse( &aftkScene, AftkUt::C_MakeDoubleSidedPolygons::C_MakeDoubleSidedPolygonsVisitor( doubleSidedFlagNames[0] ), 0);
		}
	}
	if (!success) return false;

	// Find portal room nodes, navmesh, PVSZone or any useful collision types and set all polys in their
	// meshes to be invisible, excluded from PVS consideration and not to be shadow casters.
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Set invisible flags.\n");
		Aftk::C_SceneNodeVisit::Traverse(&aftkScene, C_MakeInvisiblePolygons::C_MakeInvisiblePolygonsVisitor(&pInputConversionOptions->GetPolyflagConfig(), 
																											 &pInputConversionOptions->GetObjectflagConfig()), 0);
	}

	// Find invisible polygons and set them not to be PVS'd.
	{
		LOGDEBUG(Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Set do not PVS polygons.\n");
		Aftk::C_SceneNodeVisit::Traverse(&aftkScene, C_MakeNoPVSPolygons::C_MakeNoPVSPolygonsVisitor( &pInputConversionOptions->GetObjectflagConfig() ), 0);
	}


	// Iterate scene and generate speedtree meshes
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Build SpeedTree data.\n");
		ExecuteSpeedTree( aftkScene, *pInputConversionOptions );
	}

	// Generate low lod sprites for any lod groups
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Create Low LOD Imposters.\n");
		success = CreateLodImposters::Process(aftkScene, *pInputConversionOptions);
		if (!success)
			return false;
	}



	// TODO: make expanded small instances use different lighting (SH / Vertex instead of lightmap)
	{
		// Convert Small Instances to Mesh
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Convert Small Instances to Mesh.\n");
		success = C_ConvertSmallInstancesToMesh::ConvertSmallInstancesToMesh(&aftkScene);
	}
	if (!success) return false;

	// Weld Vertices
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Weld Vertices.\n");
		//The class below contains memory pools we'll need.
		Aftk::C_MeshWeldMemoryManager weldMemoryManager(
			AFTK_MESH_WELD_MEMORY_MANAGER_LARGE_CAPACITY);
		//This will be visiting our nodes.
		Aftk::C_MeshWeld::C_WeldVisitor visitor;
		//Set tolerances.
		visitor.positionThreshold	= Aftk::kOneMM;
		visitor.normalThreshold		= Aftk::kSlackTolerence;
		visitor.uvThreshold			= Aftk::kTightTolerence;
		visitor.tangentThreshold	= Aftk::kSlackTolerence;

		//Perform work.
		success = Aftk::C_SceneNodeVisit::Traverse(
			&aftkScene, 
			Aftk::C_MeshWeld::C_WeldVisitor(),
			weldMemoryManager, 
			0);
	}
	if (!success) return false;

	// mesh vertex snapping
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Mesh Vertex Snapping");
		Aftk::SceneMeshNodePtrArray nodeArray;
		AftkUt::C_SnapVertices::BuildNodesToSnap(nodeArray, aftkScene);
		Aftk::SceneMeshNodePtrArray nodeArrayMinusPlatforms;

		S32 platformsSkipped = 0;
		for (S32 i = 0; i < nodeArray.Size(); ++i)
		{
			//Skip platform mesh. TODO, fold this into AftkQuery used by BuildNodesToSnap.
			if (nodeArray[i]->GetType() == Aftk::E_ObjType_SceneMesh &&
				IsPlatform(pInputConversionOptions, *nodeArray[i]))
			{
				++platformsSkipped;
				continue;
			}

			nodeArrayMinusPlatforms.PushBack(nodeArray[i]);
		}
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: %i Platform meshes skipped", platformsSkipped);

		const F32 kOneCm = 0.01f;
		success = AftkUt::C_SnapVertices::Snap(nodeArrayMinusPlatforms, AftkUt::StaticSnapEquality(kOneCm));
	}
	if (!success) return false;

	// Remove Degenerates
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Remove Degenerates.\n");
		success = Aftk::C_SceneNodeVisit::Traverse(
			&aftkScene, 
			Aftk::C_MeshRemoveDegenerates::C_RemoveDegeneratesVisitor(),
			0);
	}
	if (!success) return false;


	// place platforms in a special layer for lighting
	AddPlatformsToPlatformLayer( aftkScene, pInputConversionOptions );

	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: IRM Optimise.\n");

		//Warn about nodes at the origin
		Sys::C_FilePath fp = Sys::ExtractDirectory(pInputConversionOptions->GetSourceAftkFilename());
		fp += "/";
		fp += Sys::ExtractFilename(pInputConversionOptions->GetSourceAftkFilename());
		fp = Sys::RemoveFileExtension(fp);
		fp += "_origin_debug.a3d";
		AftkUt::C_IrmOptimise::WriteOriginDebug(&aftkScene, fp.Chars());
		//Collapse IRM scenes

		if (!AftkUt::C_IrmOptimise::RemoveDuplicates(&aftkScene)) return false;

		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: IRM Optimise.\n");
	}

	//// polyflag lowest lods
	{
		LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Setting polyflags on lowest lods.\n");

		Btl::Vector<Btl::Pair<Util::C_String, Util::C_String> > polyFlags;
		polyFlags.PushBack(Btl::Pair<Util::C_String, Util::C_String>("polySet_excludeFromPrepass", "polySet_forceIncludeInPrepass"));

		success = Aftk::C_SceneNodeVisit::Traverse(
			&aftkScene, 
			AftkUt::C_PolyFlagLowestLods::C_PolyFlagLowestLodsVisitor(polyFlags),
			0);
	}
	if (!success) return false;


	if (aftkScene.Validate() != 0)
		LOGWARN( Tools::g_pMyApplication->GetLogger(), "ProcessAftk: Scene Invalid.\n");

	return success;
}

bool C_ProcessPreprocessAftk::PreprocesssedFileUpToDate( const C_DataConversionOptions* pConversionOptions ) const
{
	#ifdef ALWAYS_USE_PREPROCESSED_DATA
	return true;
	#endif

#ifdef BUILD_CONFIG_DEBUG
	return false;
#endif
	return !Tools::MetaMismatch(
		pConversionOptions->GetPreprocessedAftkFilename().Chars(), 
		pConversionOptions->GetSourceAftkFilename().Chars(), 
		Tools::GetCurrentExe().Chars());
}

